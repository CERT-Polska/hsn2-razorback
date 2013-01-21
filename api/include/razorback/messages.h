/** @file messages.h
 * Razorback API messages.
 */
#ifndef RAZORBACK_MESSAGES_H
#define RAZORBACK_MESSAGES_H

#include <razorback/visibility.h>
#include <razorback/message_formats.h>
#include <razorback/types.h>
#include <razorback/api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MESSAGE_MODE_BIN 1
#define MESSAGE_MODE_JSON 2

/** Message Groups
 * @{
 */
#define MESSAGE_GROUP_C_AND_C   0x10000000  ///< Command And Control
#define MESSAGE_GROUP_CACHE     0x20000000  ///< Global Cache
#define MESSAGE_GROUP_SUBMIT    0x40000000  ///< Submission
#define MESSAGE_GROUP_OUTPUT    0x80000000  ///< Submission
/// @}
/** Command And Control Messages
 * @{
 */
#define MESSAGE_TYPE_HELLO          ( MESSAGE_GROUP_C_AND_C | 1 )   ///< Hello Message
#define MESSAGE_TYPE_REG_REQ        ( MESSAGE_GROUP_C_AND_C | 2 )   ///< Registration Request
#define MESSAGE_TYPE_REG_RESP       ( MESSAGE_GROUP_C_AND_C | 3 )   ///< Registration Response
#define MESSAGE_TYPE_REG_ERR        ( MESSAGE_GROUP_C_AND_C | 4 )   ///< Registration Error
#define MESSAGE_TYPE_CONFIG_UPDATE  ( MESSAGE_GROUP_C_AND_C | 5 )   ///< Configuration Update Notice
#define MESSAGE_TYPE_CONFIG_ACK     ( MESSAGE_GROUP_C_AND_C | 6 )   ///< Configuration Updated OK
#define MESSAGE_TYPE_CONFIG_ERR     ( MESSAGE_GROUP_C_AND_C | 7 )   ///< Configuration Update Failed
#if 0
#define MESSAGE_TYPE_STATS_REQ      ( MESSAGE_GROUP_C_AND_C | 8 )   ///< Statistics Request
#define MESSAGE_TYPE_STATS_RESP     ( MESSAGE_GROUP_C_AND_C | 9 )   ///< Statistics Response (With Data)
#define MESSAGE_TYPE_STATS_ERR      ( MESSAGE_GROUP_C_AND_C | 10 )  ///< Statistics Error
#endif
#define MESSAGE_TYPE_PAUSE          ( MESSAGE_GROUP_C_AND_C | 11 )  ///< Pause Processing
#define MESSAGE_TYPE_PAUSED         ( MESSAGE_GROUP_C_AND_C | 12 )  ///< Pause Confirmation
#define MESSAGE_TYPE_GO             ( MESSAGE_GROUP_C_AND_C | 13 )  ///< Unpause Processing
#define MESSAGE_TYPE_RUNNING        ( MESSAGE_GROUP_C_AND_C | 14 )  ///< Unpuase Confirmation
#define MESSAGE_TYPE_TERM           ( MESSAGE_GROUP_C_AND_C | 15 )  ///< Terminate Processing
#define MESSAGE_TYPE_BYE            ( MESSAGE_GROUP_C_AND_C | 16 )  ///< Terminating Processing
#define MESSAGE_TYPE_CLEAR          ( MESSAGE_GROUP_C_AND_C | 17 )  ///< Clear Local Cache
#define MESSAGE_TYPE_REREG          ( MESSAGE_GROUP_C_AND_C | 18 )  ///< Reregister
/// @}

/** Global Cache Messages
 * @{
 */
#define MESSAGE_TYPE_REQ            ( MESSAGE_GROUP_CACHE | 1 ) ///< Global Cache Request
#define MESSAGE_TYPE_RESP           ( MESSAGE_GROUP_CACHE | 2 ) ///< Global Cache Response
/// @}

/** Submission Messages
 * @{
 */
#define MESSAGE_TYPE_BLOCK          ( MESSAGE_GROUP_SUBMIT | 1 )    ///< Data Block Submission
#define MESSAGE_TYPE_JUDGMENT       ( MESSAGE_GROUP_SUBMIT | 2 )    ///< Judgment Submission
#define MESSAGE_TYPE_INSPECTION     ( MESSAGE_GROUP_SUBMIT | 3 )    ///< Inspection Submission
#define MESSAGE_TYPE_LOG            ( MESSAGE_GROUP_SUBMIT | 4 )    ///< Log Message
#define MESSAGE_TYPE_ALERT          ( MESSAGE_GROUP_SUBMIT | 5 )    ///< Alert Message
/// @}

/** Output Messages
 * @{
 */
