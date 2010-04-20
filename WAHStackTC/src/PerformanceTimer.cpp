/*
 * PerformanceTimer.cpp
 *
 *  Created on: Apr 13, 2010
 *      Author: bas
 */

#include "PerformanceTimer.h"
#include <sys/time.h>
#include <time.h>

PerformanceTimer::PerformanceTimer() {
	// TODO Auto-generated constructor stub

}

PerformanceTimer::~PerformanceTimer() {
	// TODO Auto-generated destructor stub
}

PerformanceTimer PerformanceTimer::start(){
	PerformanceTimer timer;
	gettimeofday(&timer._startTime, NULL);
	return timer;
}

long PerformanceTimer::reset(){
	long res = currRunTime();
	struct timeval now;
	gettimeofday(&now, NULL);
	_startTime = now;
	return res;
}

long PerformanceTimer::diffTimeMilliSecs(const timeval& time1, const timeval& time2){
	long millisecs1 = time1.tv_sec * 1000 + time1.tv_usec / 1000;
	long millisecs2 = time2.tv_sec * 1000 + time2.tv_usec / 1000;
	return millisecs2 - millisecs1;
}

long PerformanceTimer::diffTimeMicroSecs(const timeval& time1, const timeval& time2){
	long microsecs1 = time1.tv_sec * 1000000 + time1.tv_usec;
	long microsecs2 = time2.tv_sec * 1000000 + time2.tv_usec;
	return microsecs2 - microsecs1;
}

long PerformanceTimer::currRunTime(){
	struct timeval now;
	gettimeofday(&now, NULL);
	long res = diffTimeMilliSecs(_startTime, now);
	return res;
}

long PerformanceTimer::currRunTimeMicro(){
	struct timeval now;
	gettimeofday(&now, NULL);
	long res = diffTimeMicroSecs(_startTime, now);
	return res;
}
