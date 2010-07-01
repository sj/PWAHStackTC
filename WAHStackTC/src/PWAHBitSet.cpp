/*
 * PWAHBitSet.cpp
 *
 *  Created on: May 25, 2010
 *      Author: bas
 */

#include "PWAHBitSet.h"
#include <iostream>
#include <cstring>
#include "LongBitMacros.cpp"
#include "PWAHBitSetIterator.h"
#include <assert.h>
#include <math.h>
#include <climits>
#include <sstream>
using namespace std;

/**
 * Define the offset of partitions within the 64 bit word.
 */
template<> const int PWAHBitSet<1>::_partitionOffsets[1] = {0};
template<> const int PWAHBitSet<2>::_partitionOffsets[2] = {0, 31};
template<> const int PWAHBitSet<4>::_partitionOffsets[4] = {0, 15, 30, 45};
template<> const int PWAHBitSet<8>::_partitionOffsets[8] = {0, 7, 14, 21, 28, 35, 42, 49};

/**
 * Define the block size for every possible number of partitions
 */
template<> const int PWAHBitSet<1>::_blockSize = 63;
template<> const int PWAHBitSet<2>::_blockSize = 31;
template<> const int PWAHBitSet<4>::_blockSize = 15;
template<> const int PWAHBitSet<8>::_blockSize = 7;

/**
 * Define the maximum number of blocks a fill can contain for every possible number of partitions
 */
template<> const long PWAHBitSet<1>::_maxBlocksPerFill = (1L << 62) - 1; // 2^63 - 1
template<> const long PWAHBitSet<2>::_maxBlocksPerFill = pow(2,31) - 1; // 2^31 - 1 = 2,147,483,647 blocks = 66,571,993,057 bits
template<> const long PWAHBitSet<4>::_maxBlocksPerFill = pow(2,15) - 1; // 2^15 - 1 = 32767 blocks = 491,505 bits
template<> const long PWAHBitSet<8>::_maxBlocksPerFill = pow(2,7) - 1;  // 2^7 - 1 = 127 blocks = 889 bits


/**
 * Constructor for PWAHBitSet
 */
template<unsigned int P> PWAHBitSet<P>::PWAHBitSet():
			// Initialise variables
			_plainBlockIndex(0), _plainBlock(0), _lastBitIndex(-1), _lastUsedPartition(-1), _compressedWords(vector<long>()){

	// Assert to check whether the long primitive type consists of 64 bits. Might the data type
	// consist of any other number of bits, weird things will happen...
	assert(sizeof(long) == (64 / CHAR_BIT));
}

template<unsigned int P> void PWAHBitSet<P>::clear(){
	_plainBlockIndex = 0;
	_lastUsedPartition = -1;
	_plainBlock = 0;
	_lastBitIndex = -1;
	_compressedWords.clear();
}

template<unsigned int P> const bool PWAHBitSet<P>::get(int bitIndex){
	const bool DEBUGGING = false;
	const int blockIndex = bitIndex / _blockSize;
	if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit in block " << blockIndex << endl;

	if (blockIndex > _plainBlockIndex){
		if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " out of bounds. Plain block has index " << _plainBlockIndex << endl;
		return false;
	} else if (blockIndex == _plainBlockIndex){
		if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " stored plain at index " << (bitIndex % _blockSize) << endl;
		return L_GET_BIT(_plainBlock, bitIndex % _blockSize);
	} else {
		// Requested bit is located somewhere in the compressed bits
		if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " already compressed, scanning..." << endl;

		// Iterate over all words
		long currWord;
		long currBlockIndex = -1;
		for (unsigned int w = 0; w < _compressedWords.size(); w++){
			currWord = _compressedWords[w];

			// Iterate over partitions in this word
			for (unsigned int p = 0; p < P; p++){
				if (is_onefill(currWord, p)){
					// Partition represents a 1-fill
					currBlockIndex += fill_length(currWord, p);
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- encountered 1-fill extending to block " << currBlockIndex << endl;
					if (currBlockIndex >= blockIndex){
						if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is set" << endl;
						return true;
					}
				} else if (is_zerofill(currWord, p)){
					// Partition represents a 0-fill
					currBlockIndex += fill_length(currWord, p);
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- encountered 0-fill extending to block " << currBlockIndex << endl;
					if (currBlockIndex >= blockIndex){
						if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is not set" << endl;
						return false;
					}
				} else {
					// Partition contains a literal word, it's safe to skip one block
					currBlockIndex++;
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- encountered literal at block " << currBlockIndex << endl;

					if (currBlockIndex == blockIndex){
						long partition = extract_partition(currWord, p);
						if (DEBUGGING){
							cout << "PWAHBitSet::get[" << bitIndex << "] -- bit resides within partition " << p << ": " << toBitString(partition) << " at position " << (bitIndex % _blockSize) << endl;
							cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is " << (L_GET_BIT(partition, bitIndex % _blockSize) ? "" : "not ") << "set" << endl;
						}
						return L_GET_BIT(partition, bitIndex % _blockSize);
					}
				}

				if (currBlockIndex > blockIndex){
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is not set" << endl;
					return false;
				}
			}

			if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- haven't found bit yet, considering next word..." << endl;
		}
	}

	throw string("unexpected state");
}

template<unsigned int P> const unsigned int PWAHBitSet<P>::size(){
	return _lastBitIndex + 1;
}



/**
 * \brief Sets the bit with index 'bitIndex' in this PWAHBitSet to '1'
 */
template<unsigned int P> void PWAHBitSet<P>::set(int bitIndex){
	set(bitIndex, true);
}

template<unsigned int P> void PWAHBitSet<P>::set(int bitIndex, bool value){
	const bool DEBUGGING = true;

	if (DEBUGGING) cout << "PWAHBitSet<P>::set -- " << (value ? "Setting" : "Unsetting") << " bit " << bitIndex << " (P=" << P << ", blockSize=" << _blockSize << ", plainBlockIndex=" << _plainBlockIndex << ")" << endl;

	const int blockIndex = bitIndex / _blockSize;
	if (DEBUGGING) cout << "PWAHBitSet<P>::set -- Bit belongs to block " << blockIndex << endl;

	if (blockIndex < _plainBlockIndex){
		// Bit index is part of compressed block, can't do.
		cerr << "Can't un(set) bit " << bitIndex << ". That bit belongs to block " << blockIndex << ", only bits belonging to blockindex " << _plainBlockIndex << " (and beyond) can be (un)set! Last bit set = " << _lastBitIndex << endl;
		throw string("requested bit is part of compressed block, can't (un)set");
	}

	if (blockIndex > _plainBlockIndex){
		// Compress plain block first... Note that compressing the plain block
		// will not change _plainBlockIndex
		compressPlainBlock();

		if (blockIndex > _plainBlockIndex + 1){
			// Add one or more 0-fills to the vector with compressed blocks
			const int numZeroFills = blockIndex - _plainBlockIndex - 1;
			addZeroFill(numZeroFills);
		}

		if (DEBUGGING) cout << "PWAHBitSet<P>::set -- Increasing index of plain block to " << blockIndex << endl;
		_plainBlockIndex = blockIndex;
	}


	// Store Bit in plain block. Go!
	if (value){
		if (DEBUGGING) cout << "PWAHBitSet::set -- setting bit " << (bitIndex % _blockSize) << " of plain block" << endl;
		L_SET_BIT(_plainBlock, bitIndex % _blockSize);
	} else {
		if (DEBUGGING) cout << "PWAHBitSet::set -- unsetting bit " << (bitIndex % _blockSize) << " of plain block" << endl;
		L_CLEAR_BIT(_plainBlock, bitIndex % _blockSize);
	}
	if (bitIndex > _lastBitIndex) _lastBitIndex = bitIndex;
}

/**
 * \brief Compressed the plain block _plainBlock to _compressedWords
 *
 * Note that this method does not increase the index _plainBlockIndex! The method will
 * reset the value of _plainBlock, though.
 */
template<unsigned int P> void PWAHBitSet<P>::compressPlainBlock(){
	const bool DEBUGGING = false;
	if (_lastBitIndex / _blockSize < _plainBlockIndex){
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- not compressing, plain block not used: lastBitIndex=" << _lastBitIndex << ", plainBlockIndex=" << _plainBlockIndex << endl;
		return;
	}

	if (_plainBlock == 0){
		// 0-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- adding 0-fill from plain block: " << toBitString(_plainBlock) << endl;
		addZeroFill(1);
	} else if (P == 1 && _plainBlock == 0b0111111111111111111111111111111111111111111111111111111111111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- adding 1-fill from plain block: " << toBitString(_plainBlock) << endl;
		addOneFill(1);
	} else if (P == 2 && _plainBlock == 0b0000000000000000000000000000000001111111111111111111111111111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- adding 1-fill from plain block: " << toBitString(_plainBlock) << endl;
		addOneFill(1);
	} else if (P == 4 && _plainBlock == 0b0000000000000000000000000000000000000000000000000111111111111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- adding 1-fill from plain block: " << toBitString(_plainBlock) << endl;
		addOneFill(1);
	} else if (P == 8 && _plainBlock == 0b0000000000000000000000000000000000000000000000000000000001111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- adding 1-fill from plain block: " << toBitString(_plainBlock) << endl;
		addOneFill(1);
	} else {
		// literal
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainBlock -- adding literal from plain block: " << toBitString(_plainBlock) << endl;
		addPartition(false, _plainBlock);
	}

	_plainBlock = 0;
}

//template<unsigned int P> void PWAHBitSet<P>::addOneFill(int numBlocks){

template<unsigned int P> void PWAHBitSet<P>::addOneFill(int numBlocks){
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet::addOneFill(" << numBlocks << ")" << endl;

	// Check last compressed word
	if (_compressedWords.size() > 0 && is_onefill(_compressedWords.back(), _lastUsedPartition)){
		// TODO: check for extended 1-fill! In PWAHBitSet<8> case, at total of 8 partitions
		// should be checked...
		numBlocks = fill_length(_compressedWords.back(), _lastUsedPartition) + numBlocks;
		if (DEBUGGING) cout << "PWAHBitSet::addOneFill -- Encountered 1-fill of size " << fill_length(_compressedWords.back(), _lastUsedPartition) << " blocks at partition " << _lastUsedPartition << ", extending to " << numBlocks << "..." << endl;

		// Decrease _lastUsedPartition, call to 'addPartition' below will increase _lastUsedPartition
		if (_lastUsedPartition == 0){
			_compressedWords.pop_back();
			_lastUsedPartition = P -1;
		} else {
			// Clear the last used partition, containing the 1-fill that is about to be extended
			_compressedWords[_compressedWords.size() - 1] = clear_partition(_compressedWords.back(), _lastUsedPartition);

			// Decrease counter, that (cleared) partition will be overwritten
			_lastUsedPartition--;
		}
	}

	if (numBlocks > _maxBlocksPerFill){
		// TODO
		throw string("not implemented: number of blocks in fill exceeds _maxBlocksPerFill");
	}

	P == 1 ? addPartition(true, (numBlocks | 0b0100000000000000000000000000000000000000000000000000000000000000)) :
	P == 2 ? addPartition(true, (numBlocks | 0b0000000000000000000000000000000001000000000000000000000000000000)) :
	P == 4 ? addPartition(true, (numBlocks | 0b0000000000000000000000000000000000000000000000000100000000000000)) :
			 addPartition(true, (numBlocks | 0b0000000000000000000000000000000000000000000000000000000001000000));
}

template<unsigned int P> void PWAHBitSet<P>::addZeroFill(int numBlocks){
	const bool DEBUGGING = false;
	if (_VERIFY) assert(numBlocks > 0);
	if (DEBUGGING) cout << "PWAHBitSet::addZeroFill(" << numBlocks << ")" << endl;

	// Check last compressed word
	if (_compressedWords.size() > 0 && is_zerofill(_compressedWords.back(), _lastUsedPartition)){
		// TODO: check for extended 0-fill! In PWAHBitSet<8> case, at total of 8 partitions
		// should be checked...
		numBlocks = fill_length(_compressedWords.back(), _lastUsedPartition) + numBlocks;
		if (DEBUGGING) cout << "PWAHBitSet::addZeroFill -- Encountered 0-fill of size " << fill_length(_compressedWords.back(), _lastUsedPartition) << " blocks at partition " << _lastUsedPartition << ", extending to size " << numBlocks << "..." << endl;

		// Decrease _lastUsedPartition, call to 'addPartition' below will increase _lastUsedPartition
		if (_lastUsedPartition == 0){
			_compressedWords.pop_back();
			_lastUsedPartition = P -1;
		} else {
			// Clear the last used partition, containing the 0-fill that is about to be extended
			_compressedWords[_compressedWords.size() - 1] = clear_partition(_compressedWords.back(), _lastUsedPartition);

			// Decrease counter, that (cleared) partition will be overwritten
			_lastUsedPartition--;
		}

	}

	if (numBlocks > _maxBlocksPerFill){
		// TODO
		throw string("not implemented: number of blocks in fill exceeds _maxBlocksPerFill");
	}

	// Simply add new fill
	addPartition(true, numBlocks);
}

template<unsigned int P> void PWAHBitSet<P>::addLiteral(long value){
	if (is_literal_onefill(value,0)){
		addOneFill(1);
	} else if (is_literal_zerofill(value, 0)) {
		addZeroFill(1);
	} else {
		addPartition(false, value);
	}
}

/**
 * \brief Takes the first _blockSize bits from the given value and adds these as a
 * new partition to the vector with compressed bits.
 *
 * Example for the PWAHBitSet<4> case:
 *  - each long word on vector<long> _compressedWords consists of 5 parts:
 *    - bits 63-60: 4 bits header, indicating whether each of the 4 partitions is a fill or literal (0 = literal, 1 = fill)
 *    - bits 59-45: 15 bits partition 3
 *    - bits 44-30: 15 bits partition 2
 *    - bits 29-15: 15 bits partition 1
 *    - bits 14-0:  15 bits partition 0
 *  - a partition can contain:
 *    - a literal block of 15 bits
 *    - a fill block. The first bit indicates 0/1 fill, the other 14 bits indicate the fill length
 *      (max. fill length = 2^14 = 16,384 blocks = 245,760 bits)
 *
 *
 *	The 'isFill' parameter tells this method to set the header bit of the partition to
 *	either 0 (literal, isFill = false) or 1 (fill, isFill = true)
 *
 *
 *  The 'value' parameter of this method is a long, formatted as follows (X and Y denote real values,
 *  0-bits indicate bits that will be ignored):
 *    - PWAHBitSet<1>: 63 bits: 0b0YXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *    - PWAHBitSet<2>: 31 bits: 0b000000000000000000000000000000000YXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *    - PWAHBitSet<4>: 15 bits: 0b0000000000000000000000000000000000000000000000000YXXXXXXXXXXXXXX
 *    - PWAHBitSet<8>: 7 bits:  0b000000000000000000000000000000000000000000000000000000000YXXXXXX
 *
 *  These n bits are the exact value of the partition. For a fill partition, the first bit (denoted using
 *  an 'Y' in the example above) should indicate the fill type (0-fill or 1-fill). For a literal
 *  partition, all n bits (both X and Y) may be used to indicate values of literal bits.
 *
 *	Note that all bits denoted with an underscore are ignored!
 */
