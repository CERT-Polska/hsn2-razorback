/*
 * io.h
 *
 *  Created on: 03-04-2012
 *      Author: wojciechm
 */

#ifndef IO_H_
#define IO_H_

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
#include "razorback/metadata.h"
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
#include <fcntl.h>
#include <sys/types.h>

char * getTmpDir();
void clearTmpDir();
int pathExists(const char *fmt, ...);
char * Transfer_generateFilename (struct Block *block);
bool Transfer_Prepare_File(struct Block *block, char *file, bool temp);
uint32_t writeWrap (int fd, uint8_t * data, uint64_t length);
bool File_Store(struct BlockPoolItem *item);

#endif /* IO_H_ */
