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
	bool get(int bitIndex);
	string toString();
	string toBitString(long value);
	unsigned int capacity();
	unsigned int size();

	int blocksize();

	BitSetIterator* iterator();
};

#endif /* STATICBITSET_H_ */
