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

#ifndef STATICBITSET_H_
#define STATICBITSET_H_
#include <string>
#include "BitSet.h"

class StaticBitSet : public BitSet {
private:
	unsigned int _capacity;
	int _firstBitSet, _lastBitSet;
	unsigned int _numLongs;
	long* _bits;

public:
	StaticBitSet(unsigned int capacity);
	virtual ~StaticBitSet();

	void init(unsigned int initialCapacity = 64);

	void clear();
	void set(unsigned int bitIndex);
	void set(unsigned int bitIndex, bool value);
	const bool get(unsigned int bitIndex);
	const string toString();
	string toBitString(long value);
	const unsigned int capacity();
	const unsigned int size();
	string bsImplementationName();

	const int blocksize();

	BitSetIterator* iterator();
};

#endif /* STATICBITSET_H_ */
