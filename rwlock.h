#include <pthread.h>
#include <semaphore.h>

typedef struct{
    pthread_mutex_t lock;
    pthread_cond_t readers;
    pthread_cond_t writers;

    int active_readers;
    int active_writers;
    int waiting_writers;

} rwlock_t;

// defining the lock acquisition and release functions
void rwlock_init(rwlock_t *lock);
void rwlock_acquire_readlock(rwlock_t *lock);
void rwlock_release_readlock(rwlock_t *lock);
void rwlock_acquire_writelock(rwlock_t *lock);
void rwlock_release_writelock(rwlock_t *lock);