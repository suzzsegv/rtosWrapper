/*
 * itronWrapperTest.c - uITRON wrapper test module.

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

#include "itronWrapper.h"


void itronTaskStartTest(void);
void itronTaskStartTestEntry(void* pArg);

void itronTaskCreateAndStartTest(void);
void itronTaskCreateAndStartTestEntry(void* pArg);

void itronTaskSleepAndWakeupTest(void);
void itronTaskSleepAndWakeupTestEntry(void* pArg);

void itronTaskAutoAssignCreateTest(void);
void itronTaskAutoAssignCreateTestEntry(void* pArg);

void itronSemTest(void);
void itronSemTestEntry(void* pArg);

void itronMboxTest(void);
void itronMboxSendTestEntry(void* pArg);



int itronWrapperTest(void)
{
	itronTaskStartTest();
	dly_tsk(250);

	itronTaskCreateAndStartTest();
	dly_tsk(2000);

	itronTaskSleepAndWakeupTest();
	dly_tsk(3000);

	itronTaskAutoAssignCreateTest();
	dly_tsk(3000);

	itronSemTest();
	dly_tsk(250);

	itronMboxTest();
	dly_tsk(250);

	printf("\n[Enter] key to exit.\n");
	int c = getchar();

	exit(0);
}

/*!
 * itronTaskStartTest
 */
void itronTaskStartTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.tskatr = (TA_HLNG | TA_ACT);
	creTask.exinf = 0x1;
	creTask.task = itronTaskStartTestEntry;
	creTask.itskpri = 100;
	creTask.stksz = 1024;
	creTask.stk = 0;

	ID taskId = 1;
	ER rc = cre_tsk(taskId, &creTask);
	if (rc != E_OK) {
		printf("%s(): cre_tsk() error: %d\n", __func__, rc);
		exit(-1);
	}

	dly_tsk(100);

	rc = del_tsk(taskId);
	assert(rc == E_OK);

	printf("=== %s(): exit.\n", __func__);
}

void itronTaskStartTestEntry(void* pArg)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, (unsigned int)pArg);

	ID taskId;
	ER rc = get_tid(&taskId);
	if (rc == E_OK) {
		printf("%s(): Current task id: %d\n", __func__, taskId);
	}else{
		printf("%s(): get_tid() error: %d\n", __func__, rc);
	}

	dly_tsk(10);

	printf("\n=== %s(0x%08x): end.\n", __func__, (unsigned int)pArg);

	ext_tsk();
}

/*!
 * itronTaskCreateAndStartTest
 */
void itronTaskCreateAndStartTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.tskatr = TA_HLNG;
	creTask.exinf = 0x2;
	creTask.task = itronTaskCreateAndStartTestEntry;
	creTask.itskpri = 100;
	creTask.stksz = 1024;
	creTask.stk = 0;

	ID taskId = 2;
	ER rc = cre_tsk(taskId, &creTask);
	if (rc != E_OK) {
		printf("%s(): cre_tsk() error: %d\n", __func__, rc);
		exit(-1);
	}

	T_RTSK taskInfo;
	ref_tsk(taskId, &taskInfo);
	printf("%s(): tskstat: 0x%02x\n", __func__, taskInfo.tskstat);

	rc = act_tsk(taskId);
	if (rc != E_OK) {
		printf("%s(): act_tsk() error: %d\n", __func__, rc);
		exit(-1);
	}

	ref_tsk(taskId, &taskInfo);
	printf("%s(): tskstat: 0x%02x\n", __func__, taskInfo.tskstat);

	dly_tsk(1000);

	ref_tsk(taskId, &taskInfo);
	printf("%s(): tskstat: 0x%02x\n", __func__, taskInfo.tskstat);

	printf("=== %s(): exit.\n", __func__);
}

void itronTaskCreateAndStartTestEntry(void* pArg)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, (unsigned int)pArg);

	dly_tsk(500);

	printf("\n=== %s(0x%08x): end.\n", __func__, (unsigned int)pArg);

	exd_tsk();
}

/*!
 * itronTaskSleepAndWakeupTest
 */
void itronTaskSleepAndWakeupTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.tskatr = (TA_HLNG | TA_ACT);
	creTask.exinf = 0x3;
	creTask.task = itronTaskSleepAndWakeupTestEntry;
	creTask.itskpri = 100;
	creTask.stksz = 1024;
	creTask.stk = 0;

	ID taskId = 3;
	ER rc = cre_tsk(taskId, &creTask);
	if (rc != E_OK) {
		printf("%s(): cre_tsk() error: %d\n", __func__, rc);
		exit(-1);
	}

	dly_tsk(1000);
	rc = wup_tsk(taskId);
	assert(rc == E_OK);

	printf("=== %s(): exit.\n", __func__);
}

