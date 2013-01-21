#include "../config.h"
#include "debug.h"
#include "uuids.h"
#include "types.h"
#include "../config_file.h"
#include "log.h"
#include "list.h"

#include "init.h"
#include <string.h>

#ifdef _MSC_VER
#include "bobins.h"
#endif

#define CORRELATION_UUID "2fd75fa5-778b-443e-b910-1e19044e81e1"
#define CORRELATION_DESC "Correlation Nugget"
#define INTEL_UUID "356112d8-f4f1-41dc-b3f7-cace5674c2ec"
#define INTEL_DESC "Intel Nugget"
#define DEFENSE_UUID "5e9c1296-ad6a-423f-9eca-9f817c72c444"
#define DEFENSE_DESC "Defense Update Nugget"
#define OUTPUT_UUID "a3d0d1f9-c049-474e-bf01-2128ea00a751"
#define OUTPUT_DESC "Output Nugget"
#define COLLECTION_UUID "c38b113a-27fd-417c-b9fa-f3aa0af5cb53"
#define COLLECTION_DESC "Data Collector Nugget"
#define INSPECTION_UUID "d95aee72-9186-4236-bf23-8ff77dac630b"
#define INSPECTION_DESC "Inspection Nugget"
#define DISPATCHER_UUID "1117de3c-6fe8-4291-84ae-36cdf2f91017"
#define DISPATCHER_DESC "Message Dispatcher Nugget"
#define MASTER_UUID "ca51afd1-41b8-4c6b-b221-9faef0d202a7"
#define MASTER_DESC "Master Nugget"


#define UUID_KEY_NAME 0
#define UUID_KEY_UUID 1
struct UUIDKey
{
    int type;
    uuid_t uuid;
    const char *name;
};
static struct List *sg_DataTypeList;
static struct List *sg_IntelTypeList;
static struct List *sg_NtlvTypeList;
static struct List *sg_NtlvNameList;
static struct List *sg_NuggetList;
static struct List *sg_NuggetTypeList;

SO_PUBLIC bool
UUID_Add_List_Entry (struct List *list, uuid_t p_uuid,
                   const char *p_sName, const char *p_sDescr)
{
    struct UUIDListNode *l_pListNode;
    size_t l_iLen;
    if ((l_pListNode = calloc (1, sizeof (struct UUIDListNode))) == NULL)
    {
        return false;
    }

    uuid_copy (l_pListNode->uuid, p_uuid);
    l_iLen = strlen (p_sName);
    if ((l_pListNode->sName = calloc (l_iLen + 1, sizeof (char))) == NULL)
    {
        free (l_pListNode);
        return false;
    }
    memcpy (l_pListNode->sName, p_sName, l_iLen + 1);
    l_iLen = strlen (p_sDescr);
    if ((l_pListNode->sDescription =
         calloc (l_iLen + 1, sizeof (char))) == NULL)
    {
        free (l_pListNode->sName);
        free (l_pListNode);
        return false;
    }
    memcpy (l_pListNode->sDescription, p_sDescr, l_iLen + 1);

    return List_Push(list, l_pListNode);
}

SO_PUBLIC bool
UUID_Add_List_Entry_By_String (struct List *list, char * p_uuid,
                   const char *p_sName, const char *p_sDescr)
{
    struct UUIDListNode *l_pListNode;
    size_t l_iLen;
    if ((l_pListNode = calloc (1, sizeof (struct UUIDListNode))) == NULL)
    {
        return false;
    }
    uuid_parse(p_uuid,l_pListNode->uuid);
    l_iLen = strlen (p_sName);
    if ((l_pListNode->sName = calloc (l_iLen + 1, sizeof (char))) == NULL)
    {
        free (l_pListNode);
        return false;
    }
    memcpy (l_pListNode->sName, p_sName, l_iLen + 1);
    l_iLen = strlen (p_sDescr);
    if ((l_pListNode->sDescription =
         calloc (l_iLen + 1, sizeof (char))) == NULL)
    {
        free (l_pListNode->sName);
        free (l_pListNode);
        return false;
    }
    memcpy (l_pListNode->sDescription, p_sDescr, l_iLen + 1);

    return List_Push(list, l_pListNode);
}

SO_PUBLIC struct List *
UUID_Get_List(int type)
{
    switch (type)
    {
    case UUID_TYPE_DATA_TYPE:
        return sg_DataTypeList;
    case UUID_TYPE_INTEL_TYPE:
        return sg_IntelTypeList;
    case UUID_TYPE_NTLV_TYPE:
        return sg_NtlvTypeList;
    case UUID_TYPE_NTLV_NAME:
        return sg_NtlvNameList;
    case UUID_TYPE_NUGGET:
        return sg_NuggetList;
    case UUID_TYPE_NUGGET_TYPE:
        return sg_NuggetTypeList;
    default:
        return NULL;
    }
}

