#ifndef CONFIG_H
#define CONFIG_H

#define  RIGCTLPORT      4532
#define  COMMPORT        5555

typedef struct{
    short int   rigctlport;             /* '-p' Num */
    short int   commport;               /* '-c' Num */
}CONFIG_t;

int parse_parm(int argc, char *argv[], CONFIG_t *parm);

#endif // CONFIG_H