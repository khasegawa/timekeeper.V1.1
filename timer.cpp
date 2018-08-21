#include "stdafx.h"
#include "timekeeper.h"

#define ID_TIMER  1

#define INTERVAL    1000 // �Ď��Ԋu(ms)
#define SIGNONTIME  300  // �_��ON����(ms)  
#define SIGNOFFTIME 150  // �_��OFF����(ms)
#define SCRALMTIME  60   // �X�N���[���Z�[�o�\������(s)

static int gFlushcount;  // �_�ŉ�

extern void showBar(bool mode);

static void CALLBACK _timerFunc(HWND, UINT, UINT *, DWORD);
static void CALLBACK _timerFuncHideSign(HWND, UINT, UINT *, DWORD);

// ���ԊĎ��p�^�C�}�[���N������
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

// ���ԊĎ��p�̃^�C�}�[�֐�
static void CALLBACK _timerFunc(HWND hWnd, UINT uMsg, UINT *pidEvent, DWORD dwTime)
{
	// �O���t�\���̈��\���E�X�V
	showBar(true);

	// �ȉ��A�X�N���[���Z�[�o�̏�ԃ`�F�b�N
	BOOL active;
	if (! SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &active, 0) || !active) {
		return;  // �X�N���[���Z�[�o���ݒ肳��Ă��Ȃ� �� �������Ȃ�
	}

	BOOL running;
	if (! SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &running, 0) || running) {
		return;  // �X�N���[���Z�[�o���쒆 �� �������Ȃ�
	}

	int waittime;
	if (! SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &waittime, 0)) {
		return;  // �X�N���[���Z�[�o�ҋ@���Ԃ��擾�ł��Ȃ� �� �������Ȃ�
	}

	LASTINPUTINFO lii;
	lii.cbSize = sizeof (LASTINPUTINFO);
	if (! GetLastInputInfo(&lii)) {
		return;  // ���[�U�ŏI���쎞�����擾�ł��Ȃ� �� �������Ȃ�
	}

	DWORD idol = (GetTickCount() - lii.dwTime) / 1000; // ms -> s
	int rest = waittime - idol;
	debugPrintInt(rest); // �c�莞�ԕ\��(�f�o�b�O��)

	// �X�N���[���Z�[�o�N���܂�60�b��؂��Ă���ꍇ�͓_��
	if (rest < SCRALMTIME) {
		gFlushcount = 2;
		_timerFuncHideSign(hWnd, (UINT)0, (UINT *)0, (DWORD)0);
	}
}

// �u�T�C���v��_����^�C�}�[�֐�
static void CALLBACK _timerFuncShowSign(HWND hWnd, UINT uMsg, UINT *pidEvent, DWORD dwTime)
{
	if (--gFlushcount > 0) {
		// �u�T�C���v�������^�C�}�[�ɒu�������Ă���
		(void)SetTimer(hWnd, (UINT)ID_TIMER, (UINT)SIGNONTIME, (TIMERPROC)_timerFuncHideSign);
	} else {
		// �Ď��^�C�}�[�ɒu�������Ă���
		(void)SetTimer(hWnd, (UINT)ID_TIMER, (UINT)INTERVAL, (TIMERPROC)_timerFunc);
	}
	// �u�T�C���v��_����
	showBar(true);
}

// �u�T�C���v�������^�C�}�[�֐�
static void CALLBACK _timerFuncHideSign(HWND hWnd, UINT uMsg, UINT *pidEvent, DWORD dwTime)
{
	// �u�T�C���v��_����^�C�}�[�ɒu�������Ă���
	(void)SetTimer(hWnd, (UINT)ID_TIMER, (UINT)SIGNOFFTIME, (TIMERPROC)_timerFuncShowSign);
	// �u�T�C���v������
	showBar(false);
}