#define MESSAGE_TYPE_ALERT_PRIMARY      ( MESSAGE_GROUP_OUTPUT | 1 )    ///< Block turns to bad
#define MESSAGE_TYPE_ALERT_CHILD        ( MESSAGE_GROUP_OUTPUT | 2 )    ///< Block turns to bad due to child
#define MESSAGE_TYPE_OUTPUT_EVENT       ( MESSAGE_GROUP_OUTPUT | 3 )    ///< Event Record
#define MESSAGE_TYPE_OUTPUT_LOG         ( MESSAGE_GROUP_OUTPUT | 4 )    ///< Log Record
#define MESSAGE_TYPE_OUTPUT_INSPECTION  ( MESSAGE_GROUP_OUTPUT | 5 )    ///< Inspection status
/// @}


/** Message Versions
 * @{
 */
#define MESSAGE_VERSION_1 1
/// @}

#define MSG_CNC_HEADER_SOURCE   "Source_Nugget"
#define MSG_CNC_HEADER_DEST     "Dest_Nugget"

#define DISP_HELLO_FLAG_LM (1 << 0)
#define DISP_HELLO_FLAG_LS (1 << 1)
#define DISP_HELLO_FLAG_DD (1 << 2)
#define DISP_HELLO_FLAG_LF (1 << 3)

struct MessageHandler
{
    uint32_t type;
    bool (*serialize)(struct Message *msg, int mode);
    bool (*deserialize)(struct Message *msg, int mode);
    void (*destroy)(struct Message *msg);
};


SO_PUBLIC extern struct List * Message_Header_List_Create(void);
SO_PUBLIC extern bool Message_Add_Header(struct List *headers, const char *p_sName, const char *p_sValue);
SO_PUBLIC extern bool Message_Register_Handler(struct MessageHandler *);

SO_PUBLIC extern bool Message_Get_Nuggets(struct Message *message, uuid_t source, uuid_t dest);

SO_PUBLIC extern bool Message_Deserialize_Empty(struct Message *message, int mode);
SO_PUBLIC extern bool Message_Serialize_Empty(struct Message *message, int mode);

SO_PUBLIC extern struct Message * Message_Create_Directed(uint32_t type, uint32_t version, size_t msgSize, const uuid_t source, const uuid_t dest);
SO_PUBLIC extern struct Message * Message_Create_Broadcast(uint32_t type, uint32_t version, size_t msgSize, const uuid_t source);
SO_PUBLIC extern void Message_Destroy(struct Message *message);


/** initializes message cache req
 * @param p_pMessage the message
 * @param p_uuidRequestor the requestor
 * @param p_pBlockId the blockid
 * @return true if ok, false otherwise
 */
SO_PUBLIC extern struct Message*  MessageCacheReq_Initialize (
                                        const uuid_t p_uuidRequestor,
                                        const struct BlockId *p_pBlockId);


/** initializes message cache resp
 * @param p_pMessage the message
 * @param p_pBlockId the blockid
 * @param p_iSfFlags  the code
 * @param p_iEntFlags the code
 * @return true if ok, false otherwise
 */
SO_PUBLIC extern struct Message* MessageCacheResp_Initialize (
                                         const struct BlockId *p_pBlockId,
                                         uint32_t p_iSfFlags, uint32_t p_iEntFlags);


/** initializes message block submission
 * @param p_pMessage the message
 * @param p_pEvent the event\
 * @param p_iReason the submission reason
 * @return true if ok, false otherwise
 */
SO_PUBLIC extern struct Message*  MessageBlockSubmission_Initialize (
                                               struct Event *p_pEvent,
                                               uint32_t p_iReason, uint8_t locality);
/** initializes message block submission
 * @param p_pMessage the message
 * @param p_pBlockIthe block
 * @param p_iReason the reason for the submission
 * @param p_iPriority the priority
 * @param p_uuidInspectorId the submitting nugget
 * @param p_uuidApplicationId the application type of the block
 * @return true if ok, false otherwise
 */
SO_PUBLIC extern struct Message* MessageJudgmentSubmission_Initialize (
                                                  uint8_t p_iReason,
                                                  struct Judgment* p_pJudgment);

/** initializes message inspection submission
 * @param p_pMessage the message
 * @param p_pEvent The event
 * @param p_iReason the reason
 * @return true if ok, false otherwise
 */
SO_PUBLIC extern struct Message* MessageInspectionSubmission_Initialize (
                                                    const struct Event
                                                    *p_pEvent,
                                                    uint32_t p_iReason,
                                                    uint32_t localityCount,
                                                    uint8_t *localities
                                                    );

/** initializes message alert
 * @param p_pMessage the message
 * @param p_pBlock the block
 * @param p_iDisposition the disposition
 * @param p_uuidInspectorId the submitting nugget
 * @return true if ok, false otherwise
 */
SO_PUBLIC extern struct Message* MessageAlert_Initialize (
                                     const struct Block *p_pBlock,
                                     uint32_t p_iDisposition,
                                     const uuid_t p_uuidInspectorId);


/** initializes a hello message
 * @return the message
 * @param context The context of this message.
 */
SO_PUBLIC extern struct Message* MessageHello_Initialize (struct RazorbackContext *context);


/** initializes a registration request message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidNuggetTye the nugget type
 * @param p_uuidApplicationType the app type
 * @param p_iDataTypeCount the number of data types
 * @param p_pDataTypeList the data types
 */
