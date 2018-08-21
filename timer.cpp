#include "stdafx.h"
#include "timekeeper.h"

#define ID_TIMER  1

#define INTERVAL    1000 // 監視間隔(ms)
#define SIGNONTIME  300  // 点滅ON時間(ms)  
#define SIGNOFFTIME 150  // 点滅OFF時間(ms)
#define SCRALMTIME  60   // スクリーンセーバ予告時間(s)

static int gFlushcount;  // 点滅回数

extern void showBar(bool mode);

static void CALLBACK _timerFunc(HWND, UINT, UINT *, DWORD);
static void CALLBACK _timerFuncHideSign(HWND, UINT, UINT *, DWORD);

// 時間監視用タイマーを起動する
BOOL startTimer(HWND hWnd)
{
	if (SetTimer(hWnd, (UINT)ID_TIMER, (UINT)INTERVAL, (TIMERPROC)_timerFunc) == 0) {
		return FALSE;
	}
	return TRUE;
}

BOOL stopTimer(HWND hWnd)
{
	if (! KillTimer(hWnd, (UINT)ID_TIMER)) {
		return FALSE;
	}
	return TRUE;
}

// 時間監視用のタイマー関数
static void CALLBACK _timerFunc(HWND hWnd, UINT uMsg, UINT *pidEvent, DWORD dwTime)
{
	// グラフ表示領域を表示・更新
	showBar(true);

	// 以下、スクリーンセーバの状態チェック
	BOOL active;
	if (! SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &active, 0) || !active) {
		return;  // スクリーンセーバが設定されていない → 何もしない
	}

	BOOL running;
	if (! SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &running, 0) || running) {
		return;  // スクリーンセーバ動作中 → 何もしない
	}

	int waittime;
	if (! SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &waittime, 0)) {
		return;  // スクリーンセーバ待機時間が取得できない → 何もしない
	}

	LASTINPUTINFO lii;
	lii.cbSize = sizeof (LASTINPUTINFO);
	if (! GetLastInputInfo(&lii)) {
		return;  // ユーザ最終操作時刻が取得できない → 何もしない
	}

	DWORD idol = (GetTickCount() - lii.dwTime) / 1000; // ms -> s
	int rest = waittime - idol;
	debugPrintInt(rest); // 残り時間表示(デバッグ時)

	// スクリーンセーバ起動まで60秒を切っている場合は点滅
	if (rest < SCRALMTIME) {
		gFlushcount = 2;
		_timerFuncHideSign(hWnd, (UINT)0, (UINT *)0, (DWORD)0);
	}
}

// 「サイン」を点けるタイマー関数
static void CALLBACK _timerFuncShowSign(HWND hWnd, UINT uMsg, UINT *pidEvent, DWORD dwTime)
{
	if (--gFlushcount > 0) {
		// 「サイン」を消すタイマーに置き換えてから
		(void)SetTimer(hWnd, (UINT)ID_TIMER, (UINT)SIGNONTIME, (TIMERPROC)_timerFuncHideSign);
	} else {
		// 監視タイマーに置き換えてから
		(void)SetTimer(hWnd, (UINT)ID_TIMER, (UINT)INTERVAL, (TIMERPROC)_timerFunc);
	}
	// 「サイン」を点ける
	showBar(true);
}

// 「サイン」を消すタイマー関数
static void CALLBACK _timerFuncHideSign(HWND hWnd, UINT uMsg, UINT *pidEvent, DWORD dwTime)
{
	// 「サイン」を点けるタイマーに置き換えてから
	(void)SetTimer(hWnd, (UINT)ID_TIMER, (UINT)SIGNOFFTIME, (TIMERPROC)_timerFuncShowSign);
	// 「サイン」を消す
	showBar(false);
}
