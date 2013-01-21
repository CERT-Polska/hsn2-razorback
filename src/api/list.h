#ifndef RAZORBACK_LIST_H
#define RAZORBACK_LIST_H
#include <stdint.h>
#include <stdlib.h>
#ifdef _MSC_VER
#else //_MSC_VER
#include <stdbool.h>
#endif //_MSC_VER
#include "visibility.h"
#include "lock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIST_MODE_GENERIC 0
#define LIST_MODE_STACK 1
#define LIST_MODE_QUEUE 2

#define LIST_EACH_OK 0
#define LIST_EACH_ERROR 1
#define LIST_EACH_REMOVE 2

struct ListNode
{
    struct ListNode *next;
    struct ListNode *prev;
    void *item;
};

struct List 
{
    struct ListNode *head;
    struct ListNode *tail;
    size_t length;
    int mode;
    int (*cmp)(void *, void *);
    int (*keyCmp)(void *, void *);
    void (*destroy)(void *);
    void *(*clone)(void *);
    void (*nodeLock)(void *);
    void (*nodeUnlock)(void *);
    struct Mutex *lock;
};

SO_PUBLIC extern struct List * List_Create(int mode, 
        int (*cmp)(void *, void *), 
        int (*keyCmp)(void *, void *), 
        void (*destroy)(void *), 
        void *(*clone)(void *),
        void (*nodeLock)(void *),
        void (*nodeUnlock)(void *));

SO_PUBLIC extern bool List_Push(struct List *list, void *item);
SO_PUBLIC extern void * List_Pop(struct List *list);
SO_PUBLIC extern void List_Remove(struct List *list, void *item);
SO_PUBLIC extern void * List_Find(struct List *list, void *id);
SO_PUBLIC extern bool List_ForEach(struct List *list, int (*op)(void *, void *), void *);
SO_PUBLIC extern size_t List_Length(struct List *list);
SO_PUBLIC extern void List_Clear(struct List *list);
SO_PUBLIC extern void List_Lock(struct List *list);
SO_PUBLIC extern void List_Unlock(struct List *list);
SO_PUBLIC extern void List_Destroy(struct List *list);
SO_PUBLIC extern struct List* List_Clone (struct List *source);
#ifdef __cplusplus
}
#endif
#endif //RAZORBACK_LIST_H
