#include "../config.h"

#include "debug.h"
#include "thread.h"
#include "ntlv.h"
#include "list.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "runtime_config.h"
#include "../main.h"

extern struct RazorbackContext * contextList;

static struct List *sg_threadList;

#ifdef _MSC_VER
static volatile int initialized = 0;

static void initThreading_win32(void);
#else //_MSC_VER

#include <pthread.h>
#include <signal.h>

static pthread_attr_t g_attr;
static pthread_once_t g_once_control = PTHREAD_ONCE_INIT;

static void initThreading_pthreads(void);


static void handler(int sig);
#endif //_MSC_VER
static void Thread_Lock(void *);
static void Thread_Unlock(void *);

static void
initThreading (void) {
	sg_threadList = List_Create(LIST_MODE_GENERIC, 
			Thread_Cmp,
			Thread_KeyCmp,
			NULL, NULL,
			Thread_Lock,
			Thread_Unlock);
	initThreading_pthreads();
}

static void *
Thread_MainWrapper (void *arg) {
	struct Thread *l_pThread = (struct Thread *) arg;
	List_Lock(sg_threadList);
	Thread_Lock (l_pThread);

	l_pThread->bRunning = true;

	Thread_Unlock (l_pThread);
	List_Unlock(sg_threadList);

	l_pThread->mainFunction (l_pThread);

	List_Lock(sg_threadList);
	Thread_Lock (l_pThread);

	l_pThread->bRunning = false;

	Thread_Unlock (l_pThread);
	List_Unlock(sg_threadList);

	// Don't destroy the thread structure, it may still be referenced
	// We should probably ref count this struct
	Thread_Destroy(l_pThread);
	//
#ifdef _MSC_VER
	return 0;
#else //_MSC_VER
	return NULL; // Implicit pthread_exit()
#endif //_MSC_VER
}

SO_PUBLIC struct Thread *
Thread_Launch (void (*p_fpFunction) (struct Thread *), void *p_pUserData, 
		char *p_sName, struct RazorbackContext *p_pContext) {
	struct Thread *l_pThread;
	
	ASSERT (p_fpFunction != NULL);

#ifdef _MSC_VER
	if (initialized == 0)
		initThreading();

#else //_MSC_VER
	pthread_once (&g_once_control, initThreading);
#endif //_MSC_VER

	// Racy
	if (sg_threadList->length == Config_getThreadLimit ())
		return NULL;

	// allocate memory for thread structure
	l_pThread = (struct Thread *)calloc (1, sizeof (struct Thread));
	if (l_pThread == NULL) {
		rzb_log (LOG_ERR,
				 "%s: Failed to launch thread in Thread_Launch due to out of memory for Thread", __func__);
		return NULL;
	}

	// initialize running indicator
	l_pThread->bRunning = false;
	l_pThread->pContext = NULL;
	l_pThread->pUserData = p_pUserData;
	l_pThread->sName = p_sName;
	l_pThread->bShutdown = false;
	// Init ref count, once for the list, once for the caller.
	l_pThread->refs =2;

	l_pThread->mainFunction = p_fpFunction;
	// initialize running mutex
	if ((l_pThread->mMutex = Mutex_Create(MUTEX_MODE_NORMAL)) == NULL) {
		free(l_pThread);
		return NULL;
	}

#ifdef _MSC_VER
	l_pThread->hThread = CreateThread(NULL, 0, Thread_MainWrapper, l_pThread, 0, &l_pThread->iThread);
#else //_MSC_VER
	// start thread, check for error
	if (pthread_create
		(&l_pThread->iThread, &g_attr, Thread_MainWrapper, l_pThread) != 0) {
		free (l_pThread);
		rzb_log (LOG_ERR,
				 "%s: Failed to launch thread in Thread_Launch due to pthread_create error (%i)", __func__,
				 errno);
		return NULL;
	}
#endif //_MSC_VER
	List_Push(sg_threadList, l_pThread);

	// done
	return l_pThread;
}

SO_PUBLIC void
Thread_Destroy (struct Thread *p_pThread) {
	List_Lock(sg_threadList);
	Thread_Lock(p_pThread);
//	rzb_log(LOG_ERR, "%s: Thread ref count: %d", __func__, p_pThread->refs);
	// Referece count should not drop below 1 as the list holds a ref.
	ASSERT(p_pThread->refs >= 1);
	if (p_pThread->refs > 1) {
		p_pThread->refs--;
		Thread_Unlock(p_pThread);
		List_Unlock(sg_threadList);
		return;
	}

	List_Remove(sg_threadList, p_pThread);
	// destroy running mutex
	Thread_Unlock(p_pThread);
	List_Unlock(sg_threadList);

	Mutex_Destroy (p_pThread->mMutex);

	free (p_pThread);
}

SO_PUBLIC bool
Thread_IsRunning (struct Thread *p_pThread) {
	bool l_bRunning;
	ASSERT (p_pThread != NULL);

	Thread_Lock (p_pThread);

	l_bRunning = p_pThread->bRunning;

	Thread_Unlock (p_pThread);

	return l_bRunning;
}

SO_PUBLIC bool
Thread_IsStopped (struct Thread *p_pThread) {
	bool l_bShutdown;

	ASSERT (p_pThread != NULL);

	Thread_Lock (p_pThread);

	l_bShutdown = p_pThread->bShutdown;

	Thread_Unlock (p_pThread);

	return l_bShutdown;
}


