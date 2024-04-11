#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reservar.h"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ticket_dist, display;
int estacionamientos[10];

void initReservar() {
    for(int i = 0; i < 10; i++) {
        estacionamientos[i] = 0;
    }
    ticket_dist = 0;
    display = 0;
}

void cleanReservar() {
}

int espaciosLibres(int k, int* estacionamientos) {
    int e = -1;
    for (int i = 0; i <= 10 - k; i++) {
        int j;
        for (j = i; j < i+k; j++) {
            if (estacionamientos[j] == 1) {
                break;
            }
        }
        if (j == i+k) {
            e = i;
            break;
        }
    }
    return e;
}

int reservar(int k) {
    pthread_mutex_lock(&m);
    int my_num = ticket_dist++;
    int e;
    while (my_num != display || (e = espaciosLibres(k, estacionamientos)) == -1) {
        pthread_cond_wait(&cond, &m);
    }
    for (int i = e; i < e + k; i++) {
        estacionamientos[i] = 1;
    }
    display++;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&m);
    return e;
}

void liberar(int e, int k) {
    pthread_mutex_lock(&m);
    for (int i = e; i < e+k; i++) {
        estacionamientos[i] = 0;
    }
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&m);
}

