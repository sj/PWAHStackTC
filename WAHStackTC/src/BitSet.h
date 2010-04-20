/*
 * BitSet.h
 *
 *  Created on: Apr 16, 2010
 *      Author: bas
 */

#ifndef BITSET_H_
#define BITSET_H_
#include <string>
using namespace std;

class BitSet {
public:
	BitSet();
	virtual ~BitSet();

	virtual string toString();
	virtual bool equals(BitSet* otherBitSet);
	virtual unsigned int size() = 0;
	virtual void clear() = 0;
	virtual void set(int bitIndex, bool value) = 0;
	virtual void set(int bitIndex);
	virtual bool get(int bitIndex) = 0;
};

#endif /* BITSET_H_ */
