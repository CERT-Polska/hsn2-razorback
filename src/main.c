#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include "config.h"
#include "config_file.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <razorback/api.h>
#include <razorback/judgment.h>
#include "razorback/metadata.h"
#include <razorback/types.h>
#include "api/block.h"
#include "api/list.h"
#include "api/ntlv.h"
#include "api/uuids.h"
#include <getopt.h>
#include "main.h"
#include "api/hash.h"
#include <razorback/block_pool.h>
#include <razorback/submission.h>
#include "io.h"
#include "util.h"

uint8_t (*function) (struct Block *, struct EventId *, struct List *, void *);
bool (*threadInit) (void ** threadData);
void (*threadCleanup) (void * threadData);
pthread_t listenerThread;
pthread_cond_t mainCond;
bool endScan = false;
struct Mutex* mutex;

extern struct FileList * fileList;
extern struct ContextList * contextList;

#ifdef SLEEP
int sleepTime = 3;
#else
int sleepTime = 0;
#endif

static const struct option options[] = {
	{"conf",optional_argument, NULL, 'c'},
	{"file",required_argument, NULL, 'f'},
	{"wait",optional_argument, NULL, 'w'},
	{"uuid",optional_argument, NULL, 'u'},
	{"type",optional_argument, NULL, 't'},
	{"daemonize",optional_argument, NULL, 'd'},
	{NULL, 0, NULL, 0}
};

bool Transfer_Prepare_File(struct Block *block, char *file, bool temp);
bool initNug();
void shutdownNug();

void* listenerThreadFunc(void* data) {
	char filepath[1000], uuidStr[36], filetype[40], line[1100];
	struct FileDefinition * f;
	uuid_t uuid;
	char c;

	while (true) {
		memset(filepath, '\0', 1000);
		memset(uuidStr, '\0', 36);
		memset(filetype, '\0', 40);
		memset(line, '\0', 1100);


		if (fgets(line, 1100, stdin) != NULL) {
			sscanf(line, "--uuid=%s --type=%s --file=%s\n", uuidStr, filetype, filepath);
			//printf("GOT: line=%s PARSED TO: --uuid=%s --type=%s --file=%s\n", line, uuidStr, filetype, filepath);
		}
		if ( strlen(filepath) > 0 ) {
			if (strcmp(uuidStr, "none") != 0)
				 uuid_parse(uuidStr, uuid);
			else if (strcmp(filetype, "none") != 0)
				 UUID_Get_UUID(filetype, UUID_TYPE_DATA_TYPE, uuid);
			else
				uuid_parse("0", uuid);

			f = createFileDefinition(filepath, uuid);
			Mutex_Lock(mutex);
			pushFileList(f);
			Mutex_Unlock(mutex);
		} else {
			usleep(500000);
		}
	}
	return NULL;
}

void initListenerThread() {
	if (pthread_create(&listenerThread, NULL, listenerThreadFunc, NULL) != 0) {
		rzb_log(LOG_ERR, "Couldn't run listener thread.");
		exit(-1);
	}
}

void inspectFile (char * fileName, uuid_t uuid) {
	void * threadData = NULL;
	if (threadInit != NULL) {
		if (!threadInit(&threadData)) {
			rzb_log(LOG_ERR, "Couldn't run nugget inspection threadInit.");
			exit(-1);
		} else {
			rzb_log(LOG_DEBUG, "Thread init for nugget complete.");
		}
	}
	sleep(sleepTime);
	struct EventId * eventId;
	eventId = calloc(1,sizeof(struct EventId));
	struct Block * block = Block_Create ();
	struct List * list	= NTLVList_Create();
	struct stat st;
	stat(fileName, &st);
	//BlockPool_Init();
	block->pId->iLength = st.st_size;
	if (!Transfer_Prepare_File(block, fileName, false)) {
		rzb_log(LOG_ERR, "Trouble preparing file transfer - '%s'", fileName);
		Block_Destroy(block);
		free(eventId);
		List_Destroy(list);
		return;
	}
	Hash_Update(block->pId->pHash, block->data.pointer, block->pId->iLength);
	Hash_Finalize(block->pId->pHash);
	uuid_copy(block->pId->uuidDataType,uuid);
	struct ContextList * current = NULL;
	while (contextList != NULL) {
		current = contextList;
		uint8_t ret = function (block, eventId, list, threadData);
		if ( ret >= 0 ) {
			rzb_log(LOG_NOTICE, "Returned with: %u", ret);
		}
		if (current == contextList)
			break;
	}
	List_Destroy(list);
	/*Don't need to free/destroy as it's done with the judgment.
	 * Was needed previously because of cloning - cloning removed*/
	//Block_Destroy(block);
	//free(eventId);
	if (threadCleanup != NULL) {
		threadCleanup(threadData);
	}
}

