#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "prio_mutex.h"

#define NUM_THREADS 5
#define MAX_PRIO 3

typedef struct {
    prio_mutex_t *mutex;
    int prio;
    int id;
} thread_arg_t;

void *thread_func(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;

    printf("Thread %d con prioridad %d intenta bloquear el mutex\n", targ->id, targ->prio);
    prio_mutex_lock(targ->mutex, targ->prio);

    printf("Thread %d con prioridad %d ha bloqueado el mutex\n", targ->id, targ->prio);
    usleep(500000); // Simula trabajo de 0.5 s

    printf("Thread %d con prioridad %d desbloquea el mutex\n", targ->id, targ->prio);
    prio_mutex_unlock(targ->mutex);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    prio_mutex_t mutex;

    prio_mutex_init(&mutex, MAX_PRIO);

    // Creamos hilos con distintas prioridades
    int prios[NUM_THREADS] = {0, 2, 1, 2, 0}; // Ejemplo de prioridades

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].mutex = &mutex;
        args[i].prio = prios[i];
        args[i].id = i;
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
        usleep(100000); // Separación para ver orden de llegada
    }

    // Esperamos a todos los hilos
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    prio_mutex_destroy(&mutex);
    return 0;
}