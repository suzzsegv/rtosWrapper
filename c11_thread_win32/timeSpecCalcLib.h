﻿/*
 * timeSpecCalcLib.h - struct timespec calculate library.
 */

/*
 * Copyright (c) 2022 Suzuki Satoshi
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
 * Copyright (c) 2022 鈴木 聡
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

#include <stdint.h>
#include <time.h>


/*!
 * timespecAddMs - 指定時刻にミリ秒単位の時間を加算する
 *
 * @param [in,out] ts 加算対象の時刻
 * @param [in] msec 加算時間 [ミリ秒単位]
 *
 * @return なし
 */
inline void timespecAddMs(struct timespec* ts, long msec)
{
	if(msec >= 1000){
		ts->tv_sec += msec / 1000;
		msec %= 1000;
	}
	ts->tv_nsec += msec * 1000000;
	if (ts->tv_nsec > 1000000000) {
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}

/*!
 * timespecDiffMs - 引数 ts2 から ts1 を引いた経過時間をミリ秒単位で返す
 *
 * @param [in] ts2 終了時刻
 * @param [in] ts1 開始時刻
 *
 * @return 経過時間 [ミリ秒単位]
 */
inline time_t timespecDiffMs(const struct timespec* ts2, const struct timespec* ts1)
{
	time_t diffSec = (ts2->tv_sec - ts1->tv_sec);
	long diffNsec = (ts2->tv_nsec - ts1->tv_nsec);

	if( diffNsec < 0 ){
		diffSec--;
		diffNsec += 1000000000;
	}

	return (diffSec * 1000) + (diffNsec / 1000000);
}

