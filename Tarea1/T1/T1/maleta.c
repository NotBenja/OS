#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "maleta.h"
#include <string.h>
#include <pthread.h>

//Definimos una estructura que guarde los argumentos que necesitaremos.
typedef struct {
    double *w;
    double *v;
    int *z;
    int n;
    double maxW;
    int k;
    //Atributo donde guardaremos el resultado.
    double best;
} Args;

//Creamos la función que se ejecutará en paralelo.
void *thread(void *p) {
    //Casteamos los args
    Args *args = (Args *)p;
    //Definimos los argumentos para luego poder usarlos.
    double *w = args->w;
    double *v = args->v;
    int n = args->n;
    double maxW = args->maxW;
    int k = args->k;
    //Pedimos memoria para crear el arreglo z dentro de la llenarMaletaSec
    int *z = (int *)malloc(sizeof(int) * n);
    //Llamamos a la función secuencial y luego guardamos el resultado en el parámetro best.
    args->best = llenarMaletaSec(w, v, z, n, maxW, k);
    //Guardamos la combinación de z dada en el arreglo de la estructura para usarlo más tarde.
    args->z = z;
    return NULL;
}

double llenarMaletaPar(double w[], double v[], int z[], int n, double maxW, int k) {
    //Definimos best
    double best = -1;
    //Creamos un arreglo de identificadores para 8 threads.
    pthread_t p_id[8];
    //Creamos un arreglo con args para cada thread.
    Args args[8];
    for (int i = 0; i < 8; i++) {
        //Definimos los argumentos que tendrá que usar cada thread.
        args[i].k = k / 8;
        args[i].w = w;
        args[i].v = v;
        args[i].n = n;
        args[i].maxW = maxW;
        //Creamos el thread.
        pthread_create(&p_id[i], NULL, thread, &args[i]);
    }
    //Iteramos sobre cada thread
    for (int i = 0; i < 8; i++) {
        //Enterramos el thread
        pthread_join(p_id[i], NULL);
        //Hacemos la elección entre los threads para ver el resultado óptimo.
        if (args[i].best > best) {
            best = args[i].best;
            //Copiamos la configuración óptima.
            memcpy(z, args[i].z, sizeof(int) * n); 
        }
        //Liberamos la memoria pedida para cada arreglo z.
        free(args[i].z);
    }
    return best;
}
