#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <signal.h>
#include <string.h>

int clo1;
int clo2;
int clo3;

typedef struct Descripteur{
    int dS;
    int *alldS;
    int dSC;
}Descripteur;

struct Descripteur *descriG;

void exitt(int n){

  for(int i=0;i<10;i++){
    if(descriG->alldS[i] !=0){
      int clo=close(descriG->alldS[i]);
      if(clo==-1){
        perror("Error close : ");
      }
      else{
        printf("ok");
      }
    }
  }

  free(descriG);
  int closecli3=close(clo3);
  if(closecli3==-1){
    perror("Error close : ");
    exit(1);
  }
  printf("end");
  exit(0);
}


void *recevoirConnexion(void * data){

  struct Descripteur *descri = data;
  int perma=descri->dSC;
  char msg[100];
  int rep = recv(perma,msg,sizeof(msg),0);
  if(rep==-1){
	  perror("Echec de la reception du pseudo");
	  exit(1);
  }
  else{
    printf("%s join !\n",msg);
  }

  while(1){
    char msg2[100];
    char msg3[100];
    rep=recv(perma,msg2,sizeof(msg2),0);
    strcpy(msg3,msg2);
    msg2[strlen(msg2)-1] = '\0';
    if(strcmp(msg2,"fin")==0 ||strcmp(msg3,"fin")==0){
      printf("%s leave ! \n",msg);
      for(int i=0;i<10;i++){ //ajout tableau
        if(descri->alldS[i]==perma){
          descri->alldS[i]=0;
          break;
        }
      }
	    break;
    }
    printf("%s dit: %s\n",msg,msg3);
    for(int i=0;i<10;i++){
      if(descri->alldS[i]!=perma && descri->alldS[i]!=0){
        int snd = send(descri->alldS[i],msg3,sizeof(msg3),0);
        if(snd == 0 || snd == -1){
          perror("erreur send");
          exit(1);
        }
      }
    }
  }
  pthread_exit(NULL);
}

void *valTab(void * data){
  struct Descripteur *descri = data;
  while(1){
    for(int i=0;i<10;i++){ //ajout tableau
        printf("%d",descri->alldS[i]);
    }
    printf("\n");
    sleep(2);
  }
  pthread_exit(NULL);
}



int main(int argc, char *argv[]){
  int dS=clo3=socket(AF_INET, SOCK_STREAM, 0);
  if(dS==-1){
    perror("Erreur ! Socket non créee");
    exit(1);
  }
  if(setsockopt(dS,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int)) < 0){
    perror("setsockopt(SO_REUSEADDR) failed");
  }
  struct sockaddr_in ad;
  
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1]));
  
  int bi= bind(dS,(struct sockaddr*)&ad,sizeof(ad));
  if(bi == -1){
    perror("Nommage échoué");
    exit(1);
  }
  
  int li = listen(dS,10);
  if(li == -1){
    perror("File de connexion non intialisée");
    exit(1);
  }
  
  signal(SIGINT,exitt);
  struct sockaddr_in aC;
  socklen_t lg = sizeof(struct sockaddr_in);
  struct Descripteur *descri=malloc(sizeof(Descripteur));
  descriG=descri;
  descri->dS=dS;
  descri->alldS=calloc(10 , sizeof(int));
  /*pthread_t two;
  pthread_create(&two, NULL,valTab, (void*)descri);*/
  while(1){
    int dSC=accept(descri->dS, (struct sockaddr*) &aC,&lg); //connexion
    if(dSC==-1){
      perror("Erreur connexion");
      exit(1);
    }
    else{
      printf("Client connecté ...\n");
    }

    for(int i=0;i<10;i++){ //ajout tableau
      if(descri->alldS[i]==0){
        descri->alldS[i]=dSC;
        break;
      }
    }
    descri->dSC=dSC;
    descriG=descri;
    pthread_t one;
    pthread_create(&one, NULL,recevoirConnexion, (void*)descri);
   
  }


  free(descri);

  }