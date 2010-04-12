/*
 * DynamicBitSet.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#include <iostream>
#include "DynamicBitSet.h"
#include "WAHBitSet.h"
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
	DynamicBitSet(64);
}

DynamicBitSet::DynamicBitSet(int initialCapacity) {
	_vec = vector<long>(ceil(initialCapacity / 64));
}

DynamicBitSet::~DynamicBitSet() {}

void DynamicBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void DynamicBitSet::set(int bitIndex, bool value){
	unsigned int vecElemIndex = bitIndex / 64;

	while(_vec.size() <= vecElemIndex){ _vec.push_back(0); }
	if (value) SET_BIT(_vec[vecElemIndex], bitIndex % 64);
	else CLEAR_BIT(_vec[vecElemIndex], bitIndex % 64);
}

bool DynamicBitSet::get(int bitIndex){
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

void DynamicBitSet::constructFailingExample(){
	_vec.push_back(0);
	_vec.push_back(109966619291156480);
}
