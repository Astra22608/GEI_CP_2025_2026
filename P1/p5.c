#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define ITERACIONES 100
#define ATHREAD 1000
#define BTHREAD 1000
#define DELAY_NS 10ULL

struct Datos{
    volatile int* suma;
    volatile int* resta;
    pthread_mutex_t *mutex;
    pthread_mutex_t *mutexresta;
};
volatile bool stop_monitor = false;

void* func_thread(void* tr){
    struct Datos* datos = (struct Datos*) tr;
    for(int i = 0; i < ITERACIONES; i++){
        pthread_mutex_lock(datos->mutex);
        (*datos->suma)++;
        pthread_mutex_unlock(datos->mutex);
        
        pthread_mutex_lock(datos->mutexresta);
        (*datos->resta)--;
        pthread_mutex_unlock(datos->mutexresta);
        usleep(DELAY_NS);
    }
    return NULL;
}

void* func_thread_monitor(void* tr){
    struct Datos* datos = (struct Datos*) tr;
    while(!stop_monitor){
        pthread_mutex_lock(datos->mutex);
        pthread_mutex_lock(datos->mutexresta);
        printf("Contador a: %3d, Contador b: %3d, Suma de a y b: %d\n", *(datos->suma),*(datos->resta),*(datos->suma) + *(datos->resta));
        pthread_mutex_unlock(datos->mutexresta);
        pthread_mutex_unlock(datos->mutex);
        usleep(DELAY_NS);
    }
    return NULL;
}

int main(){
    volatile int a = 0;
    volatile int b = 0;
    pthread_mutex_t mutex_a;
    pthread_mutex_t mutex_b;
    pthread_mutex_init(&mutex_a, NULL);
    pthread_mutex_init(&mutex_b, NULL);

    struct Datos* p = malloc(sizeof(struct Datos));
    p->suma = &a;
    p->resta = &b;
    p->mutex = &mutex_a;
    p->mutexresta = &mutex_b;

    pthread_t threadmonitor;
    pthread_create(&threadmonitor, NULL, func_thread_monitor, p);

    pthread_t threads_a[ATHREAD];
    for (int i = 0; i < ATHREAD; i++) {
        pthread_create(&threads_a[i], NULL, func_thread, p);
    }

    struct Datos threadb[BTHREAD];
    pthread_t threads_b[BTHREAD];
    for (int i = 0; i < BTHREAD; i++) {
        threadb[i].suma = &b;
        threadb[i].resta = &a;
        threadb[i].mutex = &mutex_b;
        threadb[i].mutexresta = &mutex_a;
        pthread_create(&threads_b[i], NULL, func_thread, &threadb[i]);
    }

    for (int i = 0; i < ATHREAD; i++) {
        pthread_join(threads_a[i], NULL);
    }

    for (int i = 0; i < BTHREAD; i++) {
        pthread_join(threads_b[i], NULL);
    }

    stop_monitor = true;
    pthread_join(threadmonitor, NULL);
    free(p);
    pthread_mutex_destroy(&mutex_a);
    pthread_mutex_destroy(&mutex_b);

    return 0;
}
