#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>



void RodaTerminal();

void ExecutaComandosExternos(char*** comandos, int nComandos);



#endif