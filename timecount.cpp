#include <time.h>

static time_t gTimeStart = 0;  // 開始時刻(秒)
static time_t gTimeLap = 0;    // 停止時刻(秒)
static long   gTimePause = 0;  // 停止時間(秒)

void timeStart()
{
	(void)time(&gTimeStart); // 計測開始
	gTimePause = 0;          // 計測開始
	gTimeLap = 0;            // 計測開始
}

void timePause()
{
	(void)time(&gTimeLap);   // 停止時刻を設定
}

void timeResume()
{
	time_t now;
	gTimePause += (long)(time(&now) - gTimeLap);  // 停止時間合算
	gTimeLap = 0;
}

// 実経過時間(秒)を算出する
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
