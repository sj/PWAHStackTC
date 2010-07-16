/*
 * IntervalBitSet.cpp
 *
 *  Created on: Jul 13, 2010
 *      Author: bas
 */

#include "IntervalBitSet.h"
#include <vector>
#include <iostream>
using namespace std;

IntervalBitSet::IntervalBitSet() {
	// TODO Auto-generated constructor stub

}

IntervalBitSet::~IntervalBitSet() {
	// TODO Auto-generated destructor stub
}

void IntervalBitSet::set(int bitIndex){
	if (_lower.size() > 0){
		if (_lower.back() > bitIndex) throw string ("can't do");
		if (_lower.back() == bitIndex) return; // already set
		if (_upper.back() == bitIndex - 1){
			// Adjacent
			_upper[_upper.size() - 1] = bitIndex;
			return;
		}

	}

	_lower.push_back(bitIndex);
	_upper.push_back(bitIndex);
}

void IntervalBitSet::copy(BitSet* otherBitSet, IntervalBitSet* result){
	BitSetIterator* iter = otherBitSet->iterator();
	int next = iter->next();
	while (next >= 0){
		result->set(next);
		next = iter->next();
	}

	delete iter;
}

long IntervalBitSet::memoryUsage(){
	return _lower.size() * 2 * 32;
}
