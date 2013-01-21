#include "../config.h"
#include <razorback/debug.h>
#include <razorback/block_pool.h>
#include <razorback/list.h>
#include <razorback/log.h>
#include <razorback/hash.h>
#include <razorback/uuids.h>
#include <razorback/ntlv.h>
#include <razorback/metadata.h>
#include <razorback/block_id.h>
#include <razorback/event.h>
#include <sys/stat.h>

#include "block_pool_private.h"
#include "fantasia.h"
#ifdef _MSC_VER
#else //_MSC_VER
#include <sys/mman.h>
#endif //_MSC_VER
#include <string.h>

static int BlockPool_KeyCmp(void *a, void *id);
static int BlockPool_Cmp(void *a, void *b);
static void BlockPool_Delete(void *a);
static void BlockPool_Item_Lock(void *a);
static void BlockPool_Item_Unlock(void *a);


static struct List * sg_bpList;
static bool sg_bInitDone=false;

bool
BlockPool_Init(struct RazorbackContext *p_pContext)
{
    if (sg_bInitDone) 
        return true;
    sg_bpList = List_Create(LIST_MODE_GENERIC,
            BlockPool_Cmp, // Cmp
            BlockPool_KeyCmp, // KeyCmp
            BlockPool_Delete, // destroy
            NULL, // clone
            BlockPool_Item_Lock,
            BlockPool_Item_Unlock);

    sg_bInitDone = true;
    return true;
}

SO_PUBLIC struct BlockPoolItem *
BlockPool_CreateItem (struct RazorbackContext *p_pContext)
{
    struct BlockPoolItem *l_pItem;

    ASSERT(p_pContext != NULL);
    if (p_pContext == NULL)
        return NULL;


    if ((l_pItem = calloc(1, sizeof(struct BlockPoolItem))) == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new item", __func__);
        return NULL;
    }

    if ((l_pItem->pEvent = Event_Create()) == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new event", __func__);
        free(l_pItem);
        return NULL;
    }

    if ((l_pItem->mutex = Mutex_Create(MUTEX_MODE_NORMAL)) == NULL)
    {
        Event_Destroy(l_pItem->pEvent);
        free(l_pItem);
        return NULL;
    }

    l_pItem->iStatus = BLOCK_POOL_STATUS_COLLECTING;

    uuid_copy(l_pItem->pEvent->pId->uuidNuggetId, p_pContext->uuidNuggetId);
    uuid_copy(l_pItem->pEvent->uuidApplicationType, p_pContext->uuidApplicationType);
    BlockPool_Init(p_pContext);

    List_Push(sg_bpList, l_pItem);
    return l_pItem;
}

SO_PUBLIC bool 
BlockPool_SetItemDataType(struct BlockPoolItem *p_pItem, char * p_sName)
{
    uuid_t l_pUuid;
    if (!UUID_Get_UUID(p_sName, UUID_TYPE_DATA_TYPE, l_pUuid))
    {
        rzb_log(LOG_ERR, "%s: Invalid data type name", __func__);
        return false;
    }
    uuid_copy(p_pItem->pEvent->pBlock->pId->uuidDataType, l_pUuid); 
    return true;
}

SO_PUBLIC bool
BlockPool_AddData_FromFile(struct BlockPoolItem *item, char *fileName, bool tempFile)
{
	struct BlockPoolData *dataBlock;
    struct stat sb;

    ASSERT (item != NULL);
    ASSERT (fileName != NULL);


    Mutex_Lock(item->mutex);
    ASSERT (item->iStatus == BLOCK_POOL_STATUS_COLLECTING);
    if (item->iStatus != BLOCK_POOL_STATUS_COLLECTING)
    {
        rzb_log(LOG_ERR, "%s: failed: item not collecting", __func__);
        Mutex_Unlock(item->mutex);
        return false;
    }
    if ((dataBlock = calloc(1, sizeof(struct BlockPoolData))) == NULL) 
    {
        rzb_log(LOG_ERR, "%s: failed to allocate data time", __func__);
        Mutex_Unlock(item->mutex);
        return false;
    }
    dataBlock->data.fileName = strdup(fileName);
    dataBlock->data.tempFile = tempFile;
    if ((dataBlock->data.file = fopen(fileName, "r")) == NULL)
    {
		rzb_perror("WTF: %s");
        rzb_log(LOG_ERR, "%s: failed to open file: %s", __func__, fileName);
		free(dataBlock->data.fileName);
		free(dataBlock);
        Mutex_Unlock(item->mutex);
        return false;
    }
    if (fstat (fileno (dataBlock->data.file), &sb) == -1)
    {
        rzb_log(LOG_ERR, "%s: failed to stat file: %s", __func__, fileName);
		free(dataBlock->data.fileName);
		free(dataBlock);
        Mutex_Unlock(item->mutex);
        return false;
    }

    item->pEvent->pBlock->pId->iLength += sb.st_size;
    dataBlock->iLength = sb.st_size;
    dataBlock->iFlags = BLOCK_POOL_DATA_FLAG_FILE;
    Hash_Update_File(item->pEvent->pBlock->pId->pHash, dataBlock->data.file);

    if (item->pDataHead == NULL)
    {
        item->pDataHead = dataBlock;
        item->pDataTail = dataBlock;
    }
    else
    {
        item->pDataTail->pNext = dataBlock;
        item->pDataTail = dataBlock;
    }

    Mutex_Unlock(item->mutex);

    return true; 
}

