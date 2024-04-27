#include <stdlib.h>
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
//Pista en la que se encuentra cabezal del disco
int current_track = 0;
//Cola actual de donde se extraen elementos (se usa TRUE como switch)
int current_queue = TRUE;
//Variable que indica si el disco está en uso
int using = FALSE;

//Estructura para requests: request
typedef struct {
  int ready;
  int track;
  pthread_cond_t c;
}Request;


//Función que retorna una cola en base a un id.
PriQueue *selectQueue(int q_id){
  PriQueue *selectedQueue = NULL;
  if(q_id == TRUE){
    selectedQueue = q;
  } else if (q_id == FALSE){
    selectedQueue = q2;
  }
  return selectedQueue;
}

//Funcion que realiza get en cola seleccionada
/*La clave está en que las requests en las colas están ordenadas de menor a mayor track,
  por lo que siempre se extraerá desde la request con menor track a la que tiene la mayor.
*/
Request *getRequest(PriQueue *selectedQueue) {
  Request *req = NULL;
  while (!emptyPriQueue(selectedQueue)) {
    Request *ptr = priGet(selectedQueue);//Obtener la siguiente solicitud
    if (ptr == NULL) { //Siempre verificar si es NULL
      continue; //Saltar este ciclo si la solicitud es NULL
    }
    if (ptr->track >= current_track) { //Si cumple la condición
      req = ptr; //Asignar la solicitud válida
      break; //Salir del bucle si encontramos una solicitud válida
    }
  }

  return req; //Puede ser NULL si no hay solicitud válida
}


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
  //Seleccionamos la cola actual
  PriQueue *selectedQueue = selectQueue(current_queue);
  if(!using){
    //Si el disco no se está usando, actualizamos su estado y el cabezal
    using = TRUE;
    current_track = track;
  } else {
    //En caso contrario, hacemos una request 
    //Decisión: Se tratará al número de pista de la request como la prioridad
    Request req = {FALSE, track, PTHREAD_COND_INITIALIZER};
    //Si sabemos que hay elementos pendientes que no cumplieron con track >= current_track
    //Entonces los nuevos elementos a ingresar que no cumplan deben ir a la cola de respaldo
    if(!(track >= current_track)){
      selectedQueue = selectQueue(!current_queue);
    }
    //Si hay un elemento a ingresar que cumpla la desigualdad, entonces debemos
    //Encolarlo en la cola con todos aquellos elementos que también cumplen
    priPut(selectedQueue, &req, req.track);
    //Mientras la request no sea atendida, debemos esperar
    while(!req.ready){
      pthread_cond_wait(&req.c, &m);
    }
  }
  pthread_mutex_unlock(&m);
}

void releaseDisk() {
  pthread_mutex_lock(&m);
  if(emptyPriQueue(q) && emptyPriQueue(q2)){
    //Caso base, si no hay peticiones es porque el disco no está en uso
    //O porque solo se ha atendido un acceso y ya terminó su tarea.
    using = FALSE;
  } else{
    PriQueue *selectedQueue = selectQueue(current_queue);
    //Si la cola elegida está vacía, intercambiarla por la otra,
    //la cual no puede estar vacía por el if anterior
    if(emptyPriQueue(selectedQueue)){
        current_queue = !current_queue;
        current_track = 0;
        selectedQueue = selectQueue(current_queue);
    }
    //Obtenemos la request con mayor prioridad
    Request *req = getRequest(selectedQueue);
    //Declaramos que la request fue atendida y cambiamos el cabezal del disco
    req->ready = TRUE;
    //Ya no estamos usando el disco, así que despertamos al siguiente thread
    pthread_cond_signal(&req->c);
  }
  pthread_mutex_unlock(&m);
}

