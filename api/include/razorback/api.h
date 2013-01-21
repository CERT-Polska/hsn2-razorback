/** @file api.h
 * Razorback API.
 */
#ifndef RAZORBACK_API_H
#define RAZORBACK_API_H
#include <razorback/visibility.h>
#include <razorback/types.h>
#include <razorback/queue.h>
#include <razorback/message_formats.h>
#include <razorback/lock.h>
#include <razorback/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DECL_INSPECTION_FUNC(a) uint8_t a (struct Block *block, struct EventId *eventId, struct List *eventMetadata, void *threadData)
#define DECL_NUGGET_INIT bool initNug(void)
#define DECL_NUGGET_THREAD_INIT(a) bool a (void ** threadData)
#define DECL_NUGGET_THREAD_CLEANUP(a) void a (void * threadData)
#define DECL_NUGGET_SHUTDOWN void shutdownNug(void)
#define DECL_ALERT_PRIMARY_FUNC(a) bool a (struct MessageAlertPrimary *message)
#define DECL_ALERT_CHILD_FUNC(a) bool a (struct MessageAlertChild *message)
#define DECL_OUTPUT_EVENT_FUNC(a) bool a (struct MessageOutputEvent *message)
#define DECL_OUTPUT_LOG_FUNC(a) bool a (struct MessageOutputLog *message)


/** Inspection Nugget Hooks
 */
struct RazorbackInspectionHooks
{
    uint8_t (*processBlock) (struct Block *, struct EventId *, struct List *, void *);    ///< FP to inspection handler
    bool (*processDeferredList) (struct DeferredList *);            ///< FP to pending items
    bool (*initThread) (void  **);
    void (*cleanupThread) (void *);
};

struct RazorbackOutputHooks
{
    struct Queue *queue; // reserved for in api use.
    const char *pattern;
    uint32_t messageType;
    bool (*handleAlertPrimary)(struct MessageAlertPrimary *log);
    bool (*handleAlertChild)(struct MessageAlertChild *log);
    bool (*handleEvent)(struct MessageOutputEvent *log);
    bool (*handleLog)(struct MessageOutputLog *log);
};

struct RazorbackCommandAndControlHooks
{
    bool (*processRegReqMessage) (struct Message *);
    bool (*processRegRespMessage) (struct Message *);
    bool (*processRegErrMessage) (struct Message *);
    bool (*processConfUpdateMessage) (struct Message *);
    bool (*processConfAckMessage) (struct Message *);
    bool (*processConfErrMessage) (struct Message *);
    bool (*processPauseMessage) (struct Message *);
    bool (*processPausedMessage) (struct Message *);
    bool (*processGoMessage) (struct Message *);
    bool (*processRunningMessage) (struct Message *);
    bool (*processTermMessage) (struct Message *);
    bool (*processByeMessage) (struct Message *);
    bool (*processHelloMessage) (struct Message *);
};



#define CONTEXT_FLAG_STAND_ALONE 0x00000001
#define CONTEXT_FLAG_DISPATCHER  0x00000002

/** API Context
 */
struct RazorbackContext
{
    uuid_t uuidNuggetId;
    uuid_t uuidNuggetType;
    uuid_t uuidApplicationType;
    char *sNuggetName;
    uint32_t iFlags;
    uint32_t iDataTypeCount;
    uuid_t *pDataTypeList;
    struct RazorbackInspectionHooks *pInspectionHooks;
    struct RazorbackCommandAndControlHooks *pCommandHooks;
    struct Semaphore *regSem;
    bool regOk;
    struct ThreadPool *pInspectionThreadPool;
	void *userData;
    struct Queue *judgmentQueue;
    struct List *outputThreads;
    uint8_t locality;
    uint32_t dispatcherFlags;
    uint8_t dispatcherPriority;
    uint16_t dispatcherPort;
    uint8_t dispatcherProtocol;
    struct List *dispatcherAddressList;
};

/** Initialize an API context.
 * @param The context to initilize
 * @return true on success false on failure.
 */
SO_PUBLIC extern bool Razorback_Init_Context (struct RazorbackContext *p_pContext);

/** Initialize an Inspection API context.
 * @param p_uuidNuggetId the nugget uuid
 * @param p_uuidApplicationType the application type.
 * @param p_iDataTypeCount the number of data types.
 * @param p_pDataTypeList the list of data types.
 * #param p_pInspectionHooks the inspection call backs.
 * @return true on success false on failure.
 */
SO_PUBLIC extern struct RazorbackContext * Razorback_Init_Inspection_Context (
        uuid_t p_uuidNuggetId, uuid_t p_uuidApplicationType,
        uint32_t p_iDataTypeCount, uuid_t *p_pDataTypeList,
        struct RazorbackInspectionHooks *p_pInspectionHooks, 
        uint32_t initialThreads, uint32_t maxThreads);

SO_PUBLIC extern struct RazorbackContext * Razorback_Init_Output_Context (
        uuid_t p_uuidNuggetId, uuid_t p_uuidApplicationType);

/** Initialize a Collection API context.
 * @param p_uuidNuggetId the nugget uuid
 * @param p_uuidApplicationType the application type.
 * @return true on success false on failure.
 */
SO_PUBLIC extern struct RazorbackContext * Razorback_Init_Collection_Context (
        uuid_t p_uuidNuggetId, uuid_t p_uuidApplicationType);

/** Lookup a Context by UUID.
 * @param the nugget ID uuid.
 * @return the context or NULL if there is no such context.
 */
SO_PUBLIC extern struct RazorbackContext * Razorback_LookupContext (uuid_t p_uuidNugget);

SO_PUBLIC extern void Razorback_Shutdown_Context (struct RazorbackContext *context);
SO_PUBLIC extern bool Razorback_Render_Verdict (struct Judgment *p_pJudgment);

SO_PUBLIC extern bool
Razorback_Output_Launch (struct RazorbackContext *p_pContext, struct RazorbackOutputHooks *hooks);

SO_PUBLIC extern int Razorback_Get_Message_Mode();
SO_PUBLIC extern char * Razorback_Get_Transfer_Password();
/* Make APIs standardized while keeping function naming convention */
#define RZB_Register_Collector          Razorback_Init_Collection_Context
#define RZB_DataBlock_Create            BlockPool_CreateItem
#define RZB_DataBlock_Add_Data          BlockPool_AddData
#define RZB_DataBlock_Set_Type          BlockPool_SetItemDataType
#define RZB_DataBlock_Finalize          BlockPool_FinalizeItem
#define RZB_DataBlock_Metadata_Filename(block, filename) Metadata_Add_Filename(block->pEvent->pMetaDataList, filename)
#define RZB_DataBlock_Metadata_Hostname(block, hostname) Metadata_Add_Hostname(block->pEvent->pMetaDataList, hostname)
#define RZB_DataBlock_Metadata_URI(block, uri) Metadata_Add_URI(block->pEvent->pMetaDataList, uri)
#define RZB_DataBlock_Metadata_HttpRequest(block, request) Metadata_Add_HttpRequest(block->pEvent->pMetaDataList, request)
#define RZB_DataBlock_Metadata_HttpResponse(block, response) Metadata_Add_HttpResponse(block->pEvent->pMetaDataList, response)
#define RZB_DataBlock_Metadata_HttpResponse(block, response) Metadata_Add_HttpResponse(block->pEvent->pMetaDataList, response)
#define RZB_DataBlock_Metadata_IPv4_Source(block, address) Metadata_Add_IPv4_Source(block->pEvent->pMetaDataList, address)
#define RZB_DataBlock_Metadata_IPv4_Destination(block, address) Metadata_Add_IPv4_Destination(block->pEvent->pMetaDataList, address)
#define RZB_DataBlock_Metadata_IPv6_Source(block, address) Metadata_Add_IPv6_Source(block->pEvent->pMetaDataList, address)
#define RZB_DataBlock_Metadata_IPv6_Destination(block, address) Metadata_Add_IPv6_Destination(block->pEvent->pMetaDataList, address)

#define RZB_DataBlock_Metadata_Port_Source(block, port) Metadata_Add_Port_Source(block->pEvent->pMetaDataList, port)
#define RZB_DataBlock_Metadata_Port_Destination(block, port) Metadata_Add_Port_Destination(block->pEvent->pMetaDataList, port)


#define RZB_DataBlock_Submit            Submission_Submit
#define RZB_Log                         rzb_log

#ifdef __cplusplus
}
#endif
#endif //RAZORBACK_API_H
