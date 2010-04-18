/*
 * BitSet.h
 *
 *  Created on: Apr 16, 2010
 *      Author: bas
 */

#ifndef BITSET_H_
#define BITSET_H_

class BitSet {
public:
	BitSet();
	virtual ~BitSet();

	virtual void set(int bitIndex, bool value) = 0;

	virtual void set(int bitIndex);
};

#endif /* BITSET_H_ */
