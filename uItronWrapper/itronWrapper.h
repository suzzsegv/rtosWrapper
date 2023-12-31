/*
 * itronWrapper.h - uITRON API wrapper for C11thread
 *                  Interface header file.
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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "threads.h"

typedef int8_t B; // 符号付き8ビット整数
typedef int16_t H; // 符号付き16ビット整数
typedef int32_t W; // 符号付き32ビット整数
typedef int64_t D; // 符号付き64ビット整数
typedef uint8_t UB; // 符号無し8ビット整数
typedef uint16_t UH; // 符号無し16ビット整数
typedef uint32_t UW; // 符号無し32ビット整数
typedef uint64_t UD; // 符号無し64ビット整数
typedef int8_t VB; // データタイプが定まらない8ビットの値
typedef int16_t VH; // データタイプが定まらない16 ビットの値
typedef int32_t VW; // データタイプが定まらない32 ビットの値
typedef int64_t VD; // データタイプが定まらない64 ビットの値
typedef void* VP; // データタイプが定まらないものへのポインタ
typedef void (*FP)(void*); // プログラムの起動番地（ポインタ）
typedef int INT; // プロセッサに自然なサイズの符号付き整数
typedef unsigned int UINT; // プロセッサに自然なサイズの符号無し整数
// typedef bool BOOL; // 真偽値（TRUEまたはFALSE）
typedef int FN; // 機能コード（符号付き整数）
typedef int ID; // オブジェクトのID番号（符号付き整数）
typedef unsigned int ATR; // オブジェクト属性（符号無し整数）
typedef unsigned int MODE; // サービスコールの動作モード（符号無し整数）
typedef int PRI; // 優先度（符号付き整数）
#define SIZE ITRON_TYPE__SIZE
typedef unsigned int ITRON_TYPE__SIZE; // メモリ領域のサイズ（符号無し整数）
typedef int TMO; // タイムアウト指定（符号付き整数，時間単位は実装定義）
typedef unsigned int RELTIM; // 相対時間（符号無し整数，時間単位は実装定義）
typedef unsigned int SYSTIM; // システム時刻（符号無し整数，時間単位は実装定義）
typedef intptr_t VP_INT; // データタイプが定まらないものへのポインタまたはプロセッサに自然なサイズの符号付き整数
typedef int ER_BOOL; // エラーコードまたは真偽値（符号付き整数）
typedef int ER_UINT; // エラーコードまたは符号無し整数（符号付き整数）


/*
 * ITRON 仕様共通定数
 */

/* 一般 */
#ifndef NULL
	#define NULL 0 // 無効ポインタ
#endif
//#define TRUE true // 真
//#define FALSE false // 偽

/* メインエラーコード */
typedef enum {
	E_OK = 0,
	E_SYS = -5, // システムエラー
	E_NOSPT = -9, // 未サポート機能
	E_RSFN = -10, // 予約機能コード
	E_RSATR = -11, // 予約属性
	E_PAR = -17, // パラメータエラー
	E_ID = -18, // 不正ID番号

	E_CTX = -25, // コンテキストエラー
	E_MACV = -26, // メモリアクセス違反
	E_OACV = -27, // オブジェクトアクセス違反
	E_ILUSE = -28, // サービスコール不正使用

	E_NOMEM = -33, // メモリ不足
	E_NOID = -34,// ID番号不足

	E_OBJ = -41, // オブジェクト状態エラー
	E_NOEXS = -42, // オブジェクト未生成
	E_QOVR = -43, // キューイングオーバフロー

	E_RLWAI = -49, // 待ち状態の強制解除
	E_TMOUT = -50, // ポーリング失敗またはタイムアウト
	E_DLT = -51, // 待ちオブジェクトの削除
	E_CLS = -52, // 待ちオブジェクトの状態変化

	E_WBLK = -57, // ノンブロッキング受付け
	E_BOVR = -58, // バッファオーバフロー
} ER;

#define ER_ID ER

/* オブジェクト属性 */
#define TA_NULL 0 // オブジェクト属性を指定しない

/* タイムアウト指定 */
enum {
	TMO_POL = 0, // ポーリング
	TMO_FEVR = -1, // 永久待ち
	TMO_NBLK = -2, //ノンブロッキング
};

/* オブジェクトの状態 */
typedef enum {
	TTS_NONE = 0x00, // 未生成
	TTS_RUN = 0x01, // 実行状態
	TTS_RDY = 0x02, // 実行可能状態
	TTS_WAI = 0x04, // 待ち状態
	TTS_SUS = 0x08, // 強制待ち状態
	TTS_WAS = 0x0c, // 二重待ち状態
	TTS_DMT = 0x10, // 休止状態

	TTW_SLP = 0x0001, //起床待ち状態
	TTW_DLY = 0x0002, //時間経過待ち状態
	TTW_SEM = 0x0004, //セマフォ資源の獲得待ち状態
	TTW_FLG = 0x0008, //イベントフラグ待ち状態
	TTW_SDTQ = 0x0010, // データキューへの送信待ち状態
	TTW_RDTQ = 0x0020, // データキューからの受信待ち状態
	TTW_MBX = 0x0040, //メールボックスからの受信待ち状態
	TTW_MTX = 0x0080, //ミューテックスのロック待ち状態
	TTW_SMBF = 0x0100, // メッセージバッファへの送信待ち状態
	TTW_RMBF = 0x0200, // メッセージバッファからの受信待ち状態
	TTW_CAL = 0x0400, //ランデブの呼出し待ち状態
	TTW_ACP = 0x0800, //ランデブの受付待ち状態
	TTW_RDV = 0x1000, //ランデブの終了待ち状態
	TTW_MPF = 0x2000, //固定長メモリブロックの獲得待ち状態
	TTW_MPL = 0x4000, //可変長メモリブロックの獲得待ち状態
} STAT;

