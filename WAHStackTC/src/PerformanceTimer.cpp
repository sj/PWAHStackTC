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
	struct timeval now;
	gettimeofday(&now, NULL);
	long res = diffTimeMsecs(_startTime, now);
	_startTime = now;
	return res;
}

long PerformanceTimer::diffTimeMsecs(const timeval& time1, const timeval& time2){
	long msecs1 = time1.tv_sec * 1000 + time1.tv_usec / 1000;
	long msecs2 = time2.tv_sec * 1000 + time2.tv_usec / 1000;
	return msecs2 - msecs1;
}
