/*
*Criado por:
*  -José Martins -up201404189
*  -Marcelo Ferreira -up201405323
*
*   Turma 1
*/

#include "common.h"

//variaveis globais
int lugaresDisponiveis;
clock_t start;
FILE* fp;
int closed;

//mutex
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;


void * thrArrrumador(void * arg) {
      struct ViaturaInfo *info= arg;

      printf("Chegou viatura com fifo -> %s\n",info->nomeFifo );

      int fd= open(info->nomeFifo, O_WRONLY );
      if(fd == -1){
        perror (info->nomeFifo);
        free(arg);
        return NULL;
      }

      int existeVaga=0;

      pthread_mutex_lock(&mut);
      if(closed){
        fprintf(fp, " %6d  ; %3d ; %4d    ; encerrado  \n",(int)(times(NULL)-start),lugaresDisponiveis,info->id);

        if(write(fd, "encerrado", 9) == -1){
          perror("error writing (encerrado)");
          free(arg);
          return NULL;
        }

        free(arg);
        return NULL;
      }else if(lugaresDisponiveis>0){
        lugaresDisponiveis--;
        fprintf(fp, " %6d  ; %3d ; %4d    ; estacionamento  \n",(int)(times(NULL)-start),lugaresDisponiveis,info->id);
        existeVaga=1;
      }else{
        fprintf(fp, " %6d  ; %3d ; %4d    ; cheio  \n",(int)(times(NULL)-start),lugaresDisponiveis,info->id);
      }
      pthread_mutex_unlock(&mut);

      if(existeVaga){
        if(write(fd, "entrada", 7) == -1){
          perror("error writing (entrada)");
          free(arg);
          return NULL;
        }


        mySleep(info->duracao);
        pthread_mutex_lock(&mut);
        lugaresDisponiveis++;
        fprintf(fp, " %6d  ; %3d ; %4d    ; saida  \n",(int)(times(NULL)-start),lugaresDisponiveis,info->id);
        pthread_mutex_unlock(&mut);
        if(write(fd, "saida", 5) == -1){
          perror("error writing (saida)");
          free(arg);
          return NULL;
        }
      }
      else{
        if(write(fd, "cheio", 5) == -1){
          perror("error writing (cheio)");
          free(arg);
          return NULL;
        }
      }


  if(close(fd) == -1){
    perror("error closing file");
    free(arg);
    return NULL;
  }

  free(arg);
  return NULL;
}