SO_PUBLIC extern struct Message* MessageRegistrationRequest_Initialize (
                                                   const uuid_t dispatcherId, 
                                                   const uuid_t
                                                   p_uuidSourceNugget,
                                                   const uuid_t
                                                   p_uuidNuggetType,
                                                   const uuid_t
                                                   p_uuidApplicationType,
                                                   uint32_t p_iDataTypeCount,
                                                   uuid_t * p_pDataTypeList);


/** initializes a registration response message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 */
SO_PUBLIC extern struct Message* MessageRegistrationResponse_Initialize (
                                                    const uuid_t
                                                    p_uuidSourceNugget,
                                                    const uuid_t
                                                    p_uuidDestNugget);

/** initializes a configuration update message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 * @param p_ntlvConfigurationList the config list
 */
SO_PUBLIC extern struct Message * MessageConfigurationUpdate_Initialize (
                                                   const uuid_t
                                                   p_uuidSourceNugget,
                                                   const uuid_t
                                                   p_uuidDestNugget);


/** initializes a configuration ack message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 * @param p_uuidNuggetType the type of nugget
 * @param p_uuidApplicationType the application type
 */
SO_PUBLIC extern struct Message* MessageConfigurationAck_Initialize (
                                                const uuid_t
                                                p_uuidSourceNugget,
                                                const uuid_t
                                                p_uuidDestNugget,
                                                const uuid_t p_uuidNuggetType,
                                                const uuid_t
                                                p_uuidApplicationType);


/** initializes a pause message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 */
SO_PUBLIC extern struct Message * MessagePause_Initialize (
                                     const uuid_t p_uuidSourceNugget,
                                     const uuid_t p_uuidDestNugget);


/** initializes a paused message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 */
SO_PUBLIC extern struct Message* MessagePaused_Initialize (
                                      const uuid_t p_uuidSourceNugget,
                                      const uuid_t p_uuidDestNugget);


/** initializes a go message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 */
SO_PUBLIC extern struct Message * MessageGo_Initialize (
                                  const uuid_t p_uuidSourceNugget,
                                  const uuid_t p_uuidDestNugget);



/** initializes a running message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 */
SO_PUBLIC extern struct Message * MessageRunning_Initialize (
                                       const uuid_t p_uuidSourceNugget,
                                       const uuid_t p_uuidDestNugget);


/** initializes a terminate message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 * @param p_sTerminateReason the reason
 */
SO_PUBLIC extern struct Message * MessageTerminate_Initialize (
                                         const uuid_t p_uuidSourceNugget,
                                         const uuid_t p_uuidDestNugget,
                                         const uint8_t * p_sTerminateReason);


/** initializes a bye message
 * @param p_pMessage the message
 * @param p_uuidSourceNugget the source
 * @param p_uuidDestNugget the dest
 */
SO_PUBLIC extern struct Message * MessageBye_Initialize (
                                   const uuid_t p_uuidSourceNugget);


SO_PUBLIC extern struct Message * MessageCacheClear_Initialize (
                                   const uuid_t p_uuidSourceNugget);


SO_PUBLIC extern struct Message * MessageError_Initialize (
                                    uint32_t p_iErrorCode,
                                    const char *p_sMessage,
                                    const uuid_t p_uuidSourceNugget,
                                    const uuid_t p_uuidDestNugget);


SO_PUBLIC extern struct Message *
MessageLog_Initialize (
                         const uuid_t p_uuidNuggetId,
                         uint8_t p_iPriority,
                         char *p_sMessage,
                         struct EventId *p_pEventId);

SO_PUBLIC extern struct Message *
MessageAlertPrimary_Initialize (
                                   struct Event *event,
                                   struct Block *block,
                                   struct List *metadata,
                                   struct Nugget *nugget,
                                   struct Judgment *judgment,
                                   uint32_t new_SF_Flags, uint32_t new_Ent_Flags,
                                   uint32_t old_SF_Flags, uint32_t old_Ent_Flags);

SO_PUBLIC extern struct Message *
MessageAlertChild_Initialize (
                                   struct Block *block,
                                   struct Block *child,
                                   struct Nugget *nugget,
                                   uint64_t parentCount, uint64_t eventCount,
                                   uint32_t new_SF_Flags, uint32_t new_Ent_Flags,
                                   uint32_t old_SF_Flags, uint32_t old_Ent_Flags);

SO_PUBLIC extern struct Message *
MessageOutputEvent_Initialize (
                                   struct Event *event,
                                   struct Nugget *nugget);

SO_PUBLIC extern struct Message *
MessageOutputLog_Initialize (
                                   struct MessageLogSubmission *log,
                                   struct Nugget *nugget);

SO_PUBLIC struct Message *
MessageOutputInspection_Initialize (
                                   struct Nugget *nugget,
                                   struct BlockId *blockId, uint8_t reason, bool final);

#ifdef __cplusplus
}
#endif
#endif //RAZORBACK_MESSAGES_H
