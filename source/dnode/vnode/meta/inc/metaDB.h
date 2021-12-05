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

#ifndef _TD_META_DB_H_
#define _TD_META_DB_H_

#define USE_SQLITE_IMPL 1

#include "rocksdb/c.h"
#include "sqlite3.h"

#include "meta.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !USE_SQLITE_IMPL
typedef struct {
  rocksdb_t *tbDb;      // uid -> tb obj
  rocksdb_t *nameDb;    // name -> uid
  rocksdb_t *tagDb;     // uid -> tag
  rocksdb_t *schemaDb;  // uid+version -> schema
  sqlite3 *  mapDb;     // suid -> uid_list
} meta_db_t;
#else
typedef sqlite3 meta_db_t;
#endif

int  metaOpenDB(SMeta *pMeta);
void metaCloseDB(SMeta *pMeta);
int  metaSaveTableToDB(SMeta *pMeta, const STbCfg *pTbOptions);
int  metaRemoveTableFromDb(SMeta *pMeta, tb_uid_t uid);

#ifdef __cplusplus
}
#endif

#endif /*_TD_META_DB_H_*/