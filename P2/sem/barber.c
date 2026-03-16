#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "options.h"

#include "sem.h"
#include <pthread.h>
#define MAX_CUSTOMERS 30

struct barber_shop {
    sem_t customers;
    sem_t barbers;
    sem_t free_seats;

    int clientes_restantes;
    pthread_mutex_t mutex_clientes_restantes;

    struct options opt;
};

struct thread_data {
    int id;
    int clienterestante;
    struct barber_shop *shop;
};

void* barber_thread(void* arg){
    struct thread_data *data = (struct thread_data*) arg;
    int id = data->id;
    struct barber_shop *shop = data->shop;

    while(1){
    
        sem_p(&shop->customers);
        pthread_mutex_lock(&shop->mutex_clientes_restantes);
        if(shop->clientes_restantes <= 0){
            pthread_mutex_unlock(&shop->mutex_clientes_restantes);
            break;
        }
        pthread_mutex_unlock(&shop->mutex_clientes_restantes);
        sem_v(&shop->barbers);

        printf("Barbero %d está cortando el pelo.\n", id);

        usleep(shop->opt.cut_time);
    }
    return NULL;
}

void* customer_thread(void* arg){

    struct thread_data *data = (struct thread_data*) arg;
    int id = data->id;
    struct barber_shop *shop = data->shop;

    if(sem_tryp(&shop->free_seats)==0){
        
        pthread_mutex_lock(&shop->mutex_clientes_restantes);
        shop->clientes_restantes++;
        pthread_mutex_unlock(&shop->mutex_clientes_restantes);

        sem_v(&shop->customers); //se aumenta el número de clientes esperando (+1)
        sem_p(&shop->barbers); //espera a que un barbero esté disponible
        sem_v(&shop->free_seats); //se libera el asiento (+1)
        printf("Cliente %d está siendo atendido.\n", id);
        pthread_mutex_lock(&shop->mutex_clientes_restantes);
        shop->clientes_restantes--;
        pthread_mutex_unlock(&shop->mutex_clientes_restantes);
    }
    else{
        printf("Cliente %d se fue sin ser atendido.\n", id);
    }

    return NULL;
}
    


int main (int argc, char **argv)
{

    struct barber_shop shop;

    // Default values for the options
    shop.opt.barbers = 5;
    shop.opt.customers = 1000;
    shop.opt.cut_time  = 3000;

    read_options(argc, argv, &shop.opt);
    sem_init(&shop.barbers,0);
    sem_init(&shop.customers,0);
    sem_init(&shop.free_seats,MAX_CUSTOMERS);

    shop.clientes_restantes = 0;
    pthread_mutex_init(&shop.mutex_clientes_restantes, NULL);

    pthread_t barber_threads[shop.opt.barbers];
    pthread_t customer_threads[shop.opt.customers];

    struct thread_data barber_data[shop.opt.barbers];
    struct thread_data customer_data[shop.opt.customers];


    for(int i=0; i < shop.opt.barbers; i++){
        barber_data[i].id = i+1;
        barber_data[i].shop = &shop;
        pthread_create(&barber_threads[i], NULL, barber_thread, &barber_data[i]);
    }

    for(int i=0; i<shop.opt.customers; i++){
        customer_data[i].id = i+1;
        customer_data[i].shop = &shop;
        pthread_create(&customer_threads[i], NULL, customer_thread, &customer_data[i]);
    }

    for (int i = 0; i < shop.opt.customers; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    for (int i = 0; i < shop.opt.barbers; i++) {
        sem_v(&shop.customers);
    }

    for (int i = 0; i < shop.opt.barbers; i++) {
        pthread_join(barber_threads[i], NULL);
    }

    exit (0);
}