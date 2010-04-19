/*
 * WAHBitSetIterator.cpp
 *
 *  Created on: Apr 19, 2010
 *      Author: bas
 */

#include "WAHBitSetIterator.h"
using namespace std;

WAHBitSetIterator::WAHBitSetIterator(WAHBitSet* wahBitSet) {
	_wahBitSet = wahBitSet;
	_currWordIndex = 0;
	_currWordBitIndex = 0;
}

WAHBitSetIterator::~WAHBitSetIterator() {}

bool WAHBitSetIterator::hasNext(){
	return (next() == -1);
}

int WAHBitSetIterator::next(){
	// Check whether the current word contains more set bits, after the _currWordBitIndex

}
