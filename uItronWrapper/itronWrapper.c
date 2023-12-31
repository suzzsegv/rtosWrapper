/*
 * itronWrapper.c - uITRON API wrapper for C11thread.
 */

/*
 * Copyright (c) 2023 Suzuki Satoshi
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of
 * this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *        wrote the original software. If you use this software in a product, an acknowledgment
 *        in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be 
 *        misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Copyright (c) 2023 鈴木 聡
 *
 * 本ソフトウェアは「現状のまま」で、明示であるか暗黙であるかを問わず、何らの保証もなく
 * 提供されます。 本ソフトウェアの使用によって生じるいかなる損害についても、作者は一切の責任を
 * 負わないものとします。
 *
 * 以下の制限に従う限り、商用アプリケーションを含めて、本ソフトウェアを任意の目的に使用し、
 * 自由に改変して再頒布することをすべての人に許可します。
 *
 *     1. 本ソフトウェアの出自について虚偽の表示をしてはなりません。あなたがオリジナルの
 *        ソフトウェアを作成したと主張してはなりません。 あなたが本ソフトウェアを製品内で使用
 *        する場合、製品の文書に謝辞を入れていただければ幸いですが、必須ではありません。
 *
 *     2. ソースを変更した場合は、そのことを明示しなければなりません。オリジナルのソフトウェア
 *        であるという虚偽の表示をしてはなりません。
 *
 *     3. ソースの頒布物から、この表示を削除したり、表示の内容を変更したりしてはなりません。
 */

#include "threads.h"
#include "timeSpecCalcLib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "itronWrapper.h"

/*
 * forward declarations
 */
void uitronWrapperInit(void);
ER cre_tsk(ID tskid, T_CTSK* pk_ctsk);
ER_ID acre_tsk(T_CTSK* pk_ctsk);
ER del_tsk(ID tskid);
ER act_tsk(ID tskid);
ER sta_tsk(ID tskid, VP_INT stacd);
void ext_tsk(void);
void exd_tsk(void);
ER ref_tsk(ID tskid, T_RTSK* pk_rtsk);
ER slp_tsk(void);
ER tslp_tsk(TMO tmout);
ER wup_tsk(ID tskid);
ER dly_tsk(RELTIM dlytim);
ER cre_sem(ID semid, T_CSEM* pk_csem);
ER del_sem(ID semid);
ER sig_sem(ID semid);
ER wai_sem(ID semid);
ER twai_sem(ID semid, TMO tmout);
ER cre_mbx(ID mbxid, T_CMBX* pk_cmbx);
ER snd_mbx(ID mbxid, T_MSG* pk_msg);
ER rcv_mbx(ID mbxid, T_MSG** ppk_msg);
ER trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO tmout);
ER get_tid(ID* p_tskid);


/***********************************************************************
 * uITRON Wrapper ライブラリの初期化
 */

/*!
 * uitronWrapperInit - uITRON Wrapper ライブラリの初期化
 */
void uitronWrapperInit(void)
{
	thrd_lib_init();
}

/***********************************************************************
 * タスク管理
 */
T_RTSK itronWrapper_taskInst[TW_MAX_TSKID];

/*!
 * cre_tsk - タスクの生成
 */
ER cre_tsk(ID taskId, T_CTSK* pCreTask)
{
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];

	if (pTask->tskstat != TTS_NONE) {
		return E_OBJ;
	}

	pTask->taskId = taskId;
	pTask->tskstat = TTS_DMT;
	pTask->tskpri = pCreTask->itskpri;
	pTask->tskbpri = pCreTask->itskpri;
	pTask->actcnt = 0;

	pTask->tskatr = pCreTask->tskatr;
	pTask->exinf = pCreTask->exinf;
	pTask->task = pCreTask->task;
	pTask->stksz = pCreTask->stksz;
	pTask->stk = pCreTask->stk;

	int rc = cnd_init(&pTask->c11cndSlp);
	if (rc != thrd_success) {
		return E_SYS;
	}
	rc = mtx_init(&pTask->c11mtxSlp, mtx_plain);
	if (rc == thrd_error) {
		cnd_destroy(&pTask->c11cndSlp);
		return E_SYS;
	}

	thrd_t* pThrd = &pTask->c11thrd;
	thrd_start_t pEntry = (thrd_start_t)pTask->task;
	void* pArg = (void*)pTask->exinf;

	if ((pCreTask->tskatr & TA_ACT) == TA_ACT) {
		pTask->tskstat = TTS_RUN;
		rc = thrd_create(pThrd, pEntry, pArg);
		if (rc == thrd_error) {
			pTask->tskstat = TTS_DMT;
			cnd_destroy(&pTask->c11cndSlp);
			mtx_destroy(&pTask->c11mtxSlp);
			return E_SYS;
		}

		char thrdName[16];
		snprintf(&thrdName[0], sizeof(thrdName), "%03d", taskId);
		thrd_name_set(pTask->c11thrd, &thrdName[0]);
	}

	return E_OK;
}

