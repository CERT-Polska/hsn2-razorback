/** @file thread.h
 * Threading API.
 */

#ifndef	RAZORBACK_THREAD_POOL_H
#define	RAZORBACK_THREAD_POOL_H

#include <razorback/visibility.h>
#include <razorback/types.h>
#include <razorback/api.h>
#include <razorback/list.h>

#ifdef __cplusplus
extern "C" {
#endif
struct ThreadPool;
/**
 * Thread pool item
 */
struct ThreadPoolItem
{
    struct Thread *thread;
    int id;
    struct ThreadPool *pool;
};

/**
 * Thread pool container.
 */
struct ThreadPool
{
    size_t limit;                              ///< Maximum number of threads
    int nextId;                             ///< Id of the next thread
    struct RazorbackContext *context;       ///< Context to spawn threads in
    void (*mainFunction) (struct Thread *); ///< Main function for spawned threads
    const char *namePattern;                      ///< Name pattern for threads
    struct List *list;
};

SO_PUBLIC extern struct ThreadPool *
ThreadPool_Create(int initialThreads, int maxThreads, struct RazorbackContext *context, const char* namePattern, void (*mainFunction) (struct Thread *));

SO_PUBLIC extern bool
ThreadPool_LaunchWorker(struct ThreadPool *pool);
SO_PUBLIC extern bool
ThreadPool_LaunchWorkers(struct ThreadPool *pool, int count);

SO_PUBLIC extern bool
ThreadPool_KillWorker(struct ThreadPool *pool, int id);
SO_PUBLIC extern bool
ThreadPool_KillWorkers(struct ThreadPool *pool);
#ifdef __cplusplus
}
#endif
#endif // RAZORBACK_THREAD_POOL_H

