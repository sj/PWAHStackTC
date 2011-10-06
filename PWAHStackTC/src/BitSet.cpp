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

#include "BitSet.h"
#include <iostream>
#include <sstream>
using namespace std;
BitSet::BitSet(){}
BitSet::~BitSet(){}

const bool BitSet::equals(BitSet* otherBitSet){

	if (otherBitSet->size() != this->size()){
		cerr << "size mismatch: other=" << otherBitSet->size() << " vs. this=" << this->size() << endl;
		return false;
	}

	for (unsigned int i = 0; i < otherBitSet->size(); i++){
		if (otherBitSet->get(i) != this->get(i)){
			cerr << "mismatch at bit " << i << endl;
			cerr << "other bitset says: ";
			if (otherBitSet->get(i)) cerr << "1, this bitset says: 0" << endl;
			else cerr << "0, this bitset says: 1" << endl;
			cerr << "==== this bitset ====" << endl;
			cerr << this->toString() << endl << endl;
			cerr << "==== other bitset ====" << endl;
			cerr << otherBitSet->toString() << endl;

			return false;
		}
	}
	cout << "Successfully compared " << otherBitSet->size() << " bits" << endl;
	return true;
}

void BitSet::set(unsigned int bitIndex){
	set(bitIndex, true);
}

const string BitSet::toString(){
	stringstream stream;
	for (unsigned int i = 0; i < this->size(); i++){
		if (this->get(i)) stream << 1;
		else stream << 0;

		if (i != 0 && i % 31 == 0) stream << endl;
	}
	return stream.str();
}
