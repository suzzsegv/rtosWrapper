﻿/*
 * c11_mtx.h - C11 standard (ISO/IEC 9899:2011): Thread support library for Win32
 *             Mutex functions header file.
 */

/*
 * Copyright (c) 2021-2023 Suzuki Satoshi
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
 * Copyright (c) 2021-2023 鈴木 聡
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

#include <time.h>

#include "c11_thrd_config.h"
#include "c11_thrd_common.h"


/*
 * typedefs
 */
typedef uintptr_t mtx_t;


/*
 * enums
 */
enum {
	mtx_plain = 0x00,
	mtx_recursive = 0x01,
	mtx_timed = 0x02,
};


/*
 * externals
 */
extern int mtx_init(mtx_t* pMutex, int type);
extern int mtx_lock(mtx_t* pMutex);
extern int mtx_timedlock(mtx_t* pMutex, const struct timespec* time_point);
extern int mtx_trylock(mtx_t* pMutex);
extern int mtx_unlock(mtx_t* pMutex);
extern void mtx_destroy(mtx_t* pMutex);

