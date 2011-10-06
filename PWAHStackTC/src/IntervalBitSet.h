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

#ifndef INTERVALBITSET_H_
#define INTERVALBITSET_H_
#include <vector>
#include <string>
#include "BitSet.h"
using namespace std;

class IntervalBitSet : public BitSet {
public:
	IntervalBitSet();
	virtual ~IntervalBitSet();

	static void copy(BitSet* otherBitSet, IntervalBitSet* result);
	long memoryUsage();

	const string toString();
	const string toDetailString();
	const bool equals(BitSet* otherBitSet);
	const unsigned int size();
	const unsigned int numberOfIntervals();

	void clear();
	void set(int bitIndex, bool value);
	void set(int bitIndex);
	const bool get(int bitIndex);
	string bsImplementationName();
	BitSetIterator* iterator();
	static void multiOr(IntervalBitSet** bitSets, unsigned int numBitSets, IntervalBitSet* result);
	static IntervalBitSet* constructByOr(const IntervalBitSet* first, const IntervalBitSet* second);
	const int blocksize();

	friend class IntervalBitSetIterator;

private:
	int _lastBitTouched;
	vector<int> _lower;
	vector<int> _upper;

	void addInterval(int lower, int upper);
};

#endif /* INTERVALBITSET_H_ */