int main(int argc, char ** argv) {
	mutex = Mutex_Create(MUTEX_MODE_NORMAL);
	uuid_t uuid;
	struct FileDefinition * f;
	int c, daemon = 0;
	uuid_clear(uuid);
	int prev = 0;
	int limit = 0;
	while (true) {
		 /* getopt_long stores the option index here. */
		 int option_index = 0;
		 c = getopt_long (argc, argv, "c:f:w:t:u:d", options, &option_index);
		/* Detect the end of the options. */

		if (c != -1 && optind > prev) {
			limit = optind - 1;
		} else {
			limit = argc;
		}

		for (prev++; prev < limit; prev++) {
			printf("val = %s, len = %lu\n", argv[prev], strlen(argv[prev]));
			if (strlen(argv[prev]) == 0)
				continue;
			rzb_log (LOG_DEBUG,"Positional argument %d  %s",prev, argv[prev]);
			f = createFileDefinition(argv[prev],uuid);
			Mutex_Lock(mutex);
			pushFileList(f);
			Mutex_Unlock(mutex);
		}


		if (c == -1)
			 break;

		 switch (c) {
			case 'c':
			 rzb_log (LOG_DEBUG,"option -c with value `%s'", optarg);
			 break;

			case 'f':
				f = createFileDefinition(optarg,uuid);
				Mutex_Lock(mutex);
				pushFileList(f);
				Mutex_Unlock(mutex);
				rzb_log (LOG_DEBUG,"option -f with value `%s'", optarg);
			 break;

			case 'w':
				rzb_log (LOG_DEBUG,"option -w with value `%s'", optarg);
			 break;

			case 'd':
				daemon = 1;
			 break;

			case 'u':
				rzb_log (LOG_DEBUG,"option -u with value `%s'", optarg);
				uuid_parse(optarg,uuid);
			break;

			case 't':
				rzb_log (LOG_DEBUG,"option -t with value `%s'", optarg);
				UUID_Get_UUID(optarg,UUID_TYPE_DATA_TYPE,uuid);
			break;

			case '?':
				rzb_log (LOG_DEBUG,"no option with value `%s'", optarg);
			 /* getopt_long already printed an error message. */
			break;

			default:
				rzb_log (LOG_DEBUG,"no option with value `%s'", optarg);
			 //abort ();
		}
	}

	if (daemon)
		initListenerThread();
	//RZB_Init_API();
	if (!initNug()) {
		rzb_log(LOG_ERR, "initNug failed");
		exit(-1);
	}
	rzb_log(LOG_INFO, "Done initNug");
	struct FileDefinition * tmp;
	while (!endScan) {
		if (fileList == NULL) {
			if (daemon == 0)
				break;
			usleep(500000);
			continue;
		}

		clearTmpDir();
		Mutex_Lock(mutex);
		tmp = shiftFileList();
		if (strlen(tmp->path) == 0) {
			destroyFileDefinition(tmp);
			continue;
		}
		fprintf(stderr, "value = '%s'\n", tmp == NULL ? "NULL" : tmp->path);
		Mutex_Unlock(mutex);
		rzb_log(LOG_NOTICE, "Inspecting %s",tmp->path);
		if (testFile(tmp->path)) {
			inspectFile(tmp->path,tmp->uuid);
		}
		rzb_log(LOG_NOTICE, "Done inspecting %s",tmp->path);
		destroyFileDefinition(tmp);
		rzb_log(LOG_INFO, "FINISHED");
	}

	shutdownNug();
	Mutex_Destroy(mutex);

	rzb_log(LOG_INFO, "Done shutdownNug");
}

SO_PUBLIC bool
Razorback_Render_Verdict (struct Judgment *p_pJudgment) {
	rzb_log(LOG_NOTICE, "%s - %s - %s", __func__, "Message", p_pJudgment->sMessage);
	rzb_log(LOG_NOTICE, "%s - %s - %s", __func__, "SourceFire flags", sfFlagToString(p_pJudgment->Set_SfFlags));
	rzb_log(LOG_NOTICE, "%s - %s - %d", __func__, "Priority", p_pJudgment->iPriority);
	return true;
}

/*void * sendDetectionResult () {
	rzb_log(LOG_NOTICE,"%s",__func__);
}*/

SO_PUBLIC struct RazorbackContext *
Razorback_Init_Inspection_Context (uuid_t p_uuidNuggetId,
									 uuid_t p_uuidApplicationType,
									 uint32_t p_iDataTypeCount,
									 uuid_t * p_pDataTypeList,
									 struct RazorbackInspectionHooks *p_pInspectionHooks,
									 uint32_t initialThreads, uint32_t maxThreads) {
	struct RazorbackContext * context = calloc(1, sizeof(struct RazorbackContext));
	uuid_parse("0", context->uuidApplicationType);
	uuid_parse("0", context->uuidNuggetId);
	uuid_parse("0", context->uuidNuggetType);
	context->iFlags = 0;
	context->iDataTypeCount = p_iDataTypeCount;
	context->pDataTypeList = p_pDataTypeList;
	context->pCommandHooks = NULL;
	context->pInspectionHooks = p_pInspectionHooks;
	function = p_pInspectionHooks->processBlock;
	threadInit = p_pInspectionHooks->initThread;
	threadCleanup = p_pInspectionHooks->cleanupThread;
	if (!unshiftContext(context)) {
		free(context);
		rzb_log(LOG_ERR, "%s - failed to store context", __func__);
		return NULL;
	}
	rzb_log(LOG_INFO, "Have inspection function pointer");
	return context;
}

void Razorback_Shutdown_Context (struct RazorbackContext *context) {
	rzb_log(LOG_INFO,"%s",__func__);
	free(context);
}



SO_PUBLIC int Submission_Submit(struct BlockPoolItem *p_pItem, int p_iFlags, uint32_t *p_pSf_Flags, uint32_t *p_pEnt_Flags) {
	bool ret = File_Store(p_pItem);
	BlockPool_DestroyItem(p_pItem);
	if (ret)
		return RZB_SUBMISSION_OK;
	else
		return RZB_SUBMISSION_ERROR;
}

