#include <stdio.h>
#include <pthread.h>

typedef struct {
    

} Args;

void *eleccion(void *p){

}

double llenarMaletaSec(double w[], double v[], int z[], int n, double maxW,
                       int k);
double llenarMaletaPar(double w[], double v[], int z[], int n, double maxW,int k){
    double best = -1;
    pthread_t p_id[8];
    Args *args;
    for(int i = 0; i<8 ;i++){
        pthread_create(&p_id[i],NULL, eleccion, args);
    }
}
 