#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG

void debugPrint(WCHAR *str)
{
	HWND hwnd = 0;
	HDC hdc = GetDC(hwnd);

	long textColorOld = SetTextColor(hdc, RGB(0, 0, 0));
	long bkColorOld = SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	int bkModeOld = SetBkMode(hdc, OPAQUE);

	HFONT hFont = CreateFont(11, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
					SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
					FIXED_PITCH | FF_MODERN, L"ÇlÇr ÉSÉVÉbÉN");
	HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);

	TextOut(hdc, 120, 1, L"                                                                                       ", 
	              wcslen(L"                                                                                       "));
	TextOut(hdc, 120, 1, str, wcslen(str));

	SelectObject(hdc, hFontOld);
	DeleteObject(hFont);

	SetTextColor(hdc, textColorOld);
	SetBkColor(hdc, bkColorOld);
	SetBkMode(hdc, bkModeOld);

	ReleaseDC(hwnd, hdc);
}

void debugPrintInt(int i)
{
	WCHAR buf[21];
	_snwprintf_s(buf, _countof(buf), L"%d", i);
	debugPrint(buf);
}

void debugPrintHex(int i)
{
	WCHAR buf[21];
	_snwprintf_s(buf, _countof(buf), L"%x", i);
	debugPrint(buf);
}

#else

void debugPrint(WCHAR *str)
{
	// âΩÇ‡ÇµÇ»Ç¢
}
void debugPrintInt(int i)
{
	// âΩÇ‡ÇµÇ»Ç¢
}
void debugPrintHex(int i)
{
	// âΩÇ‡ÇµÇ»Ç¢
}

#endif
