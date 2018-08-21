// timekeeper.cpp : �A�v���P�[�V�����̃G���g���|�C���g���`
//

#include "stdafx.h"
#include <crtdbg.h>
#include "timekeeper.h"
#include "util.h"

#define BARHEIGHT 2
#define MAX_LOADSTRING 100

static HINSTANCE ghInst;              // ���݂̃C���^�[�t�F�C�X
static TCHAR gTitle[MAX_LOADSTRING];  // �^�C�g�� �o�[�̃e�L�X�g
static WCHAR *gProgPath;              // ���̃v���O�����t�@�C���̃t���p�X

static HWND  ghWnd;     // ���C���E�B���h�E�n���h��
static HMENU ghMenu;    // ���j���[�n���h��
static HWND  ghWndBar;  // ���ԃo�[�E�B���h�E�n���h��
static RECT  gRcProg;   // ���ԃo�[�̕`��̈�

static long  gTimeLen = 600;  // �ݒ莞��(�b)

// ���Ԑݒ�_�C�A���O�̃��b�Z�[�W�n���h��
static INT_PTR CALLBACK _setTime(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hWndTime = GetDlgItem(hWndDlg, IDTIME); // �ݒ莞�ԕ\���E�B���h�E�n���h��
	WCHAR timeLenStrBuf[4];  // �ݒ莞�Ԃ𕪂Ƃ��ĕ\�����镶����(999���܂�)

	static bool initial = true;
	static bool changed = false;

	switch (message) {
		case WM_INITDIALOG:
		{
			setWindowRightBottom(hWndDlg);  // �_�C�A���O���X�N���[���E�����ɕ\������

			wsprintf(timeLenStrBuf, L"%d", gTimeLen / 60);
			SetWindowText(hWndTime, timeLenStrBuf);

			HFONT hFont = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Tahoma");
			SendMessage(hWndTime, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			if (initial) {
				// �A�v���P�[�V�����N�����̕\��
				EnableWindow(GetDlgItem(hWndDlg, IDTIMECONT), false); // �u�p���v�{�^���͉����Ȃ�
				initial = false;
			} else {
				// �A�v���P�[�V�������쒆�ł̕\��
				EnableWindow(GetDlgItem(hWndDlg, IDTIMECONT), true);
				timePause();  // �v�����f
			}
			changed = false;
			return (INT_PTR)TRUE;
		}

		case WM_COMMAND:
		{
			WORD cmdid = LOWORD(wParam);

			if (cmdid == IDTIMESTART) {
				// �J�n�{�^������
				GetWindowText(hWndTime, timeLenStrBuf, sizeof timeLenStrBuf / sizeof *timeLenStrBuf);
				gTimeLen = wcstol(timeLenStrBuf, NULL, 10) * 60;
				timeStart();  // �v���J�n
				EndDialog(hWndDlg, cmdid);
				return (INT_PTR)TRUE;

			} else if (cmdid == IDTIMECONT) {
				// �p���{�^������
				timeResume();  // �v���ĊJ
				EndDialog(hWndDlg, cmdid);
				return (INT_PTR)TRUE;

			} else if (cmdid == IDC_BUTTONC) {
				// �N���A�{�^������
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
					// �_�C�A���O���J��������A�܂��͕\�����u0�v�̂Ƃ��͉��������{�^�������̂܂ܕ\��
					wcscpy_s(timeLenStrBuf, bval);
					changed = true;
				} else if (wcslen(timeLenStrBuf) < sizeof timeLenStrBuf / sizeof *timeLenStrBuf - 1) {
					// �������������̏ꍇ�͉��������{�^�����E�l�߂ŕt��
					wcscat_s(timeLenStrBuf, bval);
				} else {
					// ���������ɒB�����ꍇ�͐����{�^�������s��
					Beep(880, 50);
				}

				// �ݒ�l��0���̏ꍇ�A�J�n�{�^���������Ȃ�
				bool startable = (wcscmp(timeLenStrBuf, L"0") != 0);
				EnableWindow(GetDlgItem(hWndDlg, IDTIMESTART), startable);

				SetWindowText(hWndTime, timeLenStrBuf);
			}
		}
	}

	return (INT_PTR)FALSE;
}

