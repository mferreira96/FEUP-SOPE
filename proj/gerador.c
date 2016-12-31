/*
*Criado por:
*  -José Martins -up201404189
*  -Marcelo Ferreira -up201405323
*
*   Turma 1
*/

#include "common.h"

// variaveis globais
int id_viatura;
clock_t start;
FILE* fp;

void *thr_viatura(void *arg){

  struct ViaturaInfo *info= arg;

  sem_t *sem;
  char *sem_name;
  switch (info->direcao) {
    case 'N':
      sem_name="/semaphoreN";
        break;
    case 'S':
      sem_name="/semaphoreS";
        break;
    case 'E':
      sem_name="/semaphoreE";
        break;
    case 'O':
      sem_name="/semaphoreO";
        break;
}

if((sem = sem_open(sem_name, O_EXCL,NULL,NULL))==SEM_FAILED){//se o parque estiver fechado
  printf("Gerada viatura-> %d (PARQUE ENCERRADO!)\n",info->id);
  fprintf(fp, "%8d ; %7d ; %6c ;       %10f     ;       ?      ; encerrado\n",(int)(times(NULL)-start),info->id,info->direcao,info->duracao);
  free(arg);
  return NULL;
}

  printf("Gerada viatura-> %d (PARQUE ABERTO!)\n",info->id);

  //fazer fifo privado
  if(mkfifo(info->nomeFifo,0660) == -1){
    free(arg);
    perror("creating vehicle fifo ");
    return NULL;
  }

  int myfifo;
  myfifo=open(info->nomeFifo,O_RDWR);
  if(myfifo == -1){
    perror("opening vehicle fifo");
    unlink(info->nomeFifo);
    free(arg);
    return NULL;
  }

  if(sem_wait(sem) == -1){
    free(arg);
    perror("waiting semaphore");
    return NULL;
  }

  //escrever no fifo do controlador
  int controllerFifo;
  switch (info->direcao) {
    case 'N':
      controllerFifo=open("/tmp/fifoN",O_WRONLY | O_NONBLOCK);
        break;
    case 'S':
      controllerFifo=open("/tmp/fifoS",O_WRONLY | O_NONBLOCK);
        break;
    case 'E':
      controllerFifo=open("/tmp/fifoE",O_WRONLY | O_NONBLOCK);
        break;
    case 'O':
      controllerFifo=open("/tmp/fifoO",O_WRONLY | O_NONBLOCK);
        break;
  }

  if(controllerFifo == -1){
    unlink(info->nomeFifo);
    sem_post(sem);
    sem_close(sem);
    free(arg);
    perror("error opening controller fifo");
    return NULL;
  }

  if (write(controllerFifo, info,sizeof(struct ViaturaInfo)) == -1){
    unlink(info->nomeFifo);
    sem_post(sem);
    sem_close(sem);
    close(controllerFifo);
    perror("writing on controller fifo");
    free(arg);
    return NULL;
  }

  if(close(controllerFifo) == -1){
    perror("error on closing controllerFifo");
    free(arg);
    return NULL;
  }
  if(sem_post(sem) == -1){
    perror("error unlocking semaphore");
    free(arg);
    return NULL;
  }
  if(sem_close(sem) == -1){
    perror("error closing semaphore");
    free(arg);
    return NULL;
  }


  //esperar reposta do arrumador
  char awnser[PIPE_BUF];
  read(myfifo,awnser, PIPE_BUF);
  clock_t t_vidaInit=times(NULL);


  if(strncmp("entrada",awnser,7)==0){
      //printf("entrou\n" );
      fprintf(fp, "%8d ; %7d ; %6c ;       %10f     ;       ?      ; entrada\n",(int)(times(NULL)-start),info->id,info->direcao,info->duracao);
      read(myfifo,awnser, PIPE_BUF);
      if(strncmp("saida",awnser,4)==0){
        fprintf(fp, "%8d ; %7d ; %6c ;       %10f     ;   %5d      ; saida\n",(int)(times(NULL)-start),info->id,info->direcao,info->duracao,(int)(times(NULL)-t_vidaInit));
        //printf("saiu\n" );
      }
  }else if(strncmp("encerrado",awnser,9)==0){
    fprintf(fp, "%8d ; %7d ; %6c ;       %10f     ;       ?      ; acabou de encerrar\n",(int)(times(NULL)-start),info->id,info->direcao,info->duracao);
  }else{
    //printf("cheio\n" );
    fprintf(fp, "%8d ; %7d ; %6c ;       %10f     ;       ?      ; cheio!\n",(int)(times(NULL)-start),info->id,info->direcao,info->duracao);
  }

  if(close(myfifo) == -1){
    perror("error closing myfifo");
    unlink(info->nomeFifo);
    free(arg);
    return NULL;
  }

  if (unlink(info->nomeFifo) == -1){
    perror("error unlinking fifo");
    free(arg);
    return NULL;
  }

  free(arg);
  return NULL;
}



// obter o intervalo entre as geracoes de viaturas
double intervalo_geracao_de_viaturas(double u_relogio){

    int vec[10] = {0,0,0,0,0,1,1,1,2,2};
    int id = rand() % 10;

    return vec[id] * u_relogio;
}

// obter a direcao da viatura
char getDirection(){
  int dir=rand() % 4;

  switch (dir) {
    case 0:
            return 'N';
    case 1:
            return 'S';
    case 2:
            return 'O';
  }

  return 'E';
}



int main(int argc, char const *argv[]) {

      if(argc != 3){
        printf("Usage: %s <T_GERACAO> <U_RELOGIO> \n", argv[0]);
        exit(1);
      }

      printf("Inicio do gerador\n");

      srand(time(NULL));

      //iniciar as variaveis globais

      id_viatura=1;
      start=times(NULL);

      //iniciar log do gerador
      int fd = open("gerador.log",O_RDWR | O_APPEND | O_CREAT | O_TRUNC ,0644);
      if(fd == -1){
        perror("opening gerador.log");
        exit(2);
      }

      fp = fdopen(fd, "w");
      if(fp == NULL){
        perror("error on gerador.log");
        exit(3);
      }

      fprintf(fp, "t(ticks) ; id_viat ; destin ; t_estacion(segundos) ; t_vida(ticks) ; observ \n" );



      int t_geracao = atoi(argv[1]); // segundos
      int tps = sysconf(_SC_CLK_TCK);// ticks do processador
      double u_relogio = (double)atoi(argv[2])/tps; // segundos


      time_t aux_tempo = time(NULL); // guarda o tempo do inicio do programa
      double total = 0;


      while (total < t_geracao) {



        //informacao da viatura
        struct ViaturaInfo *info= (struct ViaturaInfo*)malloc(sizeof(struct ViaturaInfo));
        info->id=id_viatura++;
        info->duracao = u_relogio * ( 1 + rand() % 10);  //ticks
        info->direcao = getDirection();
        sprintf(info->nomeFifo,"/tmp/viaturaFifo%d", info->id);


        //criacao da thread viatura
        pthread_t pViatura;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if(pthread_create(&pViatura, &attr, thr_viatura, info )){
          perror("error creating detached pthread");
          exit(4);
        }


        // determinar tempo para poder gerar uma nova viatura e esperar esse tempo
        double t_espera = intervalo_geracao_de_viaturas(u_relogio);
        mySleep(t_espera);

      total =(double) time(NULL) - aux_tempo;
    }

      printf("Fim do gerador, tempo total -> %f\n",total );

      pthread_exit(NULL);
}
