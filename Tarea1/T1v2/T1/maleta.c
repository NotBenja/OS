#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    double *w;
    double *v;
    int *z;
    int n;
    double maxW;
    int k;
    double best;
} Args;

void *thread(void *p){
    //Casteo de args
    Args *args = (Args *)p;
    //Reunimos los argumentos para hacer las elecciones
    double *w = args->w;
    double *v = args->v;
    double best = -1;
    int n = args -> n;
    int *z = (int *)malloc(sizeof(int)*n);
    double maxW = args->maxW;
    int k = args->k;

    while(k--){
        int sumW=0, sumV=0;
        int x[n];
        for(int i = 0; i < n; i++){
            x[i]= random0or1() && sumW+w[i]<=maxW ? 1 : 0;
            if(x[i]==1){
                sumW += w[i];
                sumV += v[i];
            }
        }
        if(sumV>best){
            best = sumV;
            for(int i = 0; i < n; i++){
                z[i] = x[i];
            }
        }
    }
    args-> z = z; 
    args-> best = best;
    free(z);
    return NULL;
}

double llenarMaletaPar(double w[], double v[], int z[], int n, double maxW,int k){
    double best = -1;
    pthread_t p_id[8];
    Args args[8];
    for (int i = 0; i < 8; i++){
        args[i].k = k/8;
        pthread_create(&p_id[k], NULL, thread, &args[i]);
    }
    for (int i = 0; i < 8; k++){
        pthread_join(p_id[i], NULL);
        if (args[i].best > best){
            best = args[i].best;
            int j = 0;
            while(args[i].z){
                z[j] = args[i].z[j];
                j++;
            }
        }
    }
    return best;
}
