/*
 * uT-KernelWrapperTest.c - uT-Kernel wrapper test module.

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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "uT-KernelWrapper.h"


void utkernelWrapperTest(void);

void utkernelTaskStartTest(void);
void utkernelTaskStartTestEntry(INT startCode, void* exinf);

void utkernelTaskCreateAndStartTest(void);
void utkernelTaskCreateAndStartTestEntry(INT stacd, void* exinf);

void utkernelTaskSleepAndWakeupTest(void);
void utkernelTaskSleepAndWakeupTestEntry(INT stacd, void* exinf);

void utkernelSemTest(void);
void utkernelSemTestEntry(INT stacd, void* exinf);
void utkernelTest_waiSemWaitForever(INT stacd, void* exinf);

void utkernelMboxTest(void);
void utkernelMboxSendTestEntry(INT stacd, void* exinf);

static ID utkernelTest_taskSpawn(FP taskEntry, INT stacd, void* exinf);
static void timespecPrint(struct timespec* ts);


/*!
 * utkernelWrapperTest
 */
void utkernelWrapperTest(void)
{
//	utkernelTaskStartTest();
//	tk_dly_tsk(250);
//
//	utkernelTaskCreateAndStartTest();
//	tk_dly_tsk(2000);
//
//	utkernelTaskSleepAndWakeupTest();
//	tk_dly_tsk(3000);

	utkernelSemTest();
	tk_dly_tsk(1500);

	utkernelMboxTest();
//	tk_dly_tsk(250);
//
	printf("\n[Enter] key to exit.\n");
	int c = getchar();

	exit(0);
}

/*!
 * utkernelTaskStartTest
 */
void utkernelTaskStartTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.exinf = (void*)0xDeadBeee;
	creTask.tskatr = (TA_HLNG | TA_DSNAME);
	creTask.task = utkernelTaskStartTestEntry;
	creTask.itskpri = 101;
	creTask.stksz = 1024;
	creTask.sstksz = 0;
	creTask.stkptr = (void*)0xEEEEEEEE;
	strncpy_s(&creTask.dsname[0], sizeof(creTask.dsname), "Start00", sizeof(creTask.dsname));
	creTask.bufptr = (void*)0xFFFFFFFF;

	ID taskId = tk_cre_tsk(&creTask);
	if (taskId < 0) {
		printf("%s(): tk_cre_tsk() error: %d\n", __func__, taskId);
		exit(-1);
	}
	printf("=== %s(): taskId=%d\n", __func__, taskId);


	ER rc = tk_sta_tsk(taskId, 0x0001);
	assert(rc == E_OK);

	tk_dly_tsk(100);

//	rc = del_tsk(taskId);
//	assert(rc == E_OK);

	printf("=== %s(): exit.\n", __func__);
}

void utkernelTaskStartTestEntry(INT startCode, void* exinf)
{
	printf("\n=== %s(0x%08x, 0x%08x): start.\n", __func__, (unsigned int)startCode, (unsigned int)exinf);

	ID taskId = tk_get_tid();
	if (taskId > 0) {
		printf("%s(): Current task id: %d\n", __func__, taskId);
	}else{
		printf("%s(): tk_get_tid() error: %d\n", __func__, taskId);
	}

	tk_dly_tsk(10);

	printf("\n=== %s(0x%08x, 0x%08x): end.\n", __func__, (unsigned int)startCode, (unsigned int)exinf);

	tk_ext_tsk(); // インスタンス開放なし
}

/*!
 * utkernelTaskCreateAndStartTest
 */
void utkernelTaskCreateAndStartTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.exinf = (void*)0xDeadBeef;
	creTask.tskatr = TA_HLNG;
	creTask.task = utkernelTaskCreateAndStartTestEntry;
	creTask.itskpri = 102;
	creTask.stksz = 2048;
	creTask.sstksz = 4096;
	creTask.stkptr = (void*)0xEEEEEEEE;
	strncpy_s(&creTask.dsname[0], sizeof(creTask.dsname), "Start01", sizeof(creTask.dsname));
	creTask.bufptr = (void*)0xFFFFFFFF;

	ID taskId = tk_cre_tsk(&creTask);
	if (taskId < 0) {
		printf("%s(): tk_cre_tsk() error: %d\n", __func__, taskId);
		exit(-1);
	}
	printf("=== %s(): taskId=%d\n", __func__, taskId);

	T_RTSK taskInfo;
	tk_ref_tsk(taskId, &taskInfo);
	printf("%s(): tskstat: 0x%02x\n", __func__, taskInfo.tskstat);

	ER rc = tk_sta_tsk(taskId, 0x0002);
	assert(rc == E_OK);

	tk_dly_tsk(100);

	tk_ref_tsk(taskId, &taskInfo);
	printf("%s(): tskstat: 0x%02x\n", __func__, taskInfo.tskstat);

	tk_dly_tsk(1000);

	tk_ref_tsk(taskId, &taskInfo);
	printf("%s(): tskstat: 0x%02x\n", __func__, taskInfo.tskstat);

	printf("=== %s(): exit.\n", __func__);
}

void utkernelTaskCreateAndStartTestEntry(INT stacd, void* exinf)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, stacd);

	tk_dly_tsk(500);

	printf("\n=== %s(0x%08x): end.\n", __func__, stacd);

	tk_exd_tsk(); // インスタンス開放あり
}

/*!
 * utkernelTaskSleepAndWakeupTest
 */
void utkernelTaskSleepAndWakeupTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.exinf = (void*)0xDead0003;
	creTask.tskatr = TA_HLNG;
	creTask.task = utkernelTaskSleepAndWakeupTestEntry;
	creTask.itskpri = 103;
	creTask.stksz = 1024;
	creTask.sstksz = 2048;
	creTask.stkptr = (void*)0xEEEEEEEE;
	strncpy_s(&creTask.dsname[0], sizeof(creTask.dsname), "Slp-Wup", sizeof(creTask.dsname));
	creTask.bufptr = (void*)0xFFFFFFFF;

	ID taskId = tk_cre_tsk(&creTask);
	if (taskId < 0) {
		printf("%s(): tk_cre_tsk() error: %d\n", __func__, taskId);
		exit(-1);
	}
	printf("=== %s(): taskId=%d\n", __func__, taskId);

	ER rc = tk_sta_tsk(taskId, 0x0003);
	assert(rc == E_OK);

	tk_dly_tsk(1000);

	printf("=== %s(): tk_wup_tsk(%d).\n", __func__, taskId);
	rc = tk_wup_tsk(taskId);
	assert(rc == E_OK);

	tk_dly_tsk(100);

	printf("=== %s(): exit.\n", __func__);
}

void utkernelTaskSleepAndWakeupTestEntry(INT stacd, void* exinf)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, stacd);

	ER rc = tk_slp_tsk(100);
	assert(rc == E_TMOUT);

	printf("\n=== %s(0x%08x): tk_slp_tsk(TMO_FEVR).\n", __func__, stacd);
	rc = tk_slp_tsk(TMO_FEVR);
	assert(rc == E_OK);

	printf("\n=== %s(0x%08x): end.\n", __func__, stacd);

	tk_exd_tsk();
}

/*!
 * utkernelSemTest
 */
void utkernelSemTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CSEM creSem;
	creSem.exinf = (void*)0xFFFFFFFF;
	creSem.sematr = TA_TFIFO;
	creSem.isemcnt = 0;
	creSem.maxsem = 1;
	strncpy_s(&creSem.dsname[0], sizeof(creSem.dsname), "Sem01", sizeof(creSem.dsname));

	ID semId = tk_cre_sem(&creSem);
	if (semId < 0) {
		printf("%s(): tk_cre_sem() error: %d\n", __func__, semId);
		exit(-1);
	}

	ID taskId = utkernelTest_taskSpawn(utkernelSemTestEntry, semId, (void*)0xDeadBeef);
	printf("=== %s(): taskId=%d\n", __func__, taskId);

	printf("=== %s(): exit.\n", __func__);
}

void utkernelSemTestEntry(INT stacd, void* exinf)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, stacd);

	ID semId = (ID)stacd;

	ER rc = tk_wai_sem(semId, 1, TMO_POL);
	assert(rc == E_TMOUT);

	{
		struct timespec startTs;
		struct timespec endTs;

		timespec_get(&startTs, TIME_UTC);
		rc = tk_wai_sem(semId, 1, 1000);
		timespec_get(&endTs, TIME_UTC);

		assert(rc == E_TMOUT);

		printf("%s(): Start Time: ", __func__);
		timespecPrint(&startTs);
		printf("%s():   End Time: ", __func__);
		timespecPrint(&endTs);
	}

	rc = tk_sig_sem(semId, 1000);
	assert(rc == E_QOVR);

	rc = tk_sig_sem(semId, 1);
	assert(rc == E_OK);

	rc = tk_wai_sem(semId, 1, TMO_FEVR);
	assert(rc == E_OK);

	{
		ID taskId = utkernelTest_taskSpawn(utkernelTest_waiSemWaitForever, semId, (void*)0x01);
		printf("=== %s(): taskId=%d\n", __func__, taskId);

		taskId = utkernelTest_taskSpawn(utkernelTest_waiSemWaitForever, semId, (void*)0x02);
		printf("=== %s(): taskId=%d\n", __func__, taskId);

		taskId = utkernelTest_taskSpawn(utkernelTest_waiSemWaitForever, semId, (void*)0x03);
		printf("=== %s(): taskId=%d\n", __func__, taskId);

		printf("%s(): tk_dly_tsk(100)\n", __func__);
		tk_dly_tsk(100);

		printf("%s(): tk_sig_sem(semId, 1) -- 1\n", __func__);
		rc = tk_sig_sem(semId, 1);
		assert(rc == E_OK);

		tk_dly_tsk(100);

		printf("%s(): tk_sig_sem(semId, 1) -- 2\n", __func__);
		rc = tk_sig_sem(semId, 1);
		assert(rc == E_OK);

		tk_dly_tsk(100);

		printf("%s(): tk_sig_sem(semId, 1) -- 3\n", __func__);
		rc = tk_sig_sem(semId, 1);
		assert(rc == E_OK);

		tk_dly_tsk(100);
	}

	printf("\n=== %s(0x%08x): end.\n", __func__, stacd);
}

void utkernelTest_waiSemWaitForever(INT stacd, void* exinf)
{
	printf("=== %s(0x%08x, 0x%08x): start.\n", __func__, (int)stacd, (int)exinf);

	ID semId = (ID)stacd;

	ER rc = tk_wai_sem(semId, 1, TMO_FEVR);
	assert(rc == E_OK);

	printf("=== %s(0x%08x, 0x%08x): end.\n", __func__, (int)stacd, (int)exinf);
}


/*!
 * utkernelMboxTest
 */
void utkernelMboxTest(void)
{
	printf("=== %s(): start.\n", __func__);

	T_CMBX creMbx;
	creMbx.exinf = (void*)0x9999;
	creMbx.mbxatr = TA_TFIFO;
	creMbx.dsname[0] = 0;

	ID mbxId = tk_cre_mbx(&creMbx);
	assert(mbxId > 0);


	ID taskId = utkernelTest_taskSpawn(utkernelMboxSendTestEntry, mbxId, (void*)0x01);
	printf("=== %s(): taskId=%d\n", __func__, taskId);

	tk_dly_tsk(100);

	printf("=== %s(): end.\n", __func__);
}

void utkernelMboxSendTestEntry(INT stacd, void* exinf)
{
	printf("=== %s(0x%08x, 0x%08x): start.\n", __func__, (int)stacd, (int)exinf);
	ID mbxId = (ID)stacd;

	ER rc = tk_snd_mbx(mbxId, (T_MSG*)0x1111);
	printf("snd_mbx(): rc : %d\n", rc);

	rc = tk_snd_mbx(mbxId, (T_MSG*)0x2222);
	printf("snd_mbx(): rc : %d\n", rc);

	rc = tk_snd_mbx(mbxId, (T_MSG*)0x3333);
	printf("snd_mbx(): rc : %d\n", rc);

	tk_dly_tsk(1000);

	T_MSG* pMsg;
	rc = tk_rcv_mbx(mbxId, &pMsg, TMO_POL);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);

	rc = tk_rcv_mbx(mbxId, &pMsg, TMO_FEVR);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);
	rc = tk_rcv_mbx(mbxId, &pMsg, 3000);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);
	rc = tk_rcv_mbx(mbxId, &pMsg, 3000);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);

	printf("=== %s(0x%08x, 0x%08x): end.\n", __func__, (int)stacd, (int)exinf);
}

/*!
 * utkernelTest_taskSpawn
 */
static ID utkernelTest_taskSpawn(FP taskEntry, INT stacd, void* exinf)
{
	T_CTSK creTask;
	creTask.exinf = exinf;
	creTask.tskatr = TA_HLNG;
	creTask.task = taskEntry;
	creTask.itskpri = 10;
	creTask.stksz = 1024;
	creTask.sstksz = 1024;
	creTask.stkptr = (void*)0xEEEEEEEE;
	creTask.dsname[0] = 0;
	creTask.bufptr = (void*)0xFFFFFFFF;

	ID taskId = tk_cre_tsk(&creTask);
	if (taskId < 0) {
		printf("%s(): tk_cre_tsk() error: %d\n", __func__, taskId);
		return taskId;
	}

	ER rc = tk_sta_tsk(taskId, stacd);
	assert(rc == E_OK);

	return taskId;
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
