/*
 * DynamicBitSet.h
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#ifndef DYNAMICBITSET_H_
#define DYNAMICBITSET_H_
#include <vector>
#include <string>
#include "BitSet.h"
using namespace std;

class DynamicBitSet : BitSet {
private:
	vector<long> _vec;

public:
	DynamicBitSet(int initialCapacity);
	DynamicBitSet();
	virtual ~DynamicBitSet();

	void set(int bitIndex);
	void set(int bitIndex, bool value);
	bool get(int bitIndex);
	string toString();
	string toBitString(long value);

	void constructFailingExample();
	DynamicBitSet constructByOr(const DynamicBitSet& bs1, const DynamicBitSet& bs2);
};

#endif /* DYNAMICBITSET_H_ */