/*!
 * acre_tsk - タスクの生成（ID番号自動割付け）
 */
ER_ID acre_tsk(T_CTSK* pCreTask)
{
	int taskId;

	for (taskId = TW_MIN_AUTO_ASSIGN_TSKID; taskId < TW_MAX_TSKID; taskId++) {
		T_RTSK* pTask = &itronWrapper_taskInst[taskId];
		if (pTask->tskstat == TTS_NONE) {
			break;
		}
	}

	if (taskId >= TW_MAX_TSKID) {
		return E_NOID;
	}

	ER rc = cre_tsk(taskId, pCreTask);
	if (rc != E_OK) {
		return rc;
	}

	return taskId;
}


/*!
 * del_tsk - タスクの削除
 */
ER del_tsk(ID taskId)
{
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];

	if (pTask->tskstat == TTS_NONE) {
		return E_NOEXS;
	}

	if (pTask->tskstat != TTS_DMT) {
		return E_OBJ;
	}

	pTask->tskstat = TTS_NONE;

	return E_OK;
}

/*!
 * act_tsk - タスクの起動
 */
ER act_tsk(ID taskId)
{
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];

	if (pTask->tskstat != TTS_DMT) {
		return E_OBJ;
	}

	thrd_t* pThrd = &pTask->c11thrd;
	thrd_start_t pEntry = (thrd_start_t)pTask->task;
	void* pArg = (void*)pTask->exinf;

	pTask->tskstat = TTS_RUN;
	int rc = thrd_create(pThrd, pEntry, pArg);
	if (rc == thrd_error) {
		pTask->tskstat = TTS_DMT;
		return E_SYS;
	}

	char thrdName[16];
	snprintf(&thrdName[0], sizeof(thrdName), "%03d", taskId);
	thrd_name_set(pTask->c11thrd, &thrdName[0]);

	return E_OK;
}

/*!
 * sta_tsk - タスクの起動(起動コード指定)
 */
ER sta_tsk(ID taskId, VP_INT taskArg)
{
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];

	if (pTask->tskstat != TTS_DMT) {
		return E_OBJ;
	}

	thrd_start_t pEntry = (thrd_start_t)pTask->task;
	thrd_t* pThrd = &pTask->c11thrd;

	pTask->tskstat = TTS_RUN;
	int rc = thrd_create(pThrd, pEntry, (void*)taskArg);
	if (rc == thrd_error) {
		pTask->tskstat = TTS_DMT;
		return E_SYS;
	}

	char thrdName[16];
	snprintf(&thrdName[0], sizeof(thrdName), "%03d", taskId);
	thrd_name_set(pTask->c11thrd, &thrdName[0]);

	return E_OK;
}

/*!
 * ext_tsk - 自タスクの終了
 */
void ext_tsk(void)
{
	ID taskId;
	ER er = get_tid(&taskId);
	if (er == E_OK) {
		T_RTSK* pTask = &itronWrapper_taskInst[taskId];
		thrd_detach(pTask->c11thrd);
		pTask->tskstat = TTS_DMT;
	}

	thrd_exit(0);
}

/*!
 * exd_tsk - 自タスクの終了と削除
 */
void exd_tsk(void)
{
	ID taskId;
	ER er = get_tid(&taskId);
	if (er == E_OK) {
		T_RTSK* pTask = &itronWrapper_taskInst[taskId];
		thrd_detach(pTask->c11thrd);
		pTask->tskstat = TTS_NONE;
	}

	thrd_exit(0);
}

/*!
 * ref_tsk - タスクの状態参照
 */
ER ref_tsk(ID taskId, T_RTSK* pTask)
{
	if (taskId >= TW_MAX_TSKID) {
		return E_ID;
	}

	*pTask = itronWrapper_taskInst[taskId];

	if(pTask->tskstat == TTS_NONE){
		return E_NOEXS;
	}

	return E_OK;
}

/*!
 * slp_tsk - 起床待ち
 */
ER slp_tsk(void)
{
	ID taskId;
	ER er = get_tid(&taskId);
	if (er != E_OK) {
		return E_SYS;
	}

	int rc;
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];
	mtx_lock(&pTask->c11mtxSlp);
	{
		rc = cnd_wait(&pTask->c11cndSlp, &pTask->c11mtxSlp);
	}
	mtx_unlock(&pTask->c11mtxSlp);

	if (rc != thrd_success) {
		return E_SYS;
	}

	return E_OK;
}

/*!
 * tslp_tsk - 起床待ち
 */
