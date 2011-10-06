/*
 * PerformanceTimer.cpp
 *
 *  Created on: Apr 13, 2010
 *      Author: bas
 */

#include "PerformanceTimer.h"
#include <sys/time.h>
#include <time.h>
#include <iostream>
using namespace std;

PerformanceTimer::PerformanceTimer() {
	_storedRunTime = 0;
	_startTime = NULL;
}

PerformanceTimer::~PerformanceTimer() {
	if (_startTime != NULL) delete _startTime;
}

PerformanceTimer PerformanceTimer::start(){
	PerformanceTimer timer;
	timer.resume();
	return timer;
}

double PerformanceTimer::reset(){
	double res = resetAndStop();
	_startTime = new timeval();
	gettimeofday(_startTime, NULL);
	return res;
}

double PerformanceTimer::resetAndStop(){
	double res = currRunTimeMicro() / 1000.0 + _storedRunTime;
	_storedRunTime = 0;
	delete _startTime;
	_startTime = NULL;
	return res;
}

double PerformanceTimer::diffTimeMilliSecs(const timeval* time1, const timeval* time2){
	double millisecs1 = time1->tv_sec * 1000 + time1->tv_usec / 1000.0;
	double millisecs2 = time2->tv_sec * 1000 + time2->tv_usec / 1000.0;
	return millisecs2 - millisecs1;
}

long PerformanceTimer::diffTimeMicroSecs(const timeval* time1, const timeval* time2){
	long microsecs1 = time1->tv_sec * 1000000 + time1->tv_usec;
	long microsecs2 = time2->tv_sec * 1000000 + time2->tv_usec;
	return microsecs2 - microsecs1;
}

double PerformanceTimer::currRunTime(){
	if (_startTime == NULL){
		return _storedRunTime / 1000.0;
	} else {
		struct timeval* now = new timeval();
		gettimeofday(now, NULL);
		double res = _storedRunTime / 1000.0 + diffTimeMilliSecs(_startTime, now);
		delete now;
		return res;
	}
}

long PerformanceTimer::currRunTimeMicro(){
	if (_startTime == NULL){
		return _storedRunTime;
	} else {
		struct timeval* now = new timeval();
		gettimeofday(now, NULL);
		double res =  _storedRunTime + diffTimeMicroSecs(_startTime, now);
		delete now;
		return res;
	}
}

void PerformanceTimer::pause(){
	if (_startTime == NULL) throw string("Can't pause PerformanceTimer: timer is not running!");

	_storedRunTime = currRunTimeMicro();
	delete _startTime;
	_startTime = NULL;
}

void PerformanceTimer::resume(){
	if (_startTime != NULL) throw string("Can't resume PerformanceTimer: timer is still running!");
	_startTime = new timeval();
	gettimeofday(_startTime, NULL);
}

bool PerformanceTimer::running(){
	return _startTime != NULL;
}
