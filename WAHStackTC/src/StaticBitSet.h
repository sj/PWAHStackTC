/*
 * StaticBitSet.h
 *
 *  Created on: May 17, 2010
 *      Author: bas
 */

#ifndef STATICBITSET_H_
#define STATICBITSET_H_
#include <string>
#include "BitSet.h"

class StaticBitSet : public BitSet {
private:
	unsigned int _capacity;
	int _firstBitSet, _lastBitSet;
	unsigned int _numLongs;
	long* _bits;

public:
	StaticBitSet(int capacity);
	virtual ~StaticBitSet();

	void init(int initialCapacity = 64);

	void clear();
	void set(int bitIndex);
	void set(int bitIndex, bool value);
	const bool get(int bitIndex);
	const string toString();
	string toBitString(long value);
	const unsigned int capacity();
	const unsigned int size();

	const int blocksize();

	BitSetIterator* iterator();
};

#endif /* STATICBITSET_H_ */