// �o�[�W�������{�b�N�X�̃��b�Z�[�W�n���h��
static INT_PTR CALLBACK _about(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
		case WM_INITDIALOG:
		{
			setWindowRightBottom(hWndDlg);  // �_�C�A���O���X�N���[���E�����ɕ\������

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

// ���ԃo�[����ʉ����Ɉړ�����
static void _moveBarBottom(HWND hWnd, RECT *rcProg)
{
	int barHeight = BARHEIGHT;
	int scrwidth, scrheight;
	getResolution(&scrwidth, &scrheight);
	SetWindowPos(hWnd, 0, 0, scrheight - barHeight, scrwidth, barHeight, SWP_NOZORDER);
	SetRect(rcProg, 0, 0, scrwidth, barHeight);
}

// ���ԃo�[�\���p�E�B���h�E�̃��b�Z�[�W����
static LRESULT CALLBACK _wndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_CREATE:
			break;

		case WM_MOUSEMOVE:
			// �}�E�X�I�[�o�[�ŕ\��������
			ShowWindow(hWnd, SW_HIDE);
			break;

		case WM_COMMAND:
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// ���Ԑi�s�O���t��\������
			drawPercentGraph(hdc, &gRcProg, getTimeRun(), gTimeLen);

			EndPaint(hWnd, &ps);
			break;
		}

		case WM_MOVE:
			// �^�X�N�o�[�̍�����ς���ꂽ�Ƃ��Ƀo�[�̈ʒu���ύX����Ă��܂��̂Ō��ɖ߂�
			_moveBarBottom(hWnd, &gRcProg);
			break;

		case WM_DESTROY:
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���ԕ\���p�̃E�B���h�E���쐬����
static HWND _createBar(HWND hWnd, HINSTANCE hInstance)
{
	// �E�B���h�E�N���X�̑�����ݒ肷��
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = 0;                         // �X�^�C��
	wc.lpfnWndProc = _wndProc2;
	wc.cbClsExtra = 0;                    // �g�����P
	wc.cbWndExtra = 0;                    // �g�����Q
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hIconSm = wc.hIcon;                // �q�A�C�R��
	wc.hCursor = LoadCursor(NULL, IDC_NO);
	wc.hbrBackground = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
	wc.lpszMenuName = NULL;               // ���j���[��
	wc.lpszClassName = L"TimeBar";        // �E�B���h�E�N���X��

	// �E�B���h�E�N���X��o�^����
	if (RegisterClassEx(&wc) == 0) {
		return 0;
	}

	// �E�B���h�E���쐬����
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

// ���C���E�B���h�E�̃��b�Z�[�W����
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
				MessageBox(hWnd, L"�E�B���h�E�쐬�̃G���[�ł��B�I�����܂��B", L"�G���[", MB_OK);
				DestroyWindow(hWnd);
			}
			break;
		}

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			// �I�����ꂽ���j���[�̉��:
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
				// �^�X�N�g���C�A�C�R����Ƀ}�E�X�I�[�o�[
				long timeRest = gTimeLen - getTimeRun();
				if (timeRest > 0) {
					// ���쒆�̃w���v������(�c�蕪�E�b)
					wsprintf(nid.szTip, L"�c�� %d:%02d", timeRest / 60, timeRest % 60);
				} else {
					// �ݒ莞�Ԍo�ߌ�̃w���v������
					wcscpy_s(nid.szTip, L"�I��");
				}
				Shell_NotifyIcon(NIM_MODIFY, &nid);
			} else if (lParam == WM_LBUTTONDOWN) {
				// �^�X�N�g���C�A�C�R����ō��N���b�N
				_showSetTimeDialog(hWnd);
			} else if (lParam == WM_RBUTTONDOWN) {
				// �^�X�N�g���C�A�C�R����ŉE�N���b�N
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hWnd);  // ���ꂪ�Ȃ��ƁA�|�b�v�A�b�v���j���[�������Ȃ��Ȃ�
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

// �v���O�����G���g���[�|�C���g
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    _CrtSetReportMode(_CRT_ASSERT, 0);

	gProgPath = __wargv[0];
	ghInst = hInstance;

	TCHAR szWindowClass[MAX_LOADSTRING];  // ���C�� �E�B���h�E �N���X��
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

	// �������C���X�^���X�͈����
	ghWnd = FindWindow(szWindowClass, NULL);
	if (ghWnd) {
		MessageBox(NULL, L"���̃v���O�����͊��ɓ����Ă��܂��B\n�^�X�N�g���C���m�F���Ă��������B", gTitle, MB_OK);
		SetForegroundWindow(ghWnd);
		return 0;
	}

	// �A�v���P�[�V�����̏����������s���܂�
	ghWnd = CreateWindow(szWindowClass, gTitle, WS_OVERLAPPEDWINDOW | WS_MINIMIZE,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					NULL, NULL, hInstance, NULL);
	if (!ghWnd) {
		return 0;
	}

#if 0 // �{�̃E�B���h�E�͔�\��
	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);
#endif

	// ���Ԑݒ�_�C�A���O��\��(�����ݒ�)
	_showSetTimeDialog(ghWnd);

	// �����ݒ��A�^�C�}�[�N��
	startTimer(ghWnd);
	
	// �V���[�g�J�b�g�L�[�ǂݍ���
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOTICESCR));

	// ���C�� ���b�Z�[�W ���[�v:
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