static struct UUIDListNode *
UUID_getNodeByName (const char *p_sName, int p_iType)
{
    struct List *list = NULL;
    struct UUIDKey key;
    key.type=UUID_KEY_NAME;
    key.name=p_sName;
    list = UUID_Get_List(p_iType); 
    
    return (struct UUIDListNode *)List_Find(list, &key);
}


static struct UUIDListNode *
UUID_getNodeByUUID (uuid_t p_uuid, int p_iType)
{
    struct List *list = NULL;
    struct UUIDKey key;
    key.type=UUID_KEY_UUID;
    uuid_copy(key.uuid, p_uuid);
    list = UUID_Get_List(p_iType);
    return (struct UUIDListNode *)List_Find(list, &key);
}

static void 
init_NuggetTypes(void)
{
    uuid_t uuid;
    uuid_parse(DISPATCHER_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_DISPATCHER, DISPATCHER_DESC);
    uuid_parse(MASTER_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_MASTER, MASTER_DESC);
    uuid_parse(COLLECTION_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_COLLECTION, COLLECTION_DESC);
    uuid_parse(INSPECTION_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_INSPECTION, INSPECTION_DESC);
    uuid_parse(OUTPUT_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_OUTPUT, OUTPUT_DESC);
    uuid_parse(INTEL_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_INTEL, INTEL_DESC);
    uuid_parse(DEFENSE_UUID,uuid);
    UUID_Add_List_Entry(sg_NuggetTypeList, uuid, NUGGET_TYPE_DEFENSE, DEFENSE_DESC);
}

void initDataTypeUUIDs(void) {
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "00000000-0000-0000-0000-000000000000", "ANY_DATA", "Any Data" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "005d5464-7a44-4907-af57-4db08a61e13c", "PDF_FILE", "PDF Document" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "15961cf0-78b4-4024-ae48-29ad2c86fb4b", "ELF_FILE", "ELF Executable File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "16d72948-3d2b-52ed-9ae4-43ef19ce3e69", "OLE_FILE", "OLE Document" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "2858f242-1d4e-4f80-9744-b3fed26b9d21", "ISO9660_FILE", "Iso9660 CD Image" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "2b1b387e-4c8a-490f-9d9d-bea6dea90593", "GIF_FILE", "GIF File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "2b797454-d367-4841-8c9c-a713d012b546", "JAVASCRIPT", "JavaScript" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "2d76a190-afb2-11df-91ba-1b85bf3d7e00", "TAR_FILE", "TAR Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "36966d78-afb2-11df-bfc6-1b9c2f1ce37a", "RAR_FILE", "RAR Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "3ee61f14-afb2-11df-b122-339e7fae4010", "PAR_FILE", "PAR Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "4cc82ee2-afb2-11df-bf91-4f6797c414f4", "PAR2_FILE", "PAR2 Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "4e72c8ec-ff88-4371-a0f0-dfe2b4c733dc", "SHELL_CODE", "Suspected Shellcode" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "52b2d8a8-8ccc-45af-91d3-74183f5f5f63", "CPIO_FILE", "CPIO Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "52f738c2-db1c-42ca-b5ef-50a4ba3f7527", "ZIP_FILE", "ZIP Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "532cf708-e293-4519-8796-d583a12a8fe1", "JPEG_FILE", "JPEG 2000 File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "575daa2a-2ada-42bb-818e-1ff6c36fef41", "XZ_FILE", "Xz Compressed File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "7966e375-e657-49ca-86b9-da870dd10809", "PNG_FILE", "PNG File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "7ab45fff-7c73-412c-8b86-c07619c8fc7d", "FLASH_FILE", "Adobe Flash" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "7e2a3a7c-69f5-11e0-8186-737fbe930662", "WIN_HELP_FILE", "Windows Help File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "a304dfd7-340e-4b96-b9af-ce0cb775f148", "LZMA_FILE", "Lzma Compressed File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "a6f78a4c-46c5-486c-a00a-780a8cf25e6d", "BZ2_FILE", "Bzip2 Compressed File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "b4b02352-149a-4d3b-811a-1c707ba7dd70", "HTML_FILE", "HTML File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "ba8b6784-1412-494d-91bb-d2868ecbb196", "AR_FILE", "Ar Archive File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "ba9beb5f-0653-4b04-9552-3bfb634ca7fc", "PE_FILE", "PE Executable" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "d00b0f8b-2e7a-4808-81c2-a19e86b4b4fd", "GZIP_FILE", "Gzip Compressed File" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "d147f215-128e-4746-a1e2-b6c978bb1869", "SMTP_CAPTURE", "SMTP Mail Capture" );
    UUID_Add_List_Entry_By_String( sg_DataTypeList, "fff8d04d-90e7-4eaf-be33-31b2c7e4255d", "COMPRESSION_FILE", "Compression Compressed File" );
}

