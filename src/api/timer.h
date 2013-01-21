#ifndef RAZORBACK_TIMER_H
#define RAZORBACK_TIMER_H

#include "visibility.h"
#include "types.h"

#ifdef _MSC_VER
#else //_MSC_VER
#include <signal.h>
#include <time.h>
#endif //_MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

struct Timer
{
#ifdef _MSC_VER
	HANDLE timerQueue;
	HANDLE timer;
#else
   timer_t timer;
   struct itimerspec spec;
   struct sigevent *props;
#endif
   uint32_t interval;
   void *userData;
   void (*function)(void *);
};

SO_PUBLIC extern struct Timer * Timer_Create(uint32_t interval, void (*handler)(void *), void *userData);
SO_PUBLIC extern void Timer_Destroy(struct Timer *);
#ifdef __cplusplus
}
#endif
#endif //RAZORBACK_TIMER_H
