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
pthread_mutex_t m;


//Estructura para requests: request
typedef struct {
  pthread_cond_t c;
  int ready;
  int track;
  
}Request;

//Estructura para ordenar request:PriQueue
void iniDisk(void) {
  pthread_mutex_init(&m,NULL);
  q = makePriQueue();
}

void cleanDisk(void) {
  pthread_mutex_destroy(&m);
  destroyPriQueue(q);
}

void requestDisk(int track) {
  pthread_mutex_lock(&m);


  pthread_mutex_unlock(&m);
}

void releaseDisk() {
  pthread_mutex_lock(&m);


  pthread_mutex_unlock(&m);
}