void initNtlvNameUUIDs (void) {
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "04d64a08-479e-4d64-939d-572897025a6b", "HTTP_REQUEST", "HTTP Request" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "0999d503-e293-46b1-8211-b262ff0f832c", "OSVDB", "OSVDB ID" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "0c865866-ae6b-4740-a80c-b0b0635c0930", "HTTP_RESPONSE", "HTTP Response" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "10bdfb54-2503-4157-a73e-564927566b00", "DEST", "Destination" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "70991a40-09d6-442d-824a-1c6e642ec552", "SOURCE", "Source" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "870e6920-c24c-4c66-8768-320893866d4c", "BID", "Bugtraq ID" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "9077005b-895d-47fa-abb7-19dd9d4aa6b1", "URI", "URI" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "ad515154-f6d4-4f68-9ec3-d4cf89b4c9d7", "PATH", "Path" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "ad515154-f6d4-4f68-9ec3-d4cf89b4c9d8", "REPORT", "Report" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "ad515154-f6d4-4f68-9ec3-d4cf89b4c9d9", "CVE", "CVE" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "b804c4c4-2801-49a3-a1eb-0a84318686ef", "FILENAME", "File Name" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "c79e3426-b00e-4a2c-ada6-b3f67354e169", "HOSTNAME", "Hostname" );
	UUID_Add_List_Entry_By_String( sg_NtlvNameList, "d52c4eca-71a9-4152-8cb6-692a71ef0b83", "MALWARENAME", "Name of detected malware" );
}

void initNtlvTypeUUIDs (void) {
	UUID_Add_List_Entry_By_String( sg_NtlvTypeList, "101e30d7-e6ee-4171-999a-c4d99325a93e", "IPv4_ADDR", "IPv4 Address" );
	UUID_Add_List_Entry_By_String( sg_NtlvTypeList, "3590b149-859a-4625-9d01-749346ddf103", "IPv6_ADDR", "IPv6 Address" );
	UUID_Add_List_Entry_By_String( sg_NtlvTypeList, "5375a0ec-b7ee-4985-b23d-642556b75543", "IP_PROTO", "IP Protocol" );
	UUID_Add_List_Entry_By_String( sg_NtlvTypeList, "b0abffef-01d6-487b-bff5-a9e62b17c5b6", "PORT", "Port" );
	UUID_Add_List_Entry_By_String( sg_NtlvTypeList, "c24ceea9-3906-47f3-97c4-78cb881b8de8", "STRING", "String" );
}

void
initUuids (void)
{
	uuid_t uuid;
    sg_DataTypeList = UUID_Create_List();
    initDataTypeUUIDs();
    rzb_log(LOG_DEBUG,"Done DATA TYPE UUID init.");
    sg_IntelTypeList = UUID_Create_List();
    sg_NtlvTypeList = UUID_Create_List();
    initNtlvTypeUUIDs();
    rzb_log(LOG_DEBUG,"Done UUID_TYPE_NTLV_TYPE init.");
    sg_NtlvNameList = UUID_Create_List();
    initNtlvNameUUIDs();
    rzb_log(LOG_DEBUG,"Done UUID_TYPE_NTLV_NAME init.");
    sg_NuggetList = UUID_Create_List();
    sg_NuggetTypeList = UUID_Create_List();
    init_NuggetTypes();
}

/*bool UUID_Get_UUID () {
	return true;
}*/


SO_PUBLIC bool
UUID_Get_UUID (const char *p_sName, int p_iType, uuid_t r_uuid)
{
	struct List *list;
	struct UUIDListNode *l_pListNode;
	if (p_iType != UUID_TYPE_DATA_TYPE && p_iType != UUID_TYPE_NTLV_NAME && p_iType != UUID_TYPE_NTLV_TYPE) {
		return true;
	}
	//rzb_log(LOG_DEBUG, "Requesting %s uuid", p_sName);
	list = UUID_Get_List(p_iType);
	List_Lock(list);
	if ((l_pListNode = UUID_getNodeByName (p_sName, p_iType)) == NULL)
	{
		List_Unlock(list);
		return false;
	}
	uuid_copy(r_uuid, l_pListNode->uuid);
	List_Unlock(list);
	return true;
}

