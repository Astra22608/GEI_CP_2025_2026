#include <pthread.h>
#include "rec_mutex.h"

int rec_mutex_init(rec_mutex_t *m) {
    m -> count = 0; //mutex desbloqueado
    m -> owner = 0; //propietario del mutex en cada momento (sin dueño)
    return pthread_mutex_init(&m -> mutex, NULL);
}

int rec_mutex_destroy(rec_mutex_t *m) {
    return pthread_mutex_destroy(&m -> mutex);
}

int rec_mutex_lock(rec_mutex_t *m) {
    pthread_t self = pthread_self(); //identificador del hilo
    if (m -> count > 0 && pthread_equal(m -> owner, self)) { //comprobamos si el mismo hilo ya tiene mutex
        m -> count++;
        return 0;
    }

    if (pthread_mutex_lock(&m -> mutex) != 0) { //si otro hilo lo tiene, esperamos
        return -1;
    }
    m -> owner = self; //guardamos nuevo dueño
    m -> count = 1;
    return 0;
};

int rec_mutex_unlock(rec_mutex_t *m) {
    pthread_t self = pthread_self();
    if (m -> count == 0 || !pthread_equal(m -> owner, self)) {
        return -1;
    }
    m -> count--;
    if (m -> count == 0) {
        m -> owner = 0;
        pthread_mutex_unlock(&m -> mutex);
    }
    return 0;
}


int rec_mutex_trylock(rec_mutex_t *m) {
    pthread_t self = pthread_self ();
    if (m -> count > 0 && pthread_equal(m -> owner, self)) {
        m -> count++;
        return 0;
    }
    if (pthread_mutex_trylock(&m -> mutex) != 0) {
        return -1;
    }
    m -> owner = self;
    m -> count = 1;
    return 0;
}
