#include <pthread.h>
#include <stdlib.h>
#include "prio_mutex.h"

int prio_mutex_init(prio_mutex_t *m, int max_prios) {
    m -> max_prios = max_prios;
    m -> locked = 0;

    pthread_mutex_init(&m -> lock, NULL);
    m -> conds = malloc(max_prios * sizeof(pthread_cond_t));
    m -> waiting = malloc(max_prios * sizeof(int));

    for(int i = 0; i < max_prios; i++) {
        pthread_cond_init(&m -> conds[i], NULL);
    }  
    return 0;
}

int prio_mutex_destroy(prio_mutex_t *m) {
    for (int i = 0; i < m -> max_prios; i++) {
        pthread_cond_destroy(&m -> conds[i]);
    }
    free (m -> conds);
    free (m -> waiting);
    pthread_mutex_destroy(&m -> lock);
    return 0;
}

int prio_mutex_lock(prio_mutex_t *m, int prio) {
    pthread_mutex_lock(&m -> lock);
    m -> waiting[prio]++;

    while (1){
        int higher = 0;
        for (int i = prio + 1; i < m -> max_prios; i++){ //miramos si hay prioridades mayores esperando.
            if (m -> waiting[i] > 0){
                higher = 1;
                break;
            }
        }
        if (!m -> locked && !higher){ //hilo entra si mutex libre y no hay mayor prioridad esperando.
            break;
        }
        pthread_cond_wait(&m -> conds[prio], &m -> lock);
    }
    m -> waiting[prio]--;
    m -> locked = 1;
    pthread_mutex_unlock(&m -> lock);
    return 0;
}

int prio_mutex_unlock(prio_mutex_t *m) {
    pthread_mutex_lock(&m->lock);

    m->locked = 0; 

    for (int p = m->max_prios - 1; p >= 0; p--) { //esperamos hilo mayor prioridad.
        if (m->waiting[p] > 0) {
            pthread_cond_signal(&m->conds[p]);
            break;
        }
    }

    pthread_mutex_unlock(&m->lock);
    return 0;
}

int prio_mutex_trylock(prio_mutex_t *m) {
    pthread_mutex_lock(&m -> lock);
    
    if (m -> locked) { // si esta ocupado, no esperamos.
        pthread_mutex_unlock(&m -> lock);
        return -1;
    }

    m -> locked = 1;
    pthread_mutex_unlock(&m -> lock);
    return 0;
}
