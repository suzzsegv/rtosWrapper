/*
 * vxWorksWrapper.c - VxWorks API wrapper for C11thread.
 */
/*
 * Copyright (c) 2023-2024 Suzuki Satoshi
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
 * Copyright (c) 2023-2024 鈴木 聡
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

#include "vxWorksWrapper.h"


/*
 * defines
 */
#define VW_TASK_NUM_MAX 256
#define VW_TICKS_PER_SECOND 60
#define VW_TICKS_TO_MS(ticks) ((ticks * 1000) / VW_TICKS_PER_SECOND)

/*
 * typedefs
 */
typedef struct {
	char name[32];
	int priority;
	int options;
	size_t stackSize;
	FUNCPTR pEntry;
	uintptr_t args[10];

	bool valid;
	SEM_ID pendSemId;
	bool pendSemFlushed;
	thrd_t c11thrd;
} VxWrapTask;

typedef struct {
	VxWrapTask vxWrapTask[VW_TASK_NUM_MAX];
	int vwTaskNum;
	mtx_t mutex;
} VxWrapperTaskLib;

typedef int (*vxWrapTaskEntry)(
		_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
		_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10
	);

typedef enum {
	VXW_SEM_TYPE_BINARY = 0,
	VXW_SEM_TYPE_MUTEX = 1,
	VXW_SEM_TYPE_COUNT = 2,
} VxWrapSemType;

typedef struct {
	VxWrapSemType type;
	int options;

	int curSemCnt;
	int maxSemCnt;
	TASK_ID semOwner;
	cnd_t c11cnd;
	mtx_t c11mtx;
} VxWrapSem;

typedef struct {
	size_t msgDataSize;
	uint8_t msgData[];
} VxWrapMsgData;

typedef struct {
	size_t maxMsgNum;
	size_t maxMsgSize;
	int options;

	uint8_t* pMsgData;
	size_t vwMsgDataSize;
	uint32_t msgNum;
	uint32_t msgQueHead;
	uint32_t msgQueTail;
	mtx_t mutex;
	cnd_t cndSend;
	cnd_t cndRecv;
} VxWrapMsgQue;


/*
 * static valiables
 */
VxWrapperTaskLib vxWrapper_taskLibInst;

/*
 * forward declarations
 */
STATUS taskLibInit(void);
TASK_ID taskSpawn(char* name, int priority, int options, size_t stackSize, FUNCPTR entryPt,
	_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
	_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10);
TASK_ID taskCreate(char* name, int priority, int options, size_t stackSize, FUNCPTR entryPt,
	_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
	_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10);
STATUS taskActivate(TASK_ID tid);
TASK_ID taskIdSelf(void);
char* taskName(TASK_ID tid);
STATUS taskDelay(_Vx_ticks_t ticks);
static int vxWrapTaskEntryFunc(void* pArg);
static inline void task_pendSemIdSet(SEM_ID semId, TASK_ID taskId);
static inline void task_pendSemIdClear(TASK_ID taskId);
static inline bool task_pendSemIsFlushed(TASK_ID taskId);
static inline void task_semFlushAccept(TASK_ID taskId);

SEM_ID semBCreate(int options, SEM_B_STATE initialState);
static STATUS semBTake(SEM_ID semId, _Vx_ticks_t timeoutTicks);
static STATUS semBTakeNoWait(SEM_ID semId);
static STATUS semBTakeWaitForever(SEM_ID semId);
static STATUS semBTakeTimedWait(SEM_ID semId, _Vx_ticks_t timeoutTicks);
static STATUS semBGive(SEM_ID semId);

SEM_ID semMCreate(int options);
static STATUS semMTake(SEM_ID semId, _Vx_ticks_t timeoutTicks);
static STATUS semMTakeNoWait(SEM_ID semId);
static STATUS semMTakeWaitForever(SEM_ID semId);
static STATUS semMTakeTimedWait(SEM_ID semId, _Vx_ticks_t timeoutTicks);
static inline STATUS semMTake_tryLock(VxWrapSem* pVxwSem, TASK_ID curTaskId);
static STATUS semMGive(SEM_ID semId);

STATUS semTake(SEM_ID semId, _Vx_ticks_t timeoutTicks);
STATUS semGive(SEM_ID semId);
STATUS semFlush(SEM_ID semId);
static STATUS semBFlush(SEM_ID semId);

MSG_Q_ID msgQCreate(size_t maxMsgNum, size_t maxMsgSize, int options);
STATUS msgQSend(MSG_Q_ID msgQId, char* pMsg, size_t msgSize, _Vx_ticks_t timeoutTicks, int priority);
static STATUS msgQSendNoWait(MSG_Q_ID msgQId, char* pMsg, size_t msgSize);
static STATUS msgQSendWaitForever(MSG_Q_ID msgQId, char* pMsg, size_t msgSize);
static STATUS msgQSendTimedWait(MSG_Q_ID msgQId, char* pMsg, size_t msgSize, _Vx_ticks_t timeoutTicks);
ssize_t msgQReceive(MSG_Q_ID msgQId, char* pBuf, size_t bufSize, _Vx_ticks_t timeoutTicks);
static ssize_t msgQReceiveNoWait(MSG_Q_ID msgQId, char* pBuf, size_t bufSize);
static ssize_t msgQReceiveWaitForever(MSG_Q_ID msgQId, char* pBuf, size_t bufSize);
static ssize_t msgQReceiveTimedWait(MSG_Q_ID msgQId, char* pBuf, size_t bufSize, _Vx_ticks_t timeoutTicks);
static inline void msgQue_enqueue(VxWrapMsgQue* pMsgQue, char* pMsg, size_t msgSize);
static inline ssize_t msgQue_dequeue(VxWrapMsgQue* pMsgQue, char* pBuf, size_t bufSize);


/*!
 * taskLibInit - タスクライブラリの初期化
 */
STATUS taskLibInit(void)
{
	thrd_lib_init();

	int rc = mtx_init(&vxWrapper_taskLibInst.mutex, mtx_plain);
	if (rc == thrd_error) {
		return ERROR;
	}

	return OK;
}

/*!
 * taskSpawn - タスク生成
 */
TASK_ID taskSpawn(char* name, int priority, int options, size_t stackSize, FUNCPTR pEntry,
	_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
	_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10)
{
	int taskIx;
	VxWrapTask* pVxwTask;

	mtx_lock(&vxWrapper_taskLibInst.mutex);
	{
		for (taskIx = 0; taskIx < VW_TASK_NUM_MAX; taskIx++) {
			pVxwTask = &vxWrapper_taskLibInst.vxWrapTask[taskIx];
			if (pVxwTask->valid == false) {
				break;
			}
		}

		if (taskIx >= VW_TASK_NUM_MAX) {
			mtx_unlock(&vxWrapper_taskLibInst.mutex);
			return ERROR;
		}

		pVxwTask->c11thrd = 0;
		pVxwTask->valid = true;
	}
	mtx_unlock(&vxWrapper_taskLibInst.mutex);

	strcpy_s(pVxwTask->name, sizeof(pVxwTask->name), name);
	pVxwTask->priority = priority;
	pVxwTask->options = options;
	pVxwTask->stackSize = stackSize;
	pVxwTask->pEntry = pEntry;
	pVxwTask->args[0] = arg1;
	pVxwTask->args[1] = arg2;
	pVxwTask->args[2] = arg3;
	pVxwTask->args[3] = arg4;
	pVxwTask->args[4] = arg5;
	pVxwTask->args[5] = arg6;
	pVxwTask->args[6] = arg7;
	pVxwTask->args[7] = arg8;
	pVxwTask->args[8] = arg9;
	pVxwTask->args[9] = arg10;
	pVxwTask->pendSemId = SEM_ID_NULL;
	pVxwTask->pendSemFlushed = false;

	void* pArg = (void*)pVxwTask;
	int rc = thrd_create(&pVxwTask->c11thrd, (thrd_start_t)vxWrapTaskEntryFunc, pArg);
	if (rc == thrd_error) {
		pVxwTask->valid = false;
		return ERROR;
	}

	thrd_name_set(pVxwTask->c11thrd, &pVxwTask->name[0]);

	return (TASK_ID)pVxwTask;
}

/*!
 * taskCreate - タスク起動せずにインスタンスの生成のみ行う
 */
TASK_ID taskCreate(char* name, int priority, int options, size_t stackSize, FUNCPTR pEntry,
	_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
	_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10)
{
	int taskIx;
	VxWrapTask* pVxwTask;

	mtx_lock(&vxWrapper_taskLibInst.mutex);
	{
		for (taskIx = 0; taskIx < VW_TASK_NUM_MAX; taskIx++) {
			pVxwTask = &vxWrapper_taskLibInst.vxWrapTask[taskIx];
			if (pVxwTask->valid == false) {
				break;
			}
		}

		if (taskIx >= VW_TASK_NUM_MAX) {
			mtx_unlock(&vxWrapper_taskLibInst.mutex);
			return ERROR;
		}

		pVxwTask->c11thrd = 0;
		pVxwTask->valid = true;
	}
	mtx_unlock(&vxWrapper_taskLibInst.mutex);

	strcpy_s(pVxwTask->name, sizeof(pVxwTask->name), name);
	pVxwTask->priority = priority;
	pVxwTask->options = options;
	pVxwTask->stackSize = stackSize;
	pVxwTask->pEntry = pEntry;
	pVxwTask->args[0] = arg1;
	pVxwTask->args[1] = arg2;
	pVxwTask->args[2] = arg3;
	pVxwTask->args[3] = arg4;
	pVxwTask->args[4] = arg5;
	pVxwTask->args[5] = arg6;
	pVxwTask->args[6] = arg7;
	pVxwTask->args[7] = arg8;
	pVxwTask->args[8] = arg9;
	pVxwTask->args[9] = arg10;
	pVxwTask->pendSemId = SEM_ID_NULL;
	pVxwTask->pendSemFlushed = false;

	return (TASK_ID)pVxwTask;
}


/*!
 * taskActivate - タスク起動せずに生成されたタスクを起動する
 */
STATUS taskActivate(TASK_ID tid)
{
	VxWrapTask* pVxwTask = (VxWrapTask*)tid;

	if (pVxwTask->valid != true) {
		return ERROR;
	}

	void* pArg = (void*)pVxwTask;
	int rc = thrd_create(&pVxwTask->c11thrd, (thrd_start_t)vxWrapTaskEntryFunc, pArg);
	if (rc == thrd_error) {
		return ERROR;
	}

	thrd_name_set(pVxwTask->c11thrd, &pVxwTask->name[0]);

	return OK;
}

/*!
 * taskIdSelf - 自身のタスクIDを取得する
 */
TASK_ID taskIdSelf(void)
{
	thrd_t curThr = thrd_current();

	for (int taskIx = 0; taskIx < VW_TASK_NUM_MAX; taskIx++) {
		VxWrapTask* pVxwTask = &vxWrapper_taskLibInst.vxWrapTask[taskIx];
		if ((pVxwTask->valid == true)
		 && (thrd_equal(curThr, pVxwTask->c11thrd) == true)) {
			return (TASK_ID)pVxwTask;
		}
	}

	return (TASK_ID)ERROR;
}

/*!
 * taskName - 自身のタスク名を取得する
 */
char* taskName(TASK_ID tid)
{
	VxWrapTask* pVxwTask = (VxWrapTask*)tid;

	if (pVxwTask->valid != true) {
		return NULL;
	}

	return &pVxwTask->name[0];
}

/*!
 * taskDelay - 呼び出しタスクの実行を遅らせる
 */
STATUS taskDelay(_Vx_ticks_t delayTicks)
{
	long delayMs = VW_TICKS_TO_MS(delayTicks);

	struct timespec duration = { .tv_sec = (delayMs / 1000), .tv_nsec = (delayMs % 1000) * 1000000 };
	thrd_sleep(&duration, NULL);

	return OK;
}

/*!
 * vxWrapTaskEntryFunc - VxWrapper スレッド エントリー処理
 *
 * @param pArg VxWrapTask インスタンス
 *
 * @return エントリ関数の戻り値
 */
static int vxWrapTaskEntryFunc(void* pArg)
{
	VxWrapTask* pVxwTask = (VxWrapTask*)pArg;
	vxWrapTaskEntry pVwTaskEntry = (vxWrapTaskEntry)pVxwTask->pEntry;

	int rc = pVwTaskEntry(
		pVxwTask->args[0],
		pVxwTask->args[1],
		pVxwTask->args[2],
		pVxwTask->args[3],
		pVxwTask->args[4],
		pVxwTask->args[5],
		pVxwTask->args[6],
		pVxwTask->args[7],
		pVxwTask->args[8],
		pVxwTask->args[9]);

	mtx_lock(&vxWrapper_taskLibInst.mutex);
	{
		pVxwTask->valid = false;
		thrd_detach(pVxwTask->c11thrd);
	}
	mtx_unlock(&vxWrapper_taskLibInst.mutex);

	return rc;
}

static inline void task_pendSemIdSet(SEM_ID semId, TASK_ID taskId)
{
	VxWrapTask* pVxwTask = (VxWrapTask*)taskId;
	pVxwTask->pendSemId = semId;
}

static inline void task_pendSemIdClear(TASK_ID taskId)
{
	task_pendSemIdSet(SEM_ID_NULL, taskId);
}

static inline bool task_pendSemIsFlushed(TASK_ID taskId)
{
	VxWrapTask* pVxwTask = (VxWrapTask*)taskId;
	return pVxwTask->pendSemFlushed;
}

static inline void task_semFlushAccept(TASK_ID taskId)
{
	VxWrapTask* pVxwTask = (VxWrapTask*)taskId;
	pVxwTask->pendSemFlushed = false;
}


/*!
 * semBCreate - バイナリ セマフォの作成と初期化
 */
SEM_ID semBCreate(int options, SEM_B_STATE semState)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)malloc(sizeof(VxWrapSem));
	if (pVxwSem == NULL) {
		errno = S_memLib_NOT_ENOUGH_MEMORY;
		return SEM_ID_NULL;
	}

	pVxwSem->type = VXW_SEM_TYPE_BINARY;
	pVxwSem->options = options;
	pVxwSem->maxSemCnt = 1;
	if (semState == SEM_EMPTY) {
		pVxwSem->curSemCnt = 0;
	} else {
		pVxwSem->curSemCnt = 1;
	}
	pVxwSem->semOwner = (TASK_ID)-1;

	int rc = cnd_init(&pVxwSem->c11cnd);
	if (rc != thrd_success) {
		return SEM_ID_NULL;
	}

	rc = mtx_init(&pVxwSem->c11mtx, mtx_plain);
	if (rc == thrd_error) {
		cnd_destroy(&pVxwSem->c11cnd);
		return SEM_ID_NULL;
	}

	return (SEM_ID)pVxwSem;
}

/*!
 * semBTake - バイナリ セマフォの取得
 */
static STATUS semBTake(SEM_ID semId, _Vx_ticks_t timeoutTicks)
{
	if (timeoutTicks == NO_WAIT) {
		return semBTakeNoWait(semId);
	}

	if (timeoutTicks == WAIT_FOREVER) {
		return semBTakeWaitForever(semId);
	}

	return semBTakeTimedWait(semId, timeoutTicks);
}

static STATUS semBTakeNoWait(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;

	mtx_lock(&pVxwSem->c11mtx);
	{
		if (pVxwSem->curSemCnt == 0) {
			mtx_unlock(&pVxwSem->c11mtx);
			errno = S_objLib_OBJ_UNAVAILABLE;
			return ERROR;
		}
		pVxwSem->curSemCnt = 0;
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

static STATUS semBTakeWaitForever(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;

	mtx_lock(&pVxwSem->c11mtx);
	{
		while (pVxwSem->curSemCnt == 0) {
			TASK_ID curTaskId = taskIdSelf();
			task_pendSemIdSet(semId, curTaskId);
			int rc = cnd_wait(&pVxwSem->c11cnd, &pVxwSem->c11mtx);
			if (rc == thrd_error) {
				mtx_unlock(&pVxwSem->c11mtx);
				return ERROR;
			}
			task_pendSemIdClear(curTaskId);
			if (task_pendSemIsFlushed(curTaskId)) {
				// Flush で起床していた場合、セマフォカウントはクリアせず正常終了
				task_semFlushAccept(curTaskId);
				mtx_unlock(&pVxwSem->c11mtx);
				return OK;
			}
			// if (pVxwSem->curSemCnt == 0) {
			// 	printf("%s(): spurious wakeup.", __func__);
			// }
		}
		pVxwSem->curSemCnt = 0;
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

static STATUS semBTakeTimedWait(SEM_ID semId, _Vx_ticks_t timeoutTicks)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;

	mtx_lock(&pVxwSem->c11mtx);
	{
		if (pVxwSem->curSemCnt > 0) {
			pVxwSem->curSemCnt--;
			mtx_unlock(&pVxwSem->c11mtx);
			return OK;
		}

		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long timeoutMs = VW_TICKS_TO_MS(timeoutTicks);
		timespecAddMs(&ts, timeoutMs);

		while (pVxwSem->curSemCnt == 0) {
			TASK_ID curTaskId = taskIdSelf();
			task_pendSemIdSet(semId, curTaskId);
			int rc = cnd_timedwait(&pVxwSem->c11cnd, &pVxwSem->c11mtx, &ts);
			if (rc == thrd_timedout) {
				mtx_unlock(&pVxwSem->c11mtx);
				errno = S_objLib_OBJ_TIMEOUT;
				return ERROR;
			}
			if (rc == thrd_error) {
				mtx_unlock(&pVxwSem->c11mtx);
				errno = S_objLib_OBJ_ID_ERROR;
				return ERROR;
			}
			task_pendSemIdClear(curTaskId);
			if (task_pendSemIsFlushed(curTaskId)) {
				// Flush で起床していた場合、セマフォカウントはクリアせず正常終了
				task_semFlushAccept(curTaskId);
				mtx_unlock(&pVxwSem->c11mtx);
				return OK;
			}
		}
		pVxwSem->curSemCnt = 0;
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

/*!
 * semBTake - バイナリ セマフォの開放
 */
static STATUS semBGive(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;

	mtx_lock(&pVxwSem->c11mtx);
	{
		if (pVxwSem->curSemCnt >= pVxwSem->maxSemCnt) {
			mtx_unlock(&pVxwSem->c11mtx);
			if (pVxwSem->options & SEM_EVENTSEND_ERR_NOTIFY) {
				return ERROR;
			} else {
				return OK;
			}
		}
		pVxwSem->curSemCnt = 1;
		cnd_signal(&pVxwSem->c11cnd);
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

/*!
 * semCCreate - カウンティング セマフォの作成と初期化
 */
SEM_ID semCCreate(int options, int semCount)
{
	if (semCount < 0) {
		errno = S_semLib_INVALID_INITIAL_COUNT;
		return ERROR;
	}

	VxWrapSem* pVxwSem = (VxWrapSem*)malloc(sizeof(VxWrapSem));
	if (pVxwSem == NULL) {
		errno = S_memLib_NOT_ENOUGH_MEMORY;
		return SEM_ID_NULL;
	}

	pVxwSem->type = VXW_SEM_TYPE_COUNT;
	pVxwSem->options = options;
	pVxwSem->maxSemCnt = semCount;
	pVxwSem->curSemCnt = semCount;
	pVxwSem->semOwner = (TASK_ID)-1;

	int rc = cnd_init(&pVxwSem->c11cnd);
	if (rc != thrd_success) {
		return SEM_ID_NULL;
	}

	rc = mtx_init(&pVxwSem->c11mtx, mtx_plain);
	if (rc == thrd_error) {
		cnd_destroy(&pVxwSem->c11cnd);
		return SEM_ID_NULL;
	}

	return (SEM_ID)pVxwSem;
}

/*!
 * semMCreate - ミューテックス セマフォの作成と初期化
 */
SEM_ID semMCreate(int options)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)malloc(sizeof(VxWrapSem));
	if (pVxwSem == NULL) {
		errno = S_memLib_NOT_ENOUGH_MEMORY;
		return SEM_ID_NULL;
	}

	pVxwSem->type = VXW_SEM_TYPE_MUTEX;
	pVxwSem->options = options;
	pVxwSem->maxSemCnt = -1;
	pVxwSem->curSemCnt = -1;
	pVxwSem->semOwner = TASK_ID_NULL;

	int rc = cnd_init(&pVxwSem->c11cnd);
	if (rc != thrd_success) {
		return SEM_ID_NULL;
	}

	rc = mtx_init(&pVxwSem->c11mtx, mtx_plain);
	if (rc == thrd_error) {
		cnd_destroy(&pVxwSem->c11cnd);
		return SEM_ID_NULL;
	}

	return (SEM_ID)pVxwSem;
}

/*!
 * semMTake - ミューテックス セマフォの獲得
 */
static STATUS semMTake(SEM_ID semId, _Vx_ticks_t timeoutTicks)
{
	if (timeoutTicks == NO_WAIT) {
		return semMTakeNoWait(semId);
	}

	if (timeoutTicks == WAIT_FOREVER) {
		return semMTakeWaitForever(semId);
	}

	return semMTakeTimedWait(semId, timeoutTicks);
}

static STATUS semMTakeNoWait(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;
	TASK_ID curTaskId = taskIdSelf();

	mtx_lock(&pVxwSem->c11mtx);
	{
		if (semMTake_tryLock(pVxwSem, curTaskId) == ERROR) {
			mtx_unlock(&pVxwSem->c11mtx);
			errno = S_objLib_OBJ_UNAVAILABLE;
			return ERROR;
		}
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

static STATUS semMTakeWaitForever(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;
	TASK_ID curTaskId = taskIdSelf();

	mtx_lock(&pVxwSem->c11mtx);
	{
		while (semMTake_tryLock(pVxwSem, curTaskId) == ERROR) {
			int rc = cnd_wait(&pVxwSem->c11cnd, &pVxwSem->c11mtx);
			if (rc == thrd_error) {
				mtx_unlock(&pVxwSem->c11mtx);
				errno = S_objLib_OBJ_ID_ERROR;
				return ERROR;
			}
		}
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

static STATUS semMTakeTimedWait(SEM_ID semId, _Vx_ticks_t timeoutTicks)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;
	TASK_ID curTaskId = taskIdSelf();

	mtx_lock(&pVxwSem->c11mtx);
	{
		if (semMTake_tryLock(pVxwSem, curTaskId) == OK) {
			mtx_unlock(&pVxwSem->c11mtx);
			return OK;
		}

		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long timeoutMs = VW_TICKS_TO_MS(timeoutTicks);
		timespecAddMs(&ts, timeoutMs);

		do {
			int rc = cnd_timedwait(&pVxwSem->c11cnd, &pVxwSem->c11mtx, &ts);
			if (rc == thrd_timedout) {
				mtx_unlock(&pVxwSem->c11mtx);
				errno = S_objLib_OBJ_TIMEOUT;
				return ERROR;
			}
			if (rc == thrd_error) {
				mtx_unlock(&pVxwSem->c11mtx);
				errno = S_objLib_OBJ_ID_ERROR;
				return ERROR;
			}
		} while (semMTake_tryLock(pVxwSem, curTaskId) == ERROR);
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

static inline STATUS semMTake_tryLock(VxWrapSem* pVxwSem, TASK_ID curTaskId)
{
	if (pVxwSem->semOwner == TASK_ID_NULL) {
		pVxwSem->semOwner = curTaskId;
		pVxwSem->curSemCnt = 1;
		return OK;
	}
	if (pVxwSem->semOwner == curTaskId) {
		pVxwSem->curSemCnt++;
		return OK;
	}

	return ERROR;
}

/*!
 * semMTake - ミューテックス セマフォの開放
 */
static STATUS semMGive(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;
	TASK_ID curTaskId = taskIdSelf();

	mtx_lock(&pVxwSem->c11mtx);
	{
		if (pVxwSem->semOwner != curTaskId) {
			mtx_unlock(&pVxwSem->c11mtx);
			errno = S_semLib_INVALID_OPERATION;
			return ERROR;
		}
		if (pVxwSem->curSemCnt > 0) {
			pVxwSem->curSemCnt--;
			if (pVxwSem->curSemCnt == 0) {
				pVxwSem->semOwner = TASK_ID_NULL;
				cnd_signal(&pVxwSem->c11cnd);
			}
		}
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

/*!
 * semMTake - セマフォの獲得
 */
STATUS semTake(SEM_ID semId, _Vx_ticks_t timeoutTicks)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;
	STATUS status;

	switch (pVxwSem->type) {
	case VXW_SEM_TYPE_BINARY:
		status = semBTake(semId, timeoutTicks);
		return status;
	case VXW_SEM_TYPE_MUTEX:
		status = semMTake(semId, timeoutTicks);
		return status;
	case VXW_SEM_TYPE_COUNT:
		// TODO: カウンティング セマフォに対する Take 操作は未対応
		break;
	}

	errno = S_semLib_INVALID_OPERATION;
	return ERROR;
}

/*!
 * semMTake - セマフォの開放
 */
STATUS semGive(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;
	STATUS status;

	switch (pVxwSem->type) {
	case VXW_SEM_TYPE_BINARY:
		status = semBGive(semId);
		return status;
	case VXW_SEM_TYPE_MUTEX:
		status = semMGive(semId);
		return status;
	case VXW_SEM_TYPE_COUNT:
		// TODO: カウンティング セマフォに対する Give 操作は未対応
		break;
	}

	errno = S_semLib_INVALID_OPERATION;
	return ERROR;
}

/*!
 * semFlush - セマフォ獲得待ちの全タスクのブロックを解除する
 */
STATUS semFlush(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;

	switch (pVxwSem->type) {
	case VXW_SEM_TYPE_BINARY:
		return semBFlush(semId);
	case VXW_SEM_TYPE_MUTEX:
		// ミューテックス セマフォに対する Flush 操作は無効
	case VXW_SEM_TYPE_COUNT:
		// TODO: カウンティング セマフォに対する Flush 操作は未対応
		break;
	}

	errno = S_semLib_INVALID_OPERATION;
	return ERROR;
}

/*!
 * semDelete - セマフォの削除
 */
STATUS semDelete(SEM_ID semId)
{
	// TODO: セマフォの削除は未対応
	return ERROR;
}

/*!
 * semBFlush - バイナリ セマフォ獲得待ちの全タスクのブロックを解除する
 */
static STATUS semBFlush(SEM_ID semId)
{
	VxWrapSem* pVxwSem = (VxWrapSem*)semId;

	mtx_lock(&pVxwSem->c11mtx);
	{
		// 指定バイナリセマフォでペンディングしているタスクを探す
		mtx_lock(&vxWrapper_taskLibInst.mutex);
		{
			int pendTaskNum = 0;
			for (int taskIx = 0; taskIx < VW_TASK_NUM_MAX; taskIx++) {
				VxWrapTask* pVxwTask = &vxWrapper_taskLibInst.vxWrapTask[taskIx];
				if ((pVxwTask->valid == true)
				 && (pVxwTask->pendSemId == semId)) {
					// セマフォ ID が一致した場合、Flush での起床を設定
					pVxwTask->pendSemFlushed = true;
					pendTaskNum++;
				}
			}

			if (pendTaskNum > 0) {
				// ペンディングしていたタスクが存在していた場合は、全タスク起床させる
				cnd_broadcast(&pVxwSem->c11cnd);
			}
		}
		mtx_unlock(&vxWrapper_taskLibInst.mutex);
	}
	mtx_unlock(&pVxwSem->c11mtx);

	return OK;
}

/*!
 * msgQCreate - メッセージキューの作成と初期化
 */
MSG_Q_ID msgQCreate(size_t maxMsgNum, size_t maxMsgSize, int options)
{
	size_t vwMsgDataSize = sizeof(VxWrapMsgData) + ((maxMsgSize + 0x3) & (~0x3));
	if ((maxMsgNum == 0)
	 || (maxMsgNum > (SIZE_MAX / vwMsgDataSize))) {
		errno = S_msgQLib_INVALID_MSG_COUNT;
		return (MSG_Q_ID)NULL;
	}

	void* p = malloc(sizeof(VxWrapMsgQue));
	if (p == NULL) {
		errno = S_memLib_NOT_ENOUGH_MEMORY;
		return (MSG_Q_ID)NULL;
	}
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)p;

	pMsgQue->maxMsgNum = maxMsgNum;
	pMsgQue->maxMsgSize = maxMsgSize;
	pMsgQue->options = options;

	p = (uint8_t*)malloc(vwMsgDataSize * maxMsgNum);
	if (p == NULL) {
		free(pMsgQue);
		errno = S_memLib_NOT_ENOUGH_MEMORY;
		return (MSG_Q_ID)NULL;
	}
	pMsgQue->pMsgData = (uint8_t*)p;
	pMsgQue->vwMsgDataSize = vwMsgDataSize;

	pMsgQue->msgNum = 0;
	pMsgQue->msgQueHead = 0;
	pMsgQue->msgQueTail = 0;

	mtx_init(&pMsgQue->mutex, mtx_plain);
	cnd_init(&pMsgQue->cndSend);
	cnd_init(&pMsgQue->cndRecv);

	return (MSG_Q_ID)pMsgQue;
}

/*!
 * msgQSend - メッセージキューにメッセージを送信する
 */
STATUS msgQSend(MSG_Q_ID msgQId, char* pMsg, size_t msgSize, _Vx_ticks_t timeoutTicks, int priority)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;

	if (priority != MSG_PRI_NORMAL) {
		errno = S_msgQLib_ILLEGAL_PRIORITY; // MSG_PRI_NORMAL 以外は未サポート
		return ERROR;
	}

	if (msgSize > pMsgQue->maxMsgSize) {
		errno = S_msgQLib_INVALID_MSG_LENGTH;
		return ERROR;
	}

	if (timeoutTicks == NO_WAIT) {
		return msgQSendNoWait(msgQId, pMsg, msgSize);
	}

	if (timeoutTicks == WAIT_FOREVER) {
		return msgQSendWaitForever(msgQId, pMsg, msgSize);
	}

	return msgQSendTimedWait(msgQId, pMsg, msgSize, timeoutTicks);
}

static STATUS msgQSendNoWait(MSG_Q_ID msgQId, char* pMsg, size_t msgSize)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;

	mtx_lock(&pMsgQue->mutex);
	{
		if (pMsgQue->msgNum == pMsgQue->maxMsgNum) {
			mtx_unlock(&pMsgQue->mutex);
			errno = S_objLib_OBJ_UNAVAILABLE;
			return ERROR;
		}
		msgQue_enqueue(pMsgQue, pMsg, msgSize);
	}
	mtx_unlock(&pMsgQue->mutex);
	cnd_signal(&pMsgQue->cndRecv);

	return OK;
}

static STATUS msgQSendWaitForever(MSG_Q_ID msgQId, char* pMsg, size_t msgSize)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;

	mtx_lock(&pMsgQue->mutex);
	{
		while (pMsgQue->msgNum == pMsgQue->maxMsgNum) {
			int rc = cnd_wait(&pMsgQue->cndSend, &pMsgQue->mutex);
			if (rc == thrd_error) {
				mtx_unlock(&pMsgQue->mutex);
				errno = S_objLib_OBJ_ID_ERROR;
				return ERROR;
			}
		}
		msgQue_enqueue(pMsgQue, pMsg, msgSize);
	}
	mtx_unlock(&pMsgQue->mutex);
	cnd_signal(&pMsgQue->cndRecv);

	return OK;
}

static STATUS msgQSendTimedWait(MSG_Q_ID msgQId, char* pMsg, size_t msgSize, _Vx_ticks_t timeoutTicks)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;

	mtx_lock(&pMsgQue->mutex);
	{
		if (pMsgQue->msgNum == pMsgQue->maxMsgNum) {
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long timeoutMs = VW_TICKS_TO_MS(timeoutTicks);
			timespecAddMs(&ts, timeoutMs);
			do {
				int rc = cnd_timedwait(&pMsgQue->cndSend, &pMsgQue->mutex, &ts);
				if (rc == thrd_timedout) {
					mtx_unlock(&pMsgQue->mutex);
					errno = S_objLib_OBJ_TIMEOUT;
					return ERROR;
				}
				if (rc == thrd_error) {
					mtx_unlock(&pMsgQue->mutex);
					errno = S_objLib_OBJ_ID_ERROR;
					return ERROR;
				}
			} while (pMsgQue->msgNum == pMsgQue->maxMsgNum);
		}
		msgQue_enqueue(pMsgQue, pMsg, msgSize);
	}
	mtx_unlock(&pMsgQue->mutex);
	cnd_signal(&pMsgQue->cndRecv);

	return OK;
}

/*!
 * msgQReceive - メッセージキューからメッセージを受信する
 */
ssize_t msgQReceive(MSG_Q_ID msgQId, char* pBuf, size_t bufSize, _Vx_ticks_t timeoutTicks)
{
	if (timeoutTicks == NO_WAIT) {
		return msgQReceiveNoWait(msgQId, pBuf, bufSize);
	}

	if (timeoutTicks == WAIT_FOREVER) {
		return msgQReceiveWaitForever(msgQId, pBuf, bufSize);
	}

	return msgQReceiveTimedWait(msgQId, pBuf, bufSize, timeoutTicks);
}

static ssize_t msgQReceiveNoWait(MSG_Q_ID msgQId, char* pBuf, size_t bufSize)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;
	ssize_t recvMsgSize;

	mtx_lock(&pMsgQue->mutex);
	{
		if (pMsgQue->msgNum == 0) {
			mtx_unlock(&pMsgQue->mutex);
			errno = S_objLib_OBJ_UNAVAILABLE;
			return ERROR;
		}

		recvMsgSize = msgQue_dequeue(pMsgQue, pBuf, bufSize);
		if (recvMsgSize == ERROR) {
			mtx_unlock(&pMsgQue->mutex);
			errno = S_msgQLib_INVALID_MSG_LENGTH;
			return ERROR;
		}
	}
	mtx_unlock(&pMsgQue->mutex);
	cnd_signal(&pMsgQue->cndSend);

	return recvMsgSize;
}

static ssize_t msgQReceiveWaitForever(MSG_Q_ID msgQId, char* pBuf, size_t bufSize)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;
	ssize_t recvMsgSize;

	mtx_lock(&pMsgQue->mutex);
	{
		while (pMsgQue->msgNum == 0) {
			int rc = cnd_wait(&pMsgQue->cndRecv, &pMsgQue->mutex);
			if (rc == thrd_error) {
				mtx_unlock(&pMsgQue->mutex);
				errno = S_objLib_OBJ_ID_ERROR;
				return ERROR;
			}
		}

		recvMsgSize = msgQue_dequeue(pMsgQue, pBuf, bufSize);
		if (recvMsgSize == ERROR) {
			mtx_unlock(&pMsgQue->mutex);
			errno = S_msgQLib_INVALID_MSG_LENGTH;
			return ERROR;
		}
	}
	mtx_unlock(&pMsgQue->mutex);
	cnd_signal(&pMsgQue->cndSend);

	return recvMsgSize;
}

static ssize_t msgQReceiveTimedWait(MSG_Q_ID msgQId, char* pBuf, size_t bufSize, _Vx_ticks_t timeoutTicks)
{
	VxWrapMsgQue* pMsgQue = (VxWrapMsgQue*)msgQId;
	ssize_t recvMsgSize;

	mtx_lock(&pMsgQue->mutex);
	{
		if (pMsgQue->msgNum == 0) {
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long timeoutMs = VW_TICKS_TO_MS(timeoutTicks);
			timespecAddMs(&ts, timeoutMs);
			do {
				int rc = cnd_timedwait(&pMsgQue->cndRecv, &pMsgQue->mutex, &ts);
				if (rc == thrd_timedout) {
					mtx_unlock(&pMsgQue->mutex);
					errno = S_objLib_OBJ_TIMEOUT;
					return ERROR;
				}
				if (rc == thrd_error) {
					mtx_unlock(&pMsgQue->mutex);
					errno = S_objLib_OBJ_ID_ERROR;
					return ERROR;
				}
			} while (pMsgQue->msgNum == 0);
		}
		recvMsgSize = msgQue_dequeue(pMsgQue, pBuf, bufSize);
		if (recvMsgSize == ERROR) {
			mtx_unlock(&pMsgQue->mutex);
			errno = S_msgQLib_INVALID_MSG_LENGTH;
			return ERROR;
		}
	}
	mtx_unlock(&pMsgQue->mutex);
	cnd_signal(&pMsgQue->cndSend);

	return recvMsgSize;
}

static inline void msgQue_enqueue(VxWrapMsgQue* pMsgQue, char* pMsg, size_t msgSize)
{
	size_t msgDataIx = pMsgQue->msgQueTail * pMsgQue->vwMsgDataSize;
	VxWrapMsgData* pVwMsg = (VxWrapMsgData*)&pMsgQue->pMsgData[msgDataIx];
	memcpy(pVwMsg->msgData, pMsg, msgSize);
	pVwMsg->msgDataSize = msgSize;
	pMsgQue->msgQueTail = (pMsgQue->msgQueTail + 1) % pMsgQue->maxMsgNum;
	pMsgQue->msgNum++;
}

static inline ssize_t msgQue_dequeue(VxWrapMsgQue* pMsgQue, char* pBuf, size_t bufSize)
{
	size_t msgDataIx = pMsgQue->msgQueHead * pMsgQue->vwMsgDataSize;
	VxWrapMsgData* pVwMsg = (VxWrapMsgData*)&pMsgQue->pMsgData[msgDataIx];
	if (bufSize < pVwMsg->msgDataSize) {
		return ERROR;
	}

	memcpy(pBuf, pVwMsg->msgData, pVwMsg->msgDataSize);
	pMsgQue->msgQueHead = (pMsgQue->msgQueHead + 1) % pMsgQue->maxMsgNum;
	pMsgQue->msgNum--;

	return pVwMsg->msgDataSize;
}

/*!
 * msgQDelete - メッセージキューの削除
 */
STATUS msgQDelete(MSG_Q_ID msgQId)
{
	// TODO: メッセージキューの削除は未対応
	return ERROR;
}

