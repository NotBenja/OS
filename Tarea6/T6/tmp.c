#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "pss.h"
#include "bolsa.h"
#include "spinlocks.h"

enum {TRUE = 1, FALSE = 0};
enum {PEND, ADJ, RECH};

// Declaramos una variable para simular mutex
int mutex_lck = OPEN;
// Declaramos una variable para simular condiciones de espera
int* vend_lck_global = NULL;

// Declaramos las variables globales para el precio mínimo, el vendedor y el comprador
int precio_min = INT_MAX;
char *vendedor_min = NULL;
int* VL = NULL;

char *nombre_comprador = NULL;


void reset() {
  precio_min = INT_MAX;
  vendedor_min = NULL;
  VL = NULL;
  nombre_comprador = NULL;
}



int vendo(int precio, char *vendedor, char *comprador) {
  spinLock(&mutex_lck);
  // Inicializamos el estado de la oferta
  int vl = PEND;
  // Revisamos si la oferta es la más baja
  if (precio < precio_min){
    // Revisamos si existe una oferta previa
    if (vend_lck_global != NULL){
      // Si existe, entonces la rechazamos y despertamos al vendedor
      *VL = RECH;
      spinUnlock(vend_lck_global);
    }
    // Actualizamos el precio minimo y el puntero al nuevo vendedor
    precio_min = precio;
    VL = &vl;
    vendedor_min = vendedor;
    // Luego esperamos, actualizamos el puntero al nuevo lck y lo bloqueamos
    int vend_lck = CLOSED;
    vend_lck_global = &vend_lck;
    // Liberamos el mutex mientras esperamos
    spinUnlock(&mutex_lck);
    spinLock(&vend_lck);
  } else {
    // Si la oferta no es la más baja, entonces la rechazamos y retornamos
    vl = RECH;
    spinUnlock(&mutex_lck);
    return FALSE;
  }
  // En este caso, si se llega a este punto, entonces el vendedor de esta función
  // fue despertado por alguno de estos dos casos:
  spinLock(&mutex_lck);
  // 1. Su oferta fue superada por otra oferta
  if (vl == RECH) {
    spinUnlock(&mutex_lck);
    return FALSE;
  }
  // 2. Su oferta fue la más baja hasta el momento en que un comprador apareció
  spinUnlock(&mutex_lck);
  return TRUE;
}