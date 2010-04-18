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

/**
 * Macro to determine whether a var is a literal concealing a 1-fill
 */
#define IS_LITERAL_ONEFILL(var) ((var) == 0b01111111111111111111111111111111)

/**
 * Macro to determine whether a var is a literal concealing a 0-fill
 */
#define IS_LITERAL_ZEROFILL(var) ((var) == 0b00000000000000000000000000000000)


WAHBitSet::WAHBitSet() {
	init();
}

WAHBitSet::~WAHBitSet() {
}

WAHBitSet::WAHBitSet(DynamicBitSet& dynamicBitSet){
	init();
	for (unsigned int i = 0; i < dynamicBitSet.size(); i++) this->set(i, dynamicBitSet.get(i));
}

void WAHBitSet::set(int bitIndex){
	set(bitIndex, true);
}

void WAHBitSet::init(){
	_lastBitIndex = -1;
	_plainWord = 0;
	_plainWordBlockSeq = 0;
	_compressedBits = vector<int>();
}

void WAHBitSet::set(int bitIndex, bool value){
	if (bitIndex > _lastBitIndex) _lastBitIndex = bitIndex;

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
	const bool debug = false;
	if (bitIndex > _lastBitIndex) throw range_error("Index out of bounds");

	if (bitIndex < _plainWordBlockSeq * BLOCKSIZE){
		// The passed bit was already compressed. Lets find it...
		if (debug) cout << "Bit with index " << bitIndex << " already compressed..." << endl;
		int currWord;
		int firstBitIndex = 0;
		int lastBitIndex = 0;
		for (unsigned int i = 0; i < _compressedBits.size(); i++){
			currWord = _compressedBits[i];
			if (IS_FILL(currWord) < 0){
				// Fill word
				lastBitIndex = firstBitIndex + BLOCKSIZE * FILL_LENGTH(currWord) - 1;
				if (bitIndex >= firstBitIndex && bitIndex <= lastBitIndex){
					if (debug) cout << "Bit with index " << bitIndex << " is located in fill word " << i << ": " << toBitString(currWord) << endl;
					return IS_ONEFILL(currWord);
				}
			} else {
				// Literal word containing one block

				// The index of the last bit in this block
				lastBitIndex = firstBitIndex + BLOCKSIZE - 1;
				if (bitIndex >= firstBitIndex && bitIndex <= lastBitIndex){
					// Requested bit is sitting somewhere in this literal word
					//bitIndex -= firstBitIndex;
					if (debug) cout << "Bit with index " << bitIndex << " is located in literal word " << i << " (" << firstBitIndex << "--" << lastBitIndex << "): " << toBitString(currWord) << endl;
					return GET_BIT(currWord, bitIndex % 31);
				}
			}

			firstBitIndex = lastBitIndex + 1;
		}
	} else if (bitIndex < _plainWordBlockSeq * BLOCKSIZE + BLOCKSIZE){
		// BitIndex is within the boundaries of the plain block: fetch bit from plain block.
		if (debug) cout << "Bit with index " << bitIndex << " is sitting in plain word..." << endl;
		return GET_BIT(_plainWord, bitIndex - _plainWordBlockSeq * BLOCKSIZE);
	} else {
		// Out of bounds, that's a 0-bit
		if (debug) cout << "Bit with index " << bitIndex << " out of bounds..." << endl;
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
	init();
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
	if (IS_LITERAL_ZEROFILL(value)) addZeroFill(1);
	else if (IS_LITERAL_ONEFILL(value)) addOneFill(1);
	else _compressedBits.push_back(value);
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
	const bool debug = false;
	WAHBitSet result;

	const WAHBitSet * actBitSet;
	const WAHBitSet * otherBitSet;

	int word1, word2; // words currently under investigation
	const int* actWord;
	const int* otherWord;

	// plain words of the two bitsets
	int plainWord1 = bs1._plainWord;
	int plainWord2 = bs2._plainWord;
	const int* actPlainWord;
	const int* otherPlainWord;

	int wordOffset1 = 0; // offset within the current words under investigation (counted in blocks)
	int wordOffset2 = 0;
	int* actWordOffset;
	int* otherWordOffset;

	unsigned int w1 = 0; // counter for current word index of bs1 and bs2
	unsigned int w2 = 0;
	unsigned int* actW;
	unsigned int* otherW;

	int fillLength1, fillLength2, remainingFillLength1, remainingFillLength2;
	const int* actFillLength;
	const int* otherFillLength;
	int* actRemainingFillLength;
	int* otherRemainingFillLength;

	bool processedPlainWord1 = false;
	bool processedPlainWord2 = false;
	bool* processedActPlainWord;
	bool* processedOtherPlainWord;

	while (!processedPlainWord1 || !processedPlainWord2){
		if (debug) cout << "w1=" << w1 << ", wordOffset1=" << wordOffset1 << ", w2=" << w2 << ", wordOffset2=" << wordOffset2 << endl;
		if (w1 >= bs1._compressedBits.size()){
			// End of BitSet1 was reached.
			if (debug) cout << "End of BitSet1 reached!" << endl;

			if (!processedPlainWord1){
				word1 = bs1._plainWord;
				processedPlainWord1 = true;
			} else {
				// Plain word was also processed, pretend only a huge 0-fill is left
				word1 = MAX_BLOCKS_IN_FILL | EMPTY_ZEROFILL;
			}
		} else {
			word1 = bs1._compressedBits[w1];
		}
		if (debug) cout << "Current word of BitSet 1: " << toBitString(word1) << endl;

		if (w2 >= bs2._compressedBits.size()){
			// End of BitSet2 was reached, pretend only a huge 0-fill is left
			if (debug) cout << "End of BitSet2 reached!" << endl;

			if (!processedPlainWord2){
				word2 = bs2._plainWord;
				processedPlainWord2 = true;
			} else {
				// Plain word was also processed, pretend only a huge 0-fill is left
				word2 = MAX_BLOCKS_IN_FILL | EMPTY_ZEROFILL;
			}
		} else {
			word2 = bs2._compressedBits[w2];
		}

		if (debug) cout << "Current word of BitSet 2: " << toBitString(word2) << endl;

		// Check fill length, minus the offset in the current word. This value tells us how
		// many blocks are left in the current fill.
		fillLength1 = -1;
		fillLength2 = -1;
		remainingFillLength1 = -1;
		remainingFillLength2 = -1;
		if (IS_FILL(word1)){
			fillLength1 = FILL_LENGTH(word1);
			remainingFillLength1 = fillLength1 - wordOffset1;
		}
		if (IS_FILL(word2)){
			fillLength2 = FILL_LENGTH(word2);
			remainingFillLength2 = fillLength2 - wordOffset2;
		}

		if (remainingFillLength1 == -1 && remainingFillLength2 == -1){
			// Both BitSets are at a literal word
			if (debug) cout << "Encountered two literal words, conducting simple merge " << endl;
			int merged = word1 | word2;
			if (merged == BLOCK_ONES) result.addOneFill(1);
			else if (merged == BLOCK_ZEROES) result.addZeroFill(1);
			else result.addLiteral(merged);

			w1++;
			w2++;
		} else {
			// At least one of two BitSets is at a fill word. Find out which fill word has most
			// blocks left to process. That word will become the 'active word', the other word
			// will be the 'other word'.
			if (remainingFillLength1 >= remainingFillLength2){
				// number of blocks left in the current fill of BitSets 1 is bigger than that of BitSets 2
				if (debug) cout << "Processing fill from BitSet 1: " << remainingFillLength1 << " blocks left..." << endl;
				actWord = &word1;
				actFillLength = &fillLength1;
				actRemainingFillLength = &remainingFillLength1;
				actWordOffset = &wordOffset1;
				actW = &w1;
				actBitSet = &bs1;
				actPlainWord = &plainWord1;
				processedActPlainWord = &processedPlainWord1;

				otherWord = &word2;
				otherFillLength = &fillLength2;
				otherRemainingFillLength = &remainingFillLength2;
				otherWordOffset = &wordOffset2;
				otherW = &w2;
				otherBitSet = &bs2;
				otherPlainWord = &plainWord2;
				processedOtherPlainWord = &processedPlainWord2;
			} else {
				// number of blocks left in the current fill of BitSets 1 is bigger than that of BitSets 2
				if (debug) cout << "Processing fill from BitSet 2: " << remainingFillLength2 << " blocks left..." << endl;
				actWord = &word2;
				actPlainWord =
				actFillLength = &fillLength2;
				actRemainingFillLength = &remainingFillLength2;
				actWordOffset = &wordOffset2;
				actW = &w2;
				actBitSet = &bs2;
				actPlainWord = &plainWord2;
				processedActPlainWord = &processedPlainWord2;


				otherWord = &word1;
				otherFillLength = &fillLength1;
				otherRemainingFillLength = &remainingFillLength1;
				otherWordOffset = &wordOffset1;
				otherW = &w1;
				otherBitSet = &bs1;
				otherPlainWord = &plainWord1;
				processedOtherPlainWord = &processedPlainWord1;
			}

			bool actWordIsOneFill = IS_ONEFILL(*actWord);

			if (actWordIsOneFill){
				// Process 1-fill from one of the words (ignoring the other word for the time being)
				result.addOneFill(*actRemainingFillLength);
			}

			// *actRemainingFillLength were detected in the active word. If the active word
			// was a 1-fill, the other word should be ignored. If the active word was a
			// 0-fill, the contents of the other word should be copied.
			// Either way, we need to align the two BitSets and find out where to stop processing
			// the other word. That might well be in the middle of a fill.

			if (*otherRemainingFillLength != -1 && *otherRemainingFillLength == *actRemainingFillLength){
				// Got lucky: other BitSet was at a fill too, of equal length. That's easy.
				if (debug) cout << "Other BitSet was at fill of equal length, lucky!" << endl;
				if (!actWordIsOneFill){
					// The active word is apparently a 0-fill, store the value of the other word
					if (IS_ONEFILL(*otherWord)) result.addOneFill(*otherRemainingFillLength);
					else result.addZeroFill(*otherRemainingFillLength);
				} // else: active word is 1-fill, ignore other word

				(*otherW)++;
				*otherWordOffset = 0;

				(*actW)++;
				*actWordOffset = 0;
			} else {
				// Other BitSet was at a literal, or at a shorter fill. We'll have to process
				// one or more words there
				if (debug) cout << "Skipping other BitSet for " << *actRemainingFillLength << " blocks" << endl;

				int blocksProcessed = 0;
				int foo;
				int blocksToGo;
				while (blocksProcessed < *actRemainingFillLength){
					blocksToGo = *actRemainingFillLength - blocksProcessed;
					if (debug) cout << "Skipped " << blocksProcessed << " blocks, " << blocksToGo << " blocks to go" << endl;

					if (blocksProcessed > 0){
						if (*otherW >= otherBitSet->_compressedBits.size()){
							if (*processedOtherPlainWord){
								if (debug) cout << "Reached end of other bitset: " << *otherW << endl;
								break;
							} else {
								if (debug) cout << "Processed last compressed word (" << toBitString(*otherWord) << ") of the other BitSet, processing plain word: " << toBitString(*otherPlainWord) << endl;
								*processedOtherPlainWord = true;
								otherWord = otherPlainWord;
								*otherWordOffset = 0;
							}
						} else {
							// Continue looking at the other BitSet
							otherWord = &otherBitSet->_compressedBits[*otherW];
							*otherWordOffset = 0;
						}
					}

					if (debug) cout << "Other word: " << toBitString(*otherWord) << endl;


					if (IS_FILL(*otherWord)){
						// Fill word, potential big skip. But be careful, we shouldn't skip too far...
						foo = FILL_LENGTH(*otherWord);
						otherFillLength = &foo;
						*otherRemainingFillLength = *otherFillLength - *otherWordOffset;

						if (debug) cout << "Other word is fill of length " << *otherFillLength << ", of which " << *otherRemainingFillLength << " blocks are remaining: " << toBitString(*otherWord) << endl;
						if (*otherRemainingFillLength <= blocksToGo){
							// Safe to skip the entire fill of the other word
							blocksProcessed += *otherRemainingFillLength;

							if (!actWordIsOneFill){
								// Active word is a 0-fill, blocks from the other word should be copied
								if (IS_ONEFILL(*otherWord)) result.addOneFill(*otherRemainingFillLength);
								else result.addZeroFill(*otherRemainingFillLength);
							}

							*otherWordOffset = 0;
							(*otherW)++;
						} else {
							// Destination position is somewhere in the middle of the fill of the other word

							if (!actWordIsOneFill){
								// Active BitSet is at a 0-fill, remaining blocks from the word from
								// the other BitSet should be copied
								if (IS_ONEFILL(*otherWord)) result.addOneFill(blocksToGo);
								else result.addZeroFill(blocksToGo);
							}

							blocksProcessed += blocksToGo;
							*otherWordOffset += blocksToGo;
							break;
						}
					} else {
						// Literal word, that's only one block. Small skip.
						blocksProcessed++;

						if (!actWordIsOneFill){
							// Active BitSet is at a 0-fill, add literal word from the other BitSet.
							result.addLiteral(*otherWord);
						}
						(*otherW)++;
					}
				} // end while: skipping through other word
				if (debug) cout << "Done skipping!" << endl;

				if (blocksProcessed == *actFillLength || (processedPlainWord1 && processedPlainWord2)){
					// Apparently, all blocks were processed. This means the next word from the active BitSet
					// should be inspected
					(*actW)++;
					*actWordOffset = 0;
				} else {
					// Apparently, it was not possible to process all blocks. This could happen when the end of
					// other BitSet is encountered. In that case, simply skipping to the next word in the active
					// BitSet will yield an invalid result.
					// Suppose the active word consisted of a 0-fill of 10 blocks. After skipping the first
					// 4 blocks, the end of the other BitSet was encountered. The remaining 6 blocks should
					// then be added to the result, before looking at the next word.
					// In case of a 1-fill of 10 blocks the result would be valid.
					if (!actWordIsOneFill) result.addZeroFill(*actRemainingFillLength - blocksProcessed);
					(*actW)++;
					*actWordOffset = 0;
				}
			} // end if


		}
	} // end while

	result._plainWordBlockSeq = max(bs1._plainWordBlockSeq, bs2._plainWordBlockSeq) + 1;
	result._lastBitIndex = max(bs1._lastBitIndex, bs2._lastBitIndex);
	return result;
}

unsigned int WAHBitSet::size(){
	return _lastBitIndex + 1;
}

WAHBitSet WAHBitSet::constructFailingExample1(){
	WAHBitSet res;
	res._compressedBits.clear();
	res._compressedBits.push_back(-2147483645); // 0-fill of 3 block
	res._compressedBits.push_back(-1073741822); // 1-fill of 2 blocks
	res._compressedBits.push_back(1526167291); // literal
	res._compressedBits.push_back(-2147483647); // 0-fill of 1 block
	res._compressedBits.push_back(-1073741823); // 1-fill of 1 blocks
	res._compressedBits.push_back(-2147483647); // 0-fill of 1 blocks
	res._plainWordBlockSeq = 9;
	res._plainWord = 1893837043; // literal
	res._lastBitIndex = 309;
	return res;
}

WAHBitSet WAHBitSet::constructFailingExample2(){
	WAHBitSet res;
	res._compressedBits.clear();
	res._compressedBits.push_back(237562924); // literal
	res._compressedBits.push_back(270540582); // literal
	res._compressedBits.push_back(-1073741822); // 1-fill of 2 block
	res._compressedBits.push_back(491844537); // literal
	res._compressedBits.push_back(-2147483647); // 0-fill of 1 block
	res._compressedBits.push_back(3328389); // literal
	res._plainWordBlockSeq = 7;
	res._plainWord = 1784157257; // 306848798
	res._lastBitIndex = 247;
	return res;
}
