/*
 * uT-KernelWrapper.h - uT-Kernel API wrapper for C11thread
 *                      Interface header file.
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

#include <stdint.h>
#include <threads.h>


typedef int8_t B; /* 符号付き 8ビット整数 */
typedef int16_t H; /* 符号付き 16ビット整数 */
typedef int32_t W; /* 符号付き 32ビット整数 */
typedef int64_t D; /* 符号付き 64ビット整数 */
typedef uint8_t UB; /* 符号無し 8ビット整数 */
typedef uint16_t UH; /* 符号無し 16ビット整数 */
typedef uint32_t UW; /* 符号無し 32ビット整数 */
typedef uint64_t UD; /* 符号無し 64ビット整数 */
typedef int8_t VB; /* 型が一定しない 8ビットのデータ */
typedef int16_t VH; /* 型が一定しない 16ビットのデータ */
typedef int32_t VW; /* 型が一定しない 32ビットのデータ */
typedef int64_t VD; /* 型が一定しない 64ビットのデータ */
typedef volatile B _B; /* volatile 宣言付 */
typedef volatile H _H;
typedef volatile W _W;
typedef volatile D _D;
typedef volatile UB _UB;
typedef volatile UH _UH;
typedef volatile UW _UW;
typedef volatile UD _UD;
typedef signed int INT; /* プロセッサのビット幅の符号付き整数 */
typedef unsigned int UINT; /* プロセッサのビット幅の符号無し整数 */
typedef INT SZ; /* サイズ一般 */
typedef INT ID; /* ID一般 */
typedef W MSEC; /* 時間一般(ミリ秒) */
typedef void (*FP)(); /* 関数アドレス一般 */
typedef INT (*FUNCP)(); /* 関数アドレス一般 */

#define CONST const
#define LOCAL static /* ローカルシンボル定義 */
#define EXPORT /* グローバルシンボル定義 */
#define IMPORT extern /* グローバルシンボル参照 */

/*
 * 共通定数
 */
#ifndef NULL
	#define NULL 0 /* 無効ポインタ */
#endif
#define TA_NULL 0 /* 特別な属性を指定しない */
#define TMO_POL 0 /* ポーリング */
#define TMO_FEVR (-1) /* 永久待ち */

/*
 * ブール値
 */
#undef BOOL
#define BOOL UINT;
#define TRUE 1 /* 真 */
#define FALSE 0 /* 偽 */

/*
 * エラーコード定義/分離用マクロ
 */
#define MERCD(er) ((int32_t)(er) >> 16) /* メインエラーコード */
#define SERCD(er) ((int32_t)(er) & 0xffff) /* サブエラーコード */
#define ERCD(mer, ser) (((int32_t)(mer) << 16) | ((int32_t)(ser) & 0xffff))

/*
 * データ型
 */
typedef INT FN; /* 機能コード */
typedef UW ATR; /* オブジェクト/ハンドラ属性 */
#undef E_ABORT
typedef enum { /* エラーコード */
	E_OK = 0,
	E_SYS = ERCD(-5, 0), // システムエラー
	E_NOCOP = ERCD(-6, 0), // システムエラー
	E_NOSPT = ERCD(-9, 0), // 未サポート機能
	E_RSFN = ERCD(-10, 0), // 予約機能コード
	E_RSATR = ERCD(-11, 0), // 予約属性
	E_PAR = ERCD(-17, 0), // パラメータエラー
	E_ID = ERCD(-18, 0), // 不正ID番号

	E_CTX = ERCD(-25, 0), // コンテキストエラー
	E_MACV = ERCD(-26, 0), // メモリアクセス違反
	E_OACV = ERCD(-27, 0), // オブジェクトアクセス違反
	E_ILUSE = ERCD(-28, 0), // サービスコール不正使用

	E_NOMEM = ERCD(-33, 0), // メモリ不足
	E_LIMIT = ERCD(-34, 0), // システム制限超過

	E_OBJ = ERCD(-41, 0),  // オブジェクト状態エラー
	E_NOEXS = ERCD(-42, 0),  // オブジェクト未生成
	E_QOVR = ERCD(-43, 0),  // キューイングオーバフロー

	E_RLWAI = ERCD(-49, 0),  // 待ち状態の強制解除
	E_TMOUT = ERCD(-50, 0), // ポーリング失敗またはタイムアウト
	E_DLT = ERCD(-51, 0), // 待ちオブジェクトが削除された
	E_DISWAI = ERCD(-52, 0), // 待ち禁止による待ち解除

	E_IO = ERCD(-57, 0), // 入出力エラー

	E_NOMDA = ERCD(-58, 0), // メディアがない

	E_BUSY = ERCD(-65, 0), // ビジー状態
	E_ABORT = ERCD(-66, 0), // 中止
	E_RONLY = ERCD(-67, 0), // 書込み禁止
} ER;
typedef INT PRI; /* 優先度 */
typedef W TMO; /* ミリ秒単位のタイムアウト指定 */
typedef D TMO_U; /* 64ビットでマイクロ秒単位のタイムアウト指定 */
typedef UW RELTIM; /* ミリ秒単位の相対時間 */
typedef UD RELTIM_U; /* 64ビットでマイクロ秒単位の相対時間 */
typedef struct systim { /* ミリ秒単位のシステム時刻 */
	W hi; /* 上位32ビット */
	UW lo; /* 下位32ビット */
} SYSTIM;
typedef D SYSTIM_U; /* 64ビットでマイクロ秒単位のシステム時刻 */