ER tslp_tsk(TMO waitMs)
{
	if (waitMs == TMO_FEVR) {
		return slp_tsk();
	}

	ID taskId;
	ER er = get_tid(&taskId);
	if (er != E_OK) {
		return E_SYS;
	}

	int rc;
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];
	mtx_lock(&pTask->c11mtxSlp);
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		timespecAddMs(&ts, waitMs);

		rc = cnd_timedwait(&pTask->c11cndSlp, &pTask->c11mtxSlp, &ts);
	}
	mtx_unlock(&pTask->c11mtxSlp);

	if (rc == thrd_timedout) {
		return E_TMOUT;
	}
	if (rc != thrd_success) {
		return E_SYS;
	}

	return E_OK;
}

/*!
 * wup_tsk - タスクの起床
 */
ER wup_tsk(ID taskId)
{
	T_RTSK* pTask = &itronWrapper_taskInst[taskId];
	int rc = cnd_signal(&pTask->c11cndSlp);
	if (rc != thrd_success) {
		return E_SYS;
	}

	return E_OK;
}

/*!
 * dly_tsk - 自タスクの遅延
 */
ER dly_tsk(RELTIM delayMs)
{
	struct timespec duration = { .tv_sec = (delayMs / 1000), .tv_nsec = (delayMs % 1000) * 1000000 };
	thrd_sleep(&duration, NULL);

	return E_OK;
}

/***********************************************************************
 * セマフォ
 */
T_CSEM itronWrapper_semInst[TMAX_SEMID];

/*!
 * cre_sem - セマフォの生成
 */
ER cre_sem(ID semid, T_CSEM* pCreSem)
{
	if (semid >= TMAX_SEMID) {
		return E_ID;
	}

	if (pCreSem->sematr != TA_TFIFO) {
		return E_PAR;
	}

	if (pCreSem->maxsem >= TMAX_MAXSEM) {
		return E_PAR;
	}

	if (pCreSem->isemcnt > pCreSem->maxsem) {
		return E_PAR;
	}

	T_CSEM* pSem = &itronWrapper_semInst[semid];
	pSem->sematr = pCreSem->sematr;
	pSem->isemcnt = pCreSem->isemcnt;
	pSem->maxsem = pCreSem->maxsem;
	pSem->curSemCnt = pCreSem->isemcnt;

	cnd_t* pC11Cnd = &pSem->c11cnd;
	int rc = cnd_init(pC11Cnd);
	if (rc != thrd_success) {
		return E_SYS;
	}

	mtx_t mtx;
	rc = mtx_init(&mtx, mtx_plain);
	if (rc == thrd_error) {
		cnd_destroy(pC11Cnd);
		return E_SYS;
	}
	pSem->c11mtx = mtx;

	return E_OK;
}

/*!
 * del_sem - セマフォの削除
 */
ER del_sem(ID semid)
{
	if (semid >= TMAX_SEMID) {
		return E_ID;
	}

	T_CSEM* pSem = &itronWrapper_semInst[semid];
	mtx_destroy(&pSem->c11mtx);
	cnd_destroy(&pSem->c11cnd);

	return E_OK;
}

/*!
 * sig_sem - セマフォ資源の返却
 */
ER sig_sem(ID semid)
{
	if (semid >= TMAX_SEMID) {
		return E_ID;
	}

	T_CSEM* pSem = &itronWrapper_semInst[semid];
	mtx_lock(&pSem->c11mtx);
	{
		if (pSem->curSemCnt >= pSem->maxsem) {
			mtx_unlock(&pSem->c11mtx);
			return E_QOVR;
		}
		pSem->curSemCnt++;
	}
	mtx_unlock(&pSem->c11mtx);
	cnd_broadcast(&pSem->c11cnd);

	return E_OK;
}

/*!
 * wai_sem - セマフォ資源の獲得
 */
ER wai_sem(ID semid)
{
	if (semid >= TMAX_SEMID) {
		return E_ID;
	}

	T_CSEM* pSem = &itronWrapper_semInst[semid];
	mtx_lock(&pSem->c11mtx);
	{
		while (pSem->curSemCnt == 0) {
			int rc = cnd_wait(&pSem->c11cnd, &pSem->c11mtx);
			if (rc == thrd_error) {
				mtx_unlock(&pSem->c11mtx);
				return E_SYS;
			}
		}
		pSem->curSemCnt--;
	}
	mtx_unlock(&pSem->c11mtx);

	return E_OK;
}

/*!
 * twai_sem - セマフォ資源の獲得（タイムアウトあり）
 */
ER twai_sem(ID semid, TMO timeoutMs)
{
	if (timeoutMs == TMO_FEVR) {
		return wai_sem(semid);
	}

	if (semid >= TMAX_SEMID) {
		return E_ID;
	}

	T_CSEM* pSem = &itronWrapper_semInst[semid];
	mtx_lock(&pSem->c11mtx);
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		timespecAddMs(&ts, timeoutMs);

		while (pSem->curSemCnt == 0) {
			int rc = cnd_timedwait(&pSem->c11cnd, &pSem->c11mtx, &ts);
			if (rc == thrd_timedout) {
				mtx_unlock(&pSem->c11mtx);
				return E_TMOUT;
			}
			if (rc == thrd_error) {
				mtx_unlock(&pSem->c11mtx);
				return E_SYS;
			}
		}
		pSem->curSemCnt--;
	}
	mtx_unlock(&pSem->c11mtx);

	return E_OK;
}

/***********************************************************************
 * メールボックス
 */
T_RMBX itronWrapper_mbxInst[TMAX_SEMID];

/*!
 * cre_mbx - メールボックスの生成
 */
ER cre_mbx(ID mbxid, T_CMBX* pk_cmbx)
{
	if (mbxid >= TMAX_MBXID) {
		return E_ID;
	}

	T_RMBX* pMbox = &itronWrapper_mbxInst[mbxid];

	pMbox->mbxData = malloc(sizeof(void*) * TMAX_MAXMBX);
	pMbox->num = 0;
	pMbox->head = 0;
	pMbox->tail = 0;
	mtx_init(&pMbox->mutex, mtx_plain);
	cnd_init(&pMbox->condRecv);

	return E_OK;
}

/*!
 * snd_mbx - メールボックスへの送信
 */
ER snd_mbx(ID mbxid, T_MSG* pk_msg)
{
	if (mbxid >= TMAX_MBXID) {
		return E_ID;
	}

	T_RMBX* pMbox = &itronWrapper_mbxInst[mbxid];
	mtx_lock(&pMbox->mutex);
	{
		if (pMbox->num >= TMAX_MAXMBX) {
			mtx_unlock(&pMbox->mutex);
			return E_SYS;
		}
		pMbox->mbxData[pMbox->tail] = pk_msg;
		pMbox->tail = (pMbox->tail + 1) % TMAX_MAXMBX;
		pMbox->num++;
	}
	mtx_unlock(&pMbox->mutex);
	cnd_signal(&pMbox->condRecv);

	return E_OK;
}

/*!
 * rcv_mbx - メールボックスからの受信
 */
ER rcv_mbx(ID mbxid, T_MSG** ppk_msg)
{
	if (mbxid >= TMAX_MBXID) {
		return E_ID;
	}

	T_RMBX* pMbox = &itronWrapper_mbxInst[mbxid];
	mtx_lock(&pMbox->mutex);
	{
		while (pMbox->num == 0) {
			int rc = cnd_wait(&pMbox->condRecv, &pMbox->mutex);
			if (rc == thrd_error) {
				mtx_unlock(&pMbox->mutex);
				return E_SYS;
			}
		}
		*ppk_msg = pMbox->mbxData[pMbox->head];
		pMbox->head = (pMbox->head + 1) % TMAX_MAXMBX;
		pMbox->num--;
	}
	mtx_unlock(&pMbox->mutex);

	return E_OK;
}

/*!
 * trcv_mbx - メールボックスからの受信（タイムアウトあり）
 */
ER trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO timeoutMs)
{
	if (timeoutMs == TMO_FEVR) {
		return rcv_mbx(mbxid, ppk_msg);
	}

	if (mbxid >= TMAX_MBXID) {
		return E_ID;
	}

	T_RMBX* pMbox = &itronWrapper_mbxInst[mbxid];
	mtx_lock(&pMbox->mutex);
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		timespecAddMs(&ts, timeoutMs);

		while (pMbox->num == 0) {
			int rc = cnd_timedwait(&pMbox->condRecv, &pMbox->mutex, &ts);
			if (rc == thrd_timedout) {
				mtx_unlock(&pMbox->mutex);
				return E_TMOUT;
			}
			if (rc == thrd_error) {
				mtx_unlock(&pMbox->mutex);
				return E_SYS;
			}
		}
		*ppk_msg = pMbox->mbxData[pMbox->head];
		pMbox->head = (pMbox->head + 1) % TMAX_MAXMBX;
		pMbox->num--;
	}
	mtx_unlock(&pMbox->mutex);

	return E_OK;
}

/*!
 * get_tid - 実行状態のタスク ID の参照
 */
ER get_tid(ID* pTaskId)
{
	thrd_t curThr = thrd_current();
	for (int taskId = 0; taskId < TW_MAX_TSKID; taskId++) {
		T_RTSK* pTask = &itronWrapper_taskInst[taskId];
		if ((pTask->tskstat != TTS_NONE)
		 && (thrd_equal(curThr, pTask->c11thrd) == true)) {
			if (taskId == pTask->taskId) {
				*pTaskId = taskId;
				return E_OK;
			}
		}
	}

	return E_PAR;
}