SO_PUBLIC void
Thread_Stop (struct Thread *p_pThread) {
	ASSERT (p_pThread != NULL);

	Thread_Lock (p_pThread);

	p_pThread->bShutdown = true;

	Thread_Unlock (p_pThread);

}
SO_PUBLIC void
Thread_Interrupt(struct Thread *thread) {
	ASSERT (thread != NULL);
	Thread_Stop(thread);
#ifdef _MSC_VER
	//UNIMPLEMENTED();
#else //_MSC_VER
	pthread_kill(thread->iThread, SIGUSR1);
#endif //_MSC_VER
}

SO_PUBLIC void 
Thread_Join(struct Thread *thread) {
#ifdef _MSC_VER
	WaitForSingleObject(thread->hThread,INFINITE);
#else //_MSC_VER
	void *ret;
	int res =0;
	if ((res = pthread_join(thread->iThread, &ret)) != 0)
		rzb_log(LOG_ERR, "%s: Failed to join: %i", __func__, res);
#endif //_MSC_VER
}

SO_PUBLIC void
Thread_StopAndJoin(struct Thread *thread) {
   ASSERT (thread != NULL);
   Thread_Stop(thread);
   Thread_Join(thread);
}

SO_PUBLIC void
Thread_InterruptAndJoin(struct Thread *thread) {
	ASSERT (thread != NULL);
	Thread_Stop(thread);
	Thread_Interrupt(thread);
	Thread_Join(thread);
}


SO_PUBLIC struct RazorbackContext * 
Thread_GetContext(struct Thread *p_pThread) {
	struct RazorbackContext *l_pOldContext;

	Thread_Lock (p_pThread);

	//l_pOldContext = nuggetContext;
	l_pOldContext = p_pThread->pContext;
	Thread_Unlock (p_pThread);

	return l_pOldContext;
}

SO_PUBLIC struct RazorbackContext * 
Thread_GetCurrentContext(void) {
	return shiftContext();
	/*return nuggetContext;
	struct Thread *thread;
	struct RazorbackContext *cont;
	thread = Thread_GetCurrent();
	if (thread == NULL) {
		rzb_log(LOG_ERR, "%s: Failed to get current thread", __func__);
		return NULL;
	}
	cont = Thread_GetContext(thread);
	Thread_Destroy(thread);
	return cont;*/
}

SO_PUBLIC struct RazorbackContext * 
Thread_ChangeContext(struct Thread *p_pThread, struct RazorbackContext *p_pContext) {
	struct RazorbackContext *l_pOldContext;

	Thread_Lock (p_pThread);

	l_pOldContext = p_pThread->pContext;
	p_pThread->pContext = p_pContext;

	Thread_Unlock (p_pThread);

	return l_pOldContext;
}

SO_PUBLIC rzb_thread_t
Thread_GetCurrentId(void) {
#ifdef _MSC_VER
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

SO_PUBLIC struct Thread *
Thread_GetCurrent(void) {
	struct Thread *l_pRet = NULL;
	rzb_thread_t l_tCurrent = Thread_GetCurrentId();
	l_pRet = (struct Thread *)List_Find(sg_threadList, &l_tCurrent);
	if (l_pRet == NULL)
		return NULL;

	Thread_Lock(l_pRet);
	l_pRet->refs++;
	Thread_Unlock(l_pRet);
	return l_pRet;
}

SO_PUBLIC void
Thread_Yield(void) {
#ifdef _MSC_VER
	SwitchToThread();
#else
	pthread_yield();
#endif
}

SO_PUBLIC uint32_t
Thread_getCount (void) {
	uint32_t num;
	List_Lock(sg_threadList);
	num = sg_threadList->length;
	List_Unlock(sg_threadList);
	return num;
}

SO_PUBLIC int 
Thread_KeyCmp(void *a, void *id) {
	struct Thread * cA = (struct Thread *)a;
	rzb_thread_t cId = *(rzb_thread_t *)id;

	if (cId == cA->iThread)
		return 0;
	else
		return -1;
	return -1;
}

SO_PUBLIC int 
Thread_Cmp(void *a, void *b) {
	struct Thread * cA = (struct Thread *)a;
	struct Thread * cB = (struct Thread *)b;
	if (a==b)
		return 0;
	if (cA->iThread == cB->iThread)
		return 0;
	else
		return -1;
	return -1;
}

static void 
Thread_Lock(void *a) {
	struct Thread *t = (struct Thread *)a;
	Mutex_Lock(t->mMutex);
}

static void 
Thread_Unlock(void *a) {
	struct Thread *t = (struct Thread *)a;
	Mutex_Unlock(t->mMutex);
}


#ifdef _MSC_VER

static void 
initThreading_win32(void) {
	initialized=1;
}

#else //_MSC_VER

static void
initThreading_pthreads (void) {
	struct sigaction act;
	sigset_t mask;

	pthread_attr_init (&g_attr);
	pthread_attr_setdetachstate (&g_attr, PTHREAD_CREATE_JOINABLE);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGUSR1, &act, NULL) < 0)
		rzb_log(LOG_ERR, "%s: Failed to install signal handler", __func__);
}

static void handler(int sig) {
	rzb_log(LOG_DEBUG, "Thread Got Signal");
	return;
}

#endif //_MSC_VER
