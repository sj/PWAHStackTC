/*
 * BitSet.h
 *
 *  Created on: Apr 16, 2010
 *      Author: bas
 */

#ifndef BITSET_H_
#define BITSET_H_
#include <string>
#include "BitSetIterator.h"
using namespace std;

class BitSet {
public:
	BitSet();
	virtual ~BitSet();

	virtual const string toString();
	virtual const bool equals(BitSet* otherBitSet);
	virtual const unsigned int size() = 0;
	virtual void clear() = 0;
	virtual void set(int bitIndex, bool value) = 0;
	virtual void set(int bitIndex);
	virtual const bool get(int bitIndex) = 0;

	virtual BitSetIterator* iterator() = 0;

	virtual const int blocksize() = 0;
};

#endif /* BITSET_H_ */
