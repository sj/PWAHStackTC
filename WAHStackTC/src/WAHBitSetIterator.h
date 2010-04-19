/*
 * WAHBitSetIterator.h
 *
 *  Created on: Apr 19, 2010
 *      Author: bas
 */

#ifndef WAHBITSETITERATOR_H_
#define WAHBITSETITERATOR_H_
#include "WAHBitSet.h"
using namespace std;

class WAHBitSetIterator {
private:
	WAHBitSet* _wahBitSet;
	int _currWordIndex; // current word index
	int _currWordBitIndex; // current bit index within _currWordIndex

public:
	WAHBitSetIterator(WAHBitSet* wahBitSet);
	virtual ~WAHBitSetIterator();

	/**
	 * Determines whether there actually is a next set bit
	 */
	bool hasNext();

	/**
	 * Returns the index of the next set bit. Will return -1 if there
	 * is no next set bit
	 */
	int next();
};

#endif /* WAHBITSETITERATOR_H_ */
