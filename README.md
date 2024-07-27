# RTOS Wrapper for Win32

## 概要
Win32 環境で RTOS 上で動作するアプリケーションをシミュレータとして
動作させるために開発した RTOS API ラッパーです。  
拙作の「C11 マルチスレッドライブラリ for Win32」を用いて、
以下 3 種類の RTOS API ラッパーを実装しています。  

## 対応 RTOS

| RTOS           |
|----------------|
| μITRON 4.0     |
| μT-Kernel 2.0  |
| VxWorks 7.x    |


### μITRON 4.0
μITRON 4.0 ラッパーは、以下の API に対応しています。

* タスク管理
	| API            |
	|----------------|
	| cre_tsk()      |
	| acre_tsk()     |
	| del_tsk()      |
	| act_tsk()      |
	| sta_tsk()      |
	| ext_tsk()      |
	| exd_tsk()      |
	| ref_tsk()      |
	| slp_tsk()      |
	| tslp_tsk()     |
	| wup_tsk()      |
	| dly_tsk()      |

* セマフォ
	| API            |
	|----------------|
	| cre_sem()      |
	| del_sem()      |
	| sig_sem()      |
	| wai_sem()      |
	| twai_sem()     |

* メールボックス
	| API            | 制限事項                                                 |
	|----------------|----------------------------------------------------------|
	| cre_mbx()      | メッセージキューによる実装であるため、格納数に上限がある |
	| snd_mbx()      |                                                          |
	| rcv_mbx()      |                                                          |
	| trcv_mbx()     |

* システム管理
	| API            |
	|----------------|
	| get_tid()      |


### μT-Kernel 2.0
μT-Kernel 2.0 ラッパーは、以下の API に対応しています。

* タスク管理
	| API            |
	|----------------|
	| tk_cre_tsk()   |
	| tk_del_tsk()   |
	| tk_sta_tsk()   |
	| tk_ext_tsk()   |
	| tk_exd_tsk()   |
	| tk_ref_tsk()   |
	| tk_slp_tsk()   |
	| tk_wup_tsk()   |
	| tk_dly_tsk()   |

* セマフォ
	| API            |
	|----------------|
	| tk_cre_sem()   |
	| tk_del_sem()   |
	| tk_sig_sem()   |
	| tk_wai_sem()   |
	| twai_sem()     |

* メールボックス
	| API            | 制限事項                                                 |
	|----------------|----------------------------------------------------------|
	| tk_cre_mbx()   | メッセージキューによる実装であるため、格納数に上限がある |
	| tk_snd_mbx()   |                                                          |
	| tk_rcv_mbx()   |                                                          |

* システム管理
	| API            |
	|----------------|
	| get_tid()      |

### VxWorks 7.x
VxWorks 7.x ラッパーは、以下の API に対応しています。

* タスク管理
	| API            |
	|----------------|
	| taskSpawn()    |
	| taskCreate()   |
	| taskActivate() |
	| taskName()     |
	| taskIdSelf()   |
	| taskDelay()    |

* セマフォ
	| API            | 制限事項                        |
	|----------------|---------------------------------|
	| semBCreate()   |                                 |
	| semMCreate()   |                                 |
	| semCCreate()   |                                 |
	| semDelete()    | 未対応                          |
	| semTake()      | カウンティング セマフォは未対応 |
	| semGive()      | カウンティング セマフォは未対応 |
	| semFlush()     | カウンティング セマフォは未対応 |

* メッセージ キュー
	| API            | 制限事項                          |
	|----------------|-----------------------------------|
	| msgQCreate()   |                                   |
	| msgQSend()     | priority: MSG_PRI_URGENT は未対応 |
	| msgQReceive()  |                                   |
	| msgQDelete()   | 未対応                            |


---
## 使用方法
RTOS ラッパーを使用する前に、RTOS ラッパー毎の初期化 API を、
シングルスレッドで動作している状態で実行してください。  
* RTOS Wrapper 毎の初期化 API
	| RTOS Wrapper   | 初期化 API          |
	|----------------|---------------------|
	| μITRON 4.0     | uitronWrapperInit() |
	| μT-Kernel 2.0  | utkWrapperInit()    |
	| VxWorks 7.x    | taskLibInit()       |

対象アプリケーションが本ラッパーで未実装の API を使用している場合には、
スタブ関数等を実装してください。


## ビルド

Visual Studio Community 2022 用のソリューションファイル
rtos_wrapper_win32.sln に 4 個のプロジェクトが含まれています。  
各 RTOS ラッパーで必要とするファイルはこれらのプロジェクトを
参考にしてください。

---
## ソースコード ライセンス

「zlib / libpng ライセンス」を適用しています。  
本ソフトウェアの使用によって生じるいかなる損害についても、
作者は一切の責任を負いませんが、
商用アプリケーションを含めて、自由に改変して利用してください。  

---
## 変更点

### 2023/12/31
 * 1st リリース

### 2024/07/24
 * VxWorks Wrapper: タスクの生成と削除が競合した場合に taskIdSelf() が
   誤ったタスク ID を返却してしまう不具合を修正
