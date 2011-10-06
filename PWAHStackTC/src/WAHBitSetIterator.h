/*
 * WAHBitSetIterator.h
 *
 *  Created on: Apr 19, 2010
 *      Author: bas
 */

#ifndef WAHBITSETITERATOR_H_
#define WAHBITSETITERATOR_H_
#include "BitSetIterator.h"
#include "WAHBitSet.h"
using namespace std;

class WAHBitSet;

class WAHBitSetIterator : public BitSetIterator {
private:
	WAHBitSet* _wahBitSet;
	bool _atStart;
	bool _atEnd;
	unsigned int _lastBlockIndex; // global index of the block in which the last 1-bit was found
	int _lastBlockBitIndex; // bit index (local to the last block) of the last 1-bit
	unsigned int _lastWordIndex; // index of the word containing the block of the last 1-bit
	unsigned int _lastWordBlockIndex; // index (local to the current word) of the block within the word


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

	void reset();
};

#endif /* WAHBITSETITERATOR_H_ */
