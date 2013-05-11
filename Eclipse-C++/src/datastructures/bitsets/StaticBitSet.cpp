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

#include "StaticBitSet.h"
#include <iostream>
#include "DynamicBitSet.h"
#include "BitSet.h"
#include <stdexcept>
#include <math.h>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "LongBitMacros.cpp"
using namespace std;


StaticBitSet::StaticBitSet(unsigned int initialCapacity) {
	init(initialCapacity);
}

StaticBitSet::~StaticBitSet() {
	delete[] _bits;
}


void StaticBitSet::init(unsigned int capacity){
	_capacity = capacity;
	_numLongs = capacity / sizeof(long) + 1;
	cout.flush();
	_bits = new long[_numLongs];
	_firstBitSet = -1;
	_lastBitSet = -1;
	memset(_bits, 0,  _numLongs * sizeof(long));
}

void StaticBitSet::set(unsigned int bitIndex){
	set(bitIndex, true);
}

void StaticBitSet::set(unsigned int bitIndex, bool value){
	const bool debug = false;
	if (bitIndex >= _capacity) throw string("Index out of bounds");
	if (bitIndex >= _lastBitSet) _lastBitSet = bitIndex;
	if (bitIndex <= _firstBitSet || _firstBitSet == -1) _firstBitSet = bitIndex;

	unsigned int longElemIndex = bitIndex / sizeof(long);
	if (debug) cout << "DynamicBitSet: setting bit " << bitIndex << ", at vec element index " << longElemIndex << ", bit " << (bitIndex % 64) << endl;


	if (value){
		if (debug) cout << "setting bit " << (bitIndex % sizeof(long)) << " in " << toBitString(_bits[longElemIndex]) << endl;
		L_SET_BIT(_bits[longElemIndex], bitIndex % sizeof(long));
		if (debug) cout << "Total resulting DynamicBitSet:" << endl << this->toString() <<endl;
	} else {
		L_CLEAR_BIT(_bits[longElemIndex], bitIndex % sizeof(long));
	}

}

const bool StaticBitSet::get(unsigned int bitIndex){
	if (bitIndex >= _capacity) throw string ("Index out of bounds");
	return L_GET_BIT(_bits[bitIndex / sizeof(long)], bitIndex % sizeof(long));
}

const string StaticBitSet::toString(){
	stringstream res;
	for (unsigned int i = 0; i < _numLongs; i++){
		res << toBitString(_bits[i]) << endl;
	}
	return res.str();
}

string StaticBitSet::toBitString(long value){
	stringstream res;
	for (unsigned int bit = 0; bit < sizeof(long); bit++){
		if (L_GET_BIT(value, bit)) res << "1";
		else res << "0";
	}
	res << " (= " << value << ")";
	return res.str();
}

const unsigned int StaticBitSet::size(){
	return _capacity;
}

const unsigned int StaticBitSet::capacity(){
	return _capacity;
}

void StaticBitSet::clear(){
	if (_firstBitSet == -1) return;

	memset(_bits, 0, _numLongs * sizeof(long));
	_firstBitSet = -1;
	_lastBitSet = -1;
}

const int StaticBitSet::blocksize(){
	return 64;
}

string StaticBitSet::bsImplementationName(){
	return "StaticBitSet";
}

BitSetIterator* StaticBitSet::iterator(){
	throw string("StaticBitSet::iterator() not implemented");
}

