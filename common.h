/*
*Criado por:
*  -José Martins -up201404189
*  -Marcelo Ferreira -up201405323
*
*   Turma 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <sys/times.h>
#include <semaphore.h>

//strcut que contem a informacao da viatura
struct ViaturaInfo {
   int id;
   double duracao;
   char direcao;
   char nomeFifo[500];
};

//funcao usada para esperar pelo:
//- tempo de estacionamento das viaturas;
//- intervalo entre a criacao das viaturas;
//- (sem busy waiting).
void mySleep(double t){
  sleep((int)t);
  usleep((t - (int)t)*1000000);
}
