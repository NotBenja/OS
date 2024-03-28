#include <stdio.h>
#include <pthread.h>

typedef struct {
    double *w;
    double *v;
    int z;
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
    double *v = args-v;
    int *z = (int *)malloc(sizeof(int));
    double best = -1;
    int n = args -> n;
    double maxW = args->maxW;
    int k = args->k;
    RandGen *gen= makeRandGen();

    while(k--){
        int sumW=0, sumV=0;
        int x[n];
        for(int i = 0; i < n; i++){
            x[i]= random0or1(gen) && sumW+w[i]<=maxW ? 1 : 0;
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
    free(*z);
    return NULL;
}

double llenarMaletaSec(double w[], double v[], int z[], int n, double maxW,int k);

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
        if (args[i].best > best)
            best = args[i].best;
            z = args[i].z;
    }
    return best;
}
 