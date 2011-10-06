/*
 * IntervalBitSetIterator.cpp
 *
 *  Created on: Jul 26, 2010
 *      Author: bas
 */

#include "IntervalBitSetIterator.h"
#include "IntervalBitSet.h"
#include <iostream>
IntervalBitSetIterator::IntervalBitSetIterator(IntervalBitSet* bitset) {
	_bitset = bitset;
	_lastIntervalIndex = -1;
	_lastBitIndex = -1;
}

IntervalBitSetIterator::~IntervalBitSetIterator() {}

bool IntervalBitSetIterator::hasNext(){
	int backupLastIntervalIndex = _lastIntervalIndex;
	int backupLastBitIndex = _lastBitIndex;
	int nextVal = next();

	if (nextVal >= 0){
		_lastBitIndex = backupLastBitIndex;
		_lastIntervalIndex = backupLastIntervalIndex;
		return true;
	} else {
		return false;
	}
}

int IntervalBitSetIterator::next(){
	const bool DEBUGGING = false;

	if (DEBUGGING) cout << "IntervalBitSetIterator::next() -- _lastIntervalIndex=" << _lastIntervalIndex << ", _lastBitIndex=" << _lastBitIndex << ", number of intervals: " << _bitset->_upper.size() << endl;
	if (_lastBitIndex == -1){
		if (_bitset->_lower.size() == 0) return -1;

		// Return first BitIndex of first interval
		_lastBitIndex = _bitset->_lower[0];
		_lastIntervalIndex = 0;

		if (DEBUGGING) cout << "IntervalBitSetInterator::next() -- returning bit " << _lastBitIndex << endl;
		return _lastBitIndex;
	}

	if (_lastBitIndex == _bitset->_upper[_lastIntervalIndex]){
		// Check whether a next interval exists
		if (_bitset->_upper.size() == _lastIntervalIndex + 1) return -1;

		_lastIntervalIndex++;
		_lastBitIndex = _bitset->_lower[_lastIntervalIndex];
		if (DEBUGGING) cout << "IntervalBitSetInterator::next() -- returning bit " << _lastBitIndex << endl;
		return _lastBitIndex;
	} else {
		// Still some bits left in the current interval
		_lastBitIndex++;
		if (DEBUGGING) cout << "IntervalBitSetInterator::next() -- returning bit " << _lastBitIndex << endl;
		return _lastBitIndex;
	}
}

void IntervalBitSetIterator::reset(){
	_lastBitIndex = -1;
	_lastIntervalIndex = -1;
}