/* オブジェクトの状態 */
typedef enum {
	TTS_NONE = 0x00, // 未生成
	TTS_RUN = 0x01, // 実行状態
	TTS_RDY = 0x02, // 実行可能状態
	TTS_WAI = 0x04, // 待ち状態
	TTS_SUS = 0x08, // 強制待ち状態
	TTS_WAS = 0x0c, // 二重待ち状態
	TTS_DMT = 0x10, // 休止状態
	TTS_NODISWAI = 0x80, // 待ち禁止拒否状態

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
#define TA_TFIFO	0x00	// 待ちタスクをFIFOで管理
#define TA_TPRI		0x01	// 待ちタスクを優先度順で管理
#define TA_MFIFO	0x00	// メッセージをFIFOで管理
#define TA_MPRI		0x02	// メッセージを優先度順で管理
#define TA_DSNAME	0x40	// DSオブジェクト名称を指定
#define TA_NODISWAI	0x80	// 待ち禁止拒否

extern ER utkWrapperInit(void);

/*
 * タスク
 */
#define TKW_MAX_TSKID 256 // 生成可能な最大タスク数 ... 本ラッパー固有の定義

#define TSK_SELF	0 // 自タスク指定
#define TPRI_INI	0 // タスク起動時の優先度に戻す
#define TPRI_RUN	0 // 実行状態の優先度を指定

#define TA_ASM		0x0000 // アセンブリ言語によるプログラム
#define TA_HLNG		0x0001 // 高級言語によるプログラム
#define TA_STA		0x0002 // 周期ハンドラ起動
#define TA_PHS		0x0004 // 周期ハンドラ起動位相を保存
#define TA_USERBUF	0x0020 // ユーザバッファポインタを指定
#define TA_RNG0		0x0000 // 保護レベル0 で実行
#define TA_RNG1		0x0100 // 保護レベル1 で実行
#define TA_RNG2		0x0200 // 保護レベル2 で実行
#define TA_RNG3		0x0300 // 保護レベル3 で実行
#define TA_COP0		0x1000 // ID=0 のコプロセッサを使用
#define TA_COP1		0x2000 // ID=1 のコプロセッサを使用
#define TA_COP2		0x4000 // ID=2 のコプロセッサを使用
#define TA_COP3		0x8000 // ID=3 のコプロセッサを使用

typedef struct {
	void* exinf; // Extended Information 拡張情報
	ATR tskatr; // Task Attribute タスク属性
	FP task; // Task Start Address タスク起動アドレス
	PRI itskpri; // Initial Task Priority タスク起動時優先度
	SZ stksz; // Stack Size スタックサイズ(バイト数)
	SZ sstksz; // System Stack Size システムスタックサイズ(バイト数)
	void* stkptr; // User Stack Pointer ユーザスタックポインタ
	UB dsname[8]; // DS Object name DSオブジェクト名称
	void* bufptr; // Buffer Pointer ユーザバッファポインタ
} T_CTSK;

typedef struct {
	void* exinf; // Extended Information 拡張情報
	PRI tskpri; // Task Priority 現在の優先度
	PRI tskbpri; // Task Base Priority ベース優先度
	UINT tskstat; // Task State タスク状態
	UW tskwait; // Task Wait Factor 待ち要因
	ID wid; // Waiting Object ID 待ちオブジェクトID
	INT wupcnt; // Wakeup Count 起床要求キューイング数
	INT suscnt; // Suspend Count 強制待ち要求ネスト数
	UW waitmask; // Wait Mask 待ちを禁止されている待ち要因
	UINT texmask; // Task Exception Mask 許可されているタスク例外
	UINT tskevent; // Task Event 発生しているタスクイベント

	T_CTSK ctsk;
	INT startCode;
	ID taskId; // タスクID
	thrd_t c11thrd;
	mtx_t c11mtxSlp;
	cnd_t c11cndSlp;
} T_RTSK;

extern ID tk_cre_tsk(CONST T_CTSK *pCreTask);
extern ER tk_del_tsk(ID taskId);
extern ER tk_sta_tsk(ID taskId, INT startCode);
extern void tk_ext_tsk(void);
extern void tk_exd_tsk(void);
extern ER tk_ref_tsk(ID taskId, T_RTSK* pTask);
extern ER tk_slp_tsk(TMO waitMs);
extern ER tk_wup_tsk(ID taskId);
extern ER tk_dly_tsk(RELTIM delayMs);

/*
 * セマフォ
 */
#define TKW_MAX_SEMID 512 // 生成可能な最大セマフォ数 ... 本ラッパー固有の定義
#define TKW_MAX_MAXSEM 256 // セマフォ資源数の最大値 ... 本ラッパー固有の定義

typedef struct {
	void* exinf;	// Extended Information 拡張情報
	ATR sematr;		// Semaphore Attribute セマフォ属性
	INT isemcnt;	// Initial Semaphore Count セマフォ資源数の初期値
	INT maxsem;		// Maximum Semaphore Count セマフォ資源数の最大値
	UB dsname[8];	// DS Object name DSオブジェクト名称
} T_CSEM;

typedef struct {
	void* exinf;	// Extended Information 拡張情報
	ID wtsk;		// Waiting Task ID 待ちタスクのID
	INT semcnt;		// Semaphore Count 現在のセマフォ資源数

	bool valid;
	T_CSEM csem;
	ID semId;
	INT maxSemCnt;
	mtx_t c11mtx;
	cnd_t c11cnd;
} T_RSEM;

extern ER tk_cre_sem(T_CSEM* pCreSem);
extern ER tk_del_sem(ID semId);
extern ER tk_sig_sem(ID semId, INT count);
extern ER tk_wai_sem(ID semId, INT count, TMO timeoutMs);

/*
 * メッセージボックス
 */
#define TKW_MAX_MBXID 512 // 生成可能な最大メールボックス数 ... 本ラッパー固有の定義
#define TKW_MAX_MAXMBX 256 // メールボックスに送信可能な最大メッセージ数
						   // ... 本ラッパー固有の定義.
						   // ITRON系OSのメールボックスに送信可能なメッセージ数の
						   // 上限は無いが、本実装には上限がある.

typedef uintptr_t T_MSG;

typedef struct t_cmbx {
	void* exinf; // Extended Information 拡張情報
	ATR mbxatr; // Mailbox Attribute メールボックス属性
	UB dsname[8]; // DS Object name DSオブジェクト名称
} T_CMBX;

typedef struct t_rmbx {
	void* exinf; // Extended Information 拡張情報
	ID wtsk; // Wait Task Information 待ちタスクのID
	T_MSG* pk_msg; // Packet of Message 次に受信されるメッセージ

	bool valid;
	T_CMBX cmbx;
	void** mbxData;
	uint32_t msgNum;
	uint32_t head;
	uint32_t tail;
	mtx_t c11mtx;
	cnd_t c11cndRecv;
} T_RMBX;

extern ID tk_cre_mbx(CONST T_CMBX* pCreMbx);
extern ER tk_snd_mbx(ID mbxId, T_MSG* pMsg);
extern ER tk_rcv_mbx(ID mbxId, T_MSG** ppMsg, TMO timeoutMs);

extern ID tk_get_tid(void);

