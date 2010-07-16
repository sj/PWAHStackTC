/*
 * IntervalBitSet.h
 *
 *  Created on: Jul 13, 2010
 *      Author: bas
 */

#ifndef INTERVALBITSET_H_
#define INTERVALBITSET_H_
#include <vector>
#include "BitSet.h"
using namespace std;

class IntervalBitSet  {
public:
	IntervalBitSet();
	virtual ~IntervalBitSet();

	void set(int index);
	static void copy(BitSet* otherBitSet, IntervalBitSet* result);
	long memoryUsage();

private:
	vector<int> _lower;
	vector<int> _upper;
};

#endif /* INTERVALBITSET_H_ */
