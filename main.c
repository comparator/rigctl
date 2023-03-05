#include <stdlib.h>
#include <stdio.h>      /* Standard input/output definitions */
#include <string.h>     /* String function definitions */
#include <unistd.h>     /* UNIX standard function definitions */
#include <fcntl.h>      /* File control definitions */
#include <errno.h>      /* Error number definitions */

#include <signal.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "rigctl.h"

#define RIGCTLPORT      4532

#define MAX_CLIENTS     4
#define NO_SOCKET       -1


short int port;
int listen_sock;
peer_t connection_list[MAX_CLIENTS];

int parse_parm(int argc, char *argv[])
{
  char *endptr;

  if(argc == 2) {
    port = strtol(argv[1], &endptr, 0);
    if(*endptr) {
      fprintf(stderr, "RIGSIM: Invalid port number.\n");
      return -1;
    }
  }
  else if(argc < 2) {
    port = RIGCTLPORT;
  }
  else {
    fprintf(stderr, "RIGSIM: Invalid arguments.\n");
    return -2;
  }

  return 0;
}

void shutdown_srv(int code)
{
  int i;

  close(listen_sock);

  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connection_list[i].socket != NO_SOCKET)
      close(connection_list[i].socket);

  fprintf(stdout, "Shutdown server\n");
  exit(code);
}

void handle_signal_action(int sig_number)
{
  if (sig_number == SIGINT) {
    fprintf(stdout, "\nSIGINT was catched!\n");
    shutdown_srv(EXIT_SUCCESS);
  }
  else if (sig_number == SIGPIPE) {
    fprintf(stdout, "\nSIGPIPE was catched!\n");
    shutdown_srv(EXIT_SUCCESS);
  }
}

int setup_signals()
{
  struct sigaction sa;
  sa.sa_handler = handle_signal_action;
  if (sigaction(SIGINT, &sa, 0) != 0) {
    fprintf(stderr, "SIGINT sigaction()");
    return -1;
  }
  if (sigaction(SIGPIPE, &sa, 0) != 0) {
    fprintf(stderr, "SIGPIPE sigaction()");
    return -1;
  }

  return 0;
}

/* Start listening socket */
int start_listen_socket(int *listen_socket, short int listen_port)
{
  // Obtain a file descriptor for our "listening" socket.
  *listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (*listen_socket < 0) {
    fprintf(stderr, "socket");
    return -1;
  }

  int reuse = 1;
  if (setsockopt(*listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
    fprintf(stderr, "setsockopt");
    return -1;
  }

  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  my_addr.sin_port = htons(listen_port);

  if (bind(*listen_socket, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) != 0) {
    fprintf(stderr, "bind");
    return -1;
  }

  // start accept client connections
  if (listen(*listen_socket, 10) != 0) {
    fprintf(stderr, "listen");
    return -1;
  }
  fprintf(stdout, "Accepting connections on port %d\n", listen_port);

  return 0;
}

int build_fd_sets(fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
  int i;

  FD_ZERO(read_fds);
  FD_SET(STDIN_FILENO, read_fds);
  FD_SET(listen_sock, read_fds);
  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connection_list[i].socket != NO_SOCKET)
      FD_SET(connection_list[i].socket, read_fds);

  FD_ZERO(write_fds);
  for (i = 0; i < MAX_CLIENTS; ++i)
    if ((connection_list[i].socket != NO_SOCKET) && (connection_list[i].to_send != 0))
      FD_SET(connection_list[i].socket, write_fds);

  FD_ZERO(except_fds);
  FD_SET(STDIN_FILENO, except_fds);
  FD_SET(listen_sock, except_fds);
  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connection_list[i].socket != NO_SOCKET)
      FD_SET(connection_list[i].socket, except_fds);

  return 0;
}

int handle_new_connection()
{
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  socklen_t client_len = sizeof(client_addr);
  int new_client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
  if (new_client_sock < 0) {
    fprintf(stderr, "accept()");
    return -1;
  }

  char client_ipv4_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_addr.sin_addr, client_ipv4_str, INET_ADDRSTRLEN);

  fprintf(stdout, "Incoming connection from %s:%d.\n", client_ipv4_str, client_addr.sin_port);

  int i;
  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (connection_list[i].socket == NO_SOCKET) {
      connection_list[i].socket = new_client_sock;
      connection_list[i].addres = client_addr;
      connection_list[i].to_send = 0;
      return 0;
    }
  }

  fprintf(stderr, "There is too much connections. Close new connection %s:%d.\n", client_ipv4_str, client_addr.sin_port);
  close(new_client_sock);
  return -1;
}

