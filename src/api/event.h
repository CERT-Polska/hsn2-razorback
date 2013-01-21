#ifndef RAZORBACK_EVENT_H
#define RAZORBACK_EVENT_H

#include "visibility.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

SO_PUBLIC extern struct EventId * EventId_Create (void);
SO_PUBLIC extern struct EventId * EventId_Clone (struct EventId *event);
SO_PUBLIC extern void EventId_Destroy (struct EventId *event);

SO_PUBLIC extern struct Event * Event_Create (void);

SO_PUBLIC extern void Event_Destroy (struct Event *event);

SO_PUBLIC extern uint32_t Event_BinaryLength (struct Event *event);
#ifdef __cplusplus
}
#endif
#endif
