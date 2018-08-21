// timekeeper.cpp : アプリケーションのエントリポイントを定義
//

#include "stdafx.h"
#include <crtdbg.h>
#include "timekeeper.h"
#include "util.h"

#define BARHEIGHT 2
#define MAX_LOADSTRING 100

static HINSTANCE ghInst;              // 現在のインターフェイス
static TCHAR gTitle[MAX_LOADSTRING];  // タイトル バーのテキスト
static WCHAR *gProgPath;              // このプログラムファイルのフルパス

static HWND  ghWnd;     // メインウィンドウハンドル
static HMENU ghMenu;    // メニューハンドル
static HWND  ghWndBar;  // 時間バーウィンドウハンドル
static RECT  gRcProg;   // 時間バーの描画領域

static long  gTimeLen = 600;  // 設定時間(秒)

// 時間設定ダイアログのメッセージハンドラ
static INT_PTR CALLBACK _setTime(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hWndTime = GetDlgItem(hWndDlg, IDTIME); // 設定時間表示ウィンドウハンドル
	WCHAR timeLenStrBuf[4];  // 設定時間を分として表示する文字列(999分まで)

	static bool initial = true;
	static bool changed = false;

	switch (message) {
		case WM_INITDIALOG:
		{
			setWindowRightBottom(hWndDlg);  // ダイアログをスクリーン右下部に表示する

			wsprintf(timeLenStrBuf, L"%d", gTimeLen / 60);
			SetWindowText(hWndTime, timeLenStrBuf);

			HFONT hFont = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Tahoma");
			SendMessage(hWndTime, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			if (initial) {
				// アプリケーション起動時の表示
				EnableWindow(GetDlgItem(hWndDlg, IDTIMECONT), false); // 「継続」ボタンは押せない
				initial = false;
			} else {
				// アプリケーション動作中での表示
				EnableWindow(GetDlgItem(hWndDlg, IDTIMECONT), true);
				timePause();  // 計時中断
			}
			changed = false;
			return (INT_PTR)TRUE;
		}

		case WM_COMMAND:
		{
			WORD cmdid = LOWORD(wParam);

			if (cmdid == IDTIMESTART) {
				// 開始ボタン押下
				GetWindowText(hWndTime, timeLenStrBuf, sizeof timeLenStrBuf / sizeof *timeLenStrBuf);
				gTimeLen = wcstol(timeLenStrBuf, NULL, 10) * 60;
				timeStart();  // 計時開始
				EndDialog(hWndDlg, cmdid);
				return (INT_PTR)TRUE;

			} else if (cmdid == IDTIMECONT) {
				// 継続ボタン押下
				timeResume();  // 計時再開
				EndDialog(hWndDlg, cmdid);
				return (INT_PTR)TRUE;

			} else if (cmdid == IDC_BUTTONC) {
				// クリアボタン押下
				SetWindowText(hWndTime, L"0");
				EnableWindow(GetDlgItem(hWndDlg, IDTIMESTART), false);

			} else {
				WCHAR bval[2];
				if (cmdid == IDC_BUTTON1) { wcscpy_s(bval, L"1"); }
				else if (cmdid == IDC_BUTTON2) { wcscpy_s(bval, L"2"); }
				else if (cmdid == IDC_BUTTON3) { wcscpy_s(bval, L"3"); }
				else if (cmdid == IDC_BUTTON4) { wcscpy_s(bval, L"4"); }
				else if (cmdid == IDC_BUTTON5) { wcscpy_s(bval, L"5"); }
				else if (cmdid == IDC_BUTTON6) { wcscpy_s(bval, L"6"); }
				else if (cmdid == IDC_BUTTON7) { wcscpy_s(bval, L"7"); }
				else if (cmdid == IDC_BUTTON8) { wcscpy_s(bval, L"8"); }
				else if (cmdid == IDC_BUTTON9) { wcscpy_s(bval, L"9"); }
				else if (cmdid == IDC_BUTTON0) { wcscpy_s(bval, L"0"); }
				else {
					return (INT_PTR)FALSE;
				}

				GetWindowText(hWndTime, timeLenStrBuf, sizeof timeLenStrBuf / sizeof *timeLenStrBuf);
				if (!changed || wcscmp(timeLenStrBuf, L"0") == 0) {
					// ダイアログを開いた直後、または表示が「0」のときは押下数字ボタンをそのまま表示
					wcscpy_s(timeLenStrBuf, bval);
					changed = true;
				} else if (wcslen(timeLenStrBuf) < sizeof timeLenStrBuf / sizeof *timeLenStrBuf - 1) {
					// 制限桁数未満の場合は押下数字ボタンを右詰めで付加
					wcscat_s(timeLenStrBuf, bval);
				} else {
					// 制限桁数に達した場合は数字ボタン押下不可
					Beep(880, 50);
				}

				// 設定値が0分の場合、開始ボタンを押せない
				bool startable = (wcscmp(timeLenStrBuf, L"0") != 0);
				EnableWindow(GetDlgItem(hWndDlg, IDTIMESTART), startable);

				SetWindowText(hWndTime, timeLenStrBuf);
			}
		}
	}

	return (INT_PTR)FALSE;
}

// バージョン情報ボックスのメッセージハンドラ
static INT_PTR CALLBACK _about(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
		case WM_INITDIALOG:
		{
			setWindowRightBottom(hWndDlg);  // ダイアログをスクリーン右下部に表示する

			int bufSize = GetFileVersionInfoSize(gProgPath, 0);
			if (bufSize > 0) {
				void *buf = (void *)malloc(sizeof(BYTE) * bufSize);
				if (buf != NULL) {
					GetFileVersionInfo(gProgPath, 0, bufSize, buf);

					void *str;
					unsigned int strSize;

					VerQueryValue(buf, L"\\StringFileInfo\\0411fde9\\ProductName", &str, &strSize);
					SetWindowText(hWndDlg, (WCHAR *)str);
					SetWindowText(GetDlgItem(hWndDlg, IDC_EDIT2), (WCHAR *)str);

					VerQueryValue(buf, L"\\StringFileInfo\\0411fde9\\FileVersion", &str, &strSize);
					SetWindowText(GetDlgItem(hWndDlg, IDC_EDIT1), (WCHAR *)str);

					VerQueryValue(buf, L"\\StringFileInfo\\0411fde9\\LegalCopyright", &str, &strSize);
					SetWindowText(GetDlgItem(hWndDlg, IDC_EDIT3), (WCHAR *)str);

					free(buf);
				}
			}
			return (INT_PTR)TRUE;
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hWndDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}

	return (INT_PTR)FALSE;
}

// 時間バーを画面下部に移動する
static void _moveBarBottom(HWND hWnd, RECT *rcProg)
{
	int barHeight = BARHEIGHT;
	int scrwidth, scrheight;
	getResolution(&scrwidth, &scrheight);
	SetWindowPos(hWnd, 0, 0, scrheight - barHeight, scrwidth, barHeight, SWP_NOZORDER);
	SetRect(rcProg, 0, 0, scrwidth, barHeight);
}

// 時間バー表示用ウィンドウのメッセージ処理
static LRESULT CALLBACK _wndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_CREATE:
			break;

		case WM_MOUSEMOVE:
			// マウスオーバーで表示を消す
			ShowWindow(hWnd, SW_HIDE);
			break;

		case WM_COMMAND:
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// 時間進行グラフを表示する
			drawPercentGraph(hdc, &gRcProg, getTimeRun(), gTimeLen);

			EndPaint(hWnd, &ps);
			break;
		}

		case WM_MOVE:
			// タスクバーの高さを変えられたときにバーの位置が変更されてしまうので元に戻す
			_moveBarBottom(hWnd, &gRcProg);
			break;

		case WM_DESTROY:
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 時間表示用のウィンドウを作成する
static HWND _createBar(HWND hWnd, HINSTANCE hInstance)
{
	// ウィンドウクラスの属性を設定する
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = 0;                         // スタイル
	wc.lpfnWndProc = _wndProc2;
	wc.cbClsExtra = 0;                    // 拡張情報１
	wc.cbWndExtra = 0;                    // 拡張情報２
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hIconSm = wc.hIcon;                // 子アイコン
	wc.hCursor = LoadCursor(NULL, IDC_NO);
	wc.hbrBackground = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
	wc.lpszMenuName = NULL;               // メニュー名
	wc.lpszClassName = L"TimeBar";        // ウィンドウクラス名

	// ウィンドウクラスを登録する
	if (RegisterClassEx(&wc) == 0) {
		return 0;
	}

	// ウィンドウを作成する
	HWND hWndBar = CreateWindowEx(WS_EX_TOPMOST, wc.lpszClassName, L"", WS_POPUP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, hInstance, NULL);
	if (hWndBar != 0) {
		_moveBarBottom(hWndBar, &gRcProg);
	}
	return hWndBar;
}

static void _showSetTimeDialog(HWND hWnd)
{
	static bool exist = false;
	if (!exist) {
		exist = true;
		showDialog(ghInst, hWnd, ghMenu, 0, IDD_PROPBOX, _setTime);
		exist = false;
	}
}

static void _showAboutDialog(HWND hWnd)
{
	static bool exist = false;
	if (!exist) {
		exist = true;
		showDialog(ghInst, hWnd, ghMenu, 1, IDD_ABOUTBOX, _about);
		exist = false;
	}
}

// メインウィンドウのメッセージ処理
static LRESULT CALLBACK _wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu;
	static NOTIFYICONDATA nid;

	switch (message) {
		case WM_CREATE:
		{
			ghMenu = hMenu = createMenu(hWnd, ghInst, gTitle, &nid);
			HFONT hFontS = CreateFont(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
								SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY, DEFAULT_PITCH, UIFONT);

			ghWndBar = _createBar(hWnd, ghInst);
			if (!ghWndBar) {
				MessageBox(hWnd, L"ウィンドウ作成のエラーです。終了します。", L"エラー", MB_OK);
				DestroyWindow(hWnd);
			}
			break;
		}

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			// 選択されたメニューの解析:
			switch (wmId) {
				case IDD_PROPBOX:
					_showSetTimeDialog(hWnd);
					break;
				case IDD_ABOUTBOX:
					_showAboutDialog(hWnd);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}

		case WM_TRAYICONMESSAGE:
			if (lParam == WM_MOUSEMOVE) {
				// タスクトレイアイコン上にマウスオーバー
				long timeRest = gTimeLen - getTimeRun();
				if (timeRest > 0) {
					// 動作中のヘルプ文字列(残り分・秒)
					wsprintf(nid.szTip, L"残り %d:%02d", timeRest / 60, timeRest % 60);
				} else {
					// 設定時間経過後のヘルプ文字列
					wcscpy_s(nid.szTip, L"終了");
				}
				Shell_NotifyIcon(NIM_MODIFY, &nid);
			} else if (lParam == WM_LBUTTONDOWN) {
				// タスクトレイアイコン上で左クリック
				_showSetTimeDialog(hWnd);
			} else if (lParam == WM_RBUTTONDOWN) {
				// タスクトレイアイコン上で右クリック
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hWnd);  // これがないと、ポップアップメニューが消えなくなる
				TrackPopupMenuEx(hMenu, TPM_LEFTALIGN, pt.x, pt.y, hWnd, 0);
			}
			break;

		case WM_DESTROY:
			DestroyMenu(hMenu);
			Shell_NotifyIcon(NIM_DELETE, &nid);
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// プログラムエントリーポイント
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    _CrtSetReportMode(_CRT_ASSERT, 0);

	gProgPath = __wargv[0];
	ghInst = hInstance;

	TCHAR szWindowClass[MAX_LOADSTRING];  // メイン ウィンドウ クラス名
	LoadString(hInstance, IDS_APP_TITLE, gTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NOTICESCR, szWindowClass, MAX_LOADSTRING);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = _wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LARGE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_NOTICESCR);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&wcex);

	// 動かすインスタンスは一つだけ
	ghWnd = FindWindow(szWindowClass, NULL);
	if (ghWnd) {
		MessageBox(NULL, L"このプログラムは既に動いています。\nタスクトレイを確認してください。", gTitle, MB_OK);
		SetForegroundWindow(ghWnd);
		return 0;
	}

	// アプリケーションの初期化を実行します
	ghWnd = CreateWindow(szWindowClass, gTitle, WS_OVERLAPPEDWINDOW | WS_MINIMIZE,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					NULL, NULL, hInstance, NULL);
	if (!ghWnd) {
		return 0;
	}

#if 0 // 本体ウィンドウは非表示
	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);
#endif

	// 時間設定ダイアログを表示(初期設定)
	_showSetTimeDialog(ghWnd);

	// 初期設定後、タイマー起動
	startTimer(ghWnd);
	
	// ショートカットキー読み込み
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOTICESCR));

	// メイン メッセージ ループ:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

void showBar(bool mode)
{
	if (mode) {
		ShowWindow(ghWndBar, SW_SHOWNA);
		InvalidateRect(ghWndBar, &gRcProg, TRUE);
	} else {
		ShowWindow(ghWndBar, SW_HIDE);
	}
}
