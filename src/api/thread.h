/** @file thread.h
 * Threading API.
 */

#ifndef	RAZORBACK_THREAD_H
#define	RAZORBACK_THREAD_H

#include "visibility.h"
#include "types.h"
#include "lock.h"
#ifdef _MSC_VER
typedef DWORD rzb_thread_t;
#else //_MSC_VER
#include <pthread.h>
typedef pthread_t rzb_thread_t;
#endif //_MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

/** Thread
 * Purpose:	hold the information about a thread
 */
struct Thread
{
#ifdef _MSC_VER
	HANDLE hThread;
#endif //_MSC_VER
    rzb_thread_t iThread;          ///< Pthread Thread info.
    struct Mutex * mMutex;  ///< mutex protecting this struct
    bool bRunning;              ///< true if executing, false if not:  must be managed explicitly by thread function
    void *pUserData;		///< Additional info for the thread
    char *sName;            ///< The thread name
    struct RazorbackContext *pContext; ///< The Thread Context
    void (*mainFunction) (struct Thread *); ///< Thread Main Function
    bool bShutdown;         ///< Shutdown Flag
    int refs;
};

/** Create a new thread
 * @param *p_fpFunction The function the thread will execute
 * @param *p_pUserData The thread user data
 * @return Null on error a new Thread on success.
 */
SO_PUBLIC extern struct Thread *Thread_Launch (void (*p_fpFunction) (struct Thread *),
                                     void *p_pUserData, char *p_sName,
                                     struct RazorbackContext *p_pContext);

/** Change the registered context of a running thread.
 * @param p_pThread the thread to change
 * @param p_pContext the new context
 * @return The old context
 */
SO_PUBLIC extern struct RazorbackContext * Thread_ChangeContext(struct Thread *p_pThread,
                                    struct RazorbackContext *p_pContext);

/** Get the registered context of a running thread.
 * @param p_pThread the thread to change
 * @return The context
 */
SO_PUBLIC extern struct RazorbackContext * Thread_GetContext(struct Thread *p_pThread);
SO_PUBLIC extern struct RazorbackContext * Thread_GetCurrentContext(void);

/** Destroy a threads data
 * @param *p_pThread The thread to destroy
 */
SO_PUBLIC extern void Thread_Destroy (struct Thread *p_pThread);

/** Checks whether a thread is running or not
 * @param *p_pThread The thread the test
 * @return true if running, false if not
 */
SO_PUBLIC extern bool Thread_IsRunning (struct Thread *p_pThread);

SO_PUBLIC extern bool Thread_IsStopped (struct Thread *p_pThread);
SO_PUBLIC extern void Thread_Join(struct Thread *thread);
SO_PUBLIC extern void Thread_Interrupt(struct Thread *thread);
SO_PUBLIC extern void Thread_Stop (struct Thread *p_pThread);
SO_PUBLIC extern void Thread_StopAndJoin (struct Thread *p_pThread);
SO_PUBLIC extern void Thread_InterruptAndJoin (struct Thread *p_pThread);
SO_PUBLIC extern void Thread_Yield(void);

/** Get the number of currently running threads.
 * @return the number of currently running threads.
 */
SO_PUBLIC extern uint32_t Thread_getCount (void);

/** Get the current thread.
 */
SO_PUBLIC extern struct Thread *Thread_GetCurrent(void);
SO_PUBLIC extern rzb_thread_t Thread_GetCurrentId(void);

SO_PUBLIC extern int Thread_KeyCmp(void *a, void *id);
SO_PUBLIC extern int Thread_Cmp(void *a, void *b);

#ifdef __cplusplus
}
#endif
#endif // RAZORBACK_THREAD_H
