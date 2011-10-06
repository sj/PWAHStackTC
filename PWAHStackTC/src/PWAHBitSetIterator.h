/*
 * PWAHBitSetIterator.h
 *
 *  Created on: Jun 16, 2010
 *      Author: bas
 */

#ifndef PWAHBITSETITERATOR_H_
#define PWAHBITSETITERATOR_H_
#include "BitSetIterator.h"
#include "PWAHBitSet.h"

template<unsigned int P>
class PWAHBitSetIterator : public BitSetIterator {
private:
	PWAHBitSet<P>* _bitSet;
	bool _atStart;
	bool _atEnd;
	unsigned int _lastBlockIndex; // global index of the block in which the last 1-bit was found
	int _lastBlockBitIndex; // bit index (local to the last block) of the last 1-bit
	unsigned int _lastWordIndex; // index of the word containing the block of the last 1-bit
	unsigned int _lastPartitionBlockIndex; // index (local to the current partition) of the block within the partition
	unsigned short _lastWordPartitionIndex; // index of the partition in which the last 1-bit was encountered

public:
	PWAHBitSetIterator(PWAHBitSet<P>* wahBitSet);
	virtual ~PWAHBitSetIterator();

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

#endif /* PWAHBITSETITERATOR_H_ */
