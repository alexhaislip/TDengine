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

#include "tmsg.h"
#include "queryInt.h"
#include "query.h"

int32_t (*queryBuildMsg[TDMT_MAX])(void* input, char **msg, int32_t msgSize, int32_t *msgLen) = {0};

int32_t (*queryProcessMsgRsp[TDMT_MAX])(void* output, char *msg, int32_t msgSize) = {0};

int32_t queryBuildTableMetaReqMsg(void* input, char **msg, int32_t msgSize, int32_t *msgLen) {
  if (NULL == input || NULL == msg || NULL == msgLen) {
    return TSDB_CODE_TSC_INVALID_INPUT;
  }

  SBuildTableMetaInput* bInput = (SBuildTableMetaInput *)input;

  int32_t estimateSize = sizeof(STableInfoMsg);
  if (NULL == *msg || msgSize < estimateSize) {
    tfree(*msg);
    *msg = calloc(1, estimateSize);
    if (NULL == *msg) {
      return TSDB_CODE_TSC_OUT_OF_MEMORY;
    }
  }

  STableInfoMsg *bMsg = (STableInfoMsg *)*msg;

  bMsg->vgId = bInput->vgId;

  strncpy(bMsg->tableFname, bInput->tableFullName, sizeof(bMsg->tableFname));
  bMsg->tableFname[sizeof(bMsg->tableFname) - 1] = 0;

  *msgLen = (int32_t)sizeof(*bMsg);

  return TSDB_CODE_SUCCESS;
}

int32_t queryBuildUseDbMsg(void* input, char **msg, int32_t msgSize, int32_t *msgLen) {
  if (NULL == input || NULL == msg || NULL == msgLen) {
    return TSDB_CODE_TSC_INVALID_INPUT;
  }

  SBuildUseDBInput* bInput = (SBuildUseDBInput *)input;

  int32_t estimateSize = sizeof(SUseDbMsg);
  if (NULL == *msg || msgSize < estimateSize) {
    tfree(*msg);
    *msg = calloc(1, estimateSize);
    if (NULL == *msg) {
      return TSDB_CODE_TSC_OUT_OF_MEMORY;
    }
  }

  SUseDbMsg *bMsg = (SUseDbMsg *)*msg;

  strncpy(bMsg->db, bInput->db, sizeof(bMsg->db));
  bMsg->db[sizeof(bMsg->db) - 1] = 0;

  bMsg->vgVersion = bInput->vgVersion;

  *msgLen = (int32_t)sizeof(*bMsg);

  return TSDB_CODE_SUCCESS;  
}


int32_t queryProcessUseDBRsp(void* output, char *msg, int32_t msgSize) {
  if (NULL == output || NULL == msg || msgSize <= 0) {
    return TSDB_CODE_TSC_INVALID_INPUT;
  }

  SUseDbRsp *pRsp = (SUseDbRsp *)msg;
  SUseDbOutput *pOut = (SUseDbOutput *)output;
  int32_t code = 0;

  if (msgSize <= sizeof(*pRsp)) {
    qError("invalid use db rsp msg size, msgSize:%d", msgSize);
    return TSDB_CODE_TSC_VALUE_OUT_OF_RANGE;
  }
  
  pRsp->vgVersion = ntohl(pRsp->vgVersion);
  pRsp->vgNum = ntohl(pRsp->vgNum);

  if (pRsp->vgNum < 0) {
    qError("invalid db[%s] vgroup number[%d]", pRsp->db, pRsp->vgNum);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  int32_t expectSize = pRsp->vgNum * sizeof(pRsp->vgroupInfo[0]) + sizeof(*pRsp);
  if (msgSize != expectSize) {
    qError("use db rsp size mis-match, msgSize:%d, expected:%d, vgnumber:%d", msgSize, expectSize, pRsp->vgNum);
    return TSDB_CODE_TSC_VALUE_OUT_OF_RANGE;
  }

  pOut->dbVgroup.vgVersion = pRsp->vgVersion;
  pOut->dbVgroup.hashMethod = pRsp->hashMethod;
  pOut->dbVgroup.vgInfo = taosHashInit(pRsp->vgNum, taosGetDefaultHashFunction(TSDB_DATA_TYPE_INT), true, HASH_ENTRY_LOCK);
  if (NULL == pOut->dbVgroup.vgInfo) {
    qError("hash init[%d] failed", pRsp->vgNum);
    return TSDB_CODE_TSC_OUT_OF_MEMORY;
  }

  for (int32_t i = 0; i < pRsp->vgNum; ++i) {
    pRsp->vgroupInfo[i].vgId = ntohl(pRsp->vgroupInfo[i].vgId);
    pRsp->vgroupInfo[i].hashBegin = ntohl(pRsp->vgroupInfo[i].hashBegin);
    pRsp->vgroupInfo[i].hashEnd = ntohl(pRsp->vgroupInfo[i].hashEnd);

    for (int32_t n = 0; n < pRsp->vgroupInfo[i].numOfEps; ++n) {
      pRsp->vgroupInfo[i].epAddr[n].port = ntohs(pRsp->vgroupInfo[i].epAddr[n].port);
    }

    if (0 != taosHashPut(pOut->dbVgroup.vgInfo, &pRsp->vgroupInfo[i].vgId, sizeof(pRsp->vgroupInfo[i].vgId), &pRsp->vgroupInfo[i], sizeof(pRsp->vgroupInfo[i]))) {
      qError("hash push failed");
      goto _return;
    }
  }

  memcpy(pOut->db, pRsp->db, sizeof(pOut->db));

  return code;

_return:
  if (pOut) {
    tfree(pOut->dbVgroup.vgInfo);
  }
  
  return code;
}

static int32_t queryConvertTableMetaMsg(STableMetaMsg* pMetaMsg) {
  pMetaMsg->numOfTags = ntohl(pMetaMsg->numOfTags);
  pMetaMsg->numOfColumns = ntohl(pMetaMsg->numOfColumns);
  pMetaMsg->sversion = ntohl(pMetaMsg->sversion);
  pMetaMsg->tversion = ntohl(pMetaMsg->tversion);
  pMetaMsg->tuid = htobe64(pMetaMsg->tuid);
  pMetaMsg->suid = htobe64(pMetaMsg->suid);
  pMetaMsg->vgId = ntohl(pMetaMsg->vgId);

  if (pMetaMsg->numOfTags < 0 || pMetaMsg->numOfTags > TSDB_MAX_TAGS) {
    qError("invalid numOfTags[%d] in table meta rsp msg", pMetaMsg->numOfTags);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  if (pMetaMsg->numOfColumns > TSDB_MAX_COLUMNS || pMetaMsg->numOfColumns <= 0) {
    qError("invalid numOfColumns[%d] in table meta rsp msg", pMetaMsg->numOfColumns);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  if (pMetaMsg->tableType != TSDB_SUPER_TABLE && pMetaMsg->tableType != TSDB_CHILD_TABLE && pMetaMsg->tableType != TSDB_NORMAL_TABLE) {
    qError("invalid tableType[%d] in table meta rsp msg", pMetaMsg->tableType);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  if (pMetaMsg->sversion < 0) {
    qError("invalid sversion[%d] in table meta rsp msg", pMetaMsg->sversion);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  if (pMetaMsg->tversion < 0) {
    qError("invalid tversion[%d] in table meta rsp msg", pMetaMsg->tversion);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }
  
  SSchema* pSchema = pMetaMsg->pSchema;

  int32_t numOfTotalCols = pMetaMsg->numOfColumns + pMetaMsg->numOfTags;
  for (int i = 0; i < numOfTotalCols; ++i) {
    pSchema->bytes = ntohl(pSchema->bytes);
    pSchema->colId = ntohl(pSchema->colId);

    pSchema++;
  }

  if (pMetaMsg->pSchema[0].colId != PRIMARYKEY_TIMESTAMP_COL_ID) {
    qError("invalid colId[%d] for the first column in table meta rsp msg", pMetaMsg->pSchema[0].colId);
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  return TSDB_CODE_SUCCESS;
}

int32_t queryCreateTableMetaFromMsg(STableMetaMsg* msg, bool isSuperTable, STableMeta **pMeta) {
  int32_t total = msg->numOfColumns + msg->numOfTags;
  int32_t metaSize = sizeof(STableMeta) + sizeof(SSchema) * total;
  
  STableMeta* pTableMeta = calloc(1, metaSize);
  if (NULL == pTableMeta) {
    qError("calloc size[%d] failed", metaSize);
    return TSDB_CODE_TSC_OUT_OF_MEMORY;
  }

  pTableMeta->vgId = isSuperTable ? 0 : msg->vgId;
  pTableMeta->tableType = isSuperTable ? TSDB_SUPER_TABLE : msg->tableType;
  pTableMeta->uid = msg->suid;
  pTableMeta->suid = msg->suid;
  pTableMeta->sversion = msg->sversion;
  pTableMeta->tversion = msg->tversion;

  pTableMeta->tableInfo.numOfTags = msg->numOfTags;
  pTableMeta->tableInfo.precision = msg->precision;
  pTableMeta->tableInfo.numOfColumns = msg->numOfColumns;

  memcpy(pTableMeta->schema, msg->pSchema, sizeof(SSchema) * total);

  for(int32_t i = 0; i < msg->numOfColumns; ++i) {
    pTableMeta->tableInfo.rowSize += pTableMeta->schema[i].bytes;
  }

  *pMeta = pTableMeta;
  
  return TSDB_CODE_SUCCESS;
}


int32_t queryProcessTableMetaRsp(void* output, char *msg, int32_t msgSize) {
  STableMetaMsg *pMetaMsg = (STableMetaMsg *)msg;
  int32_t code = queryConvertTableMetaMsg(pMetaMsg);
  if (code != TSDB_CODE_SUCCESS) {
    return code;
  }

  STableMetaOutput *pOut = (STableMetaOutput *)output;
  
  if (!tIsValidSchema(pMetaMsg->pSchema, pMetaMsg->numOfColumns, pMetaMsg->numOfTags)) {
    qError("validate table meta schema in rsp msg failed");
    return TSDB_CODE_TSC_INVALID_VALUE;
  }

  if (pMetaMsg->tableType == TSDB_CHILD_TABLE) {
    pOut->metaNum = 2;
    
    memcpy(pOut->ctbFname, pMetaMsg->tbFname, sizeof(pOut->ctbFname));
    memcpy(pOut->tbFname, pMetaMsg->stbFname, sizeof(pOut->tbFname));
    
    pOut->ctbMeta.vgId = pMetaMsg->vgId;
    pOut->ctbMeta.tableType = pMetaMsg->tableType;
    pOut->ctbMeta.uid = pMetaMsg->tuid;
    pOut->ctbMeta.suid = pMetaMsg->suid;

    code = queryCreateTableMetaFromMsg(pMetaMsg, true, &pOut->tbMeta);
  } else {
    pOut->metaNum = 1;
    
    memcpy(pOut->tbFname, pMetaMsg->tbFname, sizeof(pOut->tbFname));
    
    code = queryCreateTableMetaFromMsg(pMetaMsg, false, &pOut->tbMeta);
  }
  
  return code;
}


void initQueryModuleMsgHandle() {
  queryBuildMsg[TDMT_VND_TABLE_META] = queryBuildTableMetaReqMsg;
  queryBuildMsg[TDMT_MND_USE_DB] = queryBuildUseDbMsg;

  queryProcessMsgRsp[TDMT_VND_TABLE_META] = queryProcessTableMetaRsp;
  queryProcessMsgRsp[TDMT_MND_USE_DB] = queryProcessUseDBRsp;
}




