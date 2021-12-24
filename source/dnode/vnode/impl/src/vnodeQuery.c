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

#include "vnodeDef.h"
#include "vnodeQuery.h"

int vnodeQueryOpen(SVnode *pVnode) {
  return qWorkerInit(NULL, &pVnode->pQuery);
}

int vnodeProcessQueryReq(SVnode *pVnode, SRpcMsg *pMsg, SRpcMsg **pRsp) {
  vInfo("query message is processed");
  qWorkerProcessQueryMsg(pVnode, pVnode->pQuery, pMsg);
  return 0;
}

int vnodeProcessFetchReq(SVnode *pVnode, SRpcMsg *pMsg, SRpcMsg **pRsp) {
  vInfo("fetch message is processed");
  qWorkerProcessFetchMsg(pVnode, pVnode->pQuery, pMsg);
  return 0;
}

