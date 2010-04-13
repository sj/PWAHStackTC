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
	struct timeval _startTime;

public:
	PerformanceTimer();
	virtual ~PerformanceTimer();

	static PerformanceTimer start();
	static long diffTimeMsecs(const timeval& time1, const timeval& time2);
	long reset();
};

#endif /* PERFORMANCETIMER_H_ */
