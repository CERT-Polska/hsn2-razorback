/*
 * util.c
 *
 *  Created on: 06-04-2012
 *      Author: wojciechm
 */
#include <razorback/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "api/log.h"

struct FileList * fileList = NULL;
struct FileList * lastFile = NULL;
struct ContextList * contextList = NULL;

struct RazorbackContext * shiftContext() {
	struct RazorbackContext * ret = NULL;
	struct ContextList * current = NULL;
	if (contextList != NULL) {
		current = contextList;
		contextList = current->next;
		ret = current->context;
		free(current);
	}
	return ret;
}

bool unshiftContext(struct RazorbackContext * context) {
	struct ContextList * current = calloc(1, sizeof(struct ContextList));
	if (current == NULL)
		return false;
	current->context = context;
	current->next = contextList;
	contextList = current;
	return true;
}

bool pushFileList(struct FileDefinition * file) {
	struct FileList * current = NULL;
	if (file == NULL) {
		rzb_log(LOG_ERR, "Unable to allocate memory for file definition");
		return false;
	}
	current = calloc(1, sizeof(struct FileList));
	if (current == NULL)
		return false;
	current->file = file;
	current->next = NULL;
	if (fileList == NULL) {
		fileList = current;
		lastFile = current;
	} else {
		lastFile->next = current;
		lastFile = current;
	}
}

struct FileDefinition * shiftFileList() {
	struct FileDefinition * ret = NULL;
	struct FileList * current = NULL;
	if (fileList != NULL) {
		current = fileList;
		fileList = current->next;
		ret = current->file;
		free(current);
	}
	return ret;
}

bool unshiftFileList(struct FileDefinition * file) {
	struct FileList * current = NULL;
	if (file == NULL) {
		rzb_log(LOG_ERR, "Unable to allocate memory for file definition");
		return false;
	}
	current = calloc(1, sizeof(struct FileList));
	if (current == NULL)
		return false;
	current->file = file;
	current->next = fileList;
	fileList = current;
	return true;
}

struct FileDefinition * createFileDefinition(char * path, uuid_t uuid) {
	struct FileDefinition * f = calloc(1, sizeof(struct FileDefinition));
	if (asprintf(&f->path,"%s",path) == -1)
		return NULL;
	uuid_copy(f->uuid,uuid);
	return f;
}

void destroyFileDefinition(struct FileDefinition *f) {
	free(f->path);
	free(f);
}

char * sfFlagToString(uint32_t flag) {
	switch (flag) {
		case 0x00000001: {
			return "benign"; //"SF_FLAG_GOOD";
		}
		case 0x00000002: {
			return "malicious"; //"SF_FLAG_BAD";
		}
		case 0x00000004: {
			rzb_log(LOG_ERR,"DEFUNCT - SF_FLAG_WHITE_LIST");
			return "SF_FLAG_WHITE_LIST";
		}
		case 0x00000008: {
			rzb_log(LOG_ERR,"DEFUNCT - SF_FLAG_BLACK_LIST");
			return "SF_FLAG_BLACK_LIST";
		}
		case 0x00000010: {
			rzb_log(LOG_ERR,"INPUT - SF_FLAG_DIRTY");
			return "SF_FLAG_DIRTY";
		}
		case 0x00000020: {
			rzb_log(LOG_ERR,"DEFUNCT - SF_FLAG_CANHAZ");
			return "SF_FLAG_CANHAZ";
		}
		case 0x00000040: {
			rzb_log(LOG_ERR,"DEFUNCT - SF_FLAG_PROCESSING");
			return "SF_FLAG_PROCESSING";
		}
		case 0x00000080: {
			return "suspicious"; //"SF_FLAG_DODGY|SF_FLAG_SUSPICIOUS";
		}
		case 0xffffffff: {
			rzb_log(LOG_ERR,"DEFUNCT - SF_FLAG_ALL");
			return "SF_FLAG_ALL";
		}
	}
}
