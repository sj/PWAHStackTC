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
#include "BitSetIterator.h"
using namespace std;

class DynamicBitSet : public BitSet {
private:
	vector<long> _vec;
	int _lastBitIndex;

public:
	DynamicBitSet();
	//DynamicBitSet(WAHBitSet& wahBitset);
	DynamicBitSet(int initialCapacity);
	virtual ~DynamicBitSet();
	void init(int initialCapacity = 64);

	void clear();
	void set(int bitIndex);
	void set(int bitIndex, bool value);
	const bool get(int bitIndex);
	const string toString();
	string toBitString(long value);
	const unsigned int size();

	BitSetIterator* iterator();

	static DynamicBitSet* constructByOr(const DynamicBitSet* bs1, const DynamicBitSet* bs2);

	const int blocksize();
};

#endif /* DYNAMICBITSET_H_ */
