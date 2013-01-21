/*
 * io.c
 *
 *  Created on: 03-04-2012
 *	  Author: wojciechm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include "config.h"
#include "config_file.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <razorback/api.h>
#include <razorback/judgment.h>
#include "api/metadata.h"
#include <razorback/types.h>
#include "api/block.h"
#include "api/list.h"
#include "api/ntlv.h"
#include <getopt.h>
#include "main.h"
#include "config_file.h"
#include "api/hash.h"
#include <razorback/block_pool.h>
#include <razorback/submission.h>
#include "io.h"
#include <time.h>

char * basepath = "/tmp";
// 10 for unix time stamp and extra 10 for padding scenarios if directory exists.
char tmpDir[20];
bool initTmpDir = false;

char * getTmpDir() {
	if (initTmpDir) {
		return tmpDir;
	}
	int pad = 1;
	time_t t = time(NULL);
	struct tm * lt = localtime(&t);
	int accessreturn;
	char tmp[20];
	sprintf(tmpDir,"%ld",t + lt->tm_gmtoff);
	accessreturn = pathExists ("%s/%s", basepath, tmpDir);
	if (accessreturn == 0) {
		while (accessreturn == 0) {
			sprintf(tmp,"%s-%d",tmpDir,pad++);
			accessreturn = pathExists ("%s/%s", basepath, tmp);
		}
		strncpy(tmpDir,tmp,15);
	}
	if (accessreturn == 1) {
		rzb_log(LOG_ERR,"%s",errno);
		exit(errno);
	}
	rzb_log(LOG_NOTICE,"Temporary files stored to '%s/%s'", basepath, tmpDir);
	initTmpDir = true;
	return tmpDir;
}

void clearTmpDir() {
	initTmpDir = false;
}

char *
Transfer_generateFilename (struct Block *block) {
	char *hash;
	char *filename;
	if ((hash = Hash_ToText (block->pId->pHash)) == NULL) {
		rzb_log (LOG_ERR, "%s: Could not convert hash to text", __func__);
		return NULL;
	}
#define FILENAME_FMT "%s.%ju"
	if (asprintf(&filename, FILENAME_FMT, hash, block->pId->iLength) == -1) {
		free(hash);
		return NULL;
	}
	free(hash);
	return filename;
}

bool Transfer_Prepare_File(struct Block *block, char *file, bool temp) {
	if (file == NULL) {
		rzb_log(LOG_ERR, "%s: File is null", __func__);
		return false;
	}
	if ((block->data.file=fopen(file, "r")) == NULL) {
		rzb_log(LOG_ERR, "%s: Failed to open file handle: %s, File: %s", strerror(errno), file);
		return false;
	}
	block->data.fileName = file;
	block->data.tempFile = temp;

	block->data.pointer = mmap (NULL, block->pId->iLength, PROT_READ, MAP_PRIVATE, fileno(block->data.file), 0);
	if (block->data.pointer == MAP_FAILED) {
		rzb_perror("%s");
		block->data.pointer = NULL;
		fclose(block->data.file);
		return false;
	}
	return true;
}

uint32_t writeWrap (int fd, uint8_t * data, uint64_t length) {

	int SizeDword;
	int totalbytes = 0;
	int bytessofar;

	SizeDword = (int) length;

	while (totalbytes < SizeDword) {
		bytessofar = write (fd, data + totalbytes, SizeDword - totalbytes);
		if (bytessofar == -1) {
			rzb_perror ("writeWrap: Could not write data to file: %s");
			return 0;
		}
		totalbytes += bytessofar;
	}

	return 1;
}

/***
 * Checks whether the path specified by the format and parameters exists.
 * Returns 0 if yes, -1 if no and 1 if there was trouble allocating memory.
 */
int pathExists(const char *fmt, ...) {
	char *path = NULL;
	va_list argp;
	int ret;
	va_start (argp, fmt);
	if (vasprintf (&path, fmt, argp) == -1) {
		rzb_log (LOG_ERR, "%s: Could not allocate directory string",
				 __func__);
		ret = 1;
	} else {
		ret = access(path, F_OK);
		free(path);
	}
	return ret;
}

char * File_mkdir(const char *fmt, ...) {
	char *dir = NULL;
	va_list argp;
	va_start (argp, fmt);
	if (vasprintf (&dir, fmt, argp) == -1) {
		rzb_log (LOG_ERR, "%s: Could not allocate directory string",
				 __func__);
		return NULL;
	}

	if (access (dir, F_OK) == -1) {
		if (mkdir (dir, S_IRUSR | S_IWUSR | S_IXUSR |
				   S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
			rzb_log (LOG_ERR, "%s: Error creating directory %s", __func__,
					 dir);
			free (dir);
			return NULL;
		}
	}
	return dir;
}



bool File_Store(struct BlockPoolItem *item) {
	File_mkdir("%s/%s", basepath, getTmpDir());
	int fd;
	char * filename = NULL;
	char *path = NULL;
	//char *dir = NULL;
	struct BlockPoolData *dataItem = NULL;
	uint8_t data[4096];
	size_t len;
	if ((filename = Transfer_generateFilename (item->pEvent->pBlock)) == NULL) {
		rzb_log (LOG_ERR, "%s: failed to generate file name", __func__);
		return false;
	}

	if (asprintf(&path, "%s/%s/%s", basepath, tmpDir, filename) == -1) {
		rzb_log (LOG_ERR, "%s: failed to generate file path", __func__);
		free(filename);
		return false;
	}

	if ((fd = open(path, O_RDONLY, 0)) != -1) {
		rzb_log(LOG_INFO, "File '%s' exists. Skipping duplicate.", path);
		close(fd);
		free(path);
		return true;
	}
	fd = open (path, O_RDWR | O_CREAT | O_TRUNC,
			   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) {
		rzb_perror ("StoreDataAsFile: Could not open file for writing: %s");
		free (filename);
		return 0;
	}
	dataItem = item->pDataHead;
	while (dataItem != NULL) {
		if (dataItem->iFlags == BLOCK_POOL_DATA_FLAG_FILE) {
			while((len = fread(data,1,4096, dataItem->data.file)) > 0) {
				if (writeWrap(fd,data,len) == 0) {
					rzb_log (LOG_ERR, "%s: Write failed.", __func__);
					free (filename);
					close (fd);
					return false;
				}
			}
			rewind(dataItem->data.file);

		}
		else {
			if ((writeWrap (fd, dataItem->data.pointer, dataItem->iLength)) == 0) {
				rzb_log (LOG_ERR, "%s: Write failed.", __func__);
				free (filename);
				close (fd);
				return false;
			}
		}
		dataItem = dataItem->pNext;
	}
	close (fd);
	rzb_log(LOG_NOTICE,"%s - %s", __func__, path);
	free (path);
	free(filename);
	return true;
}
