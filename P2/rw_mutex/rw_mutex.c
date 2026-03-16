#include <pthread.h>

struct rw_mutex_t {
    pthread_mutex_t lock; //mutex principal
    pthread_cond_t readers; //condicion para los lectores
    pthread_cond_t writers; //condicion para los escritores
    int readers_count; //lecores activos
    int writers_count; //escritores activos (solo puede ser 0 o 1)
    int waiting_writers; //escritores esperando
};

#include "rw_mutex.h"

int rw_mutex_init(rw_mutex_t *m){
    m -> readers_count = 0;
    m -> writers_count = 0;
    m -> waiting_writers = 0;
    if (pthread_mutex_init(&m -> lock, NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&m -> readers, NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&m -> writers, NULL) != 0) {
        return -1;
    }
    return 0;
}

int rw_mutex_destroy(rw_mutex_t *m) {
    pthread_mutex_destroy(&m -> lock);
    pthread_cond_destroy(&m -> readers);
    pthread_cond_destroy(&m -> writers);
    return 0;
}

int rw_mutex_writelock(rw_mutex_t *m) {
    pthread_mutex_lock (&m -> lock);
    m -> waiting_writers++;
    while (m -> readers_count > 0 || m -> writers_count > 0) {
        pthread_cond_wait(&m -> writers, &m -> lock);
    }
    m -> waiting_writers--;
    m -> writers_count = 1;
    pthread_mutex_unlock(&m -> lock);
    return 0;
}

int rw_mutex_readunlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    m->readers_count--;
    // Si no quedan lectores, despertamos a un escritor
    if (m->readers_count == 0 && m->waiting_writers > 0) {
        pthread_cond_signal(&m->writers);
    }
    pthread_mutex_unlock(&m->lock);
    return 0;
}


int rw_mutex_writeunlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    m->writers_count = 0;
    pthread_cond_signal(&m->readers);
    pthread_mutex_unlock(&m->lock);
    return 0;
}
