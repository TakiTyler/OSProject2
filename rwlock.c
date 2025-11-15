#include "rwlock.h"

void rwlock_init(rwlock_t *lock){
    pthread_mutex_init(&lock->lock, NULL);

    pthread_cond_init(&lock->readers, NULL);
    pthread_cond_init(&lock->writers, NULL);

    lock->active_readers = 0;
    lock->active_writers = 0;
    lock->waiting_writers = 0;
}
void rwlock_acquire_readlock(rwlock_t *lock){
    pthread_mutex_lock(&lock->lock);

    // check writers before reading
    while(lock->active_writers > 0|| lock->waiting_writers > 0){
        pthread_cond_wait(&lock->readers, &lock->lock);
    }

    // after we've acquired the lock, increment active readers
    lock->active_readers++;

    pthread_mutex_unlock(&lock->lock);

    // log that the lock was acquired
}
void rwlock_release_readlock(rwlock_t *lock){
    pthread_mutex_lock(&lock->lock);

    lock->active_readers--;

    // signal to waiting writers if readers are done
    if(lock->active_readers == 0 && lock->waiting_writers > 0){
        pthread_cond_signal(&lock->writers);
    }

    pthread_mutex_unlock(&lock->lock);

    // log that the lock was released
}
void rwlock_acquire_writelock(rwlock_t *lock){
    pthread_mutex_lock(&lock->lock);

    // we increment waiting writers, just in case
    lock->waiting_writers++;

    // wait if any readers or writers are active
    while(lock->active_readers > 0 || lock->active_writers > 0){
        pthread_cond_wait(&lock->writers, &lock->lock);
    }

    // decrement when no longer waiting
    lock->waiting_writers--;
    lock->active_writers = 1;

    pthread_mutex_unlock(&lock->lock);

    // log that write lock was acquired
}
void rwlock_release_writelock(rwlock_t *lock){
    pthread_mutex_lock(&lock->lock);

    lock->active_writers = 0;

    // signal to writers first
    if(lock->waiting_writers > 0){
        pthread_cond_signal(&lock->writers);
    }
    else{
        pthread_cond_broadcast(&lock->readers);
    }

    pthread_mutex_unlock(&lock->lock);

    // log that write lock was released
}