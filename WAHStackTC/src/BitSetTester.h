/*
 * BitSetTester.h
 *
 *  Created on: Jun 7, 2010
 *      Author: bas
 */

#ifndef BITSETTESTER_H_
#define BITSETTESTER_H_
#include "BitSet.h"
#include <sstream>
using namespace std;

class BitSetTester {
public:
	BitSetTester(BitSet* bs1, BitSet* bs2);
	virtual ~BitSetTester();

	static void randomise(BitSet* bs1, BitSet* bs2, int maxBits);
	static void randomise(BitSet* bs1, int maxBits);
	inline static float rand_float();

	static void compare(BitSet* bs1, BitSet* bs2);
	void testSetGet();

	static string printBitSets(BitSet* bs1, BitSet* bs2);
private:
	BitSet* _bs1;
	BitSet* _bs2;
};

#endif /* BITSETTESTER_H_ */
