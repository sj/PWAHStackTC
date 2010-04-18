/*
 * BitSet.cpp
 *
 *  Created on: Apr 16, 2010
 *      Author: bas
 */

#include "BitSet.h"
#include <iostream>
#include <sstream>
using namespace std;
BitSet::BitSet(){}
BitSet::~BitSet(){}

bool BitSet::equals(BitSet& otherBitSet){

	if (otherBitSet.size() != this->size()){
		cerr << "size mismatch: other=" << otherBitSet.size() << " vs. this=" << this->size() << endl;
		return false;
	}

	for (unsigned int i = 0; i < otherBitSet.size(); i++){
		if (otherBitSet.get(i) != this->get(i)){
			cerr << "mismatch at bit " << i << endl;
			cerr << "other bitset says: ";
			if (otherBitSet.get(i)) cerr << "1, this bitset says: 0" << endl;
			else cerr << "0, this bitset says: 1" << endl;
			cerr << "==== this bitset ====" << endl;
			cerr << this->toString() << endl << endl;
			cerr << "==== other bitset ====" << endl;
			cerr << otherBitSet.toString() << endl;

			return false;
		}
	}
	cout << "Successfully compared " << otherBitSet.size() << " bits" << endl;
	return true;
}

void BitSet::set(int bitIndex){
	set(bitIndex, true);
}

string BitSet::toString(){
	stringstream stream;
	for (unsigned int i = 0; i < this->size(); i++){
		if (this->get(i)) stream << 1;
		else stream << 0;

		if (i != 0 && i % 31 == 0) stream << endl;
	}
	return stream.str();
}
