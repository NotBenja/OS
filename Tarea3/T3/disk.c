#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#include "pss.h"

#define TRUE 1
#define FALSE 0

/*****************************************************
 * Agregue aca los tipos, variables globales u otras
 * funciones que necesite
 *****************************************************/
PriQueue *q;
PriQueue *q2;
pthread_mutex_t m;
int current_track;
int using = FALSE;

//Estructura para requests: request
typedef struct {
  int ready;
  int track;
  pthread_cond_t c;
}Request;


//inicializar request: Request req = {FALSE, pista, PTHREAD_COND_INITIALIZER}
int findNextTrack(int track){
  
  int nextTrack = -1;


  /* Casos posibles
  1. Se busca pista t' con t<=t'en la cola
  2. No se encuentra pista y se busca la pista más cercana a 0 (el min)

  */

  //Manejar cola de prioridad: Hacer peek en requestDisk y get en releaseDisk (idea)

  /*
   while(!req.ready){
    pthread_cond_wait(&req.c, &m);
   }

   nextTrack = req.track;
   
  
  */

 


  //retornar pista 

  return nextTrack;
}


//Estructura para ordenar request:PriQueue
void iniDisk(void) {
  pthread_mutex_init(&m,NULL);
  q = makePriQueue();
  q2 = makePriQueue();
}

void cleanDisk(void) {
  pthread_mutex_destroy(&m);
  destroyPriQueue(q);
  destroyPriQueue(q2);
}

void requestDisk(int track) {
  pthread_mutex_lock(&m);
  if(!using){
    using = TRUE;
  } else {
    Request req = {FALSE, track, PTHREAD_COND_INITIALIZER};
    //priPut(PriQueue *q, void *elem, double pri)
    while(!req.ready){
      pthread_cond_wait(&req.c, &m);
    }
  }
  pthread_mutex_unlock(&m);
}

void releaseDisk() {
  pthread_mutex_lock(&m);
  //Ver si la cola está vacía con emptyPriQueue 

  /* Actualizar la request a la siguiente en la cola
    req = priGet(q);
    req.ready = TRUE;
    phtread_cond_signal(&req.cond,&m);

  */
  if(emptyPriQueue(q)){
    using = FALSE;
  } else {
    Request *req = priGet(q);
    req->ready = TRUE;
    pthread_cond_signal(&req->c);
  }
  pthread_mutex_unlock(&m);
}
