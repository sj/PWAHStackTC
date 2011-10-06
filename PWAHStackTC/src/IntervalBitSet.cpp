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

#include "IntervalBitSet.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>
#include "IntervalBitSetIterator.h"
using namespace std;

IntervalBitSet::IntervalBitSet() {
	this->clear();
}

IntervalBitSet::~IntervalBitSet() {}

void IntervalBitSet::set(int bitIndex){
	const bool DEBUGGING = false;
	if (DEBUGGING){
		cout << "IntervalBitSet::set -- setting bit " << bitIndex << endl;
	}

	if (_lastBitTouched < bitIndex) _lastBitTouched = bitIndex;

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

const string IntervalBitSet::toString(){
	stringstream ss;
	ss << "IntervalBitSet consisting of " << _lower.size() << " intervals: {";

	for (unsigned int i = 0; i < _lower.size(); i++){
		ss << "[" << _lower[i] << "," << _upper[i] << "] ";
	}
	ss << "}";
	return ss.str();
}

const bool IntervalBitSet::equals(BitSet* otherBitSet){
	throw string("not implemented");
}

const unsigned int IntervalBitSet::numberOfIntervals(){
	return _upper.size();
}


const unsigned int IntervalBitSet::size(){
	return _lastBitTouched + 1;
}

void IntervalBitSet::clear(){
	_lower.clear();
	_upper.clear();
	_lastBitTouched = -1;
}

void IntervalBitSet::set(int bitIndex, bool value){
	if (!value){
		if (_upper.size() == 0 || _upper.back() < bitIndex){
			if (_lastBitTouched < bitIndex) _lastBitTouched = bitIndex;
			return;
		}
		throw string("Clearing bits on IntervalBitSet not supported");
	}
	set(bitIndex);
}


const bool IntervalBitSet::get(int bitIndex){
	const bool DEBUGGING = false;
	int currMinIntervalIndex = 0;
	int currMaxIntervalIndex = _upper.size();
	int midIntervalIndex;
	bool res;

	if (DEBUGGING) cout << "IntervalBitSet::get(" << bitIndex << ") -- getting state of bit " << bitIndex << endl;
	if (_lower.size() == 0){
		return false;
	} else if (bitIndex < _lower[0] || bitIndex > _upper.back()){
		return false;
	}

	while (true){
		if (DEBUGGING) cout << "IntervalBitSet::get(" << bitIndex << ") -- currMinIntervalIndex=" << currMinIntervalIndex << ", currMaxIntervalIndex=" << currMaxIntervalIndex << endl;

		if (currMinIntervalIndex == currMaxIntervalIndex){
			res = (bitIndex >= _lower[currMinIntervalIndex] && bitIndex <= _upper[currMinIntervalIndex]);
			if (DEBUGGING) cout << "IntervalBitSet::get(" << bitIndex << ") -- returning " << (res ? "true" : "false") << endl;
			return res;
		}

		midIntervalIndex = (currMinIntervalIndex + currMaxIntervalIndex) / 2;

		if (DEBUGGING) cout << "IntervalBitSet::get(" << bitIndex << ") -- midIntervalIndex=" << midIntervalIndex << ": [" << _lower[midIntervalIndex] << "," << _upper[midIntervalIndex] << "]" << endl;
		if (bitIndex <= _upper[midIntervalIndex] && bitIndex >= _lower[midIntervalIndex]) return true;

		if (bitIndex < _lower[midIntervalIndex]){
			currMaxIntervalIndex = midIntervalIndex;
		} else {
			currMinIntervalIndex = midIntervalIndex + 1;
		}
	}
}

string IntervalBitSet::bsImplementationName(){
	return "IntervalBitSet";
}

BitSetIterator* IntervalBitSet::iterator(){
	return new IntervalBitSetIterator(this);
}

const int IntervalBitSet::blocksize(){
	return -1;
}

void IntervalBitSet::multiOr(IntervalBitSet** bitSets, unsigned int numBitSets, IntervalBitSet* result){
	const bool DEBUGGING = false;
	int currMinLower, currLower, currUpper;
	unsigned int* sIndex = new unsigned int[numBitSets];
	memset(sIndex, 0, numBitSets * sizeof(int));
	int maxLastBitTouched = 0;

	if (DEBUGGING){
		cout << "IntervalBitSet::multiOr -- performing Multi-OR on " << numBitSets << " input IntervalBitSets:" << endl;
		for (unsigned int i = 0; i < numBitSets; i++){
			cout << "IntervalBitSet::multiOr -- Input IntervalBitSet " << i << ": ";
			cout << bitSets[i]->toString() << endl;
		}
	}

	currMinLower = -1;
	while(true){
		if (DEBUGGING) cout << "IntervalBitSet::multiOr -- next pass: minimum lower bound = " <<  currMinLower << endl;
		currLower = -1;
		currUpper = -1;

		for (unsigned int i = 0; i < numBitSets; i++){
			// Try to find the first relevant interval in the current
			// IntervalBitSet: bitSets[i]
			if (bitSets[i]->_lastBitTouched > maxLastBitTouched) maxLastBitTouched = bitSets[i]->_lastBitTouched;

			while(true){
				// Out of bounds?
				if (bitSets[i]->_lower.size() <= sIndex[i]) break;

				if (bitSets[i]->_upper[sIndex[i]] <= currMinLower){
					// Upper bound of current interval is smaller or equal to
					// current minimum lower bound -> increase sIndex[i]
					sIndex[i]++;
				} else {
					break;
				}
			}

			// sIndex[i] is the index of the relevant interval of this bitset,
			// but sIndex[i] might be out of bounds...
			if (bitSets[i]->_lower.size() <= sIndex[i]){
				if (DEBUGGING) cout << "IntervalBitSet::multiOr -- BitSet " << i << " out of bounds" << endl;
				continue;
			}

			// Current IntervalBitSet is not out of bounds, but the upper bound of the
			// IBS is strictly larger than the current lower bound.
			if (DEBUGGING) cout << "IntervalBitSet::multiOr -- BitSet " << i << ": considering interval " << sIndex[i] << ": [" << bitSets[i]->_lower[sIndex[i]] << "," << bitSets[i]->_upper[sIndex[i]] << "]" << endl;
			if (bitSets[i]->_lower[sIndex[i]] < currLower || currLower == -1){
				// The lower bound of this IntervalBitSet is smaller than any
				// lower bound seen before

				if (bitSets[i]->_upper[sIndex[i]] < currLower - 1 || currUpper == -1){
					currUpper = bitSets[i]->_upper[sIndex[i]];
				}
				if (bitSets[i]->_upper[sIndex[i]] > currUpper || currUpper == -1){
					currUpper = bitSets[i]->_upper[sIndex[i]];
				}

				currLower = bitSets[i]->_lower[sIndex[i]];
			}

			if (DEBUGGING) cout << "IntervalBitSet::multiOr -- considering adding interval [" << currLower << "," << currUpper << "]..." << endl;
		}

		if (currLower == -1){
			// Done!
			if (DEBUGGING) cout << "IntervalBitSet::multiOr -- finished iterating over source bitsets" << endl;
			break;
		}

		if (DEBUGGING) cout << "IntervalBitSet::multiOr -- adding interval [" << currLower << "," << currUpper << "] to result" << endl;
		result->addInterval(currLower, currUpper);
		currMinLower = currUpper;
	}

	result->_lastBitTouched = maxLastBitTouched;
	if (DEBUGGING) cout << "IntervalBitSet::multiOr -- resulting IntervalBitSet:" << endl;
	if (DEBUGGING) cout << result->toString() << endl;

	delete[] sIndex;
}

void IntervalBitSet::addInterval(int lower, int upper){
	if (this->_lower.size() > 0){
		if (upper < this->_upper.back()) throw string("can't add interval!");

		if (lower <= _upper.back() + 1){
			// Adjacent to last interval, merge
			_upper[_upper.size() - 1] = upper;
		} else {
			// Separate interval
			_lower.push_back(lower);
			_upper.push_back(upper);
		}
	} else {
		// No intervals stored yet...
		_lower.push_back(lower);
		_upper.push_back(upper);
	}
	_lastBitTouched = upper;
}

IntervalBitSet* IntervalBitSet::constructByOr(const IntervalBitSet* first, const IntervalBitSet* second){
	throw string("IntervalBitSet::constructByOr not supported");
}
