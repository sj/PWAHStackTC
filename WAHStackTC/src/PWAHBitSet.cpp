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
#include <assert.h>
#include <math.h>
#include <climits>
using namespace std;

/**
 * Define the offset of partitions within the 64 bit word.
 */
template<> const int PWAHBitSet<1>::_partitionOffsets[1] = {1};
template<> const int PWAHBitSet<2>::_partitionOffsets[2] = {2, 33};
template<> const int PWAHBitSet<4>::_partitionOffsets[4] = {4, 19, 34, 49};
template<> const int PWAHBitSet<8>::_partitionOffsets[8] = {8, 15, 22, 29, 36, 43, 50, 57};

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
			_plainBlockIndex(0), _plainBlock(0), _lastBitSet(-1), _compressedBlocks(vector<long>()){

	// Assert to check whether the long primitive type consists of 64 bits. Might the data type
	// consist of any other number of bits, weird things will happen...
	assert(sizeof(long) == (64 / CHAR_BIT));
}

template<unsigned int P> void PWAHBitSet<P>::clear(){
	_plainBlockIndex = 0;
	_plainBlock = 0;
	_compressedBlocks.clear();
	_lastBitSet = 0;
}

template<unsigned int P> bool PWAHBitSet<P>::get(int bitIndex){
	const int blockIndex = bitIndex / _blockSize;

	if (blockIndex > _plainBlockIndex){
		return false;
	} else if (blockIndex == _plainBlockIndex){
		return L_GET_BIT(_plainBlock, bitIndex % _blockSize);
	} else {
		// Requested bit is located somewhere in the compressed bits

		// Iterate over all words
		long currWord;
		long currBlockIndex = -1;
		for (unsigned int w = 0; w < _compressedBlocks.size(); w++){
			currWord = _compressedBlocks[w];

			// Iterate over partitions in this word
			for (unsigned int p = 0; p < P; p++){
				if (is_onefill(currWord, p)){
					// Partition represents a 1-fill
				} else if (is_zerofill(currWord, p)){
					// Partition represents a 0-fill

					// Safe to skip the entire block. When skipped too far, the if-statement
					// below will make the method return 'false'.
					currBlockIndex += fill_length(currWord, p);
				} else {
					// Partition contains a literal word, it's safe to skip one block
					currBlockIndex++;
				}

				if (currBlockIndex == blockIndex){
					// The current block contains the requested bit
				} else if (currBlockIndex > blockIndex){
					return false;
				}
			}

		}
	}
}

template<unsigned int P> unsigned int PWAHBitSet<P>::size(){
	return _lastBitSet - 1;
}



/**
 * \brief Sets the bit with index 'bitIndex' in this PWAHBitSet to '1'
 */
template<unsigned int P> void PWAHBitSet<P>::set(int bitIndex){
	set(bitIndex, true);
}

template<unsigned int P> void PWAHBitSet<P>::set(int bitIndex, bool value){
	cout << "Setting bit " << bitIndex << " (P=" << P << ", blockSize=" << _blockSize << ". Offset of first partition: " << _partitionOffsets[0] << ")" << endl;

	const int blockIndex = bitIndex / _blockSize;
	cout << "Bit belongs to block " << blockIndex << endl;

	if (blockIndex < _plainBlockIndex){
		// Bit index is part of compressed block, can't do.
		throw string("requested bit is part of compressed block, can't (un)set");
	}

	if (blockIndex > _plainBlockIndex){
		// Compress plain block first...
		compressPlainBlock();

		if (blockIndex > _plainBlockIndex + 1){
			// Add one or more 0-fills to the vector with compressed blocks
			const int numZeroFills = blockIndex - _plainBlockIndex - 1;
			addZeroFill(numZeroFills);
		}

		_plainBlockIndex = blockIndex;
	}


	// Store Bit in plain block. Go!
	L_SET_BIT(_plainBlock, bitIndex % _blockSize);
}

/**
 * \brief Compressed the plain block _plainBlock to _compressedBlocks
 *
 * Note that this method does not increase the index _plainBlockIndex! The method will
 * reset the value of _plainBlock, though.
 */
template<unsigned int P> void PWAHBitSet<P>::compressPlainBlock(){
	addPartition(false, _plainBlock);
	_plainBlock = 0;
}

template<unsigned int P> void PWAHBitSet<P>::addOneFill(int numBlocks){
	if (numBlocks > _maxBlocksPerFill){
		throw string("not implemented: number of blocks in fill exceeds _maxBlocksPerFill");
	}

	throw string("not implemented");
}

template<unsigned int P> void PWAHBitSet<P>::addZeroFill(int numBlocks){
	throw string("not implemented");
}

template<unsigned int P> void PWAHBitSet<P>::addLiteral(long value){
	throw string("not implemented");
}

/**
 * \brief Takes the first _blockSize bits from the given value and adds these as a
 * new partition to the vector with compressed bits.
 *
 * Example for the PWAHBitSet<4> case:
 *  - each long word on vector<long> _compressedBlocks consists of 5 parts:
 *    - 4 bits header, indicating whether each of the 4 partitions is a fill or literal (0 = literal, 1 = fill)
 *    - 15 bits partition 0
 *    - 15 bits partition 1
 *    - 15 bits partition 2
 *    - 15 bits partition 3
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
 *  underscore indicates bits that will be ignored):
 *    - PWAHBitSet<1>: 63 bits: 0b_YXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *    - PWAHBitSet<2>: 31 bits: 0b_________________________________YXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *    - PWAHBitSet<4>: 15 bits: 0b_________________________________________________YXXXXXXXXXXXXXX
 *    - PWAHBitSet<8>: 7 bits:  0b_________________________________________________________YXXXXXX
 *
 *  These n bits are the exact value of the partition. For a fill partition, the first bit (denoted using
 *  an 'Y' in the example above) should indicate the fill type (0-fill or 1-fill). For a literal
 *  partition, all n bits (both X and Y) may be used to indicate values of literal bits.
 *
 *	Note that all bits denoted with an underscore are ignored!
 */
template<unsigned int P> void PWAHBitSet<P>::addPartition(bool isFill, long value){
	if (_lastUsedPartition == P - 1 || _compressedBlocks.size() == 0){
		// Add new empty long to the vector
		_compressedBlocks.push_back(0L);
		_lastUsedPartition = 0;
	} else {
		_lastUsedPartition++;
	}

	// The long value should be shifted at least 1, 2, 4 or 8 positions to the right. These
	// first positions are considered to be the header of the long word on the vector<long>
	// and should most certainly not be touched by the new long value.
	// Furthermore, an additional _lastUsedPartition * _blockSize positions should be shifted
	// to store the partition at the correct offset.

	// For the example case of PWAHBitSet<4>, _blockSize equals 15 and the header consists
	// of 4 bits. Suppose this partition will be the partition with index 2. In that case,
	// the values in 'value' will be shifted 4 + 2 * 15 = 34 positions to the right. Bit 0 will
	// become bit 34 in the long word, the last bit (index 14) will end up at index 48.
	value >>= (P + _lastUsedPartition * _blockSize);

	// Now, it's safe to merge the value with the last long of the vector
	_compressedBlocks[_compressedBlocks.size() - 1] |= value;
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
		partitionIndex == 0 ? 0b1000000000000000000000000000000000000000000000000000000000000000 : // index 63
		partitionIndex == 1 ? 0b0100000000000000000000000000000000000000000000000000000000000000 : // index 62
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<4>::is_fill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b1000000000000000000000000000000000000000000000000000000000000000 : // index 63
		partitionIndex == 1 ? 0b0100000000000000000000000000000000000000000000000000000000000000 : // index 62
		partitionIndex == 2 ? 0b0010000000000000000000000000000000000000000000000000000000000000 : // index 61
		partitionIndex == 3 ? 0b0001000000000000000000000000000000000000000000000000000000000000 : // index 60
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<8>::is_fill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b1000000000000000000000000000000000000000000000000000000000000000 : // index 63
		partitionIndex == 1 ? 0b0100000000000000000000000000000000000000000000000000000000000000 : // index 62
		partitionIndex == 2 ? 0b0010000000000000000000000000000000000000000000000000000000000000 : // index 61
		partitionIndex == 3 ? 0b0001000000000000000000000000000000000000000000000000000000000000 : // index 60
		partitionIndex == 4 ? 0b0000100000000000000000000000000000000000000000000000000000000000 : // index 59
		partitionIndex == 5 ? 0b0000010000000000000000000000000000000000000000000000000000000000 : // index 58
		partitionIndex == 6 ? 0b0000001000000000000000000000000000000000000000000000000000000000 : // index 57
		partitionIndex == 7 ? 0b0000000100000000000000000000000000000000000000000000000000000000 : // index 56
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 1-fill
 */
template<> inline bool PWAHBitSet<1>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = 0b1100000000000000000000000000000000000000000000000000000000000000; // index 63, 62
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<2>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b1010000000000000000000000000000000000000000000000000000000000000 : // index 63, 61
		partitionIndex == 1 ? 0b0100000000000000000000000000000001000000000000000000000000000000 : // index 62, 30
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<4>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b1000100000000000000000000000000000000000000000000000000000000000 : // index 63, 59
		partitionIndex == 1 ? 0b0100000000000000000100000000000000000000000000000000000000000000 : // index 62, 44
		partitionIndex == 2 ? 0b0010000000000000000000000000000000100000000000000000000000000000 : // index 61, 29
		partitionIndex == 3 ? 0b0001000000000000000000000000000000000000000000000100000000000000 : // index 60, 14
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}
template<> inline bool PWAHBitSet<8>::is_onefill(long bits, unsigned short partitionIndex){
	const long mask = (
		partitionIndex == 0 ? 0b1000000010000000000000000000000000000000000000000000000000000000 : // index 63, 55
		partitionIndex == 1 ? 0b0100000000000001000000000000000000000000000000000000000000000000 : // index 62, 48
		partitionIndex == 2 ? 0b0010000000000000000000100000000000000000000000000000000000000000 : // index 61, 41
		partitionIndex == 3 ? 0b0001000000000000000000000000010000000000000000000000000000000000 : // index 60, 34
		partitionIndex == 4 ? 0b0000100000000000000000000000000000001000000000000000000000000000 : // index 59, 27
		partitionIndex == 5 ? 0b0000010000000000000000000000000000000000000100000000000000000000 : // index 58, 20
		partitionIndex == 6 ? 0b0000001000000000000000000000000000000000000000000010000000000000 : // index 59, 13
		partitionIndex == 7 ? 0b0000000100000000000000000000000000000000000000000000000001000000 : // index 58, 6
		throw string("invalid partitionIndex")
	);
	return (bits & mask) == mask;
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 0-fill
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_zerofill(long bits, unsigned short partitionIndex){
	return is_fill(bits, partitionIndex) && is_onefill(bits, partitionIndex);
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of literal
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_literal(long bits, unsigned short partitionIndex){
	return !is_fill(bits, partitionIndex);
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
template<> inline long PWAHBitSet<1>::fill_length(long bits, unsigned short partitionIndex){
	// PWAHBitSet<1> case: one large partition. The most significant bit (index 63) of the 64-bit word
	// tells us whether the partition contains a fill or is a literal. Lets assume it's a fill.
	// The second most significant bit determines the type of the fill.
	// Clearing the two most significant bits will immediately show us the length of the fill.
	// The PWAHBitSet<1> case does not allow extended fills.
	return (bits & 0b0011111111111111111111111111111111111111111111111111111111111111);
}
template<> inline long PWAHBitSet<2>::fill_length(long bits, unsigned short partitionIndex){
	// PWAHBitSet<2> case: two partitions consisting of 31 bits. The two most significant bits
	// of the (indices 63 & 62) of the 64-bit word tell us whether the partitions contain
	// literal data or represent fills. The most significant bits of each partition (bit index
	// 61 for partition 0, bit index 30 for partition 1) denote the type of the fill.
	// In case of an extended fill (see extensive description above), both partitions need to
	// be glued together.

	if (partitionIndex == 1){
		if (_VERIFY){
			// Check whether the partition at index 0 contains an identical fill. In that case,
			// the partitions make a extended fill and an exception should be thrown. The fill
			// length of partition 1 is considered undefined.

			if (L_GET_BIT(bits, 63) && L_GET_BIT(bits, 62)){
				if (
						(L_GET_BIT(bits, 61) != 0 && L_GET_BIT(bits, 30) != 0) ||
						(L_GET_BIT(bits, 61) == 0 && L_GET_BIT(bits, 30) == 0)){
					throw string("Fill length of fill in partition is undefined");
				}
			}
		}

		// No need to check for extended fills, since this is the last partition of this word
		// and extended fills do not span multiple words.
		// Bit 30 denotes the type of fill, bits 0-29 denote the length of the fill. By just
		// trashing bits 63-30, the length of the fill will show.
		return (bits & 0b0000000000000000000000000000000000111111111111111111111111111111);
	} else if (partitionIndex == 0){
		// Check whether the fill spans partitions 0 and 1
		long andRes = (bits & 0b1110000000000000000000000000000001000000000000000000000000000000);
		long extOneFill = andRes;
		long extZeroFill = 0b1100000000000000000000000000000000000000000000000000000000000000;
		if (andRes == extZeroFill || andRes == extOneFill){
			// Extended fill! Take bits 60-31 and 29-0, concatenate => length of the fill emerges

			// Bits from partition 0 denoting length: bits 60-31
			long part0 = (bits & 0b0001111111111111111111111111111110000000000000000000000000000000);

			// Bits from partition 1 denoting length: bits 29-0
			long part1 = (bits & 0b0000000000000000000000000000000000111111111111111111111111111111);

			// Shift the bits from partition 0 one position to the right, and return the logical OR
			// with the bits from partition1.
			return ((part0 >> 1) | part1);
		} else {
			// Regular fill, just check the length of the fill in partition 0. The two most
			// significant bits are reserved for the word header. The third most significant
			// bit (index 61) tells whether the fill in partition 0 consists of 1-bits or 0-bits.
			// Bits 60-31 store the length of the fill. Bits 30 to 0 store information about partition
			// 1, and are therefore irrelevant

			// Make sure only bits 60 up to (and including) 31 are preserved
			long length = (bits & 0b0001111111111111111111111111111110000000000000000000000000000000);

			// Shift bits 31 positions to the right, bit 60 becomes bit 30, bit 31 becomes bit 0
			length >>= 31;

			return length;
		}
	}
	throw string("Invalid partition index");
}



/**
 * Tell the compiler which instantiations to generate
 */
template class PWAHBitSet<1>;
template class PWAHBitSet<2>;
template class PWAHBitSet<4>;
template class PWAHBitSet<8>;
