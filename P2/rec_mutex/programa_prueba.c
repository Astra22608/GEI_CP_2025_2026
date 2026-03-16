#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>       
#include "rec_mutex.h"

#define ARRAY_SIZE 10
#define NUM_THREADS 4
#define NUM_SWAPS 20
#define SEGUNDOS 100

typedef struct {
    int array[ARRAY_SIZE];
    rec_mutex_t mutexes[ARRAY_SIZE];
} shared_data_t;

typedef struct {
    shared_data_t *shared;
    int thread_id;
} thread_arg_t;

void swap_positions(int *array, int i, int j) {
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

void* thread_func(void* arg) {
    thread_arg_t *targ = (thread_arg_t*)arg;
    shared_data_t *shared = targ->shared;
    int id = targ->thread_id;

    for (int k = 0; k < NUM_SWAPS; k++) {
        int i = rand() % ARRAY_SIZE;
        int j = rand() % ARRAY_SIZE;

        rec_mutex_lock(&shared->mutexes[i]);
        rec_mutex_lock(&shared->mutexes[j]);

        printf("Hilo %d intercambiando %d -> %d\n", id, i, j);
        swap_positions(shared->array, i, j);

        rec_mutex_unlock(&shared->mutexes[j]);
        rec_mutex_unlock(&shared->mutexes[i]);

        usleep(SEGUNDOS);
    }

    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t threads[NUM_THREADS];
    thread_arg_t thread_args[NUM_THREADS];

    shared_data_t shared = {0};

    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared.array[i] = i;
        rec_mutex_init(&shared.mutexes[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].shared = &shared;
        thread_args[i].thread_id = i + 1;
        pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nArray final: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", shared.array[i]);
    }
    printf("\n");

    for (int i = 0; i < ARRAY_SIZE; i++) {
        rec_mutex_destroy(&shared.mutexes[i]);
    }

    return 0;
}
