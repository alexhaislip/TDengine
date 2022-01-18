/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TD_TFS_H_
#define _TD_TFS_H_

#include "tglobal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int32_t level;
  int32_t id;
} SDiskID;

#define TFS_UNDECIDED_LEVEL -1
#define TFS_UNDECIDED_ID -1
#define TFS_PRIMARY_LEVEL 0
#define TFS_PRIMARY_ID 0
#define TFS_MIN_LEVEL 0
#define TFS_MAX_LEVEL (TSDB_MAX_TIERS - 1)

// FS APIs ====================================
typedef struct {
  int64_t total;
  int64_t used;
  int64_t avail;
} SFSMeta;

int32_t tfsInit(SDiskCfg *pDiskCfg, int32_t ndisk);
void    tfsCleanup();
void    tfsUpdateSize(SFSMeta *pFSMeta);
void    tfsAllocDisk(int32_t expLevel, int32_t *level, int32_t *id);

const char *TFS_PRIMARY_PATH();
const char *TFS_DISK_PATH(int32_t level, int32_t id);

// TFILE APIs ====================================
typedef struct {
  int32_t level;
  int32_t id;
  char    rname[TSDB_FILENAME_LEN];  // REL name
  char    aname[TSDB_FILENAME_LEN];  // ABS name
} TFILE;

#define TFILE_LEVEL(pf) ((pf)->level)
#define TFILE_ID(pf) ((pf)->id)
#define TFILE_NAME(pf) ((pf)->aname)
#define TFILE_REL_NAME(pf) ((pf)->rname)

#define tfsopen(pf, flags) open(TFILE_NAME(pf), flags)
#define tfsclose(fd) close(fd)
#define tfsremove(pf) remove(TFILE_NAME(pf))
#define tfscopy(sf, df) taosCopyFile(TFILE_NAME(sf), TFILE_NAME(df))
#define tfsrename(sf, df) taosRename(TFILE_NAME(sf), TFILE_NAME(df))

void    tfsInitFile(TFILE *pf, int32_t level, int32_t id, const char *bname);
bool    tfsIsSameFile(const TFILE *pf1, const TFILE *pf2);
int32_t tfsEncodeFile(void **buf, TFILE *pf);
void   *tfsDecodeFile(void *buf, TFILE *pf);
void    tfsbasename(const TFILE *pf, char *dest);
void    tfsdirname(const TFILE *pf, char *dest);

// DIR APIs ====================================
int32_t tfsMkdirAt(const char *rname, int32_t level, int32_t id);
int32_t tfsMkdirRecurAt(const char *rname, int32_t level, int32_t id);
int32_t tfsMkdir(const char *rname);
int32_t tfsRmdir(const char *rname);
int32_t tfsRename(char *orname, char *nrname);

typedef struct TDIR TDIR;

TDIR        *tfsOpendir(const char *rname);
const TFILE *tfsReaddir(TDIR *tdir);
void         tfsClosedir(TDIR *tdir);

#ifdef __cplusplus
}
#endif

#endif /*_TD_TFS_H_*/