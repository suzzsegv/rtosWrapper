/*
 * vxWorksWrapper.h - VxWorks API wrapper for C11thread
 *                    Interface header file.
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

#pragma once

#include <stddef.h>
#include <stdint.h>

/***********************************************************************
 * defines
 */

/* status */
#define OK 0
#ifdef ERROR
	#undef ERROR
#endif
#define ERROR (-1)

/* timeout */
#define NO_WAIT (0)
#define WAIT_FOREVER (-1)

/* objLib */
#define ModId_objLib (('Obj') << 8)
#define S_objLib_OBJ_ID_ERROR		(ModId_objLib | 0x01)
#define S_objLib_OBJ_UNAVAILABLE	(ModId_objLib | 0x02)
#define S_objLib_OBJ_TIMEOUT		(ModId_objLib | 0x03)

/* memLib */
#define ModId_memLib (('Mem') << 8)
#define S_memLib_NOT_ENOUGH_MEMORY	(ModId_memLib | 0x01)

/* taskLib */
#define TASK_ID_NULL ((TASK_ID)NULL)
#define VX_FP_TASK 0

/* semLib */
#define SEM_ID_NULL ((SEM_ID)NULL)

#define SEM_Q_FIFO					0x00
#define SEM_Q_PRIORITY				0x01
#define SEM_DELETE_SAFE				0x04
#define SEM_INVERSION_SAFE			0x08
#define SEM_EVENTSEND_ERR_NOTIFY	0x10
#define SEM_INTERRUPTIBLE			0x20

#define ModId_semLib (('Sem') << 8)
#define S_semLib_INVALID_STATE			(ModId_semLib | 0x01)
#define S_semLib_INVALID_OPERATION		(ModId_semLib | 0x02)
#define S_semLib_INVALID_INITIAL_COUNT	(ModId_semLib | 0x03)

/* msgQLib */
#define MSG_Q_ID_NULL ((MSG_Q_ID)NULL)

#define MSG_Q_FIFO					0x00
#define MSG_Q_PRIORITY				0x01
#define MSG_Q_EVENTSEND_ERR_NOTIFY	0x02
#define MSG_Q_INTERRUPTIBLE			0x04
#define MSG_Q_TASK_DELETION_WAKEUP	0x08

#define MSG_PRI_NORMAL 0
#define MSG_PRI_URGENT 1

#define ModId_msgQLib (('Msg') << 8)
#define S_msgQLib_INVALID_MSG_LENGTH			(ModId_msgQLib | 0x01)
#define S_msgQLib_NON_ZERO_TIMEOUT_AT_INT_LEVEL	(ModId_msgQLib | 0x02)
#define S_msgQLib_INVALID_QUEUE_TYPE			(ModId_msgQLib | 0x03)
#define S_msgQLib_ILLEGAL_OPTIONS				(ModId_msgQLib | 0x04)
#define S_msgQLib_ILLEGAL_PRIORITY				(ModId_msgQLib | 0x05)
#define S_msgQLib_UNSUPPORTED_OPERATION			(ModId_msgQLib | 0x06)
#define S_msgQLib_INVALID_MSG_COUNT				(ModId_msgQLib | 0x07)
#define S_msgQLib_ILLEGAL_BUFFER				(ModId_msgQLib | 0x08)
#define S_msgQLib_INVALID_STATE					(ModId_msgQLib | 0x09)


/***********************************************************************
 * typedefs
 */
typedef int BOOL;
#ifdef _WIN64
	typedef int64_t ssize_t;
#else
	typedef int32_t ssize_t;
#endif
typedef int STATUS;

typedef int (*FUNCPTR)();
typedef void (*VOIDFUNCPTR)();

typedef uint32_t _Vx_ticks_t;
typedef uint64_t _Vx_ticks64_t;

/* taskLib */
typedef uintptr_t TASK_ID;
#ifdef _WIN64
	typedef intptr_t _Vx_usr_arg_t;
#else
	typedef int _Vx_usr_arg_t;
#endif
typedef int (*VxTaskEntry)(
		_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
		_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10
	);

/* semLib */
typedef uintptr_t SEM_ID;
typedef enum {
	SEM_EMPTY = 0,
	SEM_FULL = 1
} SEM_B_STATE;

/* msgQLib */
typedef uintptr_t MSG_Q_ID;


/***********************************************************************
 * externals
 */

/* taskLib */
extern STATUS taskLibInit(void);
extern TASK_ID taskSpawn(char* name, int priority, int options, size_t stackSize, FUNCPTR pEntry,
	_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
	_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10);
extern TASK_ID taskCreate(char* name, int priority, int options, size_t stackSize, FUNCPTR pEntry,
	_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2, _Vx_usr_arg_t arg3, _Vx_usr_arg_t arg4, _Vx_usr_arg_t arg5,
	_Vx_usr_arg_t arg6, _Vx_usr_arg_t arg7, _Vx_usr_arg_t arg8, _Vx_usr_arg_t arg9, _Vx_usr_arg_t arg10);
extern STATUS taskActivate(TASK_ID taskId);
extern TASK_ID taskIdSelf(void);
extern char* taskName(TASK_ID taskId);
extern STATUS taskDelay(_Vx_ticks_t ticks);

/* semLib */
extern SEM_ID semBCreate(int options, SEM_B_STATE semState);
extern SEM_ID semCCreate(int options, int semCount);
extern SEM_ID semMCreate(int options);
extern STATUS semTake(SEM_ID semId, _Vx_ticks_t timeoutTicks);
extern STATUS semGive(SEM_ID semId);
extern STATUS semFlush(SEM_ID semId);
extern STATUS semDelete(SEM_ID semId);

/* msgQLib */
extern MSG_Q_ID msgQCreate(size_t maxMsgNum, size_t maxMsgSize, int options);
extern STATUS msgQSend(MSG_Q_ID msgQId, char* pMsg, size_t msgSize, _Vx_ticks_t timeoutTicks, int priority);
extern ssize_t msgQReceive(MSG_Q_ID msgQId, char* pBuf, size_t bufSize, _Vx_ticks_t timeoutTicks);
extern STATUS msgQDelete(MSG_Q_ID msgQId);
