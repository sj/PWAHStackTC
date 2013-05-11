/**
 * Copyright 2010-2011 Sebastiaan J. van Schaik
 *
 * This file is part of PWAHStackTC.
 *
 * PWAHStackTC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PWAHStackTC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PWAHStackTC. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PWAHBITSETITERATOR_H_
#define PWAHBITSETITERATOR_H_
#include "../BitSetIterator.h"
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
