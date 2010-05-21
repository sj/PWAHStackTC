/*
 * PerformanceTimer.h
 *
 *  Created on: Apr 13, 2010
 *      Author: bas
 */

#ifndef PERFORMANCETIMER_H_
#define PERFORMANCETIMER_H_
#include <sys/time.h>
#include <time.h>

class PerformanceTimer {
private:
	struct timeval* _startTime;
	double _storedRunTime;

public:
	PerformanceTimer();
	virtual ~PerformanceTimer();

	static PerformanceTimer start();
	static double diffTimeMilliSecs(const timeval* time1, const timeval* time2);
	static long diffTimeMicroSecs(const timeval* time1, const timeval* time2);
	double reset();
	double resetAndStop();
	double currRunTime();
	long currRunTimeMicro();
	void pause();
	void resume();
	bool running();
};

#endif /* PERFORMANCETIMER_H_ */