SO_PUBLIC char *
UUID_Get_Description (const char *p_sName, int p_iType)
{
    struct List *list;
    struct UUIDListNode *l_pListNode;
    char * ret;
    list = UUID_Get_List(p_iType);
    List_Lock(list);
    if ((l_pListNode = UUID_getNodeByName (p_sName, p_iType)) == NULL)
    {
        List_Unlock(list);
        return NULL;
    }
    if (asprintf(&ret, "%s", l_pListNode->sDescription) == -1 )
    {
        List_Unlock(list);
        return NULL;
    }
    List_Unlock(list);
    return ret;
}

SO_PUBLIC char *
UUID_Get_DescriptionByUUID (uuid_t p_uuid, int p_iType)
{
    struct List *list;
    struct UUIDListNode *l_pListNode;
    char * ret;
    list = UUID_Get_List(p_iType);
    List_Lock(list);

    if ((l_pListNode = UUID_getNodeByUUID (p_uuid, p_iType)) == NULL)
    {
        List_Unlock(list);
        return NULL;
    }
    if (asprintf(&ret, "%s", l_pListNode->sDescription) == -1 )
    {
        List_Unlock(list);
        return NULL;
    }
    List_Unlock(list);
    return ret;
}

SO_PUBLIC char *
UUID_Get_NameByUUID (uuid_t p_uuid, int p_iType)
{
    struct List *list;
    struct UUIDListNode *l_pListNode;
    char * ret;
    list = UUID_Get_List(p_iType);
    List_Lock(list);

    if ((l_pListNode = UUID_getNodeByUUID (p_uuid, p_iType)) == NULL)
    {
        List_Unlock(list);
        return NULL;
    }
    if (asprintf(&ret, "%s", l_pListNode->sName) == -1 )
    {
        List_Unlock(list);
        return NULL;
    }
    List_Unlock(list);
    return ret;
}


SO_PUBLIC char *
UUID_Get_UUIDAsString (const char *p_sName, int p_iType)
{
    struct List *list;
    struct UUIDListNode *l_pListNode;
    char *l_sTemp;

    list = UUID_Get_List(p_iType);
    List_Lock(list);

    if ((l_pListNode = UUID_getNodeByName (p_sName, p_iType)) == NULL)
    {
        List_Unlock(list);
        return NULL;
    }
    if ((l_sTemp = calloc (UUID_STRING_LENGTH, sizeof (char))) == NULL)
    {
        List_Unlock(list);
        return NULL;
    }
    uuid_unparse (l_pListNode->uuid, l_sTemp);
    List_Unlock(list);
    return l_sTemp;
}

static int 
UUID_Cmp(void *a, void *b)
{
    return -1;
}

static int 
UUID_KeyCmp(void *a, void *id)
{
    struct UUIDListNode *current = (struct UUIDListNode *)a;
    struct UUIDKey *key = (struct UUIDKey *)id;
    switch (key->type)
    {
    case UUID_KEY_UUID:
        return uuid_compare(key->uuid, current->uuid);
    case UUID_KEY_NAME:
        return strcmp(key->name, current->sName);
    }

    return -1;
}
static void *
UUID_Clone(void *o)
{
    struct UUIDListNode *orig = o;
    struct UUIDListNode *new;
    if ((new =calloc(1,sizeof(struct UUIDListNode))) == NULL)
        return NULL;
    uuid_copy(new->uuid, orig->uuid);
    if (orig->sName != NULL)
    {
        if (asprintf(&new->sName, "%s", orig->sName) == -1)
        {
            free(new);
            return NULL;
        }
    }
    if (orig->sDescription != NULL)
    {
        if (asprintf(&new->sDescription, "%s", orig->sDescription) == -1)
        {
            free(new);
            return NULL;
        }
    }
            
    return new;
}

static void 
UUID_Destroy(void *a)
{
    struct UUIDListNode *current = (struct UUIDListNode *)a;
    if (current->sName != NULL)
        free(current->sName);
    if (current->sDescription != NULL)
        free(current->sDescription);
    free(current);
}

SO_PUBLIC struct List * 
UUID_Create_List (void)
{
    struct List *list;
    list = List_Create(LIST_MODE_GENERIC, 
            UUID_Cmp,
            UUID_KeyCmp,
            UUID_Destroy,
            UUID_Clone, NULL, NULL);

    if (list == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new list", __func__);
        return NULL;
    }
    return list;
}

static int
UUID_MessageAddNameSize(void *vItem, void *vCount)
{
    struct UUIDListNode *item = (struct UUIDListNode *)vItem;
    size_t * count = (size_t*)vCount;
    *count = *count + strlen(item->sName) +1;
    return LIST_EACH_OK;
}

size_t
UUIDList_BinarySize(struct List *list)
{
    size_t size = List_Length(list)*16;
    List_ForEach(list, UUID_MessageAddNameSize, &size);
    return size;
}
