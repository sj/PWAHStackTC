/*
 * DynamicBitSet.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#include <iostream>
#include "DynamicBitSet.h"
#include "WAHBitSet.h"
#include "BitSet.h"
#include <stdexcept>
#include <math.h>
#include <vector>
#include <string>
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

DynamicBitSet::DynamicBitSet() {
	init();
}

DynamicBitSet::DynamicBitSet(int initialCapacity) {
	init(initialCapacity);
}

/**DynamicBitSet::DynamicBitSet(WAHBitSet& wahBitSet){
	init(wahBitSet.size());
	for (unsigned int i = 0; i < wahBitSet.size(); i++) this->set(i, wahBitSet.get(i));
}**/

DynamicBitSet::~DynamicBitSet() {}


void DynamicBitSet::init(int initialCapacity){
	_lastBitIndex = -1;
	_vec = vector<long>(initialCapacity / 64 + 1);
}

void DynamicBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void DynamicBitSet::set(int bitIndex, bool value){
	const bool debug = false;
	if (bitIndex > _lastBitIndex) _lastBitIndex = bitIndex;
	unsigned int vecElemIndex = bitIndex / 64;
	if (debug) cout << "DynamicBitSet: setting bit " << bitIndex << ", at vec element index " << vecElemIndex << ", bit " << (bitIndex % 64) << endl;

	while(_vec.size() <= vecElemIndex){
		_vec.push_back(0);
	}

	if (value){
		if (debug) cout << "setting bit " << (bitIndex % 64) << " in " << toBitString(_vec[vecElemIndex]) << endl;
		SET_BIT(_vec[vecElemIndex], bitIndex % 64);
		if (debug) cout << "Total resulting DynamicBitSet:" << endl << this->toString() <<endl;
	} else {
		CLEAR_BIT(_vec[vecElemIndex], bitIndex % 64);
	}

}

bool DynamicBitSet::get(int bitIndex){
	if (bitIndex > _lastBitIndex) return false;
	return GET_BIT(_vec[bitIndex / 64], bitIndex % 64);
}

string DynamicBitSet::toString(){
	stringstream res;
	for (unsigned int i = 0; i < _vec.size(); i++){
		res << toBitString(_vec[i]) << endl;
	}
	return res.str();
}

string DynamicBitSet::toBitString(long value){
	stringstream res;
	for (int bit = 0; bit < 64; bit++){
		if (GET_BIT(value, bit)) res << "1";
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

unsigned int DynamicBitSet::size(){
	return _lastBitIndex + 1;
}

void DynamicBitSet::clear(){
	init();
}

void DynamicBitSet::constructFailingExample(){
	_vec.push_back(0);
	_vec.push_back(109966619291156480);
}