void itronTaskSleepAndWakeupTestEntry(void* pArg)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, (unsigned int)pArg);

	ER rc = tslp_tsk(100);
	assert(rc == E_TMOUT);

	rc = slp_tsk();
	assert(rc == E_OK);

	printf("\n=== %s(0x%08x): end.\n", __func__, (unsigned int)pArg);

	exd_tsk();
}

/*!
 * itronTaskAutoAssignCreateTest
 */
void itronTaskAutoAssignCreateTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CTSK creTask;

	creTask.tskatr = (TA_HLNG | TA_ACT);
	creTask.exinf = 0x128;
	creTask.task = itronTaskAutoAssignCreateTestEntry;
	creTask.itskpri = 100;
	creTask.stksz = 1024;
	creTask.stk = 0;

	int i;
	ER_ID taskIdOrError;
	for (i = 0; i < 128; i++) {
		taskIdOrError = acre_tsk(&creTask);
		assert(taskIdOrError > 0);
	}

	taskIdOrError = acre_tsk(&creTask);
	assert(taskIdOrError < 0);

	printf("=== %s(): exit.\n", __func__);
}

void itronTaskAutoAssignCreateTestEntry(void* pArg)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, (unsigned int)pArg);

	ID taskId;
	ER rc = get_tid(&taskId);
	if (rc == E_OK) {
		printf("%s(): Current task id: %d\n", __func__, taskId);
	} else {
		printf("%s(): get_tid() error: %d\n", __func__, rc);
	}

	rc = slp_tsk();
}


/*!
 * itronSemTest
 */
void itronSemTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CSEM creSem;

	creSem.sematr = TA_TFIFO;
	creSem.isemcnt = 0;
	creSem.maxsem = 1;

	ER rc = cre_sem(0, &creSem);
	if (rc != E_OK) {
		printf("%s(): cre_sem() error: %d\n", __func__, rc);
		exit(-1);
	}

	T_CTSK creTask;
	creTask.tskatr = (TA_HLNG | TA_ACT);
	creTask.exinf = 0x2;
	creTask.task = itronSemTestEntry;
	creTask.itskpri = 10;
	creTask.stksz = 1024;
	creTask.stk = 0;

	rc = cre_tsk(5, &creTask);
	if (rc != E_OK) {
		printf("%s(): cre_tsk() error: %d\n", __func__, rc);
		exit(-1);
	}

	printf("=== %s(): exit.\n", __func__);
}

void itronSemTestEntry(void* pArg)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, (unsigned int)pArg);

	ER rc = twai_sem(0, 300);
	if(rc != E_TMOUT){
		printf("%s(): twai_sem() error: %d\n", __func__, rc);
		exit(-1);
	}

	printf("\n=== %s(0x%08x): end.\n", __func__, (unsigned int)pArg);
}


/*!
 * itronMboxTest
 */
void itronMboxTest(void)
{
	printf("\n=== %s(): start.\n", __func__);

	T_CMBX creMbx;
	creMbx.mbxatr = 0;
	creMbx.maxmpri = 1;
	ER rc = cre_mbx(0, &creMbx);
	if (rc != E_OK) {
		printf("%s(): cre_mbx() error: %d\n", __func__, rc);
		exit(-1);
	}

	T_CTSK creTask;
	creTask.tskatr = (TA_HLNG | TA_ACT);
	creTask.exinf = 0x2;
	creTask.task = itronMboxSendTestEntry;
	creTask.itskpri = 10;
	creTask.stksz = 1024;
	creTask.stk = 0;

	rc = cre_tsk(10, &creTask);
	if (rc != E_OK) {
		printf("%s(): cre_tsk() error: %d\n", __func__, rc);
		exit(-1);
	}

	printf("=== %s(): exit.\n", __func__);
}

void itronMboxSendTestEntry(void* pArg)
{
	printf("\n=== %s(0x%08x): start.\n", __func__, (unsigned int)pArg);

	ER rc = snd_mbx(0, (T_MSG*)0x1111);
	printf("snd_mbx(): rc : %d\n", rc);

	rc = snd_mbx(0, (T_MSG*)0x2222);
	printf("snd_mbx(): rc : %d\n", rc);

	rc = snd_mbx(0, (T_MSG*)0x3333);
	printf("snd_mbx(): rc : %d\n", rc);

	dly_tsk(1000);

	T_MSG* pMsg;
	rc = rcv_mbx(0, &pMsg);
	printf("pMsg: %08x\n",(unsigned int)pMsg);

	rc = trcv_mbx(0, &pMsg, 3000);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);
	rc = trcv_mbx(0, &pMsg, 3000);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);
	rc = trcv_mbx(0, &pMsg, 3000);
	printf("rc : %d, pMsg: %08x\n", rc, (unsigned int)pMsg);

	printf("\n=== %s(0x%08x): end.\n", __func__, (unsigned int)pArg);
}

