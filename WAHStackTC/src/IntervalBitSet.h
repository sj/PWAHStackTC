/*
 * IntervalBitSet.h
 *
 *  Created on: Jul 13, 2010
 *      Author: bas
 */

#ifndef INTERVALBITSET_H_
#define INTERVALBITSET_H_
#include <vector>
#include <string>
#include "BitSet.h"
using namespace std;

class IntervalBitSet : public BitSet {
public:
	IntervalBitSet();
	virtual ~IntervalBitSet();

	static void copy(BitSet* otherBitSet, IntervalBitSet* result);
	long memoryUsage();

	const string toString();
	const bool equals(BitSet* otherBitSet);
	const unsigned int size();
	void clear();
	void set(int bitIndex, bool value);
	void set(int bitIndex);
	const bool get(int bitIndex);
	string bsImplementationName();
	BitSetIterator* iterator();
	static void multiOr(IntervalBitSet** bitSets, unsigned int numBitSets, IntervalBitSet* result);
	static IntervalBitSet* constructByOr(const IntervalBitSet* first, const IntervalBitSet* second);
	const int blocksize();

	friend class IntervalBitSetIterator;

private:
	int _lastBitTouched;
	IntervalBitSet* _bitset;
	vector<int> _lower;
	vector<int> _upper;

	void addInterval(int lower, int upper);
};

#endif /* INTERVALBITSET_H_ */
