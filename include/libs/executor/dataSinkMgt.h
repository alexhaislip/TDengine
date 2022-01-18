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

#ifndef _DATA_SINK_MGT_H
#define _DATA_SINK_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"
#include "thash.h"
#include "executor.h"

#define DS_BUF_LOW   1
#define DS_BUF_FULL  2
#define DS_BUF_EMPTY 3

struct SDataSink;
struct SSDataBlock;

typedef struct SDataSinkMgtCfg {
  uint32_t maxDataBlockNum;           // todo: this should be numOfRows?
  uint32_t maxDataBlockNumPerQuery;
} SDataSinkMgtCfg;

int32_t dsDataSinkMgtInit(SDataSinkMgtCfg *cfg);

typedef struct SInputData {
  const struct SSDataBlock* pData;
  SHashObj* pTableRetrieveTsMap;
} SInputData;

typedef struct SOutputData {
  int32_t numOfRows;
  int8_t  compressed;
  char*   pData;
  bool    queryEnd;
  int32_t scheduleJobNo;
  int32_t bufStatus;
  int64_t useconds;
  int8_t  precision;
} SOutputData;

/**
 * Create a subplan's datasinker handle for all later operations. 
 * @param pDataSink
 * @param pHandle output
 * @return error code
 */
int32_t dsCreateDataSinker(const struct SDataSink *pDataSink, DataSinkHandle* pHandle);

/**
 * Put the result set returned by the executor into datasinker.
 * @param handle
 * @param pRes
 * @return error code
 */
int32_t dsPutDataBlock(DataSinkHandle handle, const SInputData* pInput, bool* pContinue);

void dsEndPut(DataSinkHandle handle, uint64_t useconds);

/**
 * Get the length of the data returned by the next call to dsGetDataBlock.
 * @param handle
 * @param pLen data length
 */
void dsGetDataLength(DataSinkHandle handle, int32_t* pLen, bool* pQueryEnd);

/**
 * Get data, the caller needs to allocate data memory.
 * @param handle
 * @param pOutput output
 * @param pStatus output
 * @return error code
 */
int32_t dsGetDataBlock(DataSinkHandle handle, SOutputData* pOutput);

/**
 * After dsGetStatus returns DS_NEED_SCHEDULE, the caller need to put this into the work queue.
 * @param ahandle
 * @param pItem
 */
void dsScheduleProcess(void* ahandle, void* pItem);

/**
 * Destroy the datasinker handle.
 * @param handle
 */
void dsDestroyDataSinker(DataSinkHandle handle);

#ifdef __cplusplus
}
#endif

#endif /*_DATA_SINK_MGT_H*/