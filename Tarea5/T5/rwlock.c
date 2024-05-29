#define _XOPEN_SOURCE 500

#include "nKernel/nthread-impl.h"
#include "rwlock.h"

// Creamos estructura que almacena el contexto para manejar entrada y salida de lectores
struct rwlock {
  int readers;
  int writing;
  NthQueue *pending_readers;
  NthQueue *pending_writers;
};

// Funcion que se llama cuando se termina el timeout
void nDeleteFromQueue(nThread th){
  // Se llama a la cola donde esta el nThread 
  NthQueue *queue = th->ptr;
  // Borramos el nThread de la cola
  nth_delQueue(queue, th);
  // Quitamos la cola del descriptor del nThread
  th->ptr = NULL;
}


nRWLock *nMakeRWLock() {
  // Pedimos memoria para la estructura
  nRWLock *rwl = malloc(sizeof(nRWLock));
  // Lectores y escritores comienzan en 0
  rwl->readers = 0;
  rwl->writing = 0;
  rwl->pending_readers = nth_makeQueue();
  rwl->pending_writers = nth_makeQueue();
  nth_initQueue(rwl->pending_readers);
  nth_initQueue(rwl->pending_writers);
  return rwl;
}
// Funcion para liberar la memoria de la estructura
void nDestroyRWLock(nRWLock *rwl) {
  nth_destroyQueue(rwl->pending_readers);
  nth_destroyQueue(rwl->pending_writers);
}

int nEnterRead(nRWLock *rwl, int timeout) {
  // Comenzamos seccion critica
  START_CRITICAL
  // Llamamos al nThread actual
  nThread this_th = nSelf();
  // Si no hay nadie escribiendo y no hay escritores pendientes se acepta la entrada
  if (!rwl->writing && nth_emptyQueue(rwl->pending_writers)) {
    rwl->readers++;
  } else {
    // Si no, encolamos la solicitud del lector y esperamos
    nth_putBack(rwl->pending_readers, this_th);
    suspend(WAIT_RWLOCK);
    schedule();
  }
// Termina seccion critica
  END_CRITICAL
  return 1;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
  //  Comenzamos seccion critica
  START_CRITICAL
  // Si no hay nadie leyendo y nadie escribiendo se acepta la entrada
  if (rwl->readers == 0 && !rwl->writing) {
    rwl->writing = 1;
  } else {
    // Si no, encolamos la solicitud del escritor y esperamos
    nThread this_th = nSelf();
    nth_putBack(rwl->pending_writers, this_th);
    // Si el timeout es <=0, esperamos indefinidamente
    if (timeout <= 0) {
      suspend(WAIT_RWLOCK);
      schedule();
     
    } else {
      // Si no, esperamos el tiempo del timeout con nth_programTimer
      suspend(WAIT_RWLOCK_TIMEOUT);
      this_th->ptr = (NthQueue*) rwl->pending_writers;
      // En caso de exceder el tiempo, llamamos a nDeleteFromQueue que borra el nThread de la cola
      nth_programTimer(timeout*1000000LL, nDeleteFromQueue);
      schedule();
      // Si despues de llamar a schedule() el descriptor del nThread no tiene a la cola, entonces retornamos 0
      if(this_th->ptr == NULL){
        return 0;
      }
    }
  }
  // Termina seccion critica
  END_CRITICAL
  return 1;
}

void nExitRead(nRWLock *rwl) {
  // Comenzamos seccion critica
  START_CRITICAL
  // Como un lector sale, disminuimos el contador de lectores
  rwl->readers--;
  // Si no hay lectores y hay escritores pendientes, se despierta al siguiente escritor
  if (rwl->readers == 0 && !nth_emptyQueue(rwl->pending_writers)) {
    nThread th = nth_getFront(rwl->pending_writers);
    // Si el escritor esta esperando, cancelamos su timer
    if(th->status == WAIT_RWLOCK_TIMEOUT){
      nth_cancelThread(th);
    }
    // Se cambia el contexto a escribiendo
    rwl->writing = 1;
    setReady(th);
    schedule();

  }
 // Termina seccion critica
  END_CRITICAL
}

void nExitWrite(nRWLock *rwl) {
  // Comenzamos seccion critica
  START_CRITICAL
  // Como salio un escritor, se cambia el contexto a nadie escribiendo
  rwl->writing = 0;
  // Si hay lectores pendientes, se despiertan a los siguientes lectores
  if (!nth_emptyQueue(rwl->pending_readers)) {
    // Sacamos a todos los lectores pendientes
    while (!nth_emptyQueue(rwl->pending_readers)) {
      nThread th = nth_getFront(rwl->pending_readers);
      // Sumamos un contador por cada lector
      rwl->readers++;
      setReady(th);
      schedule();
    }
    // Si no hay lectores pendientes, se revisa si hay escritores pendientes
  } else if (!nth_emptyQueue(rwl->pending_writers)) {
    nThread th = nth_getFront(rwl->pending_writers);
    // Si el escritor esta esperando, cancelamos su timer
    if(th->status == WAIT_RWLOCK_TIMEOUT){
      nth_cancelThread(th);
    } 
    // Se cambia el contexto a escribiendo
    rwl->writing = 1;
    // Despertamos al escritor
    setReady(th);
    schedule();
  }
  // Termina seccion critica
  END_CRITICAL
}