SO_PUBLIC bool 
BlockPool_AddData (struct BlockPoolItem *p_pItem, uint8_t * p_pData,
                               uint32_t p_iLength, int p_iFlags)
{
	struct BlockPoolData *l_pData;

    ASSERT (p_pItem != NULL);
    ASSERT (p_pData != NULL);
    ASSERT (p_iLength > 0);
    ASSERT (p_pItem->iStatus == BLOCK_POOL_STATUS_COLLECTING);

    Mutex_Lock(p_pItem->mutex);
    if (p_pItem->iStatus != BLOCK_POOL_STATUS_COLLECTING)
    {
        rzb_log(LOG_ERR, "%s: failed: item not collecting", __func__);
        Mutex_Unlock(p_pItem->mutex);
        return false;
    }
    if ((l_pData = calloc(1, sizeof(struct BlockPoolData))) == NULL) 
    {
        rzb_log(LOG_ERR, "%s: failed to allocate data time", __func__);
        Mutex_Unlock(p_pItem->mutex);
        return false;
    }
    p_pItem->pEvent->pBlock->pId->iLength += p_iLength;
    l_pData->iLength = p_iLength;
    l_pData->iFlags = p_iFlags;
    l_pData->data.pointer = p_pData;
    Hash_Update(p_pItem->pEvent->pBlock->pId->pHash, p_pData, p_iLength);

    if (p_pItem->pDataHead == NULL)
    {
        p_pItem->pDataHead = l_pData;
        p_pItem->pDataTail = l_pData;
    }
    else
    {
        p_pItem->pDataTail->pNext = l_pData;
        p_pItem->pDataTail = l_pData;
    }

    Mutex_Unlock(p_pItem->mutex);
    return true;
}

SO_PUBLIC bool 
BlockPool_FinalizeItem (struct BlockPoolItem *p_pItem)
{
    ASSERT (p_pItem->iStatus == BLOCK_POOL_STATUS_COLLECTING);
    //const char *l_sMagicFull;

    if (p_pItem->iStatus != BLOCK_POOL_STATUS_COLLECTING)
    {
        rzb_log(LOG_ERR, "%s: failed: item not collecting", __func__);
        return false;
    }
    if (!Hash_Finalize(p_pItem->pEvent->pBlock->pId->pHash))
    {
        rzb_log(LOG_ERR, "%s: Failed to finalize hash", __func__);
        return false;
    }

    if (uuid_is_null(p_pItem->pEvent->pBlock->pId->uuidDataType) == 1 && p_pItem->pDataHead != NULL)
    {
        Magic_process(p_pItem);
    }

    p_pItem->iStatus = BLOCK_POOL_STATUS_FINALIZED;
    return true;
}
void
BlockPool_DestroyItemDataList(struct BlockPoolItem *p_pItem) 
{
    struct BlockPoolData *l_pData;
    while (p_pItem->pDataHead != NULL)
    {
        l_pData = p_pItem->pDataHead;
        p_pItem->pDataHead = p_pItem->pDataHead->pNext;
        switch (l_pData->iFlags)
        {
        case BLOCK_POOL_DATA_FLAG_FILE:
            if (l_pData->data.file != NULL)
                fclose(l_pData->data.file);

            if (l_pData->data.tempFile && (l_pData->data.fileName != NULL))
                unlink(l_pData->data.fileName);

            if (l_pData->data.fileName != NULL)
                free(l_pData->data.fileName);

            break;
        case BLOCK_POOL_DATA_FLAG_MALLOCD:
            free(l_pData->data.pointer);
            break;
        case BLOCK_POOL_DATA_FLAG_MANAGED:
            break;
        default:
            rzb_log(LOG_ERR, "%s: Failed to free block data", __func__);
            break;
        }
        free(l_pData);
    }
}


static void
BlockPool_DestroyItemData(struct BlockPoolItem *p_pItem)
{
    if (p_pItem->pEvent != NULL)
        Event_Destroy(p_pItem->pEvent);

    BlockPool_DestroyItemDataList(p_pItem);
    Mutex_Destroy(p_pItem->mutex);
    free(p_pItem);
}

SO_PUBLIC bool 
BlockPool_DestroyItem (struct BlockPoolItem *p_pItem)
{
    ASSERT(p_pItem != NULL);
    if (p_pItem == NULL)
        return false;

    List_Remove(sg_bpList, p_pItem);

    //BlockPool_DestroyItemData(p_pItem);
    return true;
}

void
BlockPool_ForEachItem(int (*function) (struct BlockPoolItem *, void *), void *userData)
{
    List_ForEach(sg_bpList, (int (*)(void *, void *))function, userData);
}

void 
BlockPool_SetStatus(struct BlockPoolItem *p_pItem, uint32_t p_iStatus)
{
    // Set the status bits retaining the flags bits
    p_pItem->iStatus = (p_iStatus & BLOCK_POOL_STATUS_MASK ) |
        ( p_pItem->iStatus & BLOCK_POOL_FLAG_MASK);
}
uint32_t 
BlockPool_GetStatus(struct BlockPoolItem *p_pItem)
{
    // Set the status bits retaining the flags bits
    return (p_pItem->iStatus & BLOCK_POOL_STATUS_MASK );
}
void 
BlockPool_SetFlags(struct BlockPoolItem *p_pItem, uint32_t p_iFlags)
{
    p_pItem->iStatus = ( p_iFlags & BLOCK_POOL_FLAG_MASK ) |
        ( p_pItem->iStatus & BLOCK_POOL_STATUS_MASK);
}

static int BlockPool_KeyCmp(void *a, void *id)
{
    return -1;
}
static int BlockPool_Cmp(void *a, void *b)
{
    return -1;
}

static void BlockPool_Delete(void *a)
{
    BlockPool_DestroyItemData(a);
}
static void BlockPool_Item_Lock(void *a)
{
    Mutex_Lock(((struct BlockPoolItem *)a)->mutex);
}
static void BlockPool_Item_Unlock(void *a)
{
    Mutex_Unlock(((struct BlockPoolItem *)a)->mutex);
}

