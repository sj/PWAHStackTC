/*
 * WAHBitSet.cpp
 *
 *  Created on: Apr 10, 2010
 *      Author: bas
 */

#include <iostream>
#include <string>
#include <sstream>
#include "WAHBitSet.h"
using namespace std;

/**
 * Macro to check the bit at position pos of variable var
 */
#define GET_BIT(var, pos) ((var) & (1 << (pos)))

/**
 * Macro to set the bit at position pos of variable var
 */
#define SET_BIT(var, pos) ((var) |= 1 << (pos))

/**
 * Macro to clear the bit at position pos of variable vat
 */
#define CLEAR_BIT(var, pos) ((var) &= ~(1 << (pos)))

/**
 * Macro to determine whether a var represents a FILL or LITERAL
 */
#define IS_FILL(var) (GET_BIT(var,0))

/**
 * Macro to determine whether a var represents a 1-fill
 */
#define IS_ONEFILL(var) (IS_FILL(var) && GET_BIT(var,1))

/**
 * Macro to determine whether a var represents a 0-fill
 */
#define IS_ZEROFILL(var) (IS_FILL(var) && !GET_BIT(var,1))

WAHBitSet::WAHBitSet() {
	WAHBitSet(32);
}

WAHBitSet::WAHBitSet(int initialCapacity) {
	_plainWord = 0;
	_plainWordOffset = 0;
}

WAHBitSet::~WAHBitSet() {
}

void WAHBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void WAHBitSet::set(int bitIndex, bool value){
	if (DEBUGGING) cout << "WAHBitSet::set setting bit with index " << bitIndex << endl;
	if (bitIndex < _plainWordOffset){
		// The passed bit was already compressed. Can't do anything about that.
		stringstream stream;
		stream << "Cannot set/unset bit at index " << bitIndex << ": bit is already compressed!";
		throw stream.str();
	}

	if (bitIndex >= _plainWordOffset + BLOCKSIZE){
		// BitIndex not within boundaries of the plain word. The plain word should be written
		// to the _compressedBits.
		if (DEBUGGING) cout << "compressing plain word (" << _plainWord << ") ..." << endl;

		// Prepare exception...
		stringstream stream;
		stream << "Cannot create a fill consisting of > " << MAX_BLOCKS_IN_FILL << " blocks!";
		string maxBlocksExceededException = stream.str();

		// blockSeq is the sequence number of the block in which the bitIndex belongs
		// Example: for BLOCKSIZE = 31, bits 0 ... 30 go in block with sequence no 0,
		// bits 31 ... 61 go in block with seq. no 1, and so on.
		int blockSeq = bitIndex / BLOCKSIZE;

		// currBlockSeq is the sequence number of the current plain block.
		int currBlockSeq = _plainWordOffset / BLOCKSIZE;
		if (DEBUGGING) cout << "Bit should go in block " << blockSeq << ", current block has sequence no " << currBlockSeq << endl;

		// Number of 0-fills which should be inserted
		int numNewZeroFills = blockSeq - currBlockSeq -1;

		int lastWordIndex = _compressedBits.size() - 1;

		// First, store the current plain block in the vector with compressed bits
		if (_plainWord == BLOCK_ZEROES){
			// The _plainBlock can be represented as a 0-fill. It might be needed to
			// append some more 0-fills. When the last block on the _compressedBits is
			// also a 0-fill, that 0-fill should be extended (if possible)
			numNewZeroFills++;

			if (numNewZeroFills > MAX_BLOCKS_IN_FILL) throw maxBlocksExceededException;

			if (lastWordIndex >= 0 && IS_ZEROFILL(_compressedBits[lastWordIndex])){
				// Last element of _compressedBits is a 0-fill: extend that one!

				// If the 1-bit at index 0 is cleared, the current size of the 0-fill
				// remains
				int numZeroFills = _compressedBits[lastWordIndex];
				CLEAR_BIT(numZeroFills,0);
				numZeroFills += numNewZeroFills;
				if (numZeroFills > MAX_BLOCKS_IN_FILL) throw maxBlocksExceededException;

				// Set the 1-bit at index 0 to indicate a fill bit.
				SET_BIT(numZeroFills, 0);

				// And replace the last word of the vector
				_compressedBits[lastWordIndex] = numZeroFills;
			} else {
				// No 0-fill on the back of the list, just append a number of 0-fills
				int compressedWord = numNewZeroFills;
				SET_BIT(compressedWord, 1);

				_compressedBits.push_back(compressedWord);
			}
		} else {
			if (_plainWord == BLOCK_ONES){
				if (DEBUGGING) cout << "Storing plain block (" << _plainWord << ") as 1-fill" << endl;

				// The _plainBlock can be represented as a 1-fill. If the last block on
				// _compressedBits is also a 1-fill, this 1-fill should be enlarged when possible
				if (lastWordIndex >= 0 && IS_ONEFILL(_compressedBits.back())){
					// Last element of _compressedBits is a 1-fill
					int numOneFills = _compressedBits[lastWordIndex];
					CLEAR_BIT(numOneFills, 0);
					CLEAR_BIT(numOneFills, 1);
					numOneFills++;
					if (numOneFills > MAX_BLOCKS_IN_FILL) throw maxBlocksExceededException;

					SET_BIT(numOneFills, 0);
					SET_BIT(numOneFills, 1);

					_compressedBits[lastWordIndex] = numOneFills;
				} else {
					// No 1-fill on the back of the list, just append simple 1-fill
					_compressedBits.push_back(+SIMPLE_ONEFILL);
				}
			} else {
				// literal block. Store _plainBlock as-is (it already contains a preceding 0).
				if (DEBUGGING) cout << "Storing plain block (" << _plainWord << ") as literal word" << endl;
				cout << this->toString() << endl;
				cout << BLOCK_ONES  << endl;
				_compressedBits.push_back(_plainWord);
			}

			// Append 0-fills when needed
			if (numNewZeroFills > 0){
				if (numNewZeroFills > MAX_BLOCKS_IN_FILL) throw maxBlocksExceededException;
				SET_BIT(numNewZeroFills, 0);
				_compressedBits.push_back(numNewZeroFills);
			}
		}

		// Now, reset the _plainBlock and determine its offset
		_plainWord = 0;
		_plainWordOffset = blockSeq * BLOCKSIZE;
	}

	// BitIndex is within the boundaries of the plain word: change bit of the plain block.
	// Note that the first bit of the plain block is not to be touched, hence the bitIndex + 1
	if (DEBUGGING) cout << "Bitindex " << bitIndex << " == index " << (bitIndex + 1 - _plainWordOffset) << " of plain word" << endl;
	if (value) SET_BIT(_plainWord, bitIndex + 1 - _plainWordOffset);
	else CLEAR_BIT(_plainWord, bitIndex + 1 - _plainWordOffset);
}

bool WAHBitSet::get(int bitIndex){
	if (bitIndex < _plainWordOffset){
		// The passed bit was already compressed. Let's find it...
		int currWord;
		int firstBitIndex = 0;
		int lastBitIndex = 0;
		for (unsigned int i = 0; i < _compressedBits.size(); i++){
			currWord = _compressedBits[i];
			if (currWord < 0){
				// First bit == 1, hence this is a fill word
				bool isOneFill = GET_BIT(currWord, 1);
				int fillNumBlocks = currWord;
				CLEAR_BIT(fillNumBlocks,0);
				CLEAR_BIT(fillNumBlocks,1);
				lastBitIndex = firstBitIndex + BLOCKSIZE * fillNumBlocks - 1;
				if (bitIndex >= firstBitIndex && bitIndex <= lastBitIndex) return isOneFill;
			} else {
				// First bit == 0, hence this is a literal word containing one block

				// The index of the last bit in this block
				lastBitIndex = firstBitIndex + BLOCKSIZE - 1;
				if (bitIndex >= firstBitIndex && bitIndex <= lastBitIndex){
					// Requested bit is sitting somewhere in this literal word
					bitIndex -= firstBitIndex;
					return GET_BIT(currWord, bitIndex + 1);
				}
			}

			firstBitIndex = lastBitIndex + 1;
		}
	} else if (bitIndex < _plainWordOffset + BLOCKSIZE){
		// BitIndex is within the boundaries of the plain block: change bit of the plain block.
		// Note that the first bit of the plain block is not to be touched, hence the bitIndex + 1
		return GET_BIT(_plainWord, bitIndex + 1 - _plainWordOffset);
	} else {
		// Out of bounds, that's a 0-bit
		return false;
	}
	return false;
}

string WAHBitSet::toBitString(int value){
	stringstream res;
	for (int bit = 0; bit < 32; bit++){
		if (GET_BIT(value, bit)) res << "1";
		else res << "0";
	}
	return res.str();
}

string WAHBitSet::toString(){
	stringstream res;
	for (unsigned int i = 0; i < _compressedBits.size(); i++){
		res << toBitString(_compressedBits[i]) << endl;
	}

	if (_plainWord != 0){
		res << endl << "Plain block:" << endl;
		res << toBitString(_plainWord) << endl;
	}

	return res.str();
}
