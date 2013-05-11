/**
 * Copyright 2010-2011 Sebastiaan J. van Schaik
 *
 * This file is part of PWAHStackTC.
 *
 * PWAHStackTC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PWAHStackTC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PWAHStackTC. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BITSETTESTER_H_
#define BITSETTESTER_H_
#include "../src/datastructures/bitsets/BitSet.h"
#include <sstream>
using namespace std;

template<class B>
class BitSetTester {
public:
	BitSetTester(BitSet* bs1, BitSet* bs2);
	virtual ~BitSetTester();

	static void randomise(BitSet* bs1, BitSet* bs2, int maxBits);
	static void randomise(BitSet* bs1, int maxBits);
	inline static float rand_float();

	static void compare(BitSet* bs1, BitSet* bs2);
	static void testSetGetIndex();
	static void testLongFill(BitSet* bitset);
	static void testIterator(BitSet* bitset, bool randomise);
	static void diff();
	static void testOr(unsigned int numBitsets);
	void testSetGet();

	static string printBitSets(BitSet** bitsets, int numBitSets);
	static string printBitSets(BitSet* bs1, BitSet* bs2);
	static string printBitSets(BitSet* bs1, BitSet* bs2, BitSet* bs3);
private:
	BitSet* _bs1;
	BitSet* _bs2;
};

#endif /* BITSETTESTER_H_ */
