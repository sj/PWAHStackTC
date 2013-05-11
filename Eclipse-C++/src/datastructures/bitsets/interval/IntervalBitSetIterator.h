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

#ifndef INTERVALBITSETITERATOR_H_
#define INTERVALBITSETITERATOR_H_

#include "../BitSetIterator.h"
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
