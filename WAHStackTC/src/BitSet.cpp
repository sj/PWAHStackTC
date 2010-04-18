/*
 * BitSet.cpp
 *
 *  Created on: Apr 16, 2010
 *      Author: bas
 */

#include "BitSet.h"

BitSet::BitSet(){}
BitSet::~BitSet(){}

void BitSet::set(int bitIndex){
	set(bitIndex, true);
}