void * thrControlador(void * arg) {
  int fdr,fdw;
  int nr;

  switch (*(char *)arg) {
    case 'N':
              nr = mkfifo("/tmp/fifoN",0660);
              if(nr == -1){
                perror("creating /tmp/fifoN");
                return NULL;
              }
              if((fdr= open("/tmp/fifoN", O_RDONLY )) == -1){
                perror("opening r /tmp/fifoN");
                return NULL;
              }
              if((fdw= open("/tmp/fifoN", O_WRONLY )) == -1){
                perror("opening w /tmp/fifoN");
                return NULL;
              }

              break;
    case 'S':
              nr = mkfifo("/tmp/fifoS",0660);

              if(nr == -1){
                perror("creating /tmp/fifoS");
                return NULL;
              }
              if((fdr= open("/tmp/fifoS", O_RDONLY )) == -1){
                perror("opening r/tmp/fifoS");
                return NULL;
              }
              if((fdw= open("/tmp/fifoS", O_WRONLY )) == -1){
                perror("opening w /tmp/fifoS");
                return NULL;
              }

              break;
    case 'E':
              nr = mkfifo("/tmp/fifoE",0660);

              if(nr == -1){
                perror("creating /tmp/fifoE");
                return NULL;
              }
              if((fdr= open("/tmp/fifoE", O_RDONLY )) == -1){
                perror("opening r /tmp/fifoE");
                return NULL;
              }
              if((fdw= open("/tmp/fifoE", O_WRONLY )) == -1){
                perror("opening w /tmp/fifoE");
                return NULL;
              }

              break;
    case 'O':

                nr = mkfifo("/tmp/fifoO",0660);

                if(nr == -1){
                  perror("creating /tmp/fifoO");
                  return NULL;
                }
                if((fdr= open("/tmp/fifoO", O_RDONLY )) == -1){
                  perror("opening r /tmp/fifoO");
                  return NULL;
                }
                if((fdw= open("/tmp/fifoO", O_WRONLY )) == -1){
                  perror("opening w /tmp/fifoO");
                  return NULL;
                }

              break;
  }

      while (1) {

        struct ViaturaInfo *info= (struct ViaturaInfo*)malloc(sizeof(struct ViaturaInfo));

        if(read(fdr,info,sizeof(struct ViaturaInfo ))==0)
            break;

        if(strncmp("sair",info->nomeFifo,4)==0){
          closed=1;
          if(close(fdw) == -1){
            perror("error closing write descriptor of controller fifo");
            return NULL;
          }
          continue;
        }

          //criar arrumador detached
          pthread_t pArrumador;
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
          if(pthread_create(&pArrumador, &attr, thrArrrumador, info)){
            perror("error creating detached pthread");
            return NULL;
        }

      }


      if(close(fdr) == -1){
        perror("error closing fifo");
        return NULL;
      }

      switch (*(char *)arg) {
        case 'N':
                  if((nr = unlink("/tmp/fifoN")) == -1){
                    perror("unlink /tmp/fifoN");
                    return NULL;
                  }
                  break;
        case 'S':
                  if((nr = unlink("/tmp/fifoS")) == -1){
                    perror("unlink /tmp/fifoS");
                    return NULL;
                  }
                  break;
        case 'E':

                  if((nr = unlink("/tmp/fifoE")) == -1){
                    perror("unlink /tmp/fifoE");
                    return NULL;
                  }
                  break;
        case 'O':
                  if((nr = unlink("/tmp/fifoO")) == -1){
                    perror("unlink /tmp/fifoO");
                    return NULL;
                  }
                  break;
      }

  return NULL;
}

