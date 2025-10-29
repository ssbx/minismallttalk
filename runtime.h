#ifndef __RUNTIME_H_
#define __RUNTIME_H_

#include <stdio.h>

#define GETOPT_STATUS_OK_CONTINUE 0
#define GETOPT_STATUS_OK_QUIT    -1
#define GETOPT_STATUS_ERROR       1

int Runtime_Getopts(int argc, char *argv[]);
void Runtime_Freeopts();

#endif /* __RUNTIME_H_ */
