/*
 * WAHBitSet.cpp
 *
 *  Created on: Apr 10, 2010
 *      Author: bas
 */

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
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
#define IS_FILL(var) (GET_BIT(var,31))

/**
 * Macro to determine the length of a FILL
 */
#define FILL_LENGTH(var) ((var) ^ EMPTY_ONEFILL)

/**
 * Macro to determine whether a var represents a 1-fill
 */
#define IS_ONEFILL(var) (IS_FILL(var) && GET_BIT(var,30))

/**
 * Macro to determine whether a var represents a 0-fill
 */
#define IS_ZEROFILL(var) (IS_FILL(var) && !GET_BIT(var,30))

WAHBitSet::WAHBitSet() {
	_plainWord = 0;
	_plainWordOffset = 0;
}

WAHBitSet::~WAHBitSet() {
}

void WAHBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void WAHBitSet::set(int bitIndex, bool value){
	//if (DEBUGGING) cout << "WAHBitSet::set setting bit with index " << bitIndex << endl;
	if (bitIndex < _plainWordOffset){
		// The passed bit was already compressed. Can't do anything about that.
		stringstream stream;
		stream << "Cannot set/unset bit at index " << bitIndex << ": bit is already compressed!";
		throw stream.str();
	}

	if (bitIndex >= _plainWordOffset + BLOCKSIZE){
		// BitIndex not within boundaries of the plain word. The plain word should be written
		// to the _compressedBits.

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
		//if (DEBUGGING) cout << "New bit should go in block " << blockSeq << ", current block has sequence no " << currBlockSeq << endl;

		// Number of 0-fills which should be inserted
		int numNewZeroFills = blockSeq - currBlockSeq -1;

		// First, store the current plain block in the vector with compressed bits
		if (_plainWord == BLOCK_ZEROES){
			// The _plainBlock can be represented as a 0-fill. It might be needed to
			// append some more 0-fills. When the last block on the _compressedBits is
			// also a 0-fill, that 0-fill should be extended (if possible)
			this->addZeroFill(++numNewZeroFills);
		} else {
			if (_plainWord == BLOCK_ONES){
				this->addOneFill();
			} else {
				// literal block. Store _plainBlock as-is (it already contains a preceding 0).
				this->addLiteral(_plainWord);
			}

			// Append 0-fills when needed
			if (numNewZeroFills > 0) this->addZeroFill(numNewZeroFills);
		}

		// Now, reset the _plainBlock and determine its offset
		_plainWord = 0;
		_plainWordOffset = blockSeq * BLOCKSIZE;
	}

	// BitIndex is within the boundaries of the plain word: change bit of the plain block.
	//if (DEBUGGING) cout << "Bitindex " << bitIndex << " == index " << (bitIndex - _plainWordOffset) << " of plain word" << endl;
	if (value) SET_BIT(_plainWord, bitIndex - _plainWordOffset);
	else CLEAR_BIT(_plainWord, bitIndex - _plainWordOffset);
}

bool WAHBitSet::get(int bitIndex){
	if (bitIndex < _plainWordOffset){
		// The passed bit was already compressed. Lets find it...
		//cout << "Bit with index " << bitIndex << " already compressed..." << endl;
		int currWord;
		int firstBitIndex = 0;
		int lastBitIndex = 0;
		for (unsigned int i = 0; i < _compressedBits.size(); i++){
			currWord = _compressedBits[i];
			if (currWord < 0){
				// Fill word
				bool isOneFill = GET_BIT(currWord, 30);
				int fillNumBlocks = currWord;
				CLEAR_BIT(fillNumBlocks,30);
				CLEAR_BIT(fillNumBlocks,31);
				lastBitIndex = firstBitIndex + BLOCKSIZE * fillNumBlocks - 1;
				if (bitIndex >= firstBitIndex && bitIndex <= lastBitIndex) return isOneFill;
			} else {
				// First bit == 0, hence this is a literal word containing one block

				// The index of the last bit in this block
				lastBitIndex = firstBitIndex + BLOCKSIZE - 1;
				if (bitIndex >= firstBitIndex && bitIndex <= lastBitIndex){
					// Requested bit is sitting somewhere in this literal word
					bitIndex -= firstBitIndex;
					return GET_BIT(currWord, bitIndex);
				}
			}

			firstBitIndex = lastBitIndex + 1;
		}
	} else if (bitIndex < _plainWordOffset + BLOCKSIZE){
		// BitIndex is within the boundaries of the plain block: fetch bit from plain block.
		return GET_BIT(_plainWord, bitIndex - _plainWordOffset);
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
	res << " (= " << value << ")";
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

void WAHBitSet::clear(){
	_compressedBits = vector<int>();
	_plainWord = 0;
	_plainWordOffset = 0;
}

void WAHBitSet::addOneFill(){
	int lastWordIndex = _compressedBits.size() - 1;
	if (lastWordIndex >= 0 && IS_ONEFILL(_compressedBits[lastWordIndex])){
		// Last element of _compressedBits is a 1-fill
		int numOneFills = FILL_LENGTH(_compressedBits[lastWordIndex]) + 1;
		if (numOneFills > MAX_BLOCKS_IN_FILL) throw range_error("Number of one blocks exceeds maximum number of blocks in a fill");

		_compressedBits[lastWordIndex] = numOneFills | EMPTY_ONEFILL;
	} else {
		// No 1-fill on the back of the list, just append simple 1-fill
		_compressedBits.push_back(+SIMPLE_ONEFILL);
	}
}

void WAHBitSet::addZeroFill(int numBlocks){
	int lastWordIndex = _compressedBits.size() - 1;

	if (lastWordIndex >= 0 && IS_ZEROFILL(_compressedBits[lastWordIndex])){
		// Last element of _compressedBits is a 0-fill: extend that one!

		int numZeroFills = FILL_LENGTH(_compressedBits[lastWordIndex]) + numBlocks;
		if (numZeroFills > MAX_BLOCKS_IN_FILL) throw range_error("Number of zero blocks exceeds maximum number of blocks in a fill");

		// And replace the last word of the vector
		_compressedBits[lastWordIndex] = numZeroFills | EMPTY_ZEROFILL;
	} else {
		// No 0-fill on the back of the list, just append a number of 0-fills
		if (numBlocks > MAX_BLOCKS_IN_FILL) throw range_error("Number of zero blocks exceeds maximum number of blocks in a fill");
		_compressedBits.push_back(numBlocks | EMPTY_ZEROFILL);
	}
}

void WAHBitSet::addLiteral(int value){
	_compressedBits.push_back(value);
}

WAHBitSet WAHBitSet::constructByOr(const WAHBitSet& bs1, const WAHBitSet& bs2){
	WAHBitSet result;

	int word1, word2; // current words under investigation
	int wordOffset1, wordOffset2; // offset within the current words under investigation
	int w1, w2; // counter for current word index of bs1 and bs2
	int bl1, bl2; // counter for current block index of bs1 and bs2
	int fillLength1, fillLength2;
	bool oneFill;
	while (w1 < bs1._compressedBits.size() || w2 < bs2._compressedBits.size()){
		word1 = bs1._compressedBits[w1];
		word2 = bs2._compressedBits[w2];

		fillLength1 = -1;
		fillLength2 = -1;
		if (IS_ONEFILL(word1)) fillLength1 = FILL_LENGTH(word1);
		if (IS_ONEFILL(word2)) fillLength2 = FILL_LENGTH(word2);

		if (fillLength1 > fillLength2){

		} else if (fillLength1 < fillLength2){

		} else {

		}
		w1++;
		w2++;
	}

	return result;
}
