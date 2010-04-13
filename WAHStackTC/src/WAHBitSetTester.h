/*
 * WAHBitSetTester.h
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#ifndef WAHBITSETTESTER_H_
#define WAHBITSETTESTER_H_
#include "WAHBitSet.h"

class WAHBitSetTester {
private:
	static const bool DEBUGGING = false;

public:
	WAHBitSetTester();
	virtual ~WAHBitSetTester();

	static void randomize(WAHBitSet& bitset);
	static void test(int runs);
	static float randFloat();
	static void testOr();
};

#endif /* WAHBITSETTESTER_H_ */
