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
	virtual void set(int bitIndex, bool value);

	virtual void set(int bitIndex){
		set(bitIndex, true);
	}
};

#endif /* BITSET_H_ */