int main(int argc, char const *argv[]) {

    if(argc != 3){
      printf("Usage: %s <N_LUGARES> <T_ABERTURA> \n", argv[0]);
      exit(1);
    }

    printf("Parque abriu!\n");

    lugaresDisponiveis=atoi(argv[1]);
    start = times(NULL);
    closed=0;

    int end_time=atoi(argv[2]);

    if(end_time==0){
      printf("Parque Encerrou!\n");
      return 0;
    }

    //garantir que os semaforos ainda não foram criados
    sem_unlink("/semaphoreN");
    sem_unlink("/semaphoreS");
    sem_unlink("/semaphoreO");
    sem_unlink("/semaphoreE");

    //abrir os semafores para fazer uma terminação de cada controlador estavel

     sem_t *semN = sem_open("/semaphoreN", O_CREAT, 0644,1);
     if(semN == SEM_FAILED){
       perror("semaphoreN");
       exit(5);
     }

     sem_t *semS = sem_open("/semaphoreS", O_CREAT, 0644,1);
     if(semS == SEM_FAILED){
       perror("semaphoreS");
       exit(5);
     }

     sem_t *semO = sem_open("/semaphoreO", O_CREAT, 0644,1);
     if(semO == SEM_FAILED){
       perror("semaphoreO");
       exit(5);
     }

     sem_t *semE = sem_open("/semaphoreE", O_CREAT, 0644,1);
     if(semE == SEM_FAILED){
       perror("semaphoreE");
       exit(5);
     }

    //iniciar log do parque
    int fd = open("parque.log",O_RDWR | O_APPEND | O_CREAT | O_TRUNC ,0644);

    if(fd == -1){
      perror("error on opening of file parque.log");
      return 1;
    }

    fp = fdopen(fd, "w");
    if(fp == NULL){
      perror("error on parque.log");
      exit(4);
    }

    fprintf(fp, "t(ticks) ; nlug; id_viat ; observ \n" );

      // cria as threads do controlador N, S, E, O
      pthread_t tN,tS,tE,tO;

      char n='N' , s='S' , e = 'E', o='O';
      if(pthread_create(&tN, NULL, thrControlador, &n)){
        perror("Error Creating Thread North!");
        exit(2);
      }
      if(pthread_create(&tS, NULL, thrControlador, &s)){
        perror("Error Creating Thread South!");
        exit(2);
      }
      if(pthread_create(&tE, NULL, thrControlador, &e)){
        perror("Error Creating Thread East!");
        exit(2);
      }
      if(pthread_create(&tO, NULL, thrControlador, &o)){
        perror("Error Creating Thread West!");
        exit(2);
      }

      //esperar que o parque termine
      sleep(end_time);

      //ciaçao da viatura que indica o fecho do parque
      struct ViaturaInfo info;
      sprintf(info.nomeFifo,"sair");

      int fdN=open("/tmp/fifoN", O_WRONLY );
      if(fdN == -1){
        perror("erro opening /tmp/fifoN");
        exit(3);
      }

      int fdS=open("/tmp/fifoS", O_WRONLY );
      if(fdS == -1){
        perror("erro opening /tmp/fifoS");
        exit(3);
      }

      int fdO=open("/tmp/fifoO", O_WRONLY );
      if(fdO == -1){
        perror("erro opening /tmp/fifoO");
        exit(3);
      }

      int fdE=open("/tmp/fifoE", O_WRONLY );
      if(fdE == -1){
        perror("erro opening /tmp/fifoE");
        exit(3);
      }


      //fechar os controladores de forma controlada
      //Norte
      if(sem_wait(semN) == -1){
        perror("error on sem_wait of semN");
        exit(4);
      }
      if(write(fdN,&info,sizeof(struct ViaturaInfo)) == -1 ){
        perror("writing on fifo North");
        exit(5);
      }
      if(close(fdN) == -1){
        perror("error closing fifo North");
        exit(6);
      }
      pthread_join(tN,NULL);
      if(sem_post(semN) == -1){
        perror("error on unlocking semaphore North");
        exit(7);
      }
      if(sem_close(semN) == -1){
        perror("error closing semaphore North");
        exit(8);
      }
      sem_unlink("/semaphoreN");


      //Sul
      if(sem_wait(semS) == -1){
        perror("error on sem_wait of semS");
        exit(4);
      }
      if(write(fdS,&info,sizeof(struct ViaturaInfo)) == -1 ){
        perror("writing on fifo South");
        exit(5);
      }

      if(close(fdS) == -1){
        perror("error closing fifo South");
        exit(6);
      }
      pthread_join(tS,NULL);
      if(sem_post(semS) == -1){
        perror("error on unlocking semaphore South");
        exit(7);
      }
      if(sem_close(semS) == -1){
        perror("error closing semaphore South");
        exit(8);
      }
      sem_unlink("/semaphoreS");


      //Oeste
      if(sem_wait(semO) == -1){
        perror("error on sem_wait of semO");
        exit(4);
      }
      if(write(fdO,&info,sizeof(struct ViaturaInfo)) == -1 ){
        perror("writing on fifo West");
        exit(5);
      }
      if(close(fdO) == -1){
        perror("error closing fifo West");
        exit(6);
      }
      pthread_join(tO,NULL);
      if(sem_post(semO) == -1){
        perror("error on unlocking semaphore West");
        exit(7);
      }
      if(sem_close(semO) == -1){
        perror("error closing semaphore West");
        exit(8);
      }
      sem_unlink("/semaphoreO");


      //Este
      if(sem_wait(semE) == -1){
        perror("error on sem_wait of semE");
        exit(4);
      }
      if(write(fdE,&info,sizeof(struct ViaturaInfo)) == -1 ){
        perror("writing on fifo East");
        exit(5);
      }
      if(close(fdE) == -1){
        perror("error closing fifo East");
        exit(6);
      }
      pthread_join(tE,NULL);
      if(sem_post(semE) == -1){
        perror("error on unlocking semaphore East");
        exit(7);
      }
      if(sem_close(semE) == -1){
        perror("error closing semaphore East");
        exit(8);
      }
      sem_unlink("/semaphoreE");

      printf("Parque Encerrou!\n");

  pthread_exit(NULL);
}
