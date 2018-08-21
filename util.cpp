#include "stdafx.h"
#include <crtdbg.h>

#define BLACK       RGB(0,0,0)
#define DARKGRAY    RGB(63,63,63)
#define GRAY        RGB(127,127,127)
#define LIGHTGRAY   RGB(240,240,240)
#define WHITE       RGB(255,255,255)
#define RED         RGB(255,0,0)
#define GREEN       RGB(0,210,0)
#define DARKGREEN   RGB(0,150,0)
#define LIGHTBLUE   RGB(100,127,255)
#define LIGHTYELLOW RGB(255,255,240)

#if 0
/*
 * ������`��
 */
void printText(HDC hdc, int x, int y, WCHAR *str, int size, COLORREF color)
{
	long textColorOld = SetTextColor(hdc, color);
	int bkModeOld = SetBkMode(hdc, TRANSPARENT);

	HFONT hFont = CreateFont(size, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
		FIXED_PITCH | FF_MODERN, UIFONT);
	HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);

	TextOut(hdc, x, y, str, wcslen(str));

	SelectObject(hdc, hFontOld);
	DeleteObject(hFont);

	SetTextColor(hdc, textColorOld);
	SetBkMode(hdc, bkModeOld);
}

#endif // 0

/*
 * �_�O���t��`�悷��
 */
static void _drawBarGraph(HDC hdc, RECT *prc, int permil, COLORREF color)
{
	HBRUSH hbr;
	RECT rcin;

	// �g
//	hbr = CreateSolidBrush(RGB(100, 100, 100));
//	FrameRect(hdc, prc, hbr);

	int margin = 0;
	long width = prc->right - prc->left - margin * 2;
	SetRect(&rcin,
			prc->left + margin,
			prc->top + margin,
			prc->left + margin + width * permil / 1000,
			prc->bottom - margin);

//	DeleteObject(hbr);

	// �g���h��Ԃ�
	hbr = CreateSolidBrush(color);
	FillRect(hdc, &rcin, hbr);
	DeleteObject(hbr);
}

/*
 * �i���\��
 */
void drawPercentGraph(HDC hdc, RECT *prect, long numerator, long denominator)
{
	COLORREF color;
	int permil;

	if (denominator <= 0) {
		permil = 0;
		color = BLACK;
	} else if (numerator < denominator) {
		// �i�s��
		permil = numerator * 1000 / denominator;
		color = GRAY;
	} else {
		// �o�ߌ�
		permil = 1000;
		color = RED;
	}
	_drawBarGraph(hdc, prect, permil, color);
#if 0
	WCHAR buf[6];  // "100 %"�܂ŕ\�� ����ȍ~�͉E������؂�̂�
	_snwprintf_s(buf, _countof(buf), _TRUNCATE, L"%d %%", permil / 10);
	PrintText(hdc, prect->left + 40, prect->top + 0, buf, 8, BLACK);

#endif // 0
}

#if 0
/*
* ���ԕ\��
*/
void dispTime(HWND hWnd, long time)
{
	WCHAR buf[7];  // "9999 �b"�܂ŕ\�� ����ȏ�͉E���̕����؂�̂�
	_snwprintf_s(buf, _countof(buf), _TRUNCATE, L"%d �b", time / 10);

	SetWindowText(hWnd, buf);
}

#endif // 0

// ���݂̉𑜓x�𓾂�
void getResolution(int *xp, int *yp)
{
	DEVMODE devMode;
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;
	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
		*xp = devMode.dmPelsWidth;
		*yp = devMode.dmPelsHeight;
	} else {
		// ����𑜓x���擾�ł��Ȃ��ꍇ�̓K���Ȓl
		*xp = 640;
		*yp = 480;
	}
}

// �E�B���h�E���X�N���[���E�����ɕ\������
void setWindowRightBottom(HWND hWnd)
{
	int scrwidth, scrheight;
	getResolution(&scrwidth, &scrheight);
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int x = scrwidth - (rect.right - rect.left);
	int y = scrheight - (rect.bottom - rect.top);
	SetWindowPos(hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
}

// �_�C�A���O�{�b�N�X���J��(���j���[�̕s��������t)
void showDialog(HINSTANCE hInst, HWND hWnd, HMENU hMenu, int menuid, int resid, INT_PTR CALLBACK func(HWND, UINT, WPARAM, LPARAM))
{
	EnableMenuItem(hMenu, menuid, MF_BYPOSITION | MF_GRAYED);  // SetTime�_�C�A���O���j���[���d�˂ČĂׂȂ��悤�ɂ���
	DialogBox(hInst, MAKEINTRESOURCE(resid), hWnd, func);
	EnableMenuItem(hMenu, menuid, MF_BYPOSITION | MF_ENABLED); // SetTime�_�C�A���O���j���[�����ɖ߂�
}
