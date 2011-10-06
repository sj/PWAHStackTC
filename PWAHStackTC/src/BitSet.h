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
	virtual string bsImplementationName() = 0;
	virtual BitSetIterator* iterator() = 0;

	virtual const int blocksize() = 0;
};

#endif /* BITSET_H_ */
