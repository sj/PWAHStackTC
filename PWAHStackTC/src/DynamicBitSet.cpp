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

#include <iostream>
#include "DynamicBitSet.h"
#include "BitSet.h"
#include <stdexcept>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include "LongBitMacros.cpp"
using namespace std;

DynamicBitSet::DynamicBitSet() {
	init(0);
}

DynamicBitSet::DynamicBitSet(unsigned int initialCapacity) {
	init(initialCapacity);
}

/**DynamicBitSet::DynamicBitSet(WAHBitSet& wahBitSet){
	init(wahBitSet.size());
	for (unsigned int i = 0; i < wahBitSet.size(); i++) this->set(i, wahBitSet.get(i));
}**/

DynamicBitSet::~DynamicBitSet() {}


void DynamicBitSet::init(unsigned int initialCapacity){
	_lastBitIndex = -1;
	_vec = vector<long>(initialCapacity / 64 + 1);
}

void DynamicBitSet::set(unsigned int bitIndex){
	set(bitIndex, true);
}

void DynamicBitSet::set(unsigned int bitIndex, bool value){
	const bool debug = false;
	if (bitIndex > _lastBitIndex) _lastBitIndex = bitIndex;
	unsigned int vecElemIndex = bitIndex / 64;
	if (debug) cout << "DynamicBitSet::set -- " << (value ? "setting" : "unsetting") << " bit " << bitIndex << ", at vec element index " << vecElemIndex << ", bit " << (bitIndex % 64) << endl;

	while(_vec.size() <= vecElemIndex){
		if (debug) cout << "expanding..." << endl;
		_vec.push_back(0);
	}

	if (value){
		if (debug) cout << "DynamicBitSet::set -- " << (value ? "setting" : "unsetting") << " bit " << (bitIndex % 64) << " in " << toBitString(_vec[vecElemIndex]) << endl;
		L_SET_BIT(_vec[vecElemIndex], bitIndex % 64);
		//if (debug) cout << "Total resulting DynamicBitSet:" << endl << this->toString() <<endl;
	} else {
		L_CLEAR_BIT(_vec[vecElemIndex], bitIndex % 64);
	}

}

const bool DynamicBitSet::get(unsigned int bitIndex){
	if (bitIndex > _lastBitIndex) return false;
	return L_GET_BIT(_vec[bitIndex / 64], bitIndex % 64);
}

const string DynamicBitSet::toString(){
	stringstream res;
	for (unsigned int i = 0; i < _vec.size(); i++){
		res << toBitString(_vec[i]) << endl;
	}
	return res.str();
}

string DynamicBitSet::toBitString(long value){
	stringstream res;
	res << "0b";
	for (int bit = 63; bit >= 0; bit--){
		if (L_GET_BIT(value, bit)) res << "1";
		else res << "0";
	}
	res << " (= " << value << ")";
	return res.str();
}

DynamicBitSet* DynamicBitSet::constructByOr(const DynamicBitSet* bs1, const DynamicBitSet* bs2){
	DynamicBitSet* res = new DynamicBitSet();
	res->_vec = vector<long>(max(bs1->_vec.size(), bs2->_vec.size()));
	long word;

	for (unsigned int i = 0; i < max(bs1->_vec.size(), bs2->_vec.size()); i++){
		word = 0;

		if (i < bs1->_vec.size()) word |= bs1->_vec[i];
		if (i < bs2->_vec.size()) word |= bs2->_vec[i];

		res->_vec[i] = word;
	}

	res->_lastBitIndex = max(bs1->_lastBitIndex, bs2->_lastBitIndex);
	return res;
}

const unsigned int DynamicBitSet::size(){
	return _lastBitIndex + 1;
}

void DynamicBitSet::clear(){
	init();
}

const int DynamicBitSet::blocksize(){
	return 64;
}

string DynamicBitSet::bsImplementationName(){
	return "DynamicBitSet";
}

BitSetIterator* DynamicBitSet::iterator(){
	throw string("DynamicBitSet::iterator() not implemented");
}
