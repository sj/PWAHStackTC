/*
 * IntervalBitSetIterator.h
 *
 *  Created on: Jul 26, 2010
 *      Author: bas
 */

#ifndef INTERVALBITSETITERATOR_H_
#define INTERVALBITSETITERATOR_H_

#include "BitSetIterator.h"
#include "IntervalBitSet.h"

class IntervalBitSetIterator: public BitSetIterator {
public:
	IntervalBitSetIterator(IntervalBitSet* bitset);
	virtual ~IntervalBitSetIterator();

	int next();
	bool hasNext();
	void reset();

private:
	IntervalBitSet* _bitset;
	int _lastIntervalIndex;
	int _lastBitIndex;
};

#endif /* INTERVALBITSETITERATOR_H_ */
