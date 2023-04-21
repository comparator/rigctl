#include <stdlib.h>
#include <stdio.h>      /* Standard input/output definitions */
#include <string.h>     /* String function definitions */
#include <unistd.h>     /* UNIX standard function definitions */
#include <errno.h>      /* Error number definitions */
#include <stdbool.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "log.h"
#include "config.h"
#include "network.h"

#include "rigctl.h"
#include "commcat.h"


#define MAX_CLIENT_CONNECTIONS  12
#define MAX_LISTEN_BACKLOG      10

#define PORT_NUM_MIN            1000
#define PORT_NUM_MAX            65535

#define PEER_MAX_RX_SIZE        80
#define PEER_MAX_TX_SIZE        1424

#define NO_SOCKET               -1

typedef struct {
    char        channel;
    char        rxbuf[PEER_MAX_RX_SIZE];
    int         rxpos;
}PEER_t;


int socket_rigctl = NO_SOCKET;
int socket_comm = NO_SOCKET;

static PEER_t           peer[MAX_CLIENT_CONNECTIONS];
static struct pollfd    net_fds[MAX_CLIENT_CONNECTIONS];
static int              net_nfds = 0;
static bool             compress_array = false;
static char             peer_txbuf[PEER_MAX_TX_SIZE];


static void receive_from_peer(int idx)
{
    bool close_conn = false;

    int sock = net_fds[idx].fd;
    char chn = peer[idx].channel;
    int pos = peer[idx].rxpos;
    char *pRxBuff = peer[idx].rxbuf;

    char ch;
    int rc;

    while((rc = recv(sock, &ch, sizeof(char), MSG_DONTWAIT)) == sizeof(char))
    {
        if(pos < PEER_MAX_RX_SIZE)
        {
            if(chn == 1)        // RigCTL;
            {
                if(((ch == '\n') || (ch == '\r')) && (pos > 0))
                {
                    pRxBuff[pos] = 0;

                    int len = rigctl_req(pRxBuff, peer_txbuf);
                    if(len < 0)
                    {
                        close_conn = true;
                    }
                    else if(len > 0)
                    {
                        if (send(sock, peer_txbuf, len, 0) < 0)
                        {
                            perror("  send() failed");
                            close_conn = true;
                        }
                    }
                    pos = 0;
                }
                else if(ch >= 0x20 && ch <= 0x7E)
                {
                    pRxBuff[pos++] = ch;
                }
                else
                {
                    pos = 0;
                }
            }
            else if(chn == 2)   // CAT
            {
                if((ch == ';') && (pos > 1))
                {
                    pRxBuff[pos] = 0;
                    int len = commcat_req(pRxBuff, peer_txbuf);
                    if(len < 0)
                    {
                        close_conn = true;
                    }
                    else if(len > 0)
                    {
                        if (send(sock, peer_txbuf, len, 0) < 0)
                        {
                            perror("  send() failed");
                            close_conn = true;
                        }
                    }
                    pos = 0;
                }
                else if((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z'))
                {
                    pRxBuff[pos++] = ch;
                }
                else
                {
                    pos = 0;
                }
            }
        }
        else
        {
            pos = 0;
        }
    }
    peer[idx].rxpos = pos;

    if (rc < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            perror("  recv() failed");
            close_conn = true;
        }
    }
    else if (rc == 0)   /* connection closed by the client */
    {
        log_info( "  Connection closed by the client");
        close_conn = true;
    }

    if (close_conn)
    {
        log_info( "  Connection closed by the server");

        close(sock);
        net_fds[idx].fd = NO_SOCKET;
        compress_array = true;
    }
}

static int start_listen_socket(int port)
{
    /* Create an AF_INET6 stream socket to receive incoming connections on */
    int listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd < 0) {
        perror("socket() failed");
        return NO_SOCKET;
    }

    /* Allow socket descriptor to be reuseable */
    int reuse = 1;
    if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt() failed");
        close(listen_sd);
        return NO_SOCKET;
    }

    /* Set socket to be nonblocking. All of the sockets for      */
    /* the incoming connections will also be nonblocking since   */
    /* they will inherit that state from the listening socket.   */
    /*
    char on = 1;
    if (ioctl(listen_sd, FIONBIO, &on) < 0) {
        perror("ioctl() failed");
        close(listen_sd);
        return NO_SOCKET;
    }
    */

    /* Bind the socket */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);
    if (bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bindA() failed");
        close(listen_sd);
        return NO_SOCKET;
    }

    /* start accept client connections */
    if (listen(listen_sd, MAX_LISTEN_BACKLOG) < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        return NO_SOCKET;
    }

    log_info( "Accepting connections on port %d", port);

    net_fds[net_nfds].fd = listen_sd;
    net_fds[net_nfds++].events = POLLIN;

    return listen_sd;
}

static int handle_new_connection(int in_socket, char chn)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_len = sizeof(client_addr);

    int new_client_sock = accept(in_socket, (struct sockaddr *)&client_addr, &client_len);
    if (new_client_sock < 0) {
        if (errno != EWOULDBLOCK)
        {
            perror("  accept() failed");
            return -1;
        }

        return 0;
    }

    char client_ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ipv4_str, INET_ADDRSTRLEN);

    log_info( "New Connection from %s:%d", client_ipv4_str, client_addr.sin_port);

    if(net_nfds >= MAX_CLIENT_CONNECTIONS)
    {
        log_error( "There is too much connections. Close new connection.");
        close(new_client_sock);
        return 0;
    }

    net_fds[net_nfds].fd = new_client_sock;
    net_fds[net_nfds].events = POLLIN;

    peer[net_nfds].channel = chn;
    peer[net_nfds].rxpos = 0;

    net_nfds++;

    return 0;
}


void net_close_sockets(void)
{
    /* Clean up all of the sockets that are open  */
    int i;
    for (i = 0; i < net_nfds; i++)
    {
        if(net_fds[i].fd >= 0) {
            close(net_fds[i].fd);
        }
    }
}

int net_start_listen(CONFIG_t *pConfig)
{
    /* Initialize the pollfd structure */
    net_nfds = 0;

    int i;
    for (i = 0; i < MAX_CLIENT_CONNECTIONS; i++)
    {
        net_fds[i].fd = NO_SOCKET;
        net_fds[i].events = 0;
        net_fds[i].revents = 0;
    }

    int port;
    socket_rigctl = NO_SOCKET;
    port = pConfig->rigctlport;
    if((port > PORT_NUM_MIN) && (port < PORT_NUM_MAX))
    {
        socket_rigctl = start_listen_socket(port);
        if(socket_rigctl < 0)
            return -1;
    }

    socket_comm = NO_SOCKET;
    port = pConfig->commport;
    if((port > PORT_NUM_MIN) && (port < PORT_NUM_MAX))
    {
        socket_comm = start_listen_socket(port);
        if(socket_comm < 0)
        {
            net_close_sockets();
            return -1;
      
        }
    }
}

int net_poll(void)
{
    int rc = poll(net_fds, net_nfds, 1000);
    if(rc < 0) {
        perror("  poll() failed");
        return -1;
    }
    else if(rc == 0)    /* time out */
    {
        return 0;
    }

    compress_array = false;

    int i, j;
    for(i = 0; i < net_nfds; i++)
    {
        if(net_fds[i].revents == 0)
            continue;

        if(net_fds[i].revents != POLLIN)
        {
            log_error( "Error! revents = %d", net_fds[i].revents);

            if(net_fds[i].fd != NO_SOCKET)
            {
                close(net_fds[i].fd);
                net_fds[i].fd = NO_SOCKET;
            }
            compress_array = true;
            continue;
        }

        if (net_fds[i].fd == socket_rigctl)
        {
            log_info( "New RigCTL connection");
            if(handle_new_connection(socket_rigctl, 1) < 0)
            {
                return -1;
            }
        }
        else if (net_fds[i].fd == socket_comm)
        {
            log_info( "New COMM connection");
            if(handle_new_connection(socket_comm, 2) < 0)
            {
                return -1;
            }
        }
        else
        {
            receive_from_peer(i);
        }
    }

    if (compress_array)
    {
      for (i = 0; i < net_nfds; i++)
      {
        if (net_fds[i].fd == NO_SOCKET)
        {
          for(j = i; j < net_nfds; j++)
          {
            net_fds[j].fd = net_fds[j+1].fd;

            peer[j].channel = peer[j+1].channel;
            memcpy(peer[j].rxbuf, peer[j+1].rxbuf, PEER_MAX_RX_SIZE);
            peer[j].rxpos = peer[j+1].rxpos;
          }
          i--;
          net_nfds--;
        }
      }
    }

    return 0;
}

