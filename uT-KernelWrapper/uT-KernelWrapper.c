/*
 * uT-KernelWrapper.c - uT-Kernel API wrapper for C11thread.
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "threads.h"
#include "timeSpecCalcLib.h"

#include "uT-KernelWrapper.h"


/*
 * typedefs
 */
typedef int (*tkWrapTaskEntry)(INT startCode, void* exinf);
typedef struct {
	T_RTSK taskInst[TKW_MAX_TSKID];
	T_RSEM semInst[TKW_MAX_SEMID];
	T_RMBX mbxInst[TKW_MAX_MBXID];
	mtx_t mutex;
} UTkernelWrapper;

/*
 * static valiables
 */
static UTkernelWrapper utkWrapperInst;

/*
 * forward declarations
 */
ER utkWrapperInit(void);

ID tk_cre_tsk(CONST T_CTSK *pCreTask);
ER tk_del_tsk(ID taskId);
ER tk_sta_tsk(ID taskId, INT startCode);
static int tkWrapTaskEntryFunc(void* pArg);
void tk_ext_tsk(void);
void tk_exd_tsk(void);
ER tk_ref_tsk(ID taskId, T_RTSK* pTask);
ER tk_slp_tsk(TMO waitMs);
static ER slp_tsk_wait_forever(void);
ER tk_wup_tsk(ID taskId);
ER tk_dly_tsk(RELTIM delayMs);

ER tk_cre_sem(T_CSEM* pCreSem);
ER tk_del_sem(ID semId);
ER tk_sig_sem(ID semId, INT count);
ER tk_wai_sem(ID semId, INT count, TMO timeoutMs);
static ER tk_wai_sem_no_wait(ID semId, INT count);
static ER tk_wai_sem_wait_forever(ID semId, INT count);

ID tk_cre_mbx(CONST T_CMBX* pCreMbx);
ER tk_snd_mbx(ID mbxId, T_MSG* pMsg);
ER tk_rcv_mbx(ID mbxId, T_MSG** ppMsg, TMO timeoutMs);
static ER tk_rcv_mbx_wait_forever(ID mbxId, T_MSG** ppMsg);
static ER tk_rcv_mbx_no_wait(ID mbxId, T_MSG** ppMsg);

ID tk_get_tid(void);

/***********************************************************************
 * uT-Kernel Wrapper ライブラリの初期化
 */

/***********************************************************************
 * utkWrapperInit - uT-Kernel Wrapper ライブラリの初期化
 */
ER utkWrapperInit(void)
{
	thrd_lib_init();

	int rc = mtx_init(&utkWrapperInst.mutex, mtx_plain);
	if (rc == thrd_error) {
		return E_SYS;
	}

	return E_OK;
}

/***********************************************************************
 * タスク
 */

/*!
 * tk_cre_tsk - タスク生成
 */
ID tk_cre_tsk(CONST T_CTSK *pCreTask)
{
	int taskId;
	T_RTSK* pTask;

	mtx_lock(&utkWrapperInst.mutex);
	{
		for (taskId = 1; taskId < TKW_MAX_TSKID; taskId++) {
			pTask = &utkWrapperInst.taskInst[taskId];
			if (pTask->tskstat == TTS_NONE) {
				pTask->tskstat = TTS_DMT;
				break;
			}
		}
	}
	mtx_unlock(&utkWrapperInst.mutex);

	if (taskId >= TKW_MAX_TSKID) {
		return E_NOMEM;
	}

	pTask->taskId = taskId;
	pTask->ctsk = *pCreTask;

	pTask->exinf = pCreTask->exinf;
	pTask->tskpri = pCreTask->itskpri;
	pTask->tskbpri = pCreTask->itskpri;
	pTask->tskwait = 0;
	pTask->wid = 0;
	pTask->wupcnt = 0;
	pTask->suscnt = 0;
	pTask->waitmask = 0;
	pTask->texmask = 0;
	pTask->tskevent = 0;
	pTask->startCode = -1;

	mtx_init(&pTask->c11mtxSlp, mtx_plain);
	cnd_init(&pTask->c11cndSlp);

	return taskId;
}

/*!
 * tk_del_tsk - タスク削除
 */
ER tk_del_tsk(ID taskId)
{
	if ((taskId <= 0) || (taskId >= TKW_MAX_TSKID)) {
		return E_ID;
	}

	T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];

	if (pTask->tskstat == TTS_NONE) {
		return E_NOEXS;
	}

	if (pTask->tskstat != TTS_DMT) {
		return E_OBJ;
	}

	// thrd_cancel() のような機能がないため、別スレッドの削除は実装できない。

	int rc = thrd_join(pTask->c11thrd, NULL);
	if (rc != thrd_success) {
		return E_SYS;
	}

	return E_OK;
}

/*!
 * tk_sta_tsk - タスク起動
 */
ER tk_sta_tsk(ID taskId, INT startCode)
{
	if ((taskId <= 0) || (taskId >= TKW_MAX_TSKID)) {
		return E_ID;
	}

	T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
	if (pTask->tskstat != TTS_DMT) {
		return E_OBJ;
	}

	pTask->startCode = startCode;
	thrd_t* pThrd = &pTask->c11thrd;
	int rc = thrd_create(pThrd, tkWrapTaskEntryFunc, (void*)pTask);
	if (rc == thrd_error) {
		return E_SYS;
	}

	// タスク名の設定: 指定された名前、または、タスクID の値を文字列に変換してタスク名とする.
	char thrdName[16];
	T_CTSK *pCreTask = &pTask->ctsk;
	if ((pCreTask->tskatr & TA_DSNAME) == TA_DSNAME) {
		strncpy_s(&thrdName[0], sizeof(thrdName), &pCreTask->dsname[0], sizeof(pCreTask->dsname));
	} else {
		char thrdName[16];
		snprintf(&thrdName[0], sizeof(thrdName), "%03d", taskId);
	}
	thrd_name_set(pTask->c11thrd, &thrdName[0]);

	return E_OK;
}

/*!
 * tkWrapTaskEntryFunc - T-Kernel Wrapper スレッド エントリー処理
 *
 * @param pArg タスク インスタンス
 *
 * @return エントリ関数の戻り値
 */
static int tkWrapTaskEntryFunc(void* pArg)
{
	T_RTSK* pTask = (T_RTSK*)pArg;

	pTask->tskstat = TTS_RUN;

	tkWrapTaskEntry pTkTaskEntry = (tkWrapTaskEntry)pTask->ctsk.task;
	int rc = pTkTaskEntry(pTask->startCode, pTask->exinf);

	thrd_detach(pTask->c11thrd);

	return rc;
}

/*!
 * tk_ext_tsk - 自タスク終了
 */
void tk_ext_tsk(void)
{
	ID taskId = tk_get_tid();
	if (taskId != 0) {
		T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
		thrd_detach(pTask->c11thrd);
		pTask->tskstat = TTS_DMT;
	}

	thrd_exit(0); // TODO: ラッパーで作成したスレッドではないので、終了すべきではない。
}

/*!
 * tk_exd_tsk - 自タスクの終了と削除
 */
void tk_exd_tsk(void)
{
	ID taskId = tk_get_tid();
	if (taskId != 0) {
		T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
		thrd_detach(pTask->c11thrd);
		pTask->tskstat = TTS_NONE;
	}

	thrd_exit(0);
}

/*!
 * tk_ref_tsk - タスクの状態参照
 */
ER tk_ref_tsk(ID taskId, T_RTSK* pTask)
{
	if (taskId == TSK_SELF) {
		taskId = tk_get_tid();
	}

	if ((taskId <= 0) || (taskId >= TKW_MAX_TSKID)) {
		return E_ID;
	}

	*pTask = utkWrapperInst.taskInst[taskId];
	if(pTask->tskstat == TTS_NONE){
		return E_NOEXS;
	}

	return E_OK;
}

/*!
 * tk_slp_tsk - 自タスクを起床待ち状態へ移行
 */
ER tk_slp_tsk(TMO waitMs)
{
	if (waitMs == TMO_FEVR) {
		return slp_tsk_wait_forever();
	}

	ID taskId = tk_get_tid();
	if (taskId == 0) {
		return E_SYS;
	}

	int rc;
	T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
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
 * slp_tsk_wait_forever - 自タスクを起床待ち状態へ移行
 */
static ER slp_tsk_wait_forever(void)
{
	ID taskId = tk_get_tid();
	if (taskId == 0) {
		return E_SYS;
	}

	int rc;
	T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
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
 * tk_wup_tsk - 他タスクの起床
 */
ER tk_wup_tsk(ID taskId)
{
	if ((taskId <= 0) || (taskId >= TKW_MAX_TSKID)) {
		return E_ID;
	}

	T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
	int rc = cnd_signal(&pTask->c11cndSlp);
	if (rc != thrd_success) {
		return E_SYS;
	}

	return E_OK;
}

/*!
 * tk_dly_tsk - タスク遅延
 */
ER tk_dly_tsk(RELTIM delayMs)
{
	struct timespec duration = { .tv_sec = (delayMs / 1000), .tv_nsec = (delayMs % 1000) * 1000000 };
	thrd_sleep(&duration, NULL);

	return E_OK;
}

/***********************************************************************
 * セマフォ
 */

/*!
 * tk_cre_sem - セマフォの生成
 */
ID tk_cre_sem(T_CSEM* pCreSem)
{
	if (pCreSem->maxsem >= TKW_MAX_MAXSEM) {
		return E_PAR;
	}

	if (pCreSem->isemcnt > TKW_MAX_MAXSEM) {
		return E_PAR;
	}

	int semId;
	T_RSEM* pSem;

	mtx_lock(&utkWrapperInst.mutex);
	{
		for (semId = 1; semId < TKW_MAX_SEMID; semId++) {
			pSem = &utkWrapperInst.semInst[semId];
			if (pSem->valid == false) {
				pSem->valid = true;
				break;
			}
		}
	}
	mtx_unlock(&utkWrapperInst.mutex);

	if (semId >= TKW_MAX_SEMID) {
		return E_LIMIT;
	}

	pSem->exinf = pCreSem->exinf;
	pSem->wtsk = 0;
	pSem->semcnt = pCreSem->isemcnt;

	pSem->csem = *pCreSem;
	pSem->semId = semId;
	pSem->maxSemCnt = pCreSem->maxsem;

	cnd_init(&pSem->c11cnd);
	mtx_init(&pSem->c11mtx, mtx_plain);

	return semId;
}

/*!
 * tk_del_sem - セマフォ削除
 */
ER tk_del_sem(ID semId)
{
	if ((semId <= 0) || (semId >= TKW_MAX_SEMID)) {
		return E_ID;
	}

	T_RSEM* pSem = &utkWrapperInst.semInst[semId];
	mtx_lock(&utkWrapperInst.mutex);
	{
		if (pSem->valid == false) {
			mtx_unlock(&utkWrapperInst.mutex);
			return E_NOEXS;
		}

		mtx_destroy(&pSem->c11mtx);
		cnd_destroy(&pSem->c11cnd);
	}
	mtx_unlock(&utkWrapperInst.mutex);

	return E_OK;
}

/*!
 * tk_sig_sem - セマフォ資源返却
 */
ER tk_sig_sem(ID semId, INT count)
{
	if ((semId <= 0) || (semId >= TKW_MAX_SEMID)) {
		return E_ID;
	}

	if (count <= 0) {
		return E_PAR;
	}

	T_RSEM* pSem = &utkWrapperInst.semInst[semId];
	if (pSem->valid == false) {
		return E_NOEXS;
	}

	mtx_lock(&pSem->c11mtx);
	{
		if (count > (pSem->maxSemCnt - pSem->semcnt)) {
			mtx_unlock(&pSem->c11mtx);
			return E_QOVR;
		}
		pSem->semcnt += count;
	}
	mtx_unlock(&pSem->c11mtx);
	cnd_broadcast(&pSem->c11cnd);

	return E_OK;
}


/*!
 * tk_wai_sem - セマフォ資源獲得
 */
ER tk_wai_sem(ID semId, INT count, TMO timeoutMs)
{
	if ((semId <= 0) || (semId >= TKW_MAX_SEMID)) {
		return E_ID;
	}

	T_RSEM* pSem = &utkWrapperInst.semInst[semId];
	if (pSem->valid == false) {
		return E_NOEXS;
	}

	if (count <= 0) {
		return E_PAR;
	}

	if (timeoutMs == TMO_FEVR) {
		return tk_wai_sem_wait_forever(semId, count);
	}

	if (timeoutMs == TMO_POL) {
		return tk_wai_sem_no_wait(semId, count);
	}

	mtx_lock(&pSem->c11mtx);
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		timespecAddMs(&ts, timeoutMs);

		while (pSem->semcnt < count) {
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
		pSem->semcnt -= count;
	}
	mtx_unlock(&pSem->c11mtx);

	return E_OK;
}

/*!
 * tk_wai_sem_no_wait - セマフォ資源獲得(待ち無し)
 */
static ER tk_wai_sem_no_wait(ID semId, INT count)
{
	T_RSEM* pSem = &utkWrapperInst.semInst[semId];
	mtx_lock(&pSem->c11mtx);
	{
		if (pSem->semcnt < count) {
			mtx_unlock(&pSem->c11mtx);
			return E_TMOUT;
		}
		pSem->semcnt -= count;
	}
	mtx_unlock(&pSem->c11mtx);

	return E_OK;
}

/*!
 * tk_wai_sem_wait_forever - セマフォ資源獲得(無限待ち)
 */
static ER tk_wai_sem_wait_forever(ID semId, INT count)
{
	T_RSEM* pSem = &utkWrapperInst.semInst[semId];
	mtx_lock(&pSem->c11mtx);
	{
		while (pSem->semcnt < count) {
			int rc = cnd_wait(&pSem->c11cnd, &pSem->c11mtx);
			if (rc == thrd_error) {
				mtx_unlock(&pSem->c11mtx);
				return E_SYS;
			}
		}
		pSem->semcnt -= count;
	}
	mtx_unlock(&pSem->c11mtx);

	return E_OK;
}

/***********************************************************************
 * メールボックス
 */

/*!
 * tk_cre_mbx - メールボックス生成
 */
ID tk_cre_mbx(CONST T_CMBX* pCreMbx)
{
	int mbxId;
	T_RMBX* pMbx;

	mtx_lock(&utkWrapperInst.mutex);
	{
		for (mbxId = 1; mbxId < TKW_MAX_MBXID; mbxId++) {
			pMbx = &utkWrapperInst.mbxInst[mbxId];
			if (pMbx->valid == false) {
				pMbx->valid = true;
				break;
			}
		}
	}
	mtx_unlock(&utkWrapperInst.mutex);

	if (mbxId >= TKW_MAX_MBXID) {
		return E_LIMIT;
	}

	pMbx->exinf = pCreMbx->exinf;
	pMbx->wtsk = 0;
	pMbx->pk_msg = NULL;

	pMbx->cmbx = *pCreMbx;
	pMbx->mbxData = malloc(sizeof(void*) * TKW_MAX_MAXMBX);
	pMbx->msgNum = 0;
	pMbx->head = 0;
	pMbx->tail = 0;

	mtx_init(&pMbx->c11mtx, mtx_plain);
	cnd_init(&pMbx->c11cndRecv);

	return mbxId;
}

/*!
 * snd_mbx - メールボックスへの送信
 *
 * uITRON 系 OS のメールボックスへの送信の実装は、リンクリストへの追加であるため、
 * 送信バッファフルは発生しない。しかし、本ラッパーの実装はリングバッファを
 * 用いているため、バッファフルが発生するが、この場合、システムエラー(E_SYS)を
 * 返している。
 */
ER tk_snd_mbx(ID mbxId, T_MSG* pMsg)
{
	if ((mbxId <= 0) || (mbxId >= TKW_MAX_MBXID)) {
		return E_ID;
	}

	T_RMBX* pMbx = &utkWrapperInst.mbxInst[mbxId];
	if (pMbx->valid == false) {
		return E_NOEXS;
	}

	mtx_lock(&pMbx->c11mtx);
	{
		if (pMbx->msgNum >= TKW_MAX_MAXMBX) {
			mtx_unlock(&pMbx->c11mtx);
			return E_SYS;
		}
		pMbx->mbxData[pMbx->tail] = pMsg;
		pMbx->tail = (pMbx->tail + 1) % TKW_MAX_MAXMBX;
		pMbx->msgNum++;
	}
	mtx_unlock(&pMbx->c11mtx);
	cnd_signal(&pMbx->c11cndRecv);

	return E_OK;
}

/*!
 * tk_rcv_mbx - メールボックスから受信
 */
ER tk_rcv_mbx(ID mbxId, T_MSG** ppMsg, TMO timeoutMs)
{
	if ((mbxId <= 0) || (mbxId >= TKW_MAX_MBXID)) {
		return E_ID;
	}

	T_RMBX* pMbx = &utkWrapperInst.mbxInst[mbxId];
	if (pMbx->valid == false) {
		return E_NOEXS;
	}

	if (timeoutMs == TMO_FEVR) {
		return tk_rcv_mbx_wait_forever(mbxId, ppMsg);
	}

	if (timeoutMs == TMO_POL) {
		return tk_rcv_mbx_no_wait(mbxId, ppMsg);
	}

	mtx_lock(&pMbx->c11mtx);
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		timespecAddMs(&ts, timeoutMs);

		while (pMbx->msgNum == 0) {
			int rc = cnd_timedwait(&pMbx->c11cndRecv, &pMbx->c11mtx, &ts);
			if (rc == thrd_timedout) {
				mtx_unlock(&pMbx->c11mtx);
				return E_TMOUT;
			}
			if (rc == thrd_error) {
				mtx_unlock(&pMbx->c11mtx);
				return E_SYS;
			}
		}
		*ppMsg = pMbx->mbxData[pMbx->head];
		pMbx->head = (pMbx->head + 1) % TKW_MAX_MAXMBX;
		pMbx->msgNum--;
	}
	mtx_unlock(&pMbx->c11mtx);

	return E_OK;
}

/*!
 * tk_rcv_mbx_wait_forever - メールボックスから受信(無限待ち)
 */
static ER tk_rcv_mbx_wait_forever(ID mbxId, T_MSG** ppMsg)
{
	T_RMBX* pMbx = &utkWrapperInst.mbxInst[mbxId];
	mtx_lock(&pMbx->c11mtx);
	{
		while (pMbx->msgNum == 0) {
			int rc = cnd_wait(&pMbx->c11cndRecv, &pMbx->c11mtx);
			if (rc == thrd_error) {
				mtx_unlock(&pMbx->c11mtx);
				return E_SYS;
			}
		}
		*ppMsg = pMbx->mbxData[pMbx->head];
		pMbx->head = (pMbx->head + 1) % TKW_MAX_MAXMBX;
		pMbx->msgNum--;
	}
	mtx_unlock(&pMbx->c11mtx);

	return E_OK;
}

/*!
 * tk_rcv_mbx_no_wait - メールボックスから受信(待ち無し)
 */
static ER tk_rcv_mbx_no_wait(ID mbxId, T_MSG** ppMsg)
{
	T_RMBX* pMbx = &utkWrapperInst.mbxInst[mbxId];
	mtx_lock(&pMbx->c11mtx);
	{
		if (pMbx->msgNum == 0) {
			mtx_unlock(&pMbx->c11mtx);
			return E_TMOUT;
		}

		*ppMsg = pMbx->mbxData[pMbx->head];
		pMbx->head = (pMbx->head + 1) % TKW_MAX_MAXMBX;
		pMbx->msgNum--;
	}
	mtx_unlock(&pMbx->c11mtx);

	return E_OK;
}

/*!
 * tk_get_tid - 実行状態タスクのタスクID参照
 */
ID tk_get_tid(void)
{
	thrd_t curThr = thrd_current();
	for (int taskId = 0; taskId < TKW_MAX_TSKID; taskId++) {
		T_RTSK* pTask = &utkWrapperInst.taskInst[taskId];
		if ((pTask->tskstat != TTS_NONE)
		 && (thrd_equal(curThr, pTask->c11thrd) == true)) {
			if (taskId == pTask->taskId) {
				return taskId;
			}
		}
	}

	return 0;
}

