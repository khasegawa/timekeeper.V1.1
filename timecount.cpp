#include <time.h>

static time_t gTimeStart = 0;  // �J�n����(�b)
static time_t gTimeLap = 0;    // ��~����(�b)
static long   gTimePause = 0;  // ��~����(�b)

void timeStart()
{
	(void)time(&gTimeStart); // �v���J�n
	gTimePause = 0;          // �v���J�n
	gTimeLap = 0;            // �v���J�n
}

void timePause()
{
	(void)time(&gTimeLap);   // ��~������ݒ�
}

void timeResume()
{
	time_t now;
	gTimePause += (long)(time(&now) - gTimeLap);  // ��~���ԍ��Z
	gTimeLap = 0;
}

// ���o�ߎ���(�b)���Z�o����
long getTimeRun()
{
	long timeRun;
	if (gTimeStart == 0) {
		timeRun = 0;
	} else if (gTimeLap == 0) {
		time_t now;
		timeRun = (long)(time(&now) - gTimeStart) - gTimePause;
	} else {
		timeRun = (long)(gTimeLap - gTimeStart) - gTimePause;
	}
	return timeRun;
}