/*
 * カーネル共通定数
 */
#define TA_TFIFO	0x00	// タスクの待ち行列を FIFO順に
#define TA_TPRI		0x01	// タスクの待ち行列をタスクの優先度順に
#define TA_MFIFO	0x00	// メッセージのキューを FIFO順に
#define TA_MPRI		0x02	// メッセージのキューをメッセージの優先度順に


/***********************************************************************
 * uITRON Wrapper ライブラリの初期化
 */
extern void uitronWrapperInit(void);


/***********************************************************************
/*
 * タスク
 */
#define TW_MIN_AUTO_ASSIGN_TSKID 128
#define TW_MAX_TSKID 256

// ATR
#define TA_HLNG 0x00
#define TA_ASM 0x01
#define TA_ACT 0x02

typedef struct {
	ATR tskatr;		// タスク属性
	VP_INT exinf;	// タスクの拡張情報
	FP task;		// タスクの起動番地
	PRI itskpri;	// タスクの起動時優先度
	SIZE stksz;		// タスクのスタック領域のサイズ（バイト数）
	VP stk;			// タスクのスタック領域の先頭番地
} T_CTSK;

typedef struct {
	STAT tskstat;	// タスク状態
	PRI tskpri;		// タスクの現在優先度
	PRI tskbpri;	// タスクのベース優先度
	STAT tskwait;	// 待ち要因
	ID wobjid;		// 待ち対象のオブジェクトの ID 番号
	TMO lefttmo;	// タイムアウトするまでの時間
	UINT actcnt;	// 起動要求キューイング数
	UINT wupcnt;	// 起床要求キューイング数
	UINT suscnt;	// 強制待ち要求ネスト数

	ATR tskatr;		// タスク属性
	VP_INT exinf;	// タスクの拡張情報
	FP task;		// タスクの起動番地
	SIZE stksz;		// タスクのスタック領域のサイズ（バイト数）
	VP stk;			// タスクのスタック領域の先頭番地

	ID taskId;		// タスクID
	thrd_t c11thrd;
	cnd_t c11cndSlp;
	mtx_t c11mtxSlp;
} T_RTSK;

extern ER cre_tsk(ID tskid, T_CTSK* pk_ctsk);
extern ER del_tsk(ID tskid);
extern ER_ID acre_tsk(T_CTSK* pk_ctsk);
extern ER act_tsk(ID tskid);
extern ER sta_tsk(ID tskid, VP_INT stacd);
extern void ext_tsk(void);
extern void exd_tsk(void);
extern ER ref_tsk(ID tskid, T_RTSK* pk_rtsk);
extern ER slp_tsk(void);
extern ER tslp_tsk(TMO tmout);
extern ER wup_tsk(ID tskid);
extern ER dly_tsk(RELTIM dlytim);


/***********************************************************************
/*
 * セマフォ
 */
#define TMAX_SEMID 512
#define TMAX_MAXSEM 256

typedef struct t_csem {
	ATR sematr;		// セマフォ属性
	UINT isemcnt;	// セマフォの資源数の初期値
	UINT maxsem;	// セマフォの最大資源数

	UINT curSemCnt;
	cnd_t c11cnd;
	mtx_t c11mtx;
} T_CSEM;

typedef struct t_rsem {
	ID wtskid;		// セマフォの待ち行列の先頭のタスクの ID 番号
	UINT semcnt;	// セマフォの現在の資源数
} T_RSEM;

extern ER cre_sem(ID semid, T_CSEM* pk_csem);
extern ER del_sem(ID semid);
extern ER sig_sem(ID semid);
extern ER wai_sem(ID semid);
extern ER twai_sem(ID semid, TMO tmout);


/***********************************************************************
/*
 * メッセージボックス
 */
#define TMAX_MBXID 512
#define TMAX_MAXMBX 256

typedef uintptr_t T_MSG;

typedef struct t_cmbx {
	ATR mbxatr; // メールボックス属性
	PRI maxmpri; // 送信されるメッセージの優先度の最大値
	VP mprihd; // 優先度別のメッセージキューヘッダ領域の先頭番地
} T_CMBX;

typedef struct t_rmbx {
	ID wtskid ; // メールボックスの待ち行列の先頭のタスクの ID 番号
	T_MSG * pk_msg ; // メッセージキューの先頭のメッセージパケットの先頭番地

	ATR mbxatr; // メールボックス属性
	PRI maxmpri; // 送信されるメッセージの優先度の最大値
	VP mprihd; // 優先度別のメッセージキューヘッダ領域の先頭番地

	void** mbxData;
	uint32_t num;
	uint32_t head;
	uint32_t tail;
	mtx_t mutex;
	cnd_t condSend;
	cnd_t condRecv;
} T_RMBX;

extern ER cre_mbx(ID mbxid, T_CMBX* pk_cmbx);
extern ER snd_mbx(ID mbxid, T_MSG* pk_msg);
extern ER rcv_mbx(ID mbxid, T_MSG** ppk_msg);
extern ER trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO tmout);


/***********************************************************************
/*
 * システム管理
 */
extern ER get_tid(ID* p_tskid);


