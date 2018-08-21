#pragma once

#include <crtdbg.h>
#include "resource.h"
#include "debugprint.h"

#define WM_TRAYICONMESSAGE WM_USER

#define UIFONT L"MS UI Gothic"

extern BOOL startTimer(HWND hWnd);
extern BOOL stopTimer(HWND hWnd);
extern HMENU createMenu(HWND hWnd, HINSTANCE hInstance, TCHAR *title, NOTIFYICONDATA *pnid);
extern void showDialog(HINSTANCE hInst, HWND hWnd, HMENU hMenu, int menuid, int resid, INT_PTR CALLBACK func(HWND, UINT, WPARAM, LPARAM));
extern void timeStart();
extern void timePause();
extern void timeResume();
extern long getTimeRun();