template<unsigned int P> void PWAHBitSet<P>::addPartition(bool isFill, long value){
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet::addPartition(" << (isFill ? "fill" : "literal") << ", " << toBitString(value) << endl;

	if (_lastUsedPartition == P - 1 || _compressedWords.size() == 0){
		// Add new empty long to the vector
		_compressedWords.push_back(0L);
		_lastUsedPartition = 0;
	} else {
		_lastUsedPartition++;
	}

	if (DEBUGGING) cout << "PWAHBitSet::addPartition -- new partition will reside at partitionindex " << _lastUsedPartition << endl;

	if (_VERIFY){
		// Make sure the input doesn't extend too far
		long oldvalue = value;

		P == 1 ? value &= 0b0111111111111111111111111111111111111111111111111111111111111111 :
		P == 2 ? value &= 0b0000000000000000000000000000000001111111111111111111111111111111 :
		P == 4 ? value &= 0b0000000000000000000000000000000000000000000000000111111111111111 :
				 value &= 0b0000000000000000000000000000000000000000000000000000000001111111;

		if (value != oldvalue){
			throw string("long value passed to addPartition too big?!");
		}
	}

	// Partition 0 is stored using the least significant bits, partition P-1 is stored using the
	// most significant bits. Therefore, the input value should be shifted
	// _lastUsedPartition * _blockSize positions to the left to store the partition at the
	// correct offset. Note that the counter _lastUsedPartition was already updated in the first
	// lines of code of this method!
	// For the example case of PWAHBitSet<4>, _blockSize equals 15. Suppose this partition
	// will be the partition with index 2. In that case, the values in 'value' will be shifted
	// 2 * 15 = 30 positions to the left. Bit 0 will become bit 30 in the long word, the last
	// bit (index 14) will end up at index 44.
	value <<= (_lastUsedPartition * _blockSize);

	if (isFill){
		// Set the correct bit in the header of the word
		P == 1 ? value |= 0b1000000000000000000000000000000000000000000000000000000000000000 :
		P == 2 ? L_SET_BIT(value, 62 + _lastUsedPartition) :
		P == 4 ? L_SET_BIT(value, 60 + _lastUsedPartition) :
				 L_SET_BIT(value, 56 + _lastUsedPartition);
	}

	_compressedWords[_compressedWords.size() - 1] |= value;

	if (DEBUGGING) cout << "PWAHBitSet::addPartition -- resulting word after adding new partition at index " << _lastUsedPartition << ": " << toBitString(_compressedWords.back()) << endl;
}


/**
 * \brief Merges multiple WAHBitSet objects into a single WAHBitSet by computing the logical OR
 *
 * \param bitSets the source WAHBitSet objects
 * \param numBitSets the number of source WAHBitSet objects
 * \param result pointer to a WAHBitSet to store the result of the logical OR
 */
template<unsigned int P> void PWAHBitSet<P>::multiOr(PWAHBitSet<P>** bitSets, unsigned int numBitSets, PWAHBitSet<P>* result){
	const bool DEBUGGING = true;
	if (DEBUGGING) cout << endl << endl << "Performing multi-way OR on " << numBitSets << " BitSets" << endl;
	if (numBitSets == 0) return;

	// TODO: in case of just 1 BitSet: make a copy

	// Current position in result BitSet: block index
	unsigned int rBlockIndex = 0;

	// Current position in source BitSets:
	unsigned int* sBlockIndex = new unsigned int[numBitSets];		// block index
	unsigned int* sWordIndex = new unsigned int[numBitSets];		// word index
	unsigned int* sPartitionIndex = new unsigned int[numBitSets];	// partition within word
	unsigned int* sPartitionOffset = new unsigned int[numBitSets];	// offset within partition
	unsigned int lastBitIndex = 0;

	// Some bookkeeping
	int largestOneFill, shortestZeroFill;
	unsigned int largestOneFillSize, shortestZeroFillSize;
	int postponedOneFillSize = 0;
	int postponedZeroFillSize = 0;
	int currMergedLiteral;
	unsigned int currFillLengthRemaining;
	long currWord;

	// Initialize values in int arrays to 0.
	memset(sWordIndex, 0, numBitSets * sizeof(int));
	memset(sPartitionIndex, 0, numBitSets * sizeof(int));
	memset(sPartitionOffset, 0, numBitSets * sizeof(int));
	memset(sBlockIndex, 0, numBitSets * sizeof(int));

	while(true){
		if (DEBUGGING) cout << "Composing result block with index " << rBlockIndex << "..." << endl;
		if (DEBUGGING) cout << "Current state of affairs: " << endl << result->toString() << endl;

		currMergedLiteral = 0;
		largestOneFill = -1;
		shortestZeroFill = -1;
		shortestZeroFillSize = 0;
		largestOneFillSize = 0;

		// Align BitSets, find largest 1-fill and merge literals in the process
		for (unsigned int i = 0; i < numBitSets; i++){
			if (DEBUGGING) cout << "Considering BitSet with index " << i << " (total number of bitsets = " << numBitSets << ")" << endl;
			if (DEBUGGING) cout << "BitSet " << i << " contains " << bitSets[i]->_compressedWords.size() << " compressed words, is now at blockindex " << sBlockIndex[i] << " which is in partitionindex " << sPartitionIndex[i] << ", wordindex " << sWordIndex[i] << ". Plain block has index " << bitSets[i]->_plainBlockIndex << endl;
			if (bitSets[i]->_lastBitIndex > lastBitIndex) lastBitIndex = bitSets[i]->_lastBitIndex;

			// Some very simple checks
			if (sBlockIndex[i] > rBlockIndex){
				// This BitSet skipped past the current result blockindex as a result of a 0-fill.

				// If needed, store the length of this zero fill
				if (sBlockIndex[i] - rBlockIndex < shortestZeroFillSize || shortestZeroFill == -1){
					shortestZeroFill = i;
					shortestZeroFillSize = sBlockIndex[i] - rBlockIndex;
				}

				// Next BitSet...
				continue;
			} else if (rBlockIndex == bitSets[i]->_plainBlockIndex){
				// Use plain block of this bitset
				if (DEBUGGING) cout << "BitSet " << i << " slightly out of bounds: using plain block (index " << bitSets[i]->_plainBlockIndex << ")" << endl;
				currWord = bitSets[i]->_plainBlock;
				sBlockIndex[i] = bitSets[i]->_plainBlockIndex;
				sWordIndex[i] = bitSets[i]->_compressedWords.size();
				sPartitionIndex[i] = 0;
				sPartitionOffset[i] = 0;
			} else if (rBlockIndex > bitSets[i]->_plainBlockIndex){
				// Out of bounds!
				if (DEBUGGING) cout << "BitSet " << i << " totally out of bounds: plain block has index " << bitSets[i]->_plainBlockIndex << ", composing result block " << rBlockIndex << endl;
				continue;
			} else {
				// Use word from vector of compressed words
				currWord = bitSets[i]->_compressedWords[sWordIndex[i]];
			}

			while (rBlockIndex > sBlockIndex[i]){
				// Skip!
				if (DEBUGGING) cout << "Aligning BitSet " << i << ": BitSet is currently at blockindex " << sBlockIndex[i] << endl;

				if (is_fill(currWord, sPartitionIndex[i])){
					currFillLengthRemaining = fill_length(currWord, sPartitionIndex[i]) - sPartitionOffset[i];
					if (DEBUGGING) cout << "Encountered fill with " << currFillLengthRemaining << " blocks remaining, while skipping BitSet " << i << endl;

					// Lets see whether the complete remaining length can be skipped
					if (currFillLengthRemaining > rBlockIndex - sBlockIndex[i]){
						// Can't skip complete length
						sPartitionOffset[i] += rBlockIndex - sBlockIndex[i];
						sBlockIndex[i] = rBlockIndex;

						if (DEBUGGING) cout << "BitSet " << i << ": skipping part of the fill to blockindex " << sBlockIndex[i] << ", new partitionOffset=" << sPartitionOffset[i] << endl;
					} else {
						// Skip complete length.

						// TODO: a fill might span multiple partitions! Check whether multiple
						// partitions should be skipped!
						sBlockIndex[i] += currFillLengthRemaining;
						sPartitionOffset[i] = 0;
						sPartitionIndex[i]++;
						if (DEBUGGING) cout << "BitSet " << i << ": skipping entire fill to blockindex " << sBlockIndex[i] << endl;
					}
				} else {
					// Partition contains literal. Can be skipped without additional checks
					sBlockIndex[i]++;
					sPartitionIndex[i]++;
				}

				// Some additional bookkeeping regarding partitions in words
				if (sPartitionIndex[i] == P){
					// Jump to next word!
					sPartitionIndex[i] = 0;
					sWordIndex[i]++;
				}

				// Select next word from vector of compressed words
				currWord = bitSets[i]->_compressedWords[sWordIndex[i]];
			} // end while: done aligning this BitSet

			// Might this BitSet be out of bounds?
			if (sWordIndex[i] > bitSets[i]->_compressedWords.size()){
				// Totally out of bounds, ignore.
				if (DEBUGGING) cout << "BitSet " << i << " totally out of bounds, ignore..." << endl;
				continue;
			}
			if (DEBUGGING) cout << "BitSet " << i << " is correctly aligned" << endl;

			// Now, see what this BitSet is offering us...
			if (is_onefill(currWord, sPartitionIndex[i])){
				currFillLengthRemaining = fill_length(currWord, sPartitionIndex[i]) - sPartitionOffset[i];
				if (DEBUGGING) cout << "Encountered 1-fill in BitSet " << i << "; " << currFillLengthRemaining << " blocks remaining" << endl;

				if (currFillLengthRemaining > largestOneFillSize || largestOneFill == -1){
					largestOneFillSize = currFillLengthRemaining;
					largestOneFill = i;
				} // Else: larger one fill encountered

				// It is safe to skip the entire 1-fill from this BitSet,
				// since a skip of at least the size of this 1-fill will occur in this pass
				if (_VERIFY) assert(currFillLengthRemaining > 0);

				// TODO it might be necessary to skip multiple partitions if an extended fill
				// was encountered!
				sBlockIndex[i] += currFillLengthRemaining;
				sPartitionOffset[i] = 0;
			} else if (is_zerofill(currWord, sPartitionIndex[i])){
				currFillLengthRemaining = fill_length(currWord, sPartitionIndex[i]) - sPartitionOffset[i];
				if (DEBUGGING) cout << "Encountered 0-fill in BitSet " << i << "; " << currFillLengthRemaining << " blocks remaining" << endl;

				if (currFillLengthRemaining < shortestZeroFillSize || shortestZeroFill == -1){
					shortestZeroFillSize = currFillLengthRemaining;
					shortestZeroFill = i;
				}

				// It is safe to skip this 0-fill. 0-fills are boring anyway
				// TODO it might be necessary to skip multiple partitions if an extended fill
				// was encountered!
				sBlockIndex[i] += currFillLengthRemaining;
				sPartitionOffset[i] = 0;
			} else {
				// At literal
				if (DEBUGGING) cout << "Encountered literal in BitSet " << i << ": " << toBitString(extract_partition(currWord, sPartitionIndex[i])) << endl;
				currMergedLiteral |= extract_partition(currWord, sPartitionIndex[i]);

				// Safe to skip one block
				sBlockIndex[i]++;
				sPartitionOffset[i] = 0;
			}

			sPartitionIndex[i]++;
			if (sPartitionIndex[i] == P){
				sPartitionIndex[i] = 0;
				sWordIndex[i]++;
			}
		} // end for: done aligning and investigating all bitsets

		// Now decide on the most efficient operation to perform on the result in this pass
		if (largestOneFill != -1){
			// Do something w.r.t. 1-fill
			if (DEBUGGING) cout << "Adding 1-fill of length " << largestOneFillSize << " to result (postponed) "<< endl;

			// First check whether there is a postponed 0-fill floating around
			if (postponedZeroFillSize != 0){
				if (DEBUGGING) cout << "Before adding 1-fill: processing postponed 0-fill of length " << postponedZeroFillSize << endl;
				result->addZeroFill(postponedZeroFillSize);
				postponedZeroFillSize = 0;
			}

			// Postpone addition of 1-fill. It's more efficient to add large 1-fills
			// at once, in stead of in parts.
			postponedOneFillSize += largestOneFillSize;
			rBlockIndex += largestOneFillSize;

			if (DEBUGGING) cout << "Total length of postponed 1-fill: " << postponedOneFillSize <<  " block(s)" << endl;
		} else if (currMergedLiteral != 0){
			// No 1-fill available, store merged literal
			if (DEBUGGING) cout << "Adding literal to result: " << toBitString(currMergedLiteral) << endl;

			// First, check whether there is a postponed 0-fill or 1-fill floating around
			if (postponedOneFillSize != 0){
				if (DEBUGGING) cout << "Before adding literal: processing postponed 1-fill of length " << postponedOneFillSize << endl;
				result->addOneFill(postponedOneFillSize);
				postponedOneFillSize = 0;
			} else if (postponedZeroFillSize != 0){
				if (DEBUGGING) cout << "Before adding literal: processing postponed 0-fill of length " << postponedZeroFillSize << endl;
				result->addZeroFill(postponedZeroFillSize);
				postponedZeroFillSize = 0;
			}

			// Add literal to the result
			result->addLiteral(currMergedLiteral);
			rBlockIndex++;
		} else if (shortestZeroFill != -1) {
			// No 1-fill or literals seen, but a 0-fill is available
			if (DEBUGGING) cout << "Adding 0-fill of length " << largestOneFill << " to result (postponed) "<< endl;

			// First, check whether there is a postponed 1-fill floating around
			if (postponedOneFillSize != 0){
				if (DEBUGGING) cout << "Before adding 0-fill: processing postponed 1-fill of length " << postponedOneFillSize << endl;
				result->addOneFill(postponedOneFillSize);
				postponedOneFillSize = 0;
			}

			postponedZeroFillSize += shortestZeroFillSize;
			rBlockIndex += shortestZeroFillSize;

			if (DEBUGGING) cout << "Total length of postponed 0-fill: " << postponedZeroFillSize << endl;
		} else {
			// Nothing more to do!
			if (DEBUGGING) cout << "Done processing source BitSets!" << endl;
			// Before stopping, check whether there is a postponed 0-fill or 1-fill floating around
			if (postponedOneFillSize != 0){
				if (DEBUGGING) cout << "Processing postponed 1-fill of length " << postponedOneFillSize << endl;
				result->addOneFill(postponedOneFillSize);
				postponedOneFillSize = 0;
			} else if (postponedZeroFillSize != 0){
				if (DEBUGGING) cout << "Processing postponed 0-fill of length " << postponedZeroFillSize << endl;
				result->addZeroFill(postponedZeroFillSize);
				postponedZeroFillSize = 0;
			}

			break;
		}
	} // end while

	if (DEBUGGING) cout << "Cleaning up..." << endl;
	delete[] sWordIndex;
	delete[] sPartitionOffset;
	delete[] sPartitionIndex;
	delete[] sBlockIndex;

	// Decompress last block of resulting BitSet to plain block
	if (result->_compressedWords.size() > 0){
		result->decompressLastBlock();
	}

	// Can't do 'rBlockIndex - 1', since that would result in weird numbers when
	// rBlockIndex == 1 because rBlockIndex is unsigned.
	result->_plainBlockIndex = rBlockIndex;
	result->_plainBlockIndex--;

	result->_lastBitIndex = lastBitIndex;

	if (_VERIFY){
		// Recount number of blocks in this BitSet. Value of _plainBlockIndex should be
		// _plainBlockIndex = blockcount - 1
		cout.flush();

		int count = result->countNumberOfBlocks();
		if (result->_plainBlockIndex != count - 1){
			cerr << "The resulting PWAHBitSet contains " << count << " blocks, but _plainBlockIndex=" << result->_plainBlockIndex << "?!" << endl;
			cerr << result->toString() << endl;
			throw string("Unexpected state");
		}

		// _lastBitIndex should be within the plain block
		if (result->_lastBitIndex / result->_blockSize != result->_plainBlockIndex){
			cerr << "Last bit in resulting PWAHBitSet has index " << result->_lastBitIndex << " and therefore belongs to block " << (result->_lastBitIndex / result->_blockSize) << ", but _plainBlockIndex=" << result->_plainBlockIndex << "?!" << endl;
			cerr << result->toString() << endl;
			throw string("Unexpected state");
		}
	}

	if (DEBUGGING) cout << "done multiway!" << endl;
}

/**
 * \brief Performs a count of the number of blocks in this PWAHBitSet
 *
 * Only intended for verification purposes!
 */
template<unsigned int P> int PWAHBitSet<P>::countNumberOfBlocks(){
	cout << "Warning: call to countNumberOfBlocks!" << endl;
	unsigned int count = 0;
	unsigned int maxP;
	long currWord;
	for(unsigned int i = 0; i < _compressedWords.size(); i++){
		currWord = _compressedWords[i];

		if (i == _compressedWords.size() - 1){
			maxP = _lastUsedPartition;
		} else {
			maxP = P-1;
		}

		for (unsigned int p = 0; p <= maxP; p++){
			if (is_fill(currWord, p)){
				count += fill_length(currWord, p);
			} else {
				// literal
				count++;
			}
		}
	}

	// Count extra block for plain block
	count++;

	return count;
}

/**
 * \brief Removes the last partition. This will effectively remove one or more blocks
 *
 * This function will update _compressedWords and _lastUsedPartition
 */
template<unsigned int P> void PWAHBitSet<P>::popLastPartition(){
	const bool DEBUGGING = false;
	if (_VERIFY) assert(_compressedWords.size() > 0);

	if (_lastUsedPartition == 0){
		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- removing last word" << endl;
		_compressedWords.pop_back();
		_lastUsedPartition = P - 1;
	} else {
		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- clearing partition " << _lastUsedPartition << endl;
		_compressedWords[_compressedWords.size() - 1] = clear_partition(_compressedWords.back(), _lastUsedPartition);

		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- decreasing _lastUsedPartition" << endl;
		_lastUsedPartition--;
	}
}

template<unsigned int P> void PWAHBitSet<P>::decompressLastBlock(){
	const bool DEBUGGING = false;
	if (_VERIFY) assert(_plainBlock == 0);
	if (_compressedWords.size() == 0) return;

	if (is_fill(_compressedWords.back(), _lastUsedPartition)){
		const long word = _compressedWords.back();
		const long length = fill_length(word, _lastUsedPartition);
		const short part = _lastUsedPartition;

		// Remove last partition from vector with compressed bits
		if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- popping last partition" << endl;
		popLastPartition();

		if (is_onefill(word, part)){
			if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- last block is incorporated in 1-fill of length " << length << " (partition " << part << ": " << toBitString(word) << endl;
			P == 1 ? _plainBlock = 0b0111111111111111111111111111111111111111111111111111111111111111 :
			P == 2 ? _plainBlock = 0b0000000000000000000000000000000001111111111111111111111111111111 :
			P == 4 ? _plainBlock = 0b0000000000000000000000000000000000000000000000000111111111111111 :
					 _plainBlock = 0b0000000000000000000000000000000000000000000000000000000001111111;

			if (length > 1){
				// Re-add 1-fill with size (length - 1)
				if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- re-adding 1-fill of length " << (length - 1) << endl;
				addOneFill(length - 1);
			} // else: no need to add anything
		} else {
			if (_VERIFY) assert(is_zerofill(word, part));
			if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- last block is incorporated in 0-fill of length " << length << " (partition " << part << ": " << toBitString(word) << endl;
			_plainBlock = 0;

			if (length > 1){
				// Re-add 0-fill with size (length - 1)
				if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- re-adding 0-fill of length " << (length - 1) << endl;
				addZeroFill(length - 1);
			} // else: no need to add anything
		}
	} else {
		// Literal word: creating a copy suffices.
		_plainBlock = extract_partition(_compressedWords.back(), _lastUsedPartition);

		popLastPartition();
	}
}

template<unsigned int P> PWAHBitSet<P>* PWAHBitSet<P>::constructByOr(const PWAHBitSet<P>* first, const PWAHBitSet<P>* second){
	throw string("PWAHBitSet::constructByOr not implemented");
}

template<unsigned int P> const long PWAHBitSet<P>::memoryUsage(){
	throw string("PWAHBitSet::memoryUsage not implemented");
}

template<unsigned int P> BitSetIterator* PWAHBitSet<P>::iterator(){
	return new PWAHBitSetIterator<P>(this);
}

template<unsigned int P> const int PWAHBitSet<P>::blocksize(){
	return _blockSize;
}

// ================================== BEGIN STATIC OPERATIONS ON BITS ===================================

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a fill
 */
template<> inline bool PWAHBitSet<1>::is_fill(long bits, unsigned short partitionIndex){
	const long mask = 0b1000000000000000000000000000000000000000000000000000000000000000; // index 63
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<2>::is_fill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b0100000000000000000000000000000000000000000000000000000000000000 : // index 62
		partitionIndex == 1 ? 0b1000000000000000000000000000000000000000000000000000000000000000 : // index 63
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<4>::is_fill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b0001000000000000000000000000000000000000000000000000000000000000 : // index 63
		partitionIndex == 1 ? 0b0010000000000000000000000000000000000000000000000000000000000000 : // index 62
		partitionIndex == 2 ? 0b0100000000000000000000000000000000000000000000000000000000000000 : // index 61
		partitionIndex == 3 ? 0b1000000000000000000000000000000000000000000000000000000000000000 : // index 60
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<8>::is_fill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b0000000100000000000000000000000000000000000000000000000000000000 : // index 63
		partitionIndex == 1 ? 0b0000001000000000000000000000000000000000000000000000000000000000 : // index 62
		partitionIndex == 2 ? 0b0000010000000000000000000000000000000000000000000000000000000000 : // index 61
		partitionIndex == 3 ? 0b0000100000000000000000000000000000000000000000000000000000000000 : // index 60
		partitionIndex == 4 ? 0b0001000000000000000000000000000000000000000000000000000000000000 : // index 59
		partitionIndex == 5 ? 0b0010000000000000000000000000000000000000000000000000000000000000 : // index 58
		partitionIndex == 6 ? 0b0100000000000000000000000000000000000000000000000000000000000000 : // index 57
		partitionIndex == 7 ? 0b1000000000000000000000000000000000000000000000000000000000000000 : // index 56
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 1-fill
 */
template<> bool PWAHBitSet<1>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = 0b1100000000000000000000000000000000000000000000000000000000000000; // index 63, 62
	return (bits & mask) == mask;
}
template<> bool PWAHBitSet<2>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b0100000000000000000000000000000001000000000000000000000000000000 : // index 63, 61
		partitionIndex == 1 ? 0b1010000000000000000000000000000000000000000000000000000000000000 : // index 62, 30
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> bool PWAHBitSet<4>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b0001000000000000000000000000000000000000000000000100000000000000 : // index 63, 59
		partitionIndex == 1 ? 0b0010000000000000000000000000000000100000000000000000000000000000 : // index 62, 44
		partitionIndex == 2 ? 0b0100000000000000000100000000000000000000000000000000000000000000 : // index 61, 29
		partitionIndex == 3 ? 0b1000100000000000000000000000000000000000000000000000000000000000 : // index 60, 14
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> bool PWAHBitSet<8>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b0000000100000000000000000000000000000000000000000000000001000000 : // index 63, 55
		partitionIndex == 1 ? 0b0000001000000000000000000000000000000000000000000010000000000000 : // index 62, 48
		partitionIndex == 2 ? 0b0000010000000000000000000000000000000000000100000000000000000000 : // index 61, 41
		partitionIndex == 3 ? 0b0000100000000000000000000000000000001000000000000000000000000000 : // index 60, 34
		partitionIndex == 4 ? 0b0001000000000000000000000000010000000000000000000000000000000000 : // index 59, 27
		partitionIndex == 5 ? 0b0010000000000000000000100000000000000000000000000000000000000000 : // index 58, 20
		partitionIndex == 6 ? 0b0100000000000001000000000000000000000000000000000000000000000000 : // index 59, 13
		partitionIndex == 7 ? 0b1000000010000000000000000000000000000000000000000000000000000000 : // index 58, 6
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}

/**
 * \brief Returns a partition from a word.
 *
 * Will not return the type of the partition (fill / literal), only its contents. Note that
 * the contents will always reside in the least significant bits of the word
 *
 * PWAHBitSet<8>: 7 bits,  0b000000000000000000000000000000000000000000000000000000000XXXXXXX;
 * PWAHBitSet<4>: 15 bits, 0b0000000000000000000000000000000000000000000000000XXXXXXXXXXXXXXX;
 * PWAHBitSet<2>: 31 bits, 0b000000000000000000000000000000000XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;
 * PWAHBitSet<1>: 63 bits, 0b0XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;
 */
template<> long PWAHBitSet<1>::extract_partition(long word, unsigned short partitionIndex){
	return (word & 0b0111111111111111111111111111111111111111111111111111111111111111);
}
template<> long PWAHBitSet<2>::extract_partition(long word, unsigned short partitionIndex){
	word >>= (_blockSize * partitionIndex);
	return (word & 0b0000000000000000000000000000000001111111111111111111111111111111);
}
template<> long PWAHBitSet<4>::extract_partition(long word, unsigned short partitionIndex){
	word >>= (_blockSize * partitionIndex);
	return (word & 0b0000000000000000000000000000000000000000000000000111111111111111);
}
template<> long PWAHBitSet<8>::extract_partition(long word, unsigned short partitionIndex){
	word >>= (_blockSize * partitionIndex);
	return (word & 0b0000000000000000000000000000000000000000000000000000000001111111);
}

/**
 * \brief Clears a partition of a word and returns the result
 *
 * Will not clear the type of the partition (fill / literal) in the header. Ergo, only the
 * bits indicated with 'X' are cleared:
 *
 * PWAHBitSet<8>: 7 bits,  0b000000000000000000000000000000000000000000000000000000000XXXXXXX;
 * PWAHBitSet<4>: 15 bits, 0b0000000000000000000000000000000000000000000000000XXXXXXXXXXXXXXX;
 * PWAHBitSet<2>: 31 bits, 0b000000000000000000000000000000000XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;
 * PWAHBitSet<1>: 63 bits, 0b0XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;
 */
template<> inline long PWAHBitSet<1>::clear_partition(long word, unsigned short partitionIndex){
	return (word & 0b1000000000000000000000000000000000000000000000000000000000000000);
}
template<> inline long PWAHBitSet<2>::clear_partition(long word, unsigned short partitionIndex){
	if (partitionIndex == 0) return (word & 0b1111111111111111111111111111111110000000000000000000000000000000);
	return (word & 0b1100000000000000000000000000000001111111111111111111111111111111);
}
template<> inline long PWAHBitSet<4>::clear_partition(long word, unsigned short partitionIndex){
	if (partitionIndex == 0) return (word & 0b1111111111111111111111111111111111111111111111111000000000000000);
	if (partitionIndex == 1) return (word & 0b1111111111111111111111111111111111000000000000000111111111111111);
	if (partitionIndex == 1) return (word & 0b1111111111111111111000000000000000111111111111111111111111111111);
	return (word & 0b1111000000000000000111111111111111111111111111111111111111111111);
}
template<> inline long PWAHBitSet<8>::clear_partition(long word, unsigned short partitionIndex){
	if (partitionIndex == 0) return (word & 0b1111111111111111111111111111111111111111111111111111111110000000);
	if (partitionIndex == 1) return (word & 0b1111111111111111111111111111111111111111111111111100000001111111);
	if (partitionIndex == 2) return (word & 0b1111111111111111111111111111111111111111111000000011111111111111);
	if (partitionIndex == 3) return (word & 0b1111111111111111111111111111111111110000000111111111111111111111);
	if (partitionIndex == 4) return (word & 0b1111111111111111111111111111100000001111111111111111111111111111);
	if (partitionIndex == 5) return (word & 0b1111111111111111111111000000011111111111111111111111111111111111);
	if (partitionIndex == 6) return (word & 0b1111111111111110000000111111111111111111111111111111111111111111);
	return (word & 0b1111111100000001111111111111111111111111111111111111111111111111);
}


/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 0-fill
 * TODO: check can be performed using a single instruction...
 */
template<unsigned int P> bool PWAHBitSet<P>::is_zerofill(long bits, unsigned short partitionIndex){
	return is_fill(bits, partitionIndex) && !is_onefill(bits, partitionIndex);
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of literal
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_literal(long bits, unsigned short partitionIndex){
	return !is_fill(bits, partitionIndex);
}

/**
 * \brief Determines whether the specified partition within 'bits' contains a 1-fill disguised as a literal
 */
template<> inline bool PWAHBitSet<1>::is_literal_onefill(long bits, unsigned short partitionIndex){
	return (bits == 0b0111111111111111111111111111111111111111111111111111111111111111);
}
template<> inline bool PWAHBitSet<2>::is_literal_onefill(long bits, unsigned short partitionIndex){
	const long mask = partitionIndex == 0 ? 0b0100000000000000000000000000000001111111111111111111111111111111 :
											0b1011111111111111111111111111111110000000000000000000000000000000;
	const long expected = partitionIndex == 0 ? 0b0000000000000000000000000000000001111111111111111111111111111111 :
												0b0011111111111111111111111111111110000000000000000000000000000000;

	return (bits & mask) == expected;
}
template<> inline bool PWAHBitSet<4>::is_literal_onefill(long bits, unsigned short partitionIndex){
	const long mask = 	partitionIndex == 0 ? 	0b0001000000000000000000000000000000000000000000000111111111111111 :
						partitionIndex == 1 ?	0b0010000000000000000000000000000000111111111111111000000000000000 :
						partitionIndex == 2 ?	0b0100000000000000000111111111111111000000000000000000000000000000 :
												0b1000111111111111111000000000000000000000000000000000000000000000;
	const long expected =	partitionIndex == 0 ? 	0b0000000000000000000000000000000000000000000000000111111111111111 :
							partitionIndex == 1 ?	0b0000000000000000000000000000000000111111111111111000000000000000 :
							partitionIndex == 2 ?	0b0000000000000000000111111111111111000000000000000000000000000000 :
													0b0000111111111111111000000000000000000000000000000000000000000000;


	return (bits & mask) == expected;
}
template<> inline bool PWAHBitSet<8>::is_literal_onefill(long bits, unsigned short partitionIndex){
	const long mask =	partitionIndex == 0 ? 	0b0000000100000000000000000000000000000000000000000000000001111111 :
						partitionIndex == 1 ?	0b0000001000000000000000000000000000000000000000000011111110000000 :
						partitionIndex == 2 ?	0b0000010000000000000000000000000000000000000111111100000000000000 :
						partitionIndex == 3 ?	0b0000100000000000000000000000000000001111111000000000000000000000 :
						partitionIndex == 4 ?	0b0001000000000000000000000000011111110000000000000000000000000000 :
						partitionIndex == 5 ?	0b0010000000000000000000111111100000000000000000000000000000000000 :
						partitionIndex == 6 ?	0b0100000000000001111111000000000000000000000000000000000000000000 :
												0b1000000011111110000000000000000000000000000000000000000000000000;
	const long expected =	partitionIndex == 0 ? 	0b0000000000000000000000000000000000000000000000000000000001111111 :
							partitionIndex == 1 ?	0b0000000000000000000000000000000000000000000000000011111110000000 :
							partitionIndex == 2 ?	0b0000000000000000000000000000000000000000000111111100000000000000 :
							partitionIndex == 3 ?	0b0000000000000000000000000000000000001111111000000000000000000000 :
							partitionIndex == 4 ?	0b0000000000000000000000000000011111110000000000000000000000000000 :
							partitionIndex == 5 ?	0b0000000000000000000000111111100000000000000000000000000000000000 :
							partitionIndex == 6 ?	0b0000000000000001111111000000000000000000000000000000000000000000 :
													0b0000000011111110000000000000000000000000000000000000000000000000;

	return (bits & mask) == expected;
}


/**
 * \brief Determines whether the specified partition within 'bits' contains a 0-fill disguised as a literal
 */
template<> inline bool PWAHBitSet<1>::is_literal_zerofill(long bits, unsigned short partitionIndex){
	return (bits == 0b0000000000000000000000000000000000000000000000000000000000000000);
}
template<> inline bool PWAHBitSet<2>::is_literal_zerofill(long bits, unsigned short partitionIndex){
	const long mask = partitionIndex == 0 ? 0b0100000000000000000000000000000001111111111111111111111111111111 :
											0b1011111111111111111111111111111110000000000000000000000000000000;
	return (bits & mask) == 0;
}
template<> inline bool PWAHBitSet<4>::is_literal_zerofill(long bits, unsigned short partitionIndex){
	const long mask = 	partitionIndex == 0 ? 	0b0001000000000000000000000000000000000000000000000111111111111111 :
						partitionIndex == 1 ?	0b0010000000000000000000000000000000111111111111111000000000000000 :
						partitionIndex == 2 ?	0b0100000000000000000111111111111111000000000000000000000000000000 :
												0b1000111111111111111000000000000000000000000000000000000000000000;
	return (bits & mask) == 0;
}
template<> inline bool PWAHBitSet<8>::is_literal_zerofill(long bits, unsigned short partitionIndex){
	const long mask =	partitionIndex == 0 ? 	0b0000000100000000000000000000000000000000000000000000000001111111 :
						partitionIndex == 1 ?	0b0000001000000000000000000000000000000000000000000011111110000000 :
						partitionIndex == 2 ?	0b0000010000000000000000000000000000000000000111111100000000000000 :
						partitionIndex == 3 ?	0b0000100000000000000000000000000000001111111000000000000000000000 :
						partitionIndex == 4 ?	0b0001000000000000000000000000011111110000000000000000000000000000 :
						partitionIndex == 5 ?	0b0010000000000000000000111111100000000000000000000000000000000000 :
						partitionIndex == 6 ?	0b0100000000000001111111000000000000000000000000000000000000000000 :
												0b1000000011111110000000000000000000000000000000000000000000000000;
	return (bits & mask) == 0;
}

/**
 * \brief Determines the length of a fill (number of blocks).
 *
 * A fill might span multiple partitions, but will never span over two or more words.
 * For example take a look at these two words, compressed using PWAHBitSet<8>:
 *
 *   0b00111111|_______|_______|0000010|1000011|0000101|1010010|1011011|1001001
 *   0b10000000|1010001|_______|.......
 *
 * The first word consists of: 2 literal partitions, 1 0-fill partition, 1 1-fill partition,
 * 1 0-fill partition, 3 1-fill partitions
 *
 * The 3 consecutive 1-fill partitions at the end of the first word denote one large 1-fill of
 * length 0b010010011011001001 (= 75,465 blocks). Note that this is NOT equal to
 * 0b010010 + 0b1011011 + 0b1011011.
 *
 * Also note that the second word contains a 1-fill in the first partition. This 1-fill is stored
 * separate from the extended 1-fill in the first word. Therefore, the total number of adjacent
 * 1-bits which were compressed equals 0b010010011011001001 + 0b1010001 = 75546 blocks = 528,822 bits
 *
 * (note that this is an arbitrary example, the 528,822 1-bits would have been compressed using 20
 * bits (3 blocks) in the first partition)
 */
template<> long PWAHBitSet<1>::fill_length(long bits, unsigned short partitionIndex){
	// PWAHBitSet<1> case: one large partition. The most significant bit (index 63) of the 64-bit word
	// tells us whether the partition contains a fill or is a literal. Lets assume it's a fill.
	// The second most significant bit determines the type of the fill.
	// Clearing the two most significant bits will immediately show us the length of the fill.
	// The PWAHBitSet<1> case does not allow extended fills.
	return (bits & 0b0011111111111111111111111111111111111111111111111111111111111111);
}
template<> long PWAHBitSet<2>::fill_length(long bits, unsigned short partitionIndex){
	// PWAHBitSet<2> case: two partitions consisting of 31 bits. The two most significant bits
	// of the (indices 63 & 62) of the 64-bit word tell us whether the partitions contain
	// literal data or represent fills. The most significant bits of each partition (bit index
	// 61 for partition 1, bit index 30 for partition 0) denote the type of the fill.
	// In case of an extended fill (see extensive description above), both partitions need to
	// be glued together.
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- partition " << partitionIndex << " of " << toBitString(bits) << endl;

	if (partitionIndex == 1){
		if (_VERIFY){
			// Check whether the partition at index 0 contains an identical fill. In that case,
			// the partitions make a extended fill and an exception should be thrown. The fill
			// length of partition 1 is considered undefined.

			if (L_GET_BIT(bits, 63) && L_GET_BIT(bits, 62)){
				// Both partitions represent a fill

				if (
						(L_GET_BIT(bits, 61) != 0 && L_GET_BIT(bits, 30) != 0) ||
						(L_GET_BIT(bits, 61) == 0 && L_GET_BIT(bits, 30) == 0)){
					// Both partitions represent the _same type_ of fill
					throw string("Fill length of fill in partition is undefined");
				}
			}
		}

		// No need to check for extended fills, since this is the last partition of this word
		// and extended fills do not span multiple words.
		// Bit 61 denotes the type of fill, bits 31-60 denote the length of the fill. After clearing
		// the other bits, shifting the bits 31-60 to the right (31 positions) will yield the length
		// of the fill.
		if (DEBUGGING) cout << "PWAHBitSet::fill_length -- result: " << ((bits & 0b0001111111111111111111111111111110000000000000000000000000000000) >> 31) << endl;
		return ((bits & 0b0001111111111111111111111111111110000000000000000000000000000000) >> 31);
	} else if (partitionIndex == 0){
		// Check whether the fill spans both partitions 0 and 1
		const long extOneFill = 0b1110000000000000000000000000000001000000000000000000000000000000;
		const long extZeroFill = 0b1100000000000000000000000000000000000000000000000000000000000000;
		const long andRes = (bits & extOneFill);

		if (andRes == extZeroFill || andRes == extOneFill){
			// Extended fill! Take bits 60-31 and 29-0, concatenate => length of the fill emerges
			if (DEBUGGING) cout << "PWAHBitSet::fill_length -- encountered an extended 1-fill!" << endl;

			// Bits from partition 0 denoting length: bits 0-29
			long part0 = (bits & 0b0000000000000000000000000000000000111111111111111111111111111111);

			// Bits from partition 1 denoting length: bits 31-60
			long part1 = (bits & 0b0001111111111111111111111111111110000000000000000000000000000000);


			// Shift the bits from partition 1 one position to the right, and return the logical OR
			// with the bits from partition0.
			if (DEBUGGING) cout << "PWAHBitSet::fill_length -- result: " << ((part1 >> 1) | part0) << endl;
			return ((part1 >> 1) | part0);
		} else {
			// Regular fill, just check the length of the fill in partition 0. Partition 0 resides
			// in the least significant bits of the 64-bit word. The type of the fill is stored
			// in bit 30, bits 0-29 are used to indicate the length of the fill.
			// Simply clearing all other bits will yield the requested length.
			if (DEBUGGING) cout << "PWAHBitSet::fill_length -- result: " << (bits & 0b0000000000000000000000000000000000111111111111111111111111111111) << endl;
			return (bits & 0b0000000000000000000000000000000000111111111111111111111111111111);
		}
	}
	throw string("Invalid partition index");
}
template<> long PWAHBitSet<4>::fill_length(long bits, unsigned short partitionIndex){
	/**
	 * PWAHBitSet<4> case: 4 partitions of 15 bits and a header of 4 bits. An extended fill
	 * can not cover multiple words. Therefore, only partitions 0, 1 and 2 can be the
	 * first partition of an extended fill.
	 */
	const bool DEBUGGING = true;
	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- partition " << partitionIndex << " of " << toBitString(bits) << endl;

	if (partitionIndex != 3) {
		// Might be an extended fill. Check the fill type (0 or 1) of this partition and check
		// whether the next partition contains a fill of the same type.
		bool thisIsOneFill = is_onefill(bits, partitionIndex);
		if (is_fill(bits, partitionIndex + 1) && is_onefill(bits, partitionIndex + 1) == thisIsOneFill){
			// Extended fill
			// TODO
			throw string("Extended fill not supported yet");
		}
	}

	// Regular fill, otherwise the 'return' above would have been called
	long mask = 0b0000000000000000000000000000000000000000000000000011111111111111;
	mask <<= (partitionIndex * _blockSize);
	return (bits & mask);
}
template<> long PWAHBitSet<8>::fill_length(long bits, unsigned short partitionIndex){
	// TODO
	throw string("not implemented");
}

template<unsigned int P> string PWAHBitSet<P>::toBitString(long value){
	int signedP = P;

	stringstream res;
	res << "0b";
	for (int bit = 63; bit >= 0; bit--){
		if ((63 - bit - signedP) % _blockSize == 0) res << "|";
		if (L_GET_BIT(value, bit)) res << "1";
		else res << "0";
	}
	res << " (= " << value << ")";
	return res.str();
}

template<unsigned int P> const string PWAHBitSet<P>::toString(){
	stringstream res;

	res << "Last bit set: " << _lastBitIndex << endl;
	res << "Compressed words (last used partition=" << _lastUsedPartition << "):" << endl;
	if (_compressedWords.size() > 0){
		for (unsigned int i = 0; i < _compressedWords.size(); i++){
			res << toBitString(_compressedWords[i]) << endl;
		}
	} else {
		res << "(none)";
	}

	res << endl << "Plain block (blockindex=" << _plainBlockIndex << "):" << endl;
	if (_lastBitIndex < _plainBlockIndex * _blockSize){
		res << "(none)" << endl;
	} else {
		res << toBitString(_plainBlock) << endl;
	}

	return res.str();
}


/**
 * Tell the compiler which instantiations to generate
 */
template class PWAHBitSet<1>;
template class PWAHBitSet<2>;
template class PWAHBitSet<4>;
template class PWAHBitSet<8>;
