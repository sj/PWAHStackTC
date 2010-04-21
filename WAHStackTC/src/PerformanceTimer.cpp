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

double PerformanceTimer::reset(){
	double res = currRunTimeMicro() / 1000.0;
	struct timeval now;
	gettimeofday(&now, NULL);
	_startTime = now;
	return res;
}

double PerformanceTimer::diffTimeMilliSecs(const timeval& time1, const timeval& time2){
	double millisecs1 = time1.tv_sec * 1000 + time1.tv_usec / 1000.0;
	double millisecs2 = time2.tv_sec * 1000 + time2.tv_usec / 1000.0;
	return millisecs2 - millisecs1;
}

long PerformanceTimer::diffTimeMicroSecs(const timeval& time1, const timeval& time2){
	long microsecs1 = time1.tv_sec * 1000000 + time1.tv_usec;
	long microsecs2 = time2.tv_sec * 1000000 + time2.tv_usec;
	return microsecs2 - microsecs1;
}

double PerformanceTimer::currRunTime(){
	struct timeval now;
	gettimeofday(&now, NULL);
	return diffTimeMilliSecs(_startTime, now);
}

long PerformanceTimer::currRunTimeMicro(){
	struct timeval now;
	gettimeofday(&now, NULL);
	return diffTimeMicroSecs(_startTime, now);
}
