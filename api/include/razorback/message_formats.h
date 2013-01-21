#ifndef RAZORBACK_MESSAGES_FORMATS_H
#define RAZORBACK_MESSAGES_FORMATS_H

#include <razorback/visibility.h>
#include <razorback/types.h>


struct MessageHeader 
{
    char *sName;
    char *sValue;
};

struct Message
{
    uint32_t type;             ///< Message type
    size_t length;           ///< Message length
    uint32_t version;          ///< Message version
    struct List *headers;
    void *message;
    uint8_t *serialized;
    bool (*serialize)(struct Message *, int);
    bool (*deserialize)(struct Message *, int);
    void (*destroy)(struct Message *);
};



/** Command and Control Messages
 * @{
 */


/** Error Message
 */
struct MessageError
{
    uint8_t *sMessage;          ///< Error Message Text
};

/** Hello Message
 * This message is a broadcast message.
 */
struct MessageHello
{
    uuid_t uuidNuggetType;      ///< Nugget Type
    uuid_t uuidApplicationType; ///< Type of nugget sending the hello.
    uint8_t locality;           ///< Nugget Locality
    uint8_t priority;           ///< Dispatcher priority
    uint32_t flags;             ///< Dispatcher flags
    struct List *addressList;   ///< Dispatcher address list.
    uint8_t protocol;           ///< Dispatcher transfer protocol.
    uint16_t port;              ///< Dispatcher transfer server port.
};

/** Registration Request Message
 * This message is a broadcase message.
 */
struct MessageRegistrationRequest
{
    uuid_t uuidNuggetType;      ///< Nugget Type
    uuid_t uuidApplicationType; ///< Application Type
    uint32_t iDataTypeCount;    ///< Number of supported data types.
    uuid_t *pDataTypeList;      ///< Supported data type list.
};

/** Configuration Update Message
 */
struct MessageConfigurationUpdate
{
    struct List *ntlvTypes;
    struct List *ntlvNames;
    struct List *dataTypes;
};

/** Configuration Update Success
 */
struct MessageConfigurationAck
{
    uuid_t uuidNuggetType;      ///< Nugget Type
    uuid_t uuidApplicationType; ///< Type of nugget sending the config ack.
};

/** Terminate Message
 */
struct MessageTerminate
{
    uint8_t *sTerminateReason;  ///< String with termination reason in.
};
/// @}
//
// End of Command and Control Messages

/** Cache Control Messages
 * @{
 */

/** Glocal Cache Request Message
 */
struct MessageCacheReq
{
    uuid_t uuidRequestor;       ///< UUID of the nugget requesting the data.
    struct BlockId *pId;    ///< Data Block ID
};

/** Global Cache Response Message
 */
struct MessageCacheResp
{
    struct BlockId *pId;    ///< Data Block ID
    uint32_t iSfFlags;             ///< Data block code
    uint32_t iEntFlags;             ///< Data block code
};


/// @}
// End Cache Control Messages

/** Submission Messages
 * @{
 */

/** Block Submission Message
 */
struct MessageBlockSubmission
{
    uint32_t iReason;           ///< Submisson Reason
    struct Event *pEvent;
    uint8_t storedLocality;
};


/** Judgment Submission Message
 */
struct MessageJudgmentSubmission
{
    uint8_t iReason;                ///< Alert, Error, Done, Log
    struct Judgment *pJudgment;
};

/** Log Submission Message
 */
struct MessageLogSubmission
{
    uuid_t uuidNuggetId;            ///< who wrote it
    uint8_t iPriority;              ///< Meh, Dodgy, YF, YRF
    struct EventId *pEventId;       ///< The event id.
    uint8_t *sMessage;              ///< The message.
};

/** Inspection Submission Message
 */
struct MessageInspectionSubmission
{
    uint32_t iReason;           ///< Submisson Reason
	struct Block *pBlock;        ///< Datablock
    struct EventId *eventId;
    struct List *pEventMetadata;
    uint32_t localityCount;
    uint8_t *localityList;
};
/// @}
// End Submission Messages

/** Output Messages
 * @{
 */

/** Primary Alert Message
 */
struct MessageAlertPrimary
{
    struct Nugget *nugget;
    struct Block *block;        ///< The block
    struct Event *event;        ///< The event
    uint32_t gid;
    uint32_t sid;
    struct List *metadata;
    uint8_t priority;
    char *message;
    uint64_t seconds;
    uint64_t nanosecs;
    uint32_t SF_Flags;
    uint32_t Ent_Flags;
    uint32_t Old_SF_Flags;
    uint32_t Old_Ent_Flags;
};

/** Primary Alert Message
 */
struct MessageAlertChild
{
    struct Nugget *nugget;
    struct Block *block;        ///< The block
    struct Block *child;
    uint64_t eventCount;
    uint64_t parentCount;
    uint32_t SF_Flags;
    uint32_t Ent_Flags;
    uint32_t Old_SF_Flags;
    uint32_t Old_Ent_Flags;
};

/** Event Output
 */
struct MessageOutputEvent
{
    struct Nugget *nugget;
    struct Event *event;
};

/** Log Output
 */
struct MessageOutputLog
{
    struct Nugget *nugget;
    char *message;
    uint8_t priority;
    struct Event *event;
    uint64_t seconds;
    uint64_t nanosecs;
};

/** Inspection Output
 */
struct MessageOutputInspection
{
    struct Nugget *nugget;
    uint64_t seconds;
    uint64_t nanosecs;
    struct BlockId *blockId;
    uint8_t status;
    bool final;
};


#endif

