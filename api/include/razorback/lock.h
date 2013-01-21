#ifndef RAZORBACK_LOCK_H
#define RAZORBACK_LOCK_H

#include <razorback/visibility.h>
#include <razorback/types.h>

#ifdef _MSC_VER
#else //_MSC_VER
#include <pthread.h>
#include <semaphore.h>
#endif //_MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

#define MUTEX_MODE_NORMAL 0
#define MUTEX_MODE_RECURSIVE 1

struct Mutex
{
#ifdef _MSC_VER
	HANDLE recursiveLock;
	CRITICAL_SECTION cs;
#else
    pthread_mutex_t lock;
    pthread_mutexattr_t attrs;
#endif
	int mode;
};

struct Semaphore
{
#ifdef _MSC_VER
	HANDLE sem;
#else
    sem_t sem;
#endif
};

SO_PUBLIC extern struct Mutex * Mutex_Create(int mode);
SO_PUBLIC extern bool Mutex_Lock(struct Mutex *);
SO_PUBLIC extern bool Mutex_Unlock(struct Mutex *);
SO_PUBLIC extern void Mutex_Destroy(struct Mutex *);

SO_PUBLIC extern struct Semaphore * Semaphore_Create(bool shared, unsigned int value);
SO_PUBLIC extern bool Semaphore_Post(struct Semaphore *);
SO_PUBLIC extern bool Semaphore_TimedWait(struct Semaphore *);
SO_PUBLIC extern bool Semaphore_Wait(struct Semaphore *);
SO_PUBLIC extern void Semaphore_Destroy(struct Semaphore *);
#ifdef __cplusplus
}
#endif
#endif //RAZORBACK_LOCK_H
