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

#include "tarray.h"
#include "dataSinkMgt.h"
#include "dataSinkInt.h"
#include "planner.h"

static SDataSinkManager gDataSinkManager = {0};

int32_t dsDataSinkMgtInit(SDataSinkMgtCfg *cfg) {
  gDataSinkManager.cfg = *cfg;
  pthread_mutex_init(&gDataSinkManager.mutex, NULL);
}

int32_t dsCreateDataSinker(const struct SDataSink *pDataSink, DataSinkHandle* pHandle) {
  if (DSINK_Dispatch == pDataSink->info.type) {
    return createDataDispatcher(&gDataSinkManager, pDataSink, pHandle);
  }
  return TSDB_CODE_FAILED;
}

int32_t dsPutDataBlock(DataSinkHandle handle, const SInputData* pInput, bool* pContinue) {
  SDataSinkHandle* pHandleImpl = (SDataSinkHandle*)handle;
  return pHandleImpl->fPut(pHandleImpl, pInput, pContinue);
}

void dsEndPut(DataSinkHandle handle, int64_t useconds) {
  SDataSinkHandle* pHandleImpl = (SDataSinkHandle*)handle;
  return pHandleImpl->fEndPut(pHandleImpl, useconds);
}

void dsGetDataLength(DataSinkHandle handle, int32_t* pLen, bool* pQueryEnd) {
  SDataSinkHandle* pHandleImpl = (SDataSinkHandle*)handle;
  pHandleImpl->fGetLen(pHandleImpl, pLen, pQueryEnd);
}

int32_t dsGetDataBlock(DataSinkHandle handle, SOutputData* pOutput) {
  SDataSinkHandle* pHandleImpl = (SDataSinkHandle*)handle;
  return pHandleImpl->fGetData(pHandleImpl, pOutput);
}

void dsScheduleProcess(void* ahandle, void* pItem) {
  // todo
}

void dsDestroyDataSinker(DataSinkHandle handle) {
  SDataSinkHandle* pHandleImpl = (SDataSinkHandle*)handle;
  pHandleImpl->fDestroy(pHandleImpl);
}