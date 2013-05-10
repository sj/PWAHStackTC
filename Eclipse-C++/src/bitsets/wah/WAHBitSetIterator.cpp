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

#include "WAHBitSetIterator.h"
#include "IntBitMacros.cpp"
#include <stdexcept>
#include <iostream>
using namespace std;

WAHBitSetIterator::WAHBitSetIterator(WAHBitSet* wahBitSet) {
	_wahBitSet = wahBitSet;
	reset();
}

WAHBitSetIterator::~WAHBitSetIterator() {}

bool WAHBitSetIterator::hasNext(){
	throw string("not implemented");
}

void WAHBitSetIterator::reset(){
	_lastBlockIndex = 0;
	_lastBlockBitIndex = -1;
	_lastWordIndex = 0;
	_lastWordBlockIndex = 0;

	_atStart = true;
	_atEnd = false;
}

int WAHBitSetIterator::next(){
	// Check whether the current word contains more set bits, after the _currWordBitIndex

	if (_atEnd){
		return -1;
	}

	_atStart = false;

	// Skip bits and words until a 1-bit is found
	int currWord;
	while (true){ // Iterate through all words in the BitSet
		if (_lastWordIndex < _wahBitSet->_compressedBits.size()){
			// Word index in bounds
			currWord = _wahBitSet->_compressedBits[_lastWordIndex];
		} else if (_lastWordIndex == _wahBitSet->_compressedBits.size()){
			// Word index out of bounds for the first time, let's check the plain word
			currWord = _wahBitSet->_plainWord;
		} else {
			// Word index totally out of bounds, we're at the end.
			_atEnd = true;
			return -1;
		}

		if (IS_ONEFILL(currWord)){
			// Check whether the next 1-bit resides within this fill word
			_lastBlockBitIndex++;

			if (_lastBlockBitIndex >= WAHBitSet::BLOCKSIZE){
				// No more bits in this block, is there a next block in this 1-fill word?
				_lastBlockBitIndex = 0;
				_lastWordBlockIndex++;
				_lastBlockIndex++;

				if (_lastWordBlockIndex >= FILL_LENGTH(currWord)){
					// No more blocks in this 1-fill word, skip to the next word
					_lastWordIndex++;
					_lastWordBlockIndex = 0;

					// Enter next iteration of while-loop to find the next word
					_lastBlockBitIndex = -1;
					continue;
				} else {
					// There are more blocks in this 1-fill word
					_lastBlockBitIndex = 0;
				}
			} // else: new bit index still within this block

			// Return the absolute position of the 1-bit
			return _lastBlockIndex * WAHBitSet::BLOCKSIZE + _lastBlockBitIndex;
		} else if (IS_ZEROFILL(currWord)){
			// 0-fill words are useless, skip to the next word
			_lastWordIndex++;
			_lastBlockIndex += FILL_LENGTH(currWord);
			_lastBlockBitIndex = -1;
			continue;
		} else {
			// literal word

			while (true){ // Skip through bits in this literal word
				_lastBlockBitIndex++;

				if (_lastBlockBitIndex >= WAHBitSet::BLOCKSIZE){
					// The bit is in the next block. Since this is a literal word,
					// it only contains one block. The next block is located in the next word,
					// enter the next iteration of the outer while loop to find it.
					break;
				} // else: bit within this literal

				if (I_GET_BIT(currWord, _lastBlockBitIndex)){
					// Encountered a 1-bit! Return its absolute position within this BitSet
					return _lastBlockIndex * WAHBitSet::BLOCKSIZE + _lastBlockBitIndex;
				} // else: 0-bit, check next bit in this literal word
			}

			_lastWordBlockIndex = 0;
			_lastBlockBitIndex = -1;
			_lastWordIndex++;
			_lastBlockIndex++;
		}
	} // end while-iteration over all words

	// We should never end up here?!
	throw range_error("whuh?!");
}
