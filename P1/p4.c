#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define ITERACIONES 100
#define ATHREAD 3
#define BTHREAD 3
#define DELAY_NS 10ULL

struct Datos{
    volatile int* suma; //(contador a sumar)
    volatile int* resta; //(contador a restar)
    pthread_mutex_t *mutex;
};
volatile bool stop_monitor = false;

void* func_thread(void* tr){
    struct Datos* datos = (struct Datos*) tr;
    for(int i = 0; i < ITERACIONES; i++){
        pthread_mutex_lock(datos->mutex);
        (*datos->suma)++;
        usleep(DELAY_NS);
        (*datos->resta)--;
        pthread_mutex_unlock(datos->mutex);
        usleep(DELAY_NS);
    }
    return NULL;
}

//funcion para thread monitorización
void* func_thread_monitor(void* tr){
    struct Datos* datos = (struct Datos*) tr;
    while(!stop_monitor){
        pthread_mutex_lock(datos->mutex);
        printf("Contador a: %3d", *(datos->suma));
        printf("  Contador b: %3d", *(datos->resta));
        printf("  Suma de a y b: %d\n", *(datos->suma) + *(datos->resta));
        pthread_mutex_unlock(datos->mutex);
        usleep(DELAY_NS);
    }
    return NULL;
}

int main(){
    volatile int a = 0;
    volatile int b = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
//TYPEA
    struct Datos* p = malloc(sizeof(struct Datos)); //heap
    p->suma = &a;
    p->resta = &b;
    p->mutex = &mutex;
//THREAD MONITORIZACIÓN
    pthread_t threadmonitor;
    pthread_create(&threadmonitor, NULL, func_thread_monitor, p);
//
    pthread_t threads_a[ATHREAD];
    for (int i = 0; i < ATHREAD; i++) {
        pthread_create(&threads_a[i], NULL, func_thread, p);

    }

//TYPEB
    struct Datos threadb[BTHREAD]; //stack
    pthread_t threads_b[BTHREAD];
    for (int i = 0; i < BTHREAD; i++) {
        threadb[i].suma = &b;
        threadb[i].resta = &a;
        threadb[i].mutex = &mutex;
        pthread_create(&threads_b[i], NULL, func_thread, &threadb[i]);
    }

    for (int i = 0; i < ATHREAD; i++) {//pthread_join de A
        pthread_join(threads_a[i], NULL);
    }

    for (int i = 0; i < BTHREAD; i++) {//pthread_join de B
        pthread_join(threads_b[i], NULL);
    }

    stop_monitor = true; //parar monitor
    pthread_join(threadmonitor, NULL);//pthread_join del monitor
    free(p);
    pthread_mutex_destroy(&mutex);
    return 0;
}
