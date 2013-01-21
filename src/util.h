/*
 * util.h
 *
 *  Created on: 06-04-2012
 *      Author: wojciechm
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <razorback/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ContextList {
	struct RazorbackContext *context;
	struct ContextList *next;
};

struct FileDefinition {
	char * path;
	uuid_t uuid;
};

struct FileList {
	struct FileDefinition *file;
	struct FileList *next;
};

struct RazorbackContext * shiftContext();
bool unshiftContext(struct RazorbackContext * context);
bool pushFileList(struct FileDefinition * file);
struct FileDefinition * shiftFileList();
bool unshiftFileList(struct FileDefinition * file);
struct FileDefinition * createFileDefinition(char * path, uuid_t uuid);
void destroyFileDefinition(struct FileDefinition * file);
char * sfFlagToString(uint32_t flag);
#endif /* UTIL_H_ */
