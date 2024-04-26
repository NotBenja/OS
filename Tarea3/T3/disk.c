#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#include "pss.h"

#define TRUE 1
#define FALSE 0

PriQueue *q;
PriQueue *q2;
pthread_mutex_t m;
int current_track;
int current_queue = TRUE;
int traspaso = FALSE;
int using = FALSE;

//Estructura para requests: request
typedef struct {
  int ready;
  int track;
  pthread_cond_t c;
}Request;

/*La clave está en que las requests en las colas están ordenadas de menor a mayor track,
  por lo que siempre se extraerá desde la request con menor track a la que tiene la mayor.
*/

PriQueue *selectQueue(int q_id){
  PriQueue *selectedQueue = NULL;
  if(q_id == TRUE){
    selectedQueue = q;
  } else if (q_id == FALSE){
    selectedQueue = q2;
  }
  return selectedQueue;
}

Request *getRequest(PriQueue *selectedQueue) {
  //Inicializamos la request prioritaria
  Request *req = NULL;
  Request *ptr;
  //Invocamos a la cola sin seleccionar, en caso de sacar elementos que no
  //cumplen con t>=current_t
  PriQueue *unselectedQueue = selectQueue(!current_queue);
  while(!emptyPriQueue(selectedQueue)){
    //Llamamos a la request con menor track
    ptr = priGet(selectedQueue);
    //Si cumple t>=current_t, la guardamos
    if(ptr->track >= current_track){
      req = ptr;
      break;
    } else {
      //Si no, encolamos en la otra cola
      traspaso = TRUE;
      priPut(unselectedQueue, ptr, ptr->track);
    }
  }
  //Una vez que se quitan todos los elementos con t>=current_t
  //Se reinicia la búsqueda desde 0
  if(emptyPriQueue(selectedQueue) && traspaso){
    current_track = 0;
  }
  //Retornamos la request que cumple con la desigualdad
  //Se retorna NULL si: 
  //1.La cola actual está vacía 
  //2.Ningún elemento cumplía la desigualdad
  return req;
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
  PriQueue *selectedQueue = selectQueue(current_queue);
  if(!using){
    using = TRUE;
  } else {
    Request req = {FALSE, track, PTHREAD_COND_INITIALIZER};
    //Decisión: Se tratará al número de pista como la prioridad
    priPut(selectedQueue, &req, req.track);
    while(!req.ready){
      pthread_cond_wait(&req.c, &m);
    }
    using = TRUE;
  }
  pthread_mutex_unlock(&m);
}

void releaseDisk() {
  pthread_mutex_lock(&m);
  if(emptyPriQueue(q) && emptyPriQueue(q2)){
    using = FALSE;
  } else{
    PriQueue *selectedQueue = selectQueue(current_queue);
    PriQueue *unselectedQueue = selectQueue(current_queue);
    Request *req = NULL;
    //En caso de estar en traspaso, se debe vaciar la lista no seleccionada
    if(traspaso && emptyPriQueue(selectedQueue)){
      req = priGet(unselectedQueue);
    } else{
      if(emptyPriQueue(selectedQueue)){
      //Se intercambia con la otra lista, que no puede ser vacía
      current_queue = !current_queue;
      selectedQueue = selectQueue(current_queue);
      req = getRequest(selectedQueue);
      } 
    }
    //Se obtiene la request con más prioridad
    req->ready = TRUE;
    current_track = req->track;
    using = FALSE;
    pthread_cond_signal(&req->c);
  }
  pthread_mutex_unlock(&m);
}