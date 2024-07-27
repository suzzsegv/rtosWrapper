/*
 * vxWorksWrapperTest.c - VxWorks wrapper test module.
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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "vxWorksWrapper.h"


void vxWorksWrapperTaskCreateTest(void);
int vxWorksWrapperTaskCreateTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2);
void vxWorksWrapperTaskStartTest(void);
int vxWorksWrapperTaskStartTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2);
static void timespecPrint(struct timespec* ts);

void vxWorksWrapper_semTest(void);
void vxWorksWrapper_SemBTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2);
void vxWorksWrapper_SemBTestEntry_waitForever(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2);

void vxWorksWrapper_SemMTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2);
void vxWorksWrapper_semMTest_5secTake_entry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2);

void vxWorksWrapper_msgQueTest(void);
void vxWorksWrapper_msgQueTest_msgQRecv_entry(_Vx_usr_arg_t arg0);
void vxWorksWrapper_msgQueTest_msgQSend_entry(_Vx_usr_arg_t arg0);


int vxWorksWrapperTest(void)
{
	vxWorksWrapperTaskCreateTest();
	vxWorksWrapperTaskStartTest();
	vxWorksWrapper_semTest();
	vxWorksWrapper_msgQueTest();

	printf("\n[Enter] key to exit.\n");
	int c = getchar();

	exit(0);
}


/*!
 * vxWorksWrapperTaskCreateTest
 */
void vxWorksWrapperTaskCreateTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	TASK_ID taskId = taskCreate("vxwTaskCreateTest",
		100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapperTaskCreateTestEntry,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

	if (taskId == TASK_ID_NULL) {
		printf("%s(): taskCreate() error: %" PRIdPTR "\n", __func__, taskId);
		exit(-1);
	}

	printf("%s(): Create task id: 0x%" PRIxPTR "\n", __func__, taskId);

	STATUS status = taskActivate(taskId);
	assert(status == OK);
	printf("%s(): Activate task id: 0x%" PRIxPTR "\n", __func__, taskId);

	printf("=== %s(): exit.\n", __func__);
}

/*!
 * vxWorksWrapperTaskCreateTestEntry
 */
int vxWorksWrapperTaskCreateTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2)
{
	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): start.\n", __func__,
		arg0, arg1, arg2);

	TASK_ID taskId = taskIdSelf();
	assert(taskId != ERROR);

	printf("%s(): Current task id: 0x%" PRIxPTR "\n", __func__, taskId);
	printf("%s(): Current task name: %s\n", __func__, taskName(taskId));

	printf("--- %s(...): end.\n\n", __func__);

	return 0x12340001;
}

/*!
 * vxWorksWrapperTaskStartTest
 */
void vxWorksWrapperTaskStartTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	TASK_ID taskId = taskSpawn("vxwTaskStartTest",
		100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapperTaskStartTestEntry,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

	if (taskId == TASK_ID_NULL) {
		printf("%s(): taskSpawn() error: %" PRIdPTR "\n", __func__, taskId);
		exit(-1);
	}

	printf("%s(): Spawn task id: 0x%" PRIxPTR "\n", __func__, taskId);

	printf("=== %s(): exit.\n", __func__);
}

int vxWorksWrapperTaskStartTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2)
{
	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): start.\n", __func__,
		arg0, arg1, arg2);

	TASK_ID taskId = taskIdSelf();
	assert(taskId != ERROR);

	printf("%s(): Current task id: 0x%" PRIxPTR "\n", __func__, taskId);
	printf("%s(): Current task name: %s\n", __func__, taskName(taskId));

	{
		struct timespec startTs;
		struct timespec endTs;

		timespec_get(&startTs, TIME_UTC);
		STATUS status = taskDelay(100);
		timespec_get(&endTs, TIME_UTC);

		assert(status == OK);
		printf("%s(): Start Time: ", __func__);
		timespecPrint(&startTs);
		printf("%s():   End Time: ", __func__);
		timespecPrint(&endTs);
	}

	printf("--- %s(...): end.\n\n", __func__);

	return 0x12340002;
}

/*!
 * timespecPrint
 */
static void timespecPrint(struct timespec* ts)
{
	char dateTimeStr[32];

	ctime_s(&dateTimeStr[0], sizeof(dateTimeStr), &ts->tv_sec);
	printf("%.24s, %09lu ns\n", &dateTimeStr[0], ts->tv_nsec);
}


/*!
 * vxWorksWrapper_semTest
 */
void vxWorksWrapper_semTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	// Binary Semaphore
	SEM_ID emptyBinSemId = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	assert(emptyBinSemId != SEM_ID_NULL);

	TASK_ID taskId = taskSpawn("semB_Test",
		100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemBTestEntry,
		(_Vx_usr_arg_t)emptyBinSemId, 2, 3, 4, 5, 6, 7, 8, 9, 10);

	assert(taskId != TASK_ID_NULL);

	taskDelay(600);

	// MUtex Semaphore
	SEM_ID mtxSemId = semMCreate(SEM_Q_FIFO | SEM_INVERSION_SAFE);
	assert(mtxSemId != SEM_ID_NULL);

	taskId = taskSpawn("semM_Test",
		100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemMTestEntry,
		(_Vx_usr_arg_t)mtxSemId, 2, 3, 4, 5, 6, 7, 8, 9, 10);

	assert(taskId != TASK_ID_NULL);

	printf("=== %s(): exit.\n", __func__);
}


void vxWorksWrapper_SemBTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2)
{
	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): start.\n", __func__,
		arg0, arg1, arg2);

	SEM_ID emptyBinSemId = (SEM_ID)arg0;
	STATUS status;

	status = semTake(emptyBinSemId, NO_WAIT);
	assert(status == ERROR);
	assert(errno == S_objLib_OBJ_UNAVAILABLE);

	{
		struct timespec startTs;
		struct timespec endTs;

		timespec_get(&startTs, TIME_UTC);
		status = semTake(emptyBinSemId, 30);
		timespec_get(&endTs, TIME_UTC);

		assert(status == ERROR);
		assert(errno == S_objLib_OBJ_TIMEOUT);

		printf("%s(): Start Time: ", __func__);
		timespecPrint(&startTs);
		printf("%s():   End Time: ", __func__);
		timespecPrint(&endTs);
	}

	status = semGive(emptyBinSemId);
	assert(status == OK);

	status = semTake(emptyBinSemId, WAIT_FOREVER);
	assert(status == OK);

	{
		TASK_ID taskId = taskSpawn("semB_Wait0",
			100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemBTestEntry_waitForever,
			(_Vx_usr_arg_t)emptyBinSemId, 3, 1, 4, 5, 6, 7, 8, 9, 10);

		taskId = taskSpawn("semB_Wait1",
			100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemBTestEntry_waitForever,
			(_Vx_usr_arg_t)emptyBinSemId, 3, 2, 4, 5, 6, 7, 8, 9, 10);

		taskId = taskSpawn("semB_Wait2",
			100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemBTestEntry_waitForever,
			(_Vx_usr_arg_t)emptyBinSemId, 3, 3, 4, 5, 6, 7, 8, 9, 10);

		printf("... %s(): taskDelay(100)\n", __func__);
		taskDelay(100);

		printf("... %s(): semGive()\n", __func__);
		status = semGive(emptyBinSemId);
		assert(status == OK);

		taskDelay(100);

		printf("... %s(): semFlush()\n", __func__);
		status = semFlush(emptyBinSemId);
		assert(status == OK);

		taskDelay(100);

		printf("... %s(): semGive()\n", __func__);
		status = semGive(emptyBinSemId);
		assert(status == OK);
	}

	printf("--- %s(): end.\n", __func__);
}


void vxWorksWrapper_SemBTestEntry_waitForever(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2)
{
	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): "
		"start.\n",
		__func__, arg0, arg1, arg2);

	SEM_ID emptyBinSemId = (SEM_ID)arg0;
	int	waitTimes = (int)arg1;

	for (int i = 0; i < waitTimes; i++) {
		STATUS status;

		status = semTake(emptyBinSemId, WAIT_FOREVER);
		assert(status == OK);

		printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): "
			"sem taked.\n",
			__func__, arg0, arg1, arg2);
	}

	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): "
		"end.\n",
		__func__, arg0, arg1, arg2);
}

void vxWorksWrapper_SemMTestEntry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2)
{
	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): "
		"start.\n",
		__func__, arg0, arg1, arg2);

	SEM_ID mtxSemId = (SEM_ID)arg0;

	STATUS status;

	{	// 2回 take、 2回 give、3回目の give は失敗。
		
		status = semTake(mtxSemId, NO_WAIT);
		assert(status == OK);
		status = semTake(mtxSemId, NO_WAIT);
		assert(status == OK);

		status = semGive(mtxSemId);
		assert(status == OK);
		status = semGive(mtxSemId);
		assert(status == OK);

		status = semGive(mtxSemId);
		assert(status == ERROR);
	}

	{
		status = semTake(mtxSemId, WAIT_FOREVER);
		assert(status == OK);
		status = semGive(mtxSemId);
		assert(status == OK);
	}

	{
		TASK_ID taskId = taskSpawn("semMTake5sec",
			100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_semMTest_5secTake_entry,
			(_Vx_usr_arg_t)mtxSemId, 0, 0, 4, 5, 6, 7, 8, 9, 10);

		// 上記タスクが起動するまでウェイト。
		(void)taskDelay(30);

		// NoWait で取得失敗
		printf("... semTake( , timeoutTicks: NO_WAIT) Error ...\n");
		status = semTake(mtxSemId, NO_WAIT);
		assert(status == ERROR);
		assert(errno == S_objLib_OBJ_UNAVAILABLE);

		// 30tick Wait で取得失敗
		printf("... semTake( , timeoutTicks: 30) Timeout ...\n");
		struct timespec startTs;
		struct timespec endTs;

		timespec_get(&startTs, TIME_UTC);
		status = semTake(mtxSemId, 30);
		timespec_get(&endTs, TIME_UTC);

		assert(status == ERROR);
		assert(errno == S_objLib_OBJ_TIMEOUT);

		printf("Start Time: "); timespecPrint(&startTs);
		printf("  End Time: "); timespecPrint(&endTs);

		// WaitForever で取得成功
		printf("... semTake( , timeoutTicks: WAIT_FOREVER) OK ...\n");
		status = semTake(mtxSemId, WAIT_FOREVER);
		assert(status == OK);
	}

//	{
//		TASK_ID taskId = taskSpawn("semB_Wait0",
//			100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemBTestEntry_waitForever,
//			(_Vx_usr_arg_t)mtxSemId, 0, 0, 4, 5, 6, 7, 8, 9, 10);
//
//		taskId = taskSpawn("semB_Wait1",
//			100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_SemBTestEntry_waitForever,
//			(_Vx_usr_arg_t)mtxSemId, 1, 0, 4, 5, 6, 7, 8, 9, 10);
//
//		printf("%s(): taskDelay(100)\n", __func__);
//		taskDelay(100);
//
//		status = semGive(mtxSemId);
//		assert(status == OK);
//	}

	printf("--- %s(): end.\n", __func__);
}

void vxWorksWrapper_semMTest_5secTake_entry(_Vx_usr_arg_t arg0, _Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2)
{
	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): "
		"start.\n",
		__func__, arg0, arg1, arg2);

	SEM_ID mtxSemId = (SEM_ID)arg0;
	STATUS status;

	status = semTake(mtxSemId, WAIT_FOREVER);
	assert(status == OK);

	(void)taskDelay(60 * 5);

	status = semGive(mtxSemId);
	assert(status == OK);

	printf("\n--- %s(0x%" PRIxPTR ", 0x%" PRIxPTR ", 0x%" PRIxPTR "): "
		"end.\n",
		__func__, arg0, arg1, arg2);
}

/*!
 * vxWorksWrapper_msgQueTest
 */
void vxWorksWrapper_msgQueTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	// Msg Que

	// size = 0
	MSG_Q_ID msgQId = msgQCreate(0, 123, MSG_Q_FIFO);
	assert(msgQId == 0);

	// size over
#ifdef _WIN64
	msgQId = msgQCreate(0x100000000, 0x100000000, MSG_Q_FIFO);
	assert(msgQId == 0);
#else
	msgQId = msgQCreate(65536, 65536, MSG_Q_FIFO);
	assert(msgQId == 0);
#endif

	// memory full
#ifdef _WIN64
	msgQId = msgQCreate(0x100000000, 0xfffffff0, MSG_Q_FIFO);
	assert(msgQId == 0);
#else
	msgQId = msgQCreate(65536, 65535, MSG_Q_FIFO);
	assert(msgQId == 0);
#endif

	// success
	printf("... msgQCreate(maxMsgNum: 8, maxMsgSize: 5, options: MSG_Q_FIFO) ...\n");
	msgQId = msgQCreate(8, 5, MSG_Q_FIFO);
	assert(msgQId != 0);

	STATUS status;

	// 送信メッセージのサイズエラーで失敗
	printf("... msgQSend( , , maxMsgSize: 6, , ) Error ...\n");
	status = msgQSend(msgQId, "123456", 6, WAIT_FOREVER, MSG_PRI_NORMAL);
	assert(status == ERROR);
	assert(errno == S_msgQLib_INVALID_MSG_LENGTH);

	status = msgQSend(msgQId, "0000", 1, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "1111", 2, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "2222", 3, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "3333", 4, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "44444", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "55555", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "66666", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "77777", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	assert(status == OK);

	// NoWait で送信失敗
	printf("... msgQSend( , , , timeoutTicks: NO_WAIT, ) Error ...\n");
	status = msgQSend(msgQId, "88888", 5, NO_WAIT, MSG_PRI_NORMAL);
	assert(status == ERROR);
	assert(errno == S_objLib_OBJ_UNAVAILABLE);

	// 60tick Wait で取得失敗
	printf("... msgQSend( , , , timeoutTicks: 60, ) Error ...\n");
	status = msgQSend(msgQId, "99999", 5, 60, MSG_PRI_NORMAL);
	assert(status == ERROR);
	assert(errno == S_objLib_OBJ_TIMEOUT);

	for (int i = 0; i < 8; i++) {
		char recvMsg[8];
		ssize_t recvMsgSize = msgQReceive(msgQId, &recvMsg[0], sizeof(recvMsg), NO_WAIT);
		assert(recvMsgSize != ERROR);
		printf("... RecvMsg: ");
		for (int ix = 0; ix < recvMsgSize; ix++) {
			printf("%c", recvMsg[ix]);
		}
		printf("\n");
	}

	TASK_ID taskId = taskSpawn("tMsgQRecv",
		100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_msgQueTest_msgQRecv_entry,
		(_Vx_usr_arg_t)msgQId, 0, 0, 4, 5, 6, 7, 8, 9, 10);

	taskId = taskSpawn("tMsgQSend",
		100, VX_FP_TASK, 8192, (FUNCPTR)vxWorksWrapper_msgQueTest_msgQSend_entry,
		(_Vx_usr_arg_t)msgQId, 1, 0, 4, 5, 6, 7, 8, 9, 10);

	printf("\n=== %s(): exit.\n", __func__);
}


void vxWorksWrapper_msgQueTest_msgQRecv_entry(_Vx_usr_arg_t arg0)
{
	printf("\n--- %s(0x%" PRIxPTR "): start.\n", __func__, arg0);

	MSG_Q_ID msgQId = (MSG_Q_ID)arg0;

	printf("... msgQReceive( , , , timeoutTicks: WAIT_FOREVER): 8 times ...\n");
	for (int i = 0; i < 8; i++) {
		char recvMsg[8];
		ssize_t recvMsgSize = msgQReceive(msgQId, &recvMsg[0], sizeof(recvMsg), WAIT_FOREVER);
		assert(recvMsgSize != ERROR);
		printf("RecvMsg: ");
		for (int ix = 0; ix < recvMsgSize; ix++) {
			printf("%c", recvMsg[ix]);
		}
		printf("\n");
	}

	printf("... Wait 3 sec ...\n");
	(void)taskDelay(60 * 3);

	printf("... msgQReceive( , , , timeoutTicks: 60): Recv 4 times ...\n");
	for (int i = 0; i < 4; i++) {
		char recvMsg[8];
		ssize_t recvMsgSize = msgQReceive(msgQId, &recvMsg[0], sizeof(recvMsg), 60);
		assert(recvMsgSize != ERROR);
		printf("RecvMsg: ");
		for (int ix = 0; ix < recvMsgSize; ix++) {
			printf("%c", recvMsg[ix]);
		}
		printf("\n");
	}

	printf("... msgQReceive( , , , timeoutTicks: 60): Timeout 3 times ...\n");
	for (int i = 0; i < 3; i++) {
		char recvMsg[8];
		ssize_t recvMsgSize = msgQReceive(msgQId, &recvMsg[0], sizeof(recvMsg), 60);
		assert(recvMsgSize == ERROR);
		assert(errno == S_objLib_OBJ_TIMEOUT);
	}

	printf("--- %s(0x%" PRIxPTR "): end.\n", __func__, arg0);
}

void vxWorksWrapper_msgQueTest_msgQSend_entry(_Vx_usr_arg_t arg0)
{
	printf("\n--- %s(0x%" PRIxPTR "): start.\n", __func__, arg0);

	MSG_Q_ID msgQId = (MSG_Q_ID)arg0;
	STATUS status;

	printf("... msgQSend( , , timeoutTicks: WAIT_FOREVER): 12 times ...\n");
	status = msgQSend(msgQId, "00000", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "11111", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "22222", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "33333", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "44444", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "55555", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "66666", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "77777", 5, WAIT_FOREVER, MSG_PRI_NORMAL);

	status = msgQSend(msgQId, "88888", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "99999", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "AAAAA", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	status = msgQSend(msgQId, "BBBBB", 5, WAIT_FOREVER, MSG_PRI_NORMAL);
	assert(status == OK);

	printf("--- %s(0x%" PRIxPTR "): end.\n", __func__, arg0);
}
