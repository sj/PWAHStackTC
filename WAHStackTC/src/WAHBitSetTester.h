/*
 * WAHBitSetTester.h
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#ifndef WAHBITSETTESTER_H_
#define WAHBITSETTESTER_H_

class WAHBitSetTester {
private:
	static const bool DEBUGGING = false;

public:
	WAHBitSetTester();
	virtual ~WAHBitSetTester();

	static void test(int runs);
	static float randFloat();
};

#endif /* WAHBITSETTESTER_H_ */
