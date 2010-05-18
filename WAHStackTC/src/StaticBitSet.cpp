/*
 * StaticBitSet.cpp
 *
 *  Created on: May 17, 2010
 *      Author: bas
 */

#include "StaticBitSet.h"
#include <iostream>
#include "DynamicBitSet.h"
#include "WAHBitSet.h"
#include "BitSet.h"
#include <stdexcept>
#include <math.h>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
using namespace std;


/**
 * Macro to check the bit at position pos of variable var
 */
#define GET_BIT(var, pos) ((var) & (1L << (pos)))

/**
 * Macro to set the bit at position pos of variable var
 */
#define SET_BIT(var, pos) ((var) |= 1L << (pos))

/**
 * Macro to clear the bit at position pos of variable vat
 */
#define CLEAR_BIT(var, pos) ((var) &= ~(1L << (pos)))

StaticBitSet::StaticBitSet(int initialCapacity) {
	init(initialCapacity);
}

StaticBitSet::~StaticBitSet() {
	cout.flush();
	delete[] _bits;
}


void StaticBitSet::init(int capacity){
	_capacity = capacity;
	_numLongs = capacity / sizeof(long) + 1;
	cout.flush();
	_bits = new long[_numLongs];
	_firstBitSet = -1;
	_lastBitSet = -1;
	memset(_bits, 0,  _numLongs * sizeof(long));
}

void StaticBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void StaticBitSet::set(int bitIndex, bool value){
	const bool debug = false;
	if (bitIndex >= _capacity) throw string("Index out of bounds");
	if (bitIndex >= _lastBitSet) _lastBitSet = bitIndex;
	if (bitIndex <= _firstBitSet || _firstBitSet == -1) _firstBitSet = bitIndex;

	unsigned int longElemIndex = bitIndex / sizeof(long);
	if (debug) cout << "DynamicBitSet: setting bit " << bitIndex << ", at vec element index " << longElemIndex << ", bit " << (bitIndex % 64) << endl;


	if (value){
		if (debug) cout << "setting bit " << (bitIndex % sizeof(long)) << " in " << toBitString(_bits[longElemIndex]) << endl;
		SET_BIT(_bits[longElemIndex], bitIndex % sizeof(long));
		if (debug) cout << "Total resulting DynamicBitSet:" << endl << this->toString() <<endl;
	} else {
		CLEAR_BIT(_bits[longElemIndex], bitIndex % sizeof(long));
	}

}

bool StaticBitSet::get(int bitIndex){
	if (bitIndex >= _capacity) throw string ("Index out of bounds");
	return GET_BIT(_bits[bitIndex / sizeof(long)], bitIndex % sizeof(long));
}

string StaticBitSet::toString(){
	stringstream res;
	for (unsigned int i = 0; i < _numLongs; i++){
		res << toBitString(_bits[i]) << endl;
	}
	return res.str();
}

string StaticBitSet::toBitString(long value){
	stringstream res;
	for (unsigned int bit = 0; bit < sizeof(long); bit++){
		if (GET_BIT(value, bit)) res << "1";
		else res << "0";
	}
	res << " (= " << value << ")";
	return res.str();
}

unsigned int StaticBitSet::size(){
	return _capacity;
}

unsigned int StaticBitSet::capacity(){
	return _capacity;
}

void StaticBitSet::clear(){
	if (_firstBitSet == -1) return;
	int offset = (_firstBitSet + 1) / sizeof(long);
	int numBytes = ceil((float)(_lastBitSet - _firstBitSet) / sizeof(char));

	memset(_bits, 0, _numLongs * sizeof(long));
	_firstBitSet = -1;
	_lastBitSet = -1;
}