char *peer_get_addres_str(peer_t *peer)
{
  static char ret[INET_ADDRSTRLEN + 10];
  char peer_ipv4_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &peer->addres.sin_addr, peer_ipv4_str, INET_ADDRSTRLEN);
  sprintf(ret, "%s:%d", peer_ipv4_str, peer->addres.sin_port);

  return ret;
}

int close_client_connection(peer_t *client)
{
  fprintf(stdout, "Close client socket for %s.\n", peer_get_addres_str(client));

  close(client->socket);
  client->socket = NO_SOCKET;
}

/* Receive message from peer */
int receive_from_peer(peer_t *peer)
{
  //fprintf(stdout, "Ready for recv() from %s.\n", peer_get_addres_str(peer));

  size_t len_to_receive;
  ssize_t received_count;

  do {
    received_count = recv(peer->socket, peer->rx_buff, (SIZE_OF_RXBUFF - 1), MSG_DONTWAIT);

    if(received_count < 0) {
      if((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
        fprintf(stderr, "recv() from peer error");
        return -1;
      }
    }
    else if (received_count == 0) {
      //fprintf(stdout, "Peer gracefully shutdown.\n");
      return -1;
    }
    else {
      peer->rx_buff[received_count] = 0;
      if(rigctl_req(peer) < 0)
        return -1;
    }
  } while (received_count > 0);

  //fprintf(stdout, "Total recv()'ed %zu bytes.\n", received_total);

  return 0;
}

int send_to_peer(peer_t *peer)
{
  //fprintf(stdout, "Ready for send() to %s.\n", peer_get_addres_str(peer));

  size_t len_to_send;
  ssize_t send_count;

  len_to_send = peer->to_send;
  peer->to_send = 0;
  
  //fprintf(stdout, "Let's try to send() %zd bytes... ", len_to_send);
  send_count = send(peer->socket, (char *)&peer->tx_buff[0], len_to_send, 0);

  if(send_count < 0) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      fprintf(stdout, "peer is not ready right now, try again later.\n");
    }
    else {
      fprintf(stderr,"send() from peer error");
      return -1;
    }
  }
  else if (send_count == 0) {
    fprintf(stdout, "send()'ed 0 bytes. It seems that peer can't accept data right now. Try again later.\n");
  }
  //else if (send_count > 0) {
  //  fprintf(stdout, "send()'ed %zd bytes.\n", send_count);
  //}

  return 0;
}


int main(int argc, char *argv[]) {
  if(parse_parm(argc, argv) != 0)
    exit(EXIT_FAILURE);

  if(setup_signals() != 0)
    exit(EXIT_FAILURE);

  if (start_listen_socket(&listen_sock, port) != 0)
    exit(EXIT_FAILURE);

  int i;
  for (i = 0; i < MAX_CLIENTS; ++i) {
    connection_list[i].socket = NO_SOCKET;
    connection_list[i].to_send = 0;
  }

  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;

  int high_sock = listen_sock;

  fprintf(stdout, "Waiting for incoming connections.\n");

  while (1) {
    build_fd_sets(&read_fds, &write_fds, &except_fds);

    high_sock = listen_sock;
    for (i = 0; i < MAX_CLIENTS; ++i) {
      if (connection_list[i].socket > high_sock)
        high_sock = connection_list[i].socket;
    }

    int activity = select(high_sock + 1, &read_fds, &write_fds, &except_fds, NULL);

    if(activity == -1) {
      fprintf(stderr, "select() -1\n");
      shutdown_srv(EXIT_FAILURE);
    }
    else if(activity == 0) {
      // you should never get here
      fprintf(stdout, "select() returns 0.\n");
      shutdown_srv(EXIT_FAILURE);
    }
    else
    {
      if (FD_ISSET(listen_sock, &read_fds)) {
        handle_new_connection();
      }

      if (FD_ISSET(listen_sock, &except_fds)) {
        fprintf(stderr, "Exception listen socket fd.\n");
        shutdown_srv(EXIT_FAILURE);
      }

      for (i = 0; i < MAX_CLIENTS; ++i) {
        if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &read_fds)) {
          if (receive_from_peer(&connection_list[i]) != 0) {
            close_client_connection(&connection_list[i]);
            continue;
          }
        }
  
        if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &write_fds)) {
          if (send_to_peer(&connection_list[i]) != 0) {
            close_client_connection(&connection_list[i]);
            continue;
          }
        }

        if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &except_fds)) {
          fprintf(stderr, "Exception client fd.\n");
          close_client_connection(&connection_list[i]);
          continue;
        }
      }
    }
  }

  return 0;
}
