#include "stdafx.h"
#include "timekeeper.h"

// タスクトレイ上のアイコンの右クリックメニューを作成する
HMENU createMenu(HWND hWnd, HINSTANCE hInstance, TCHAR *title, NOTIFYICONDATA *pnid)
{
	HMENU hMenu = CreatePopupMenu();
	MENUITEMINFO menuiteminfo[4], *mp;

	mp = menuiteminfo;
	mp->cbSize = sizeof(*menuiteminfo);
	mp->fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mp->fType = MFT_STRING;
	mp->fState = MFS_DEFAULT | MFS_HILITE;
	mp->wID = IDD_PROPBOX;
	mp->dwTypeData = L"時間設定...";
	mp->cch = wcslen(mp->dwTypeData);
	InsertMenuItem(hMenu, mp->wID, false, mp);

	mp++;
	mp->cbSize = sizeof(*menuiteminfo);
	mp->fMask = MIIM_TYPE | MIIM_ID;
	mp->fType = MFT_STRING;
	mp->wID = IDD_ABOUTBOX;
	mp->dwTypeData = L"このソフトウェアについて...";
	mp->cch = wcslen(mp->dwTypeData);
	InsertMenuItem(hMenu, mp->wID, false, mp);

	mp++;
	mp->cbSize = sizeof(*menuiteminfo);
	mp->fMask = MIIM_TYPE;
	mp->fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 0, false, mp);

	mp++;
	mp->cbSize = sizeof(*menuiteminfo);
	mp->fMask = MIIM_TYPE | MIIM_ID;
	mp->fType = MFT_STRING;
	mp->wID = IDM_EXIT;
	mp->dwTypeData = L"終了";
	mp->cch = wcslen(mp->dwTypeData);
	InsertMenuItem(hMenu, mp->wID, false, mp);

	pnid->cbSize = sizeof(NOTIFYICONDATA);
	pnid->hWnd = hWnd;
	pnid->uID = 0;
	pnid->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	pnid->uCallbackMessage = WM_TRAYICONMESSAGE;
	pnid->hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcsncpy_s(pnid->szTip, title, _TRUNCATE);
	Shell_NotifyIcon(NIM_ADD, pnid);

	return hMenu;
}
