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
#include <stdlib.h>
using namespace std;

#define RAND_FLOAT() ((rand() % 1000) / 1000.0)

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
#define FILL_LENGTH(var) ((var) & 0b00111111111111111111111111111111)

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
	_plainWordBlockSeq = 0;
}

WAHBitSet::~WAHBitSet() {
}

void WAHBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void WAHBitSet::set(int bitIndex, bool value){
	//if (DEBUGGING) cout << "WAHBitSet::set setting bit with index " << bitIndex << endl;
	if (bitIndex < _plainWordBlockSeq * BLOCKSIZE){
		// The passed bit was already compressed. Can't do anything about that.
		stringstream stream;
		stream << "Cannot set/unset bit at index " << bitIndex << ": bit is already compressed!";
		throw stream.str();
	}

	if (bitIndex >= _plainWordBlockSeq * BLOCKSIZE + BLOCKSIZE){
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

		//if (DEBUGGING) cout << "New bit should go in block " << blockSeq << ", current block has sequence no " << currBlockSeq << endl;

		// Number of 0-fills which should be inserted
		int numNewZeroFills = blockSeq - _plainWordBlockSeq -1;

		// First, store the current plain block in the vector with compressed bits
		if (_plainWord == BLOCK_ZEROES){
			// The _plainBlock can be represented as a 0-fill. It might be needed to
			// append some more 0-fills. When the last block on the _compressedBits is
			// also a 0-fill, that 0-fill should be extended (if possible)
			this->addZeroFill(++numNewZeroFills);
		} else {
			if (_plainWord == BLOCK_ONES){
				this->addOneFill(1);
			} else {
				// literal block. Store _plainBlock as-is (it already contains a preceding 0).
				this->addLiteral(_plainWord);
			}

			// Append 0-fills when needed
			if (numNewZeroFills > 0) this->addZeroFill(numNewZeroFills);
		}

		// Now, reset the _plainBlock and determine its offset
		_plainWord = 0;
		_plainWordBlockSeq = blockSeq;
	}

	// BitIndex is within the boundaries of the plain word: change bit of the plain block.
	//if (DEBUGGING) cout << "Bitindex " << bitIndex << " == index " << (bitIndex - _plainWordOffset) << " of plain word" << endl;
	if (value) SET_BIT(_plainWord, bitIndex - _plainWordBlockSeq * BLOCKSIZE);
	else CLEAR_BIT(_plainWord, bitIndex - _plainWordBlockSeq * BLOCKSIZE);
}

bool WAHBitSet::get(int bitIndex){
	if (bitIndex < _plainWordBlockSeq * BLOCKSIZE){
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
	} else if (bitIndex < _plainWordBlockSeq * BLOCKSIZE + BLOCKSIZE){
		// BitIndex is within the boundaries of the plain block: fetch bit from plain block.
		return GET_BIT(_plainWord, bitIndex - _plainWordBlockSeq * BLOCKSIZE);
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
	_plainWordBlockSeq = 0;
}

void WAHBitSet::addOneFill(int numBlocks){
	int lastWordIndex = _compressedBits.size() - 1;

	if (lastWordIndex >= 0 && IS_ONEFILL(_compressedBits[lastWordIndex])){
		// Last element of _compressedBits is a 1-fill
		int numOneFills = FILL_LENGTH(_compressedBits[lastWordIndex]) + numBlocks;
		if (numOneFills > MAX_BLOCKS_IN_FILL) throw range_error("Number of one blocks exceeds maximum number of blocks in a fill");

		_compressedBits[lastWordIndex] = numOneFills | EMPTY_ONEFILL;
	} else {
		// No 1-fill on the back of the list, just append simple 1-fill
		_compressedBits.push_back(numBlocks | EMPTY_ONEFILL);
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
		_compressedBits.push_back((numBlocks | EMPTY_ZEROFILL));
	}
}

void WAHBitSet::addLiteral(int value){
	_compressedBits.push_back(value);
}

/**
 * Don't use this, it's slow and only for testing purposes! Not that only the first 31 bits
 * from the paramater 'value' are actually used.
 */
void WAHBitSet::setBits(int blockSeq, int value){
	for (int i = 0; i < 31; i++){
		bool val = GET_BIT(value, i);
		string strval;
		if (val) strval = "1";
		else strval = "0";

		this->set(blockSeq * BLOCKSIZE + i, val);
	}
}

/**
 * Generates 31 random bits which cannot be represented as a 0-fill or 1-fill.
 */
int WAHBitSet::generateRandomLiteralBlock(){
	int value = 0;

	for (int i = 0; i < 31; i++){
		if (RAND_FLOAT() > 0.5) SET_BIT(value, i);
	}

	if (value == BLOCK_ONES || value == BLOCK_ZEROES) return generateRandomLiteralBlock();
	return value;
}

WAHBitSet WAHBitSet::constructByOr(const WAHBitSet& bs1, const WAHBitSet& bs2){
	WAHBitSet result;

	const WAHBitSet * actBitSet;
	const WAHBitSet * otherBitSet;

	int word1, word2; // current words under investigation
	int* actWord;
	int* otherWord;

	int wordOffset1 = 0; // offset within the current words under investigation (counted in blocks)
	int wordOffset2 = 0;
	int* actWordOffset;
	int* otherWordOffset;

	unsigned int w1 = 0; // counter for current word index of bs1 and bs2
	unsigned int w2 = 0;
	unsigned int* actW;
	unsigned int* otherW;

	int fillLength1, fillLength2;
	int* actFillLength;
	int* otherFillLength;
	while (w1 < bs1._compressedBits.size() || w2 < bs2._compressedBits.size()){
		cout << "w1=" << w1 << ", wordOffset1=" << wordOffset1 << ", w2=" << w2 << ", wordOffset2=" << wordOffset2 << endl;
		if (w1 >= bs1._compressedBits.size()){
			// End of BitSet1 was reached, pretend only a huge 0-fill is left
			w1 = MAX_BLOCKS_IN_FILL | EMPTY_ZEROFILL;
		} else {
			word1 = bs1._compressedBits[w1];
		}

		if (w2 >= bs2._compressedBits.size()){
			// End of BitSet2 was reached, pretend only a huge 0-fill is left
			w2 = MAX_BLOCKS_IN_FILL | EMPTY_ZEROFILL;
		} else {
			word2 = bs2._compressedBits[w2];
		}

		// Check fill length, minus the offset in the current word. This value tells us how
		// many blocks are left in the current fill.
		fillLength1 = -1;
		fillLength2 = -1;
		if (IS_FILL(word1)) fillLength1 = FILL_LENGTH(word1) - wordOffset1;
		if (IS_FILL(word2)) fillLength2 = FILL_LENGTH(word2) - wordOffset2;

		if (fillLength1 == -1 && fillLength2 == -1){
			// Both BitSets are at a literal word
			cout << "at literal " << endl;
			int merged = word1 | word2;
			if (merged == BLOCK_ONES) result.addOneFill(1);
			else if (merged == BLOCK_ZEROES) result.addZeroFill(1);
			else result.addLiteral(merged);

			w1++;
			w2++;
		} else {
			// At least one of two BitSets is at a fill word

			if (fillLength1 >= fillLength2){
				// number of blocks left in the current fill of BitSets 1 is bigger than that of BitSets 2
				actWord = &word1;
				actFillLength = &fillLength1;
				otherWord = &word2;
				otherFillLength = &fillLength2;
				actWordOffset = &wordOffset1;
				otherWordOffset = &wordOffset2;
				actW = &w1;
				otherW = &w2;
				actBitSet = &bs1;
				otherBitSet = &bs2;
			} else {
				// number of blocks left in the current fill of BitSets 1 is bigger than that of BitSets 2
				actWord = &word2;
				actFillLength = &fillLength2;
				otherWord = &word1;
				otherFillLength = &fillLength1;
				actWordOffset = &wordOffset2;
				otherWordOffset = &wordOffset1;
				actW = &w2;
				otherW = &w1;
				actBitSet = &bs2;
				otherBitSet = &bs1;
			}

			if (IS_ONEFILL(*actWord)){
				// Process 1-fill from one of the words (ignoring the other word for the time being)
				result.addOneFill(*actFillLength);
				*actW++;
				*actWordOffset = 0;

				// *actFillLength blocks were processed by adding the 1-fill, we should increase the
				// active position in the other (ignored) BitSet too.
				if (*otherFillLength != -1 && *otherFillLength == *actFillLength){
					// Got lucky: other BitSet was at a fill too, of equal length. That's easy.
					*otherW++;
					*otherWordOffset = 0;
				} else {
					// Other BitSet was at a literal, or at a shorter fill. We'll have to skip
					// one or more words there
					*otherWordOffset = 0;
					int skipped = 0;
					int* currWord = otherWord;
					int currFillLength;
					int skipsToGo;
					while (skipped < *actFillLength){
						skipsToGo = *actFillLength - skipped;

						if (IS_FILL(*currWord)){
							// Fill word, potential big skip. But be careful, we shouldn't skip too far...
							currFillLength = FILL_LENGTH(*currWord);
							if (currFillLength < skipsToGo){
								// Safe to skip the entire fill
								skipped += currFillLength;
								*otherW++;
							} else {
								// Destination is somewhere in the middle of this fill
								*otherWordOffset = skipsToGo;
								break;
							}
						} else {
							// Literal word, that's only one block. Small skip.
							skipped++;
							*otherW++;
						}

						if (*otherW >= otherBitSet->_compressedBits.size()){
							// Reached end of the other BitSet
							break;
						} else {
							// Increase pointer position to next element in vector _compressedBits
							currWord++;
						}
					} // end while: skipping through other word
				} // end if
			} else {
				// Process 0-fill from one of the words (copy the other word for the time being)
			}
		}
	}

	// Only thing left: merge plain words
	result._plainWord = bs1._plainWord | bs2._plainWord;

	// TODO:!! _plainWordOffset!!!
	return result;
}
