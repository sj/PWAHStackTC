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
#include "IntMacros.cpp"
using namespace std;

#define RAND_FLOAT() ((rand() % 1000) / 1000.0)


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
	_empty = true;
}

void WAHBitSet::set(int bitIndex, bool value){
	if (DEBUGGING) cout << "WAHBitSet::set setting bit with index " << bitIndex << endl;
	if (bitIndex < _plainWordBlockSeq * BLOCKSIZE){
		// The passed bit was already compressed. Can't do anything about that.
		stringstream stream;
		stream << "Cannot set/unset bit at index " << bitIndex << ": bit is already compressed!";
		throw stream.str();
	}

	if (bitIndex > _lastBitIndex) _lastBitIndex = bitIndex;
	if (value) _empty = false;

	if (bitIndex >= _plainWordBlockSeq * BLOCKSIZE + BLOCKSIZE){
		// BitIndex not within boundaries of the plain word. The plain word should be written
		// to the _compressedBits.
		if (DEBUGGING) cout << "WAHBitSet::set compressing plain word.." << endl;

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

	if (DEBUGGING) cout << "WAHBitSet::set done setting bit with index " << bitIndex <<  "!" << endl;
}

bool WAHBitSet::get(int bitIndex){
	const bool debug = false;
	if (_empty) return false;
	if (bitIndex > _lastBitIndex) return false;

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
	if (numBlocks < 1) throw range_error("Cannot add 1-fill of size < 1");
	_empty = false;
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
	if (numBlocks < 1) throw range_error("Cannot add 0-fill of size < 1");
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
	if (value != 0) _empty = false;

	if (IS_LITERAL_ZEROFILL(value)) addZeroFill(1);
	else if (IS_LITERAL_ONEFILL(value)) addOneFill(1);
	else _compressedBits.push_back(value);
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

WAHBitSet* WAHBitSet::constructByOr(const WAHBitSet* bs1, const WAHBitSet* bs2){
	const bool debug = false;

	if (bs1->_empty){
		// Use copy constructor to create a copy of second BitSet
		return new WAHBitSet(*bs2);
	} else if (bs2->_empty){
		// Use copy constructor to create a copy of first BitSet
		return new WAHBitSet(*bs1);
	}

	WAHBitSet* result = new WAHBitSet();

	const WAHBitSet * actBitSet;
	const WAHBitSet * otherBitSet;

	const int* word1; // words currently under investigation
	const int* word2;
	const int* actWord;
	const int* otherWord;

	// plain words of the two bitsets
	const int* plainWord1 = &bs1->_plainWord;
	const int* plainWord2 = &bs2->_plainWord;
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
	int huge_zerofill = MAX_BLOCKS_IN_FILL | EMPTY_ZEROFILL;

	while (!processedPlainWord1 || !processedPlainWord2){
		if (debug) cout << "w1=" << w1 << ", wordOffset1=" << wordOffset1 << ", w2=" << w2 << ", wordOffset2=" << wordOffset2 << endl;
		if (w1 >= bs1->_compressedBits.size()){
			// End of BitSet1 was reached.
			if (debug) cout << "End of BitSet1 reached!" << endl;

			if (!processedPlainWord1){
				word1 = &bs1->_plainWord;
				processedPlainWord1 = true;
			} else {
				// Plain word was also processed, pretend only a huge 0-fill is left
				word1 = &huge_zerofill;
			}
		} else {
			word1 = &bs1->_compressedBits[w1];
		}
		if (debug) cout << "Current word of BitSet 1: " << toBitString(*word1) << endl;

		if (w2 >= bs2->_compressedBits.size()){
			// End of BitSet2 was reached, pretend only a huge 0-fill is left
			if (debug) cout << "End of BitSet2 reached!" << endl;

			if (!processedPlainWord2){
				word2 = &bs2->_plainWord;
				processedPlainWord2 = true;
			} else {
				// Plain word was also processed, pretend only a huge 0-fill is left
				word2 = &huge_zerofill;
			}
		} else {
			word2 = &bs2->_compressedBits[w2];
		}

		if (debug) cout << "Current word of BitSet 2: " << toBitString(*word2) << endl;

		// Check fill length, minus the offset in the current word. This value tells us how
		// many blocks are left in the current fill.
		fillLength1 = -1;
		fillLength2 = -1;
		remainingFillLength1 = -1;
		remainingFillLength2 = -1;
		if (IS_FILL(*word1)){
			fillLength1 = FILL_LENGTH(*word1);
			remainingFillLength1 = fillLength1 - wordOffset1;
		}
		if (IS_FILL(*word2)){
			fillLength2 = FILL_LENGTH(*word2);
			remainingFillLength2 = fillLength2 - wordOffset2;
		}

		if (remainingFillLength1 == -1 && remainingFillLength2 == -1){
			// Both BitSets are at a literal word
			if (debug) cout << "Encountered two literal words, conducting simple merge " << endl;
			int merged = *word1 | *word2;
			if (merged == BLOCK_ONES) result->addOneFill(1);
			else if (merged == BLOCK_ZEROES) result->addZeroFill(1);
			else result->addLiteral(merged);

			w1++;
			w2++;
		} else {
			// At least one of two BitSets is at a fill word. Find out which fill word has most
			// blocks left to process. That word will become the 'active word', the other word
			// will be the 'other word'.
			if (remainingFillLength1 >= remainingFillLength2){
				// number of blocks left in the current fill of BitSets 1 is bigger than that of BitSets 2
				if (debug) cout << "Processing fill from BitSet 1: " << remainingFillLength1 << " blocks left..." << endl;
				actWord = word1;
				actFillLength = &fillLength1;
				actRemainingFillLength = &remainingFillLength1;
				actWordOffset = &wordOffset1;
				actW = &w1;
				actBitSet = bs1;
				actPlainWord = plainWord1;
				processedActPlainWord = &processedPlainWord1;

				otherWord = word2;
				otherFillLength = &fillLength2;
				otherRemainingFillLength = &remainingFillLength2;
				otherWordOffset = &wordOffset2;
				otherW = &w2;
				otherBitSet = bs2;
				otherPlainWord = plainWord2;
				processedOtherPlainWord = &processedPlainWord2;
			} else {
				// number of blocks left in the current fill of BitSets 1 is bigger than that of BitSets 2
				if (debug) cout << "Processing fill from BitSet 2: " << remainingFillLength2 << " blocks left..." << endl;
				actWord = word2;
				actPlainWord =
				actFillLength = &fillLength2;
				actRemainingFillLength = &remainingFillLength2;
				actWordOffset = &wordOffset2;
				actW = &w2;
				actBitSet = bs2;
				actPlainWord = plainWord2;
				processedActPlainWord = &processedPlainWord2;


				otherWord = word1;
				otherFillLength = &fillLength1;
				otherRemainingFillLength = &remainingFillLength1;
				otherWordOffset = &wordOffset1;
				otherW = &w1;
				otherBitSet = bs1;
				otherPlainWord = plainWord1;
				processedOtherPlainWord = &processedPlainWord1;
			}

			bool actWordIsOneFill = IS_ONEFILL(*actWord);

			if (actWordIsOneFill){
				// Process 1-fill from one of the words (ignoring the other word for the time being)
				result->addOneFill(*actRemainingFillLength);
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
					if (IS_ONEFILL(*otherWord)) result->addOneFill(*otherRemainingFillLength);
					else result->addZeroFill(*otherRemainingFillLength);
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
								if (IS_ONEFILL(*otherWord)) result->addOneFill(*otherRemainingFillLength);
								else result->addZeroFill(*otherRemainingFillLength);
							}

							*otherWordOffset = 0;
							(*otherW)++;
						} else {
							// Destination position is somewhere in the middle of the fill of the other word

							if (!actWordIsOneFill){
								// Active BitSet is at a 0-fill, remaining blocks from the word from
								// the other BitSet should be copied
								if (IS_ONEFILL(*otherWord)) result->addOneFill(blocksToGo);
								else result->addZeroFill(blocksToGo);
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
							result->addLiteral(*otherWord);
						}
						(*otherW)++;
					}
				} // end while: skipping through other word
				if (debug) cout << "Done skipping!" << endl;

				if (blocksProcessed == *actRemainingFillLength || (processedPlainWord1 && processedPlainWord2)){
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
					if (!actWordIsOneFill) result->addZeroFill(*actRemainingFillLength - blocksProcessed);
					(*actW)++;
					*actWordOffset = 0;
				}
			} // end if


		}
	} // end while

	// If the last word of the result is a literal, store that one uncompressed
	if (result->_compressedBits.size() > 0 && !IS_FILL(result->_compressedBits.back())){
		result->_plainWord = result->_compressedBits.back();
		result->_compressedBits.pop_back();
		result->_plainWordBlockSeq = max(bs1->_plainWordBlockSeq, bs2->_plainWordBlockSeq);
	} else {
		result->_plainWordBlockSeq = max(bs1->_plainWordBlockSeq, bs2->_plainWordBlockSeq) + 1;
	}
	result->_lastBitIndex = max(bs1->_lastBitIndex, bs2->_lastBitIndex);
	return result;
}

/**
 * \brief Merges multiple WAHBitSet objects into a single WAHBitSet by computing the logical OR
 *
 * \param bitSets the source WAHBitSet objects
 * \param numBitSets the number of source WAHBitSet objects
 * \param result pointer to a WAHBitSet to store the result of the logical OR
 */
void WAHBitSet::multiOr(WAHBitSet** bitSets, unsigned int numBitSets, WAHBitSet* result){
	const bool debug = false;
	if (debug) cout << "Performing multi-way OR on " << numBitSets << " BitSets" << endl;
	if (numBitSets == 0) return;

	// TODO: in case of just 1 BitSet: make a copy

	int rBlockIndex = 0;
	int* sWordIndex = new int[numBitSets];
	int* sWordOffset = new int[numBitSets];
	int* sBlockIndex = new int[numBitSets];
	int largestOneFill, largestOneFillSize;
	int shortestZeroFill, shortestZeroFillSize;
	int currMergedLiteral, currWord, currFillLengthRemaining;


	while(true){
		currMergedLiteral = 0;
		shortestZeroFillSize = -1;
		largestOneFillSize = -1;

		// Align BitSets, find largest 1-fill and merge literals in the process
		for (unsigned int i = 0; i < numBitSets; i++){
			if (debug) cout << "Considering BitSet with index " << i << " (total number of bitsets = " << numBitSets << ")" << endl;
			if (debug) cout << "BitSet " << i << " contains " << bitSets[i]->_compressedBits.size() << " compressed words" << endl;

			// Check whether the BitSet contains any more interesting bits, to prevent
			// running out of bounds.
			if (sWordIndex[i] > bitSets[i]->_compressedBits.size()){
				// Totally out of bounds. Note that the '>' in stead of '>=' is intentional!
				// Skip this BitSet
				if (debug) cout << "BitSet " << i << ": totally out of bounds (" << sWordIndex[i] << " > " << bitSets[i]->_compressedBits.size() << "), skipping..." << endl;
				continue;
			} else if (sWordIndex[i] < bitSets[i]->_compressedBits.size()){
				// Still within bounds, but some aligning might be needed
				if (debug) cout << "Aligning BitSet " << i << endl;

				while (sBlockIndex[i] < rBlockIndex){
					if (debug) cout << "BitSet " << i << ": at word " << sWordIndex[i] << endl;

					// Perform some aligning: this BitSet is not yet at block index rBlockIndex
					if (sWordIndex[i] >= bitSets[i]->_compressedBits.size()){
						// Out of bounds, no more skipping
						if (debug) cout << "BitSet " << i << ": running out of bounds, breaking alignment..." << endl;
						break;
					}

					currWord = bitSets[i]->_compressedBits[sWordIndex[i]];

					if (IS_FILL(currWord)){
						// Fill word. Check length and skip accordingly
						currFillLengthRemaining = FILL_LENGTH(currWord) - sWordOffset[i];
						if (currFillLengthRemaining <= rBlockIndex - sBlockIndex[i]){
							// Skip entire length
							if (debug) cout << "BitSet " << i << ": skipping one entire fill word of length " << currFillLengthRemaining << endl;
							sWordIndex[i]++;
							sWordOffset[i] = 0;
							sBlockIndex[i] += currFillLengthRemaining;
						} else {
							// Can't skip the entire length...
							if (debug) cout << "BitSet " << i << ": skipping part of fill word: only " << (rBlockIndex - sBlockIndex[i]) << " of " << currFillLengthRemaining << " blocks" << endl;
							sWordOffset[i] += rBlockIndex - sBlockIndex[i];
							sBlockIndex[i] += rBlockIndex - sBlockIndex[i];
						}
					} else {
						// Literal, skip one block
						if (debug) cout << "BitSet " << i << ": skipping one literal word (one block)" << endl;
						sBlockIndex[i]++;
						sWordIndex[i]++;
					}
				} // end while: skipping done

				if (debug) cout << "Done aligning BitSet " << i << endl;
			}

			if (sWordIndex[i] < bitSets[i]->_compressedBits.size()){
				// Within bounds
				if (debug) cout << "Current compressed word of bitset " << i << ": " << toBitString(bitSets[i]->_compressedBits[sWordIndex[i]]) << endl;
				currWord = bitSets[i]->_compressedBits[sWordIndex[i]];
			} else if (sWordIndex[i] == bitSets[i]->_compressedBits.size()){
				// One out of bound, take plain word
				if (debug) cout << "Considering plain word of bitset " << i << ": " << toBitString(bitSets[i]->_plainWord) << endl;
				currWord = bitSets[i]->_plainWord;
			} else {
				// Totally out of bounds, skip this BitSet
				if (debug) cout << "BitSet " << i << ": totally out of bounds after skipping" << endl;
				continue;
			}

			if (IS_ONEFILL(currWord)){
				currFillLengthRemaining = FILL_LENGTH(currWord) - sWordOffset[i];
				if (currFillLengthRemaining > largestOneFillSize){
					largestOneFill = i;
					largestOneFillSize = currFillLengthRemaining;
				}
				sBlockIndex[i] += currFillLengthRemaining;
				sWordIndex[i]++;
				sWordOffset[i] = 0;
			} else if (IS_ZEROFILL(currWord)){
				if (largestOneFill == -1){
					currFillLengthRemaining = FILL_LENGTH(currWord) - sWordOffset[i];
					if (currFillLengthRemaining < shortestZeroFillSize || shortestZeroFillSize == -1){
						shortestZeroFill = i;
						shortestZeroFillSize = currFillLengthRemaining;
					}
				}
				sBlockIndex[i] += FILL_LENGTH(currWord) - sWordOffset[i];
				sWordIndex[i]++;
				sWordOffset[i] = 0;
			} else {
				// Literal
				if (largestOneFillSize != -1){
					cout << "BitSet " << i << ": merging literal word in resulting literal" << endl;
					currMergedLiteral |= currWord;
				}
				sBlockIndex[i]++;
				sWordIndex[i]++;
			}
		} // end for
		if (debug) cout << "Done considering " << numBitSets << " source BitSets" << endl;

		// Perform optimal action
		if (largestOneFillSize != -1){
			// Add 1-fill
			if (debug) cout << "Adding 1-fill of size " << (FILL_LENGTH(currWord) - sWordOffset[largestOneFill]) << " to result." <<endl;
			currWord = bitSets[largestOneFill]->_compressedBits[sWordIndex[largestOneFill]];
			currFillLengthRemaining = FILL_LENGTH(currWord) - sWordOffset[largestOneFill];
			result->addOneFill(currFillLengthRemaining);
		} else if (currMergedLiteral != 0){
			// Add literal
			if (debug) cout << "Adding literal word to result" << endl;
			result->addLiteral(currMergedLiteral);
		} else if (shortestZeroFillSize != -1){
			// Add zero fill
			if (debug) cout << "Adding 0-fill of size " << (FILL_LENGTH(currWord) - sWordOffset[shortestZeroFill]) << " to result." <<endl;
			currWord = bitSets[shortestZeroFill]->_compressedBits[sWordIndex[shortestZeroFill]];
			currFillLengthRemaining = FILL_LENGTH(currWord) - sWordOffset[shortestZeroFill];
			result->addZeroFill(currFillLengthRemaining);
		} else {
			// Nothing more to do!
			break;
		}
	} // end while

	if (debug) cout << "done multiway!" << endl;
}

bool WAHBitSet::isEmpty(){
	return _empty;
}

unsigned int WAHBitSet::size(){
	return _lastBitIndex + 1;
}

/**
 * Reports the number of bits used to represent this BitSet. This includes the bits
 * used to store the last plain word, but not the bits in bookkeeping variables
 */
long WAHBitSet::memoryUsage(){
	long res = _compressedBits.size() * 32;
	res += 1; // plain word
	return res;
}
