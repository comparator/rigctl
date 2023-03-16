#include <stdlib.h>
#include <error.h>
#include <argp.h>

#include "config.h"

// #include <arpa/inet.h>
// convert IPv4 and IPv6 addresses from text to binary form
// int inet_pton(int af, const char *restrict src, void *restrict dst);

static char parm_doc[] = "RigCTL Server Simulator\n";

static char args_doc[] = "ARG1 [STRING...]";

/* The options we understand. */
static struct argp_option options[] = {
  {"verbose",   'v', 0,       0, "Produce verbose output" },
  {"output",    'o', "FILE",  0, "Output to FILE instead of standard output" },
  {"port",      'p', "PORT", 0, "Change RigCTL Server PORT"},
  {"comm",      'c', "PORT", 0, "Change COMM Server PORT"},
  //{"client",    'c', "IP",   0, "Connect to RIGCTL"},

  { 0 }
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  CONFIG_t * arguments = state->input;
  
  int port;
  
  switch (key)
    {
    case 'v':
      arguments->verbose = 1;
      break;

    case 'o':
      arguments->output_file = arg;
      break;

    case 'p':
        arguments->rigctlport = atoi(arg);
        break;

    case 'c':
        arguments->commport = atoi(arg);
        break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, parm_doc };


int parse_parm(int argc, char *argv[], CONFIG_t *parm)
{
    parm->verbose = 0;
    parm->rigctlport = RIGCTLPORT;
    parm->commport = COMMPORT;
    parm->output_file = "-";

    argp_parse (&argp, argc, argv, 0, 0, parm);

    printf("Verb: %d, RigCTL Port: %d, Comm Port: %d, File: %s\n", 
        parm->verbose,
        parm->rigctlport,
        parm->commport,
        parm->output_file);

    if ((parm->rigctlport > 1000 && parm->rigctlport < 655535) ||
        (parm->commport > 1000 && parm->commport < 65535))
        return 0;

    return -1;
}
