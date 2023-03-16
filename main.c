#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "config.h"
#include "network.h"

CONFIG_t  config;

void shutdown_srv(int code)
{
    net_close_sockets();

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



int main(int argc, char *argv[]) {
  if(parse_parm(argc, argv, &config) < 0)
    exit(EXIT_FAILURE);

  if(setup_signals() != 0)
    exit(EXIT_FAILURE);

  if(net_start_listen(&config) < 0)
    exit(EXIT_FAILURE);

  int run = 1;
  while (run >= 0) {
      run = net_poll();
  }

  net_close_sockets();
  fprintf(stdout, "Abnormal Shutdown server\n");
  exit(EXIT_FAILURE);

  return 0;
}
