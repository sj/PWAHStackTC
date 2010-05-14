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

	static void randomise(BitSet& bitset, int maxBits);
	static void test(int runs);
	static float randFloat();
	static void testOr();
	static void testMultiOr();
	static void testIterator();
};

#endif /* WAHBITSETTESTER_H_ */
