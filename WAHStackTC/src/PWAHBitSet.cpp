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
template<> const long PWAHBitSet<1>::_maxBlocksPerFill = (1L << 61) - 1;// 2^62 - 1
template<> const long PWAHBitSet<2>::_maxBlocksPerFill = pow(2,30) - 1; // 2^30 - 1 = 1,073,741,823 blocks = 33,285,996,511 bits
template<> const long PWAHBitSet<4>::_maxBlocksPerFill = pow(2,14) - 1; // 2^14 - 1 = 16383 blocks 	= 245,745 bits
template<> const long PWAHBitSet<8>::_maxBlocksPerFill = pow(2,6) - 1;	// 2^6 - 1	= 63 blocks 	= 441 bits



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
	return (word & 0b0000000000000000000000000000000000000000000000000000000000000000);
}
template<> inline long PWAHBitSet<2>::clear_partition(long word, unsigned short partitionIndex){
	if (partitionIndex == 0) return (word & 0b1011111111111111111111111111111110000000000000000000000000000000);
							 return (word & 0b0100000000000000000000000000000001111111111111111111111111111111);
}
template<> inline long PWAHBitSet<4>::clear_partition(long word, unsigned short partitionIndex){
	if (partitionIndex == 0) return (word & 0b1110111111111111111111111111111111111111111111111000000000000000);
	if (partitionIndex == 1) return (word & 0b1101111111111111111111111111111111000000000000000111111111111111);
	if (partitionIndex == 2) return (word & 0b1011111111111111111000000000000000111111111111111111111111111111);
							 return (word & 0b0111000000000000000111111111111111111111111111111111111111111111);
}
template<> inline long PWAHBitSet<8>::clear_partition(long word, unsigned short partitionIndex){
	if (partitionIndex == 0) return (word & 0b1111111011111111111111111111111111111111111111111111111110000000);
	if (partitionIndex == 1) return (word & 0b1111110111111111111111111111111111111111111111111100000001111111);
	if (partitionIndex == 2) return (word & 0b1111101111111111111111111111111111111111111000000011111111111111);
	if (partitionIndex == 3) return (word & 0b1111011111111111111111111111111111110000000111111111111111111111);
	if (partitionIndex == 4) return (word & 0b1110111111111111111111111111100000001111111111111111111111111111);
	if (partitionIndex == 5) return (word & 0b1101111111111111111111000000011111111111111111111111111111111111);
	if (partitionIndex == 6) return (word & 0b1011111111111110000000111111111111111111111111111111111111111111);
							 return (word & 0b0111111100000001111111111111111111111111111111111111111111111111);
}

/**
 * \brief Determines the number of partitions required to store a fill of a certain
 * number of blocks
 */
template<> short PWAHBitSet<8>::blocks_num_partitions(int numBlocks){
	if (numBlocks > 1073741823) return 6; 	// 2^(6*5) - 1
	else if (numBlocks > 16777215) return 5;// 2^(6*4) - 1
	else if (numBlocks > 262143) return 4; 	// 2^(6*3) - 1
	else if (numBlocks > 4095) return 3; 	// 2^(6*2) - 1
	else if (numBlocks > 63) return 2; 		// 2^6 - 1
	else return 1;
}
template<> short PWAHBitSet<4>::blocks_num_partitions(int numBlocks){
	if (numBlocks > 268435455) return 3; 	// 2^(14*2) - 1
	else if (numBlocks > 16383) return 2;	// 2^14 - 1
	else return 1;
}
template<> short PWAHBitSet<2>::blocks_num_partitions(int numBlocks){
	return 1;
}
template<> short PWAHBitSet<1>::blocks_num_partitions(int numBlocks){
	return 1;
}

/**
Generic implementation stated below works just fine, but is slower...
template<unsigned int P> inline short PWAHBitSet<P>::blocks_num_partitions(int numBlocks){
	// number of bits available for expressing fill length
	const short flBits = _blockSize - 1;

	// Number of bits required to store a specific integer value
	const int numBits = log2(numBlocks) + 1;

	// Compute number of partitions required to store the number of blocks, without
	// having to call ceil(...)
	return (numBits + flBits - 1) / flBits;
}**/




/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 0-fill
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
	// NOTE: PWAHBitSet<2> does not support extended fills. A fill can contain at most
	// 2^30 - 1 = 1,073,741,823 blocks, representing 33,285,996,513 bits.
	const bool DEBUGGING = false;
	if (_VERIFY) assert(is_fill(bits, partitionIndex));
	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- partition " << partitionIndex << " of " << toBitString(bits) << endl;

	if (partitionIndex == 0) return (bits & 0b0000000000000000000000000000000000111111111111111111111111111111);
	else return ((bits & 0b0001111111111111111111111111111110000000000000000000000000000000) >> 31);
}
template<> long PWAHBitSet<4>::fill_length(long bits, unsigned short partitionIndex){
	/**
	 * PWAHBitSet<4> case: 4 partitions of 15 bits and a header of 4 bits. An extended fill
	 * can not cover multiple words. Therefore, only partitions 0, 1 and 2 can be the
	 * first partition of an extended fill. Furthermore, an extended fill will cover at
	 * most 3 partitions.
	 */
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- partition " << partitionIndex << " of " << toBitString(bits) << endl;

	if (_VERIFY){
		// Check whether the previous partition (when applicable) contains a fill
		// of the same type. In that case, throw an exception.
		if (partitionIndex > 0){
			bool thisIsOneFill = is_onefill(bits, partitionIndex);
			if (is_fill(bits, partitionIndex - 1) && is_onefill(bits, partitionIndex - 1) == thisIsOneFill){
				cout.flush();
				cerr << "Length of fill in partition " << partitionIndex << " is undefined: extended fill!" << endl;
				cerr << toBitString(bits) << endl;
				throw string("Partition is part of extended fill, but is not the head partition");
			}
		}
	}

	if (partitionIndex != 3) {
		// Might be the first partition of an extended fill. Check the fill type (0 or 1)
		// of this partition and check whether the next partition contains a fill of the same type.
		bool thisIsOneFill = is_onefill(bits, partitionIndex);
		if (is_fill(bits, partitionIndex + 1) && is_onefill(bits, partitionIndex + 1) == thisIsOneFill){
			// Extended fill!
			if (DEBUGGING) cout << "PWAHBitSet::fill_length -- partition " << partitionIndex << " of " << toBitString(bits) << ": encountered extended fill!" << endl;
			// Note that an extended fill can cover at most 3 partitions, since the number of blocks
			// is stored as an integer.

			// 14 least significant bits
			int part1 = extract_partition(bits, partitionIndex) & 0b0000000000000000000000000000000000000000000000000011111111111111;

			// 14 more significant bits
			int part2 = extract_partition(bits, partitionIndex + 1) & 0b0000000000000000000000000000000000000000000000000011111111111111;
			part2 <<= 14;

			if (partitionIndex <= 1 && is_fill(bits, partitionIndex + 2) && is_onefill(bits, partitionIndex + 2) == thisIsOneFill){
				// 4 most significant bits
				int part3 = extract_partition(bits, partitionIndex + 2) & 0b0000000000000000000000000000000000000000000000000000000000001111;
				part3 <<= 18;

				return (part1 | part2 | part3);
			}

			return (part1 | part2);
		}
	}

	// Regular fill, otherwise the 'return' above would have been called
	bits = extract_partition(bits, partitionIndex);
	long mask = 0b0000000000000000000000000000000000000000000000000011111111111111;
	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- encountered regular fill of size " << (bits & mask) << endl;

	return (bits & mask);
}
template<> long PWAHBitSet<8>::fill_length(long bits, unsigned short partitionIndex){
	const bool DEBUGGING = false;

	bool thisIsOneFill = is_onefill(bits, partitionIndex);

	if (_VERIFY){
		// Check whether the partition actually contains a fill...
		if (!is_fill(bits, partitionIndex)){
			cerr << "PWAHBitSet<8>::fill_length called on literal partition? (partitionIndex=" << partitionIndex << ")" << endl;
			cerr << toBitString(bits) << endl;
			throw string("Can't determine fill length of literal word");
		}
		// Check whether the previous partition contains a fill of equal type
		if (partitionIndex > 0){
			if (is_fill(bits, partitionIndex - 1) && thisIsOneFill == is_onefill(bits, partitionIndex - 1)){
				throw string("Fill length undefined: not head of fill!");
			}
		}
	}

	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- investigating fill starting at partition " << partitionIndex << " of " << toBitString(bits) << endl;

	// Generic method for determining fill length (works for extended and regular fills)
	const long mask6 = 0b0000000000000000000000000000000000000000000000000000000000111111;
	long numBlocks = 0L;
	long currPartBits = 0L;
	unsigned short numPartitions = 0;
	for (int i = 0; i + partitionIndex <= 7; i++){
		// i denotes the sequence number of the partition which is still part of the same fill
		// 0 - denotes first partition of the fill (= partitionIndex)
		// 1 - second partition of the fill (= partitionIndex + 1)
		// ...
		if (is_fill(bits, partitionIndex + i) && is_onefill(bits, partitionIndex + i) == thisIsOneFill){
			if (DEBUGGING) cout << "PWAHBitSet::fill_length -- including partition " << (partitionIndex + i) << endl;
			currPartBits = extract_partition(bits, partitionIndex + i) & mask6;
			numBlocks |= (currPartBits << (6 * i));
			if (DEBUGGING) cout << "PWAHBitSet::fill_length -- new number of blocks: " << toBitString(numBlocks) << endl;
			numPartitions++;
		} else {
			// End of fill
			break;
		}
	}

	if (_VERIFY){
		if (blocks_num_partitions(numBlocks) != numPartitions){
			cerr << "Inconsistency occurred when determining fill length starting at partition " << partitionIndex << " of " << toBitString(bits) << ":" << endl;
			cerr << "blocks_num_partitions(" << numBlocks << ") reports the fill should cover " << blocks_num_partitions(numBlocks) << " partitions, but counter says the fill covers " << numPartitions << " partitions?" << endl;
			throw string("inconsistency");
		}
	}
	if (DEBUGGING) cout << "PWAHBitSet::fill_length -- encountered fill of size " << numBlocks << " blocks, covering " << numPartitions << " partitions" << endl;
	return numBlocks;
}

template<unsigned int P> string PWAHBitSet<P>::toBitString(long value){
	short signedP = P;

	stringstream res;
	res << "0b";
	for (int bit = 63; bit >= 0; bit--){
		if ((63 - bit - signedP) >= 0 && (63 - bit - signedP) % _blockSize == 0){
			//cout << (63 - bit - signedP) << endl;
			//cout << _blockSize << endl;
			res << "|";
		}
		if (L_GET_BIT(value, bit)) res << "1";
		else res << "0";
	}
	res << " (= " << value << ")";
	return res.str();
}






/**
 * Constructor for PWAHBitSet
 */
template<unsigned int P> PWAHBitSet<P>::PWAHBitSet(int indexChunkSize):
			// Initialise variables
			_plainBlockIndex(0), _lastBitIndex(-1), _lastUsedPartition(0), _words(vector<long>()), _indexChunkSize(indexChunkSize){

	// Assert to check whether the long primitive type consists of 64 bits. Might the data type
	// consist of any other number of bits, weird things will happen...
	assert(sizeof(long) == (64 / CHAR_BIT));
}

template<unsigned int P> void PWAHBitSet<P>::clear(){
	_plainBlockIndex = 0;
	_lastUsedPartition = 0;
	_lastBitIndex = -1;
	_words.clear();
	_indexPartitionOffset.clear();
	_indexPartition.clear();
	_indexWord.clear();
}

template<unsigned int P> const bool PWAHBitSet<P>::get(int bitIndex){
	return get(bitIndex, false);
}

/**
 * \brief Determines whether the bit at the given index is set
 *
 * This operation takes O(n) times on PWAHBitSet instances without index,
 * or O(k) time on indexes PWAHBitSet instances (k denotes the index granularity)
 *
 * \param bitIndex the index of the desired bit
 * \param disableIndex when true, prevents the 'get' method from using the search index
 */
template<unsigned int P> const bool PWAHBitSet<P>::get(int bitIndex, bool disableIndex){
	const bool DEBUGGING = false;
	const int blockIndex = bitIndex / _blockSize;
	if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit in block " << blockIndex << endl;

	if (blockIndex > _plainBlockIndex){
		if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " out of bounds. Plain block has index " << _plainBlockIndex << endl;
		return false;
	} else if (blockIndex == _plainBlockIndex){
		if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " stored plain at index " << (bitIndex % _blockSize) << endl;
		return L_GET_BIT(_words[0], bitIndex % _blockSize);
	} else {
		// Requested bit is located somewhere in the compressed bits

		unsigned int initialWordIndex = 0;
		unsigned int initialPartitionIndex = 0;
		unsigned int currPartitionIndex = 0;
		long currBlockIndex = -1;
		int currFillLength;

		if (_indexChunkSize > 0 && !disableIndex){
			// Index available
			const int chunkIndex = bitIndex / _indexChunkSize;
			const int chunkFirstBit = chunkIndex * _indexChunkSize;

			initialWordIndex = _indexWord[chunkIndex];
			currPartitionIndex = _indexPartition[chunkIndex];
			currBlockIndex = chunkFirstBit / _blockSize - _indexPartitionOffset[chunkIndex] - 1;
			if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " already compressed, using index: chunkIndex=" << chunkIndex << ", initialWordIndex=" << initialWordIndex << ", initialPartitionIndex=" << initialPartitionIndex << ", currBlockIndex=" << currBlockIndex << endl;
		} else {
			if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- block " << blockIndex << " already compressed, scanning..." << endl;
		}


		// Iterate over all words
		long currWord;
		for (unsigned int w = initialWordIndex; w < _words.size(); w++){
			currWord = _words[w];

			// Iterate over partitions in this word
			while (currPartitionIndex < P){
				if (w == 0 && currPartitionIndex == 0){
					// skip: word 0, partition 0 contains plain block
					currPartitionIndex++;
					continue;
				}

				if (is_onefill(currWord, currPartitionIndex)){
					// Partition represents a 1-fill

					currFillLength = fill_length(currWord, currPartitionIndex);
					currBlockIndex += currFillLength;
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- encountered 1-fill of " << fill_length(currWord, currPartitionIndex) << " blocks, extending to block " << currBlockIndex << endl;
					if (currBlockIndex >= blockIndex){
						if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is set" << endl;
						return true;
					}

					currPartitionIndex += blocks_num_partitions(currFillLength);
				} else if (is_zerofill(currWord, currPartitionIndex)){
					// Partition represents a 0-fill

					currFillLength = fill_length(currWord, currPartitionIndex);
					currBlockIndex += currFillLength;
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- encountered 0-fill of " << fill_length(currWord, currPartitionIndex) << " blocks, extending to block " << currBlockIndex << endl;
					if (currBlockIndex >= blockIndex){
						if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is not set" << endl;
						return false;
					}

					currPartitionIndex += blocks_num_partitions(currFillLength);
				} else {
					// Partition contains a literal word, it's safe to skip one block
					currBlockIndex++;
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- encountered literal at block " << currBlockIndex << ", which belongs to partition " << currPartitionIndex << " of the word: " << toBitString(currWord) << endl;

					if (currBlockIndex == blockIndex){
						long partition = extract_partition(currWord, currPartitionIndex);
						if (DEBUGGING){
							cout << "PWAHBitSet::get[" << bitIndex << "] -- bit resides within partition " << currPartitionIndex << ": " << toBitString(partition) << " at position " << (bitIndex % _blockSize) << endl;
							cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is " << (L_GET_BIT(partition, bitIndex % _blockSize) ? "" : "not ") << "set" << endl;
						}
						return L_GET_BIT(partition, bitIndex % _blockSize);
					}

					currPartitionIndex++;
				}

				if (currBlockIndex > blockIndex){
					if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- bit is not set" << endl;
					return false;
				}
			}

			currPartitionIndex = 0;
			if (DEBUGGING) cout << "PWAHBitSet::get[" << bitIndex << "] -- haven't found bit yet, considering next word..." << endl;
		} // end if: index or not
	}

	stringstream ss;
	ss << "PWAHBitSet::get[" << bitIndex << "]: unexpected state";
	throw string(ss.str());
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
	const bool DEBUGGING = false;

	if (_words.size() == 0) _words.push_back(0L);
	if (DEBUGGING) cout << "PWAHBitSet::set -- " << (value ? "Setting" : "Unsetting") << " bit " << bitIndex << " (P=" << P << ", blockSize=" << _blockSize << ", plainBlockIndex=" << _plainBlockIndex << ")" << endl;
	if (DEBUGGING) cout << "PWAHBitSet::set -- current state: " << toBitString(_words[0]) << endl;

	const int blockIndex = bitIndex / _blockSize;
	if (DEBUGGING) cout << "PWAHBitSet::set -- Bit belongs to block " << blockIndex << endl;

	if (blockIndex < _plainBlockIndex){
		// Bit index is part of compressed block, can't do.
		cerr << "Can't un(set) bit " << bitIndex << ". That bit belongs to block " << blockIndex << ", only bits belonging to blockindex " << _plainBlockIndex << " (and beyond) can be (un)set! Last bit set = " << _lastBitIndex << endl;
		throw string("requested bit is part of compressed block, can't (un)set");
	}

	if (blockIndex > _plainBlockIndex){
		// Compress plain block first... Note that compressing the plain block
		// will not change _plainBlockIndex
		compressPlainPartition();

		if (blockIndex > _plainBlockIndex + 1){
			// Add one or more 0-fills to the vector with compressed blocks
			const int numZeroFills = blockIndex - _plainBlockIndex - 1;
			addFill(false, numZeroFills, _plainBlockIndex + 1);
		}

		if (DEBUGGING) cout << "PWAHBitSet::set -- Increasing index of plain block to " << blockIndex << endl;
		_plainBlockIndex = blockIndex;
	}


	// Store Bit in plain block. Go!
	if (value){
		if (DEBUGGING) cout << "PWAHBitSet::set -- setting bit " << (bitIndex % _blockSize) << " of plain block" << endl;
		L_SET_BIT(_words[0], bitIndex % _blockSize);
	} else {
		if (DEBUGGING) cout << "PWAHBitSet::set -- unsetting bit " << (bitIndex % _blockSize) << " of plain block" << endl;
		L_CLEAR_BIT(_words[0], bitIndex % _blockSize);
		if (DEBUGGING) cout << "PWAHBitSet::set -- result: " << toBitString(_words[0]) << endl;
	}
	if (bitIndex > _lastBitIndex) _lastBitIndex = bitIndex;
}

/**
 * \brief Compress the uncompressed bits in the plain partition
 *
 * Note that this method does not increase the index _plainBlockIndex! The method will
 * update the value of _lastUsedPartition, though.
 */
template<unsigned int P> void PWAHBitSet<P>::compressPlainPartition(){
	const bool DEBUGGING = false;
	assert(_words.size() > 0);
	if (_lastBitIndex / _blockSize < _plainBlockIndex){
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- not compressing, partition after _lastCompressedPartition doesn't contain any data: lastBitIndex=" << _lastBitIndex << ", plainBlockIndex=" << _plainBlockIndex << ", lastUsedPartition=" << _lastUsedPartition << endl;
		return;
	}

	long plainBlock = extract_partition(_words[0], 0);
	if (plainBlock == 0){
		// 0-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- adding 0-fill from plain partition: " << toBitString(plainBlock) << endl;
		addFill(false, 1, _plainBlockIndex);
	} else if (P == 1 && plainBlock == 0b0111111111111111111111111111111111111111111111111111111111111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- adding 1-fill from plain partition: " << toBitString(plainBlock) << endl;
		addFill(true, 1, _plainBlockIndex);
	} else if (P == 2 && plainBlock == 0b0000000000000000000000000000000001111111111111111111111111111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- adding 1-fill from plain partition: " << toBitString(plainBlock) << endl;
		addFill(true, 1, _plainBlockIndex);
	} else if (P == 4 && plainBlock == 0b0000000000000000000000000000000000000000000000000111111111111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- adding 1-fill from plain partition: " << toBitString(plainBlock) << endl;
		addFill(true, 1, _plainBlockIndex);
	} else if (P == 8 && plainBlock == 0b0000000000000000000000000000000000000000000000000000000001111111){
		// 1-fill
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- adding 1-fill from plain partition: " << toBitString(plainBlock) << endl;
		addFill(true, 1, _plainBlockIndex);
	} else {
		// literal
		if (DEBUGGING) cout << "PWAHBitSet::compressPlainPartition -- adding literal from plain partition: " << toBitString(plainBlock) << endl;
		addLiteral(plainBlock, _plainBlockIndex);
	}

	_words[0] = clear_partition(_words[0], 0);
	if (DEBUGGING){
		cout << "PWAHBitSet::compressPlainPartition -- resulting first word after compressing plain partition: " << toBitString(_words[0]) << endl;
		if (_words.size() > 1) cout << "PWAHBitSet::compressPlainPartition -- resulting last word after compressing plain partition: " << toBitString(_words.back()) << endl;
	}
}



/**
 * \brief Adds a partition containing a fill (type is indicated by boolean parameter 'oneFill')
 * of size 'numBlocks' to the compressed words. Might the last partition contain a fill of equal
 * type, this existing fill is extended.
 *
 */
template<unsigned int P> void PWAHBitSet<P>::addFill(bool oneFill, int numBlocks, int firstBlockIndex){
	const bool DEBUGGING = false;
	if (_VERIFY) assert(numBlocks > 0);
	if (DEBUGGING) cout << "PWAHBitSet::addFill(" << (oneFill ? "1" : "0") << ", " << numBlocks << " blocks). firstBlockIndex=" << firstBlockIndex << endl;

	// Check last compressed word
	if (_words.size() > 0){
		// Starting with the last used partition, scan all partitions to identify a possibly
		// existing (extended or simple) fill of the same type
		short prevFillHeadPartition = -1;
		for (short i = _lastUsedPartition; i >= 0; i--){
			if (_words.size() == 1 && i == 0) continue; // w = 0, i = 0  ==> plain block

			if ((oneFill && is_onefill(_words.back(), i)) || (!oneFill && is_zerofill(_words.back(), i))){
				// Partition is a fill of same type as fill being added!
				prevFillHeadPartition = i;
			} else {
				// Partition is not a fill, or not of the same type as the fill currently being added
				break;
			}
		}

		if (prevFillHeadPartition != -1){
			const int existingFillLength = fill_length(_words.back(), prevFillHeadPartition);
			firstBlockIndex -= existingFillLength;
			numBlocks = existingFillLength + numBlocks;
			if (DEBUGGING) cout << "PWAHBitSet::addFill(" << (oneFill ? "1" : "0") << ")" << "-- Encountered fill of size " << fill_length(_words.back(), prevFillHeadPartition) << " blocks at partition " << _lastUsedPartition << "(spanning " << (_lastUsedPartition - prevFillHeadPartition + 1) << " partition(s)), extending fill to size " << numBlocks << "..." << endl;

			// Decrease _lastUsedPartition, call to 'addPartition' below will increase _lastUsedPartition
			if (prevFillHeadPartition == 0){
				_words.pop_back();
				_lastUsedPartition = P - 1;
			} else {
				// Clear the partition(s) containing the fill that is about to be extended
				for (short i = prevFillHeadPartition; i <= _lastUsedPartition; i++){
					if (DEBUGGING) cout << "PWAHBitSet::addFill -- clearing partition " << i << endl;
					_words[_words.size() - 1] = clear_partition(_words.back(), i);
				}

				if (DEBUGGING) cout << "PWAHBitSet::addFill -- result after clearing partition(s): " << toBitString(_words.back()) << endl;
				// Decrease counter, that (cleared) partition will be overwritten
				_lastUsedPartition = prevFillHeadPartition - 1;
			}
		} // else: last partitions do not represent a fill or not a fill of equal type
	} // else: no compressed words available

	if (numBlocks > _maxBlocksPerFill && _lastUsedPartition == P - 2){
		if (DEBUGGING) cout << "PWAHBitSet::addFill -- extended fill requested, but at last partition of current word" << endl;
		// Need to store an extended fill, but can't do that as it would span multiple words.
		// Store the first part of the fill in the last partition of this word, the second part
		// of the fill should go into the next word. When necessary, the second part can be stored
		// as an extended fill (see below).

		// Store _maxBlocksPerFill of numBlocks in the last partition of this word
		addFillPartition(oneFill, _maxBlocksPerFill, firstBlockIndex);

		firstBlockIndex += _maxBlocksPerFill;
		numBlocks -= _maxBlocksPerFill;
	}

	if (numBlocks > _maxBlocksPerFill){
		// Add extended fill.
		if (DEBUGGING) cout << "PWAHBitSet::addFill -- adding extended fill of " << numBlocks << " blocks..." << endl;
		addExtendedFillPartitions(oneFill, numBlocks, firstBlockIndex);
	} else {
		// Simply add new fill
		if (DEBUGGING) cout << "PWAHBitSet::addFill -- adding simple fill of " << numBlocks << " blocks..." << endl;
		addFillPartition(oneFill, numBlocks, firstBlockIndex);
	}

	if (DEBUGGING) cout << "PWAHBitSet::addFill -- resulting last word: " << toBitString(_words.back()) << endl;
}

template<unsigned int P> void PWAHBitSet<P>::addLiteral(long value, int blockIndex){
	if (is_literal_onefill(value,0)){
		addFill(true, 1, blockIndex);
	} else if (is_literal_zerofill(value, 0)) {
		addFill(false, 1, blockIndex);
	} else {
		addPartition(false, value);
		updateIndex(1, blockIndex);
	}
}

template<unsigned int P> void PWAHBitSet<P>::addFillPartition(bool oneFill, int numBlocks, int firstBlockIndex){
	assert(numBlocks <= _maxBlocksPerFill);

	if (!oneFill){
		// 0-fills are very easy to add
		addPartition(true, numBlocks);
	} else {
		// 1-fills need to be preceded by a 1-bit
		P == 1 ? addPartition(true, (numBlocks | 0b0100000000000000000000000000000000000000000000000000000000000000)) :
		P == 2 ? addPartition(true, (numBlocks | 0b0000000000000000000000000000000001000000000000000000000000000000)) :
		P == 4 ? addPartition(true, (numBlocks | 0b0000000000000000000000000000000000000000000000000100000000000000)) :
				 addPartition(true, (numBlocks | 0b0000000000000000000000000000000000000000000000000000000001000000));
	}

	// After adding a partition: update index
	if (firstBlockIndex >= 0){
		updateIndex(numBlocks, firstBlockIndex);
	}
}

/**
 * When necessary, updates the search indices of this PWAHBitSet. This method must
 * be called after having actually added a partition.
 *
 * This method will decide whether it is necessary to add an entry to the list of
 * search indices, based on the two parameters. If it turns out to be unnecessary to
 * add information to the search indices, the method will simply do nothing and return.
 *
 * \param numBlocks the number of blocks in the last added partition
 * \param firstBlockIndex the index of the first block which was added to the partition
 */
template<unsigned int P> void PWAHBitSet<P>::updateIndex(int numBlocks, int firstBlockIndex){
	const bool DEBUGGING = false;
	if (_indexChunkSize <= 0) return;
	if (DEBUGGING) cout << "PWAHBitSet::updateIndex: numBlocks=" << numBlocks << ", firstBlockIndex=" << firstBlockIndex << ", indexChunkSize=" << _indexChunkSize << endl;

	// One or more entries should be added to the search indices when one or more
	// of the added blocks contain bits which are supposed to be indexed.
	const int firstBitIndex = firstBlockIndex * _blockSize;
	const int numBits = numBlocks * _blockSize;
	const int lastBitIndex = firstBitIndex + numBits - 1;

	const int firstChunk = firstBitIndex / _indexChunkSize;
	const int lastChunk = lastBitIndex / _indexChunkSize;

	if (DEBUGGING){
		cout << "PWAHBitSet::updateIndex: firstChunk=" << firstChunk << ", lastChunk=" << lastChunk << endl;
		cout << "PWAHBitSet::updateIndex: current state:" << endl;
		cout << this->toString() << endl;
	}

	if (firstBitIndex % _indexChunkSize == 0){
		// Add entry for the first bit
		setIndexEntry(firstChunk, _words.size() - 1, _lastUsedPartition, 0);
	}

	// Add entries for (firstChunk + 1) up to and including lastChunk
	int chunkFirstBit, chunkFirstBitBlockIndex;
	for (int i = firstChunk + 1; i <= lastChunk; i++){
		chunkFirstBit = i * _indexChunkSize;
		chunkFirstBitBlockIndex = (chunkFirstBit / _blockSize);
		setIndexEntry(i, _words.size() - 1, _lastUsedPartition, chunkFirstBitBlockIndex - firstBlockIndex);
	}

	if ((firstBlockIndex * _blockSize) % _indexChunkSize != 0) return;
}

/**
 * This method will set the search location for the specific chunk to the specified
 * word, partition and partitionOffset.
 *
 * Example (chunk size = 1000, PWAHBitSet<8>, block size = 7):
 * Suppose a fill consisting of 8 words is added to the PWAHBitSet<8>. The fill
 * is located in the word with index 39, in partition with index 2.
 * The first fill block has index 140 and contains bit indices 980 to 986. The last word
 * of the fill has index 147 and contains bit indices 1029 to 1035. The chunk size
 * specifies that the location of the blocks containing bits 0, 1000, 2000, (...)
 * should be explicitly stored. The bit with index 1000 is located within block
 * 142, the third block in the fill.
 * Directly after adding the partition with the fill containing bit 1000, this method should be called
 * with the following parameters:
 * chunkIndex = 1, indexWord = 39, indexPartition = 2, indexPartitionOffset = 3
 *
 * \param chunkIndex the chunk index of the chunk to register
 * \param indexWord the word index in which the chunk starts
 * \param indexPartition the partition index in which the chunk starts
 * \param indexPartitionOffset the offset (in blocks) within the partition
 */
template<unsigned int P> void PWAHBitSet<P>::setIndexEntry(int chunkIndex, int indexWord, int indexPartition, int indexPartitionOffset){
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet::setIndexEntry -- setting index for chunk " << chunkIndex << endl;
	if (_VERIFY){
		if (_indexWord.size() < chunkIndex){
			cerr << "Trying to add index for chunk " << chunkIndex << ", but chunk " << (chunkIndex -1) << " hasn't been indexed yet? Number of indexed chunks = " << _indexWord.size() << endl;
			cerr << this->toString() << endl;
			throw string("inconsistency");
		}
	}

	if (_indexWord.size() > chunkIndex){
		// This chunk has been indexed before, update entries
		if (DEBUGGING) cout << "PWAHBitSet::setIndexEntry -- updating chunkIndex=" << chunkIndex << ", indexword=" << indexWord << ", indexPartition=" << indexPartition << ", indexPartitionOffset=" << indexPartitionOffset << endl;
		_indexWord[chunkIndex] = indexWord;
		_indexPartition[chunkIndex] = indexPartition;
		_indexPartitionOffset[chunkIndex] = indexPartitionOffset;
	} else {
		// Chunk not seen before, add entries
		if (DEBUGGING) cout << "PWAHBitSet::setIndexEntry -- adding chunkIndex=" << chunkIndex << ", indexword=" << indexWord << ", indexPartition=" << indexPartition << ", indexPartitionOffset=" << indexPartitionOffset << endl;
		_indexWord.push_back(indexWord);
		_indexPartition.push_back(indexPartition);
		_indexPartitionOffset.push_back(indexPartitionOffset);
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

	if (_lastUsedPartition == P - 1 || _words.size() == 0){
		// Add new empty long to the vector
		_words.push_back(0L);
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

	_words[_words.size() - 1] |= value;

	if (DEBUGGING) cout << "PWAHBitSet::addPartition -- resulting word after adding new partition at index " << _lastUsedPartition << ": " << toBitString(_words.back()) << endl;
}


/**
 * \brief Merges multiple WAHBitSet objects into a single WAHBitSet by computing the logical OR
 *
 * \param bitSets the source WAHBitSet objects
 * \param numBitSets the number of source WAHBitSet objects
 * \param result pointer to a WAHBitSet to store the result of the logical OR
 */
template<unsigned int P> void PWAHBitSet<P>::multiOr(PWAHBitSet<P>** bitSets, unsigned int numBitSets, PWAHBitSet<P>* result){
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << endl << endl << "Performing multi-way OR on " << numBitSets << " BitSets" << endl;
	if (numBitSets == 0) return;
	if (result->_words.size() == 0) result->_words.push_back(0L);

	// Current position in result BitSet: block index
	unsigned int rBlockIndex = 0;

	// Current position in source BitSets:
	unsigned int* sBlockIndex = new unsigned int[numBitSets];		// block index
	unsigned int* sWordIndex = new unsigned int[numBitSets];		// word index
	unsigned int* sPartitionIndex = new unsigned int[numBitSets];	// partition within word
	unsigned int* sPartitionOffset = new unsigned int[numBitSets];	// offset within partition
	unsigned int lastBitIndex = 0;
	unsigned int maxPlainBlockIndex = 0;

	// Some bookkeeping
	int largestOneFill, shortestZeroFill;
	unsigned int largestOneFillSize, shortestZeroFillSize;
	int postponedOneFillSize = 0;
	int postponedZeroFillSize = 0;
	long currMergedLiteral;
	bool mergedLiteral;
	unsigned int currFillLengthRemaining;
	long currWord;
	long prevLastBitIndex = -1;
	bool increasingOrder = true;
	int firstRelevantBitSet = 0;

	// Initialize values in int arrays to 0.
	memset(sWordIndex, 0, numBitSets * sizeof(int));
	memset(sPartitionIndex, 0, numBitSets * sizeof(int));
	memset(sPartitionOffset, 0, numBitSets * sizeof(int));
	memset(sBlockIndex, 0, numBitSets * sizeof(int));

	if (DEBUGGING){
		cout << "Input to multi-or:" << endl;
		for (int i = 0; i < numBitSets; i++){
			cout << "BitSet " << i << ":" << endl;
			cout << bitSets[i]->toString() << endl;
		}
		cout << endl;
	}
	while(true){
		if (DEBUGGING) cout << "Composing result block with index " << rBlockIndex << "..." << endl;
		//if (DEBUGGING) cout << "Current state of affairs: " << endl << result->toString() << endl;

		currMergedLiteral = 0;
		largestOneFill = -1;
		shortestZeroFill = -1;
		shortestZeroFillSize = 0;
		largestOneFillSize = 0;
		mergedLiteral = false;

		// Align BitSets, find largest 1-fill and merge literals in the process
		for (unsigned int i = firstRelevantBitSet; i < numBitSets; i++){
			if (rBlockIndex == 0){
				// Checks to perform only during the first pass
				if (bitSets[i]->_lastBitIndex > lastBitIndex) lastBitIndex = bitSets[i]->_lastBitIndex;
				if (bitSets[i]->_plainBlockIndex > maxPlainBlockIndex) maxPlainBlockIndex = bitSets[i]->_plainBlockIndex;

				// Determine whether the BitSets are provided in order of increasing size. In that
				// case, an additional optimisation can be applied.
				increasingOrder = (bitSets[i]->_lastBitIndex >= prevLastBitIndex) && increasingOrder;
				prevLastBitIndex = bitSets[i]->_lastBitIndex;

				// Start at partition index 1 of the first word, since partition 0 will always
				// contain a plain block
				sPartitionIndex[i] = 1;
			}

			if (DEBUGGING) cout << "Considering BitSet with index " << i << " (total number of bitsets = " << numBitSets << ")" << endl;
			if (DEBUGGING) cout << "BitSet " << i << " contains " << bitSets[i]->_words.size() << " compressed words, is now at blockindex " << sBlockIndex[i] << " which is in partitionindex " << sPartitionIndex[i] << ", wordindex " << sWordIndex[i] << ". Plain block has index " << bitSets[i]->_plainBlockIndex << ", lastbitindex=" << bitSets[i]->_lastBitIndex << endl;

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

				// Plain block is stored within the first partition of the first word
				currWord = bitSets[i]->_words[0];
				sBlockIndex[i] = bitSets[i]->_plainBlockIndex;
				sWordIndex[i] = bitSets[i]->_words.size();
				sPartitionIndex[i] = 0;
				sPartitionOffset[i] = 0;
			} else if (rBlockIndex > bitSets[i]->_plainBlockIndex){
				// Out of bounds!
				if (DEBUGGING) cout << "BitSet " << i << " totally out of bounds: plain block has index " << bitSets[i]->_plainBlockIndex << ", while composing result block " << rBlockIndex << " (lastBitIndex=" << bitSets[i]->_lastBitIndex << ")" << endl;
				if (increasingOrder) firstRelevantBitSet = i + 1;
				continue;
			} else {
				// Use word from vector of compressed words
				currWord = bitSets[i]->_words[sWordIndex[i]];
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
						sBlockIndex[i] += currFillLengthRemaining;
						sPartitionOffset[i] = 0;
						sPartitionIndex[i] += blocks_num_partitions(currFillLengthRemaining + sPartitionOffset[i]);
						if (DEBUGGING){
							cout << "BitSet " << i << ": skipping entire fill to blockindex " << sBlockIndex[i] << endl;
							cout << "BitSet " << i << ": skipping " << currFillLengthRemaining << " blocks, covering " << blocks_num_partitions(currFillLengthRemaining + sPartitionOffset[i]) << " partition(s)" << endl;
						}
					}
				} else {
					// Partition contains literal. Can be skipped without additional checks
					sBlockIndex[i]++;
					sPartitionIndex[i]++;
				}

				// Some additional bookkeeping regarding partitions in words
				if (_VERIFY) assert(sPartitionIndex[i] <= P);
				if (sPartitionIndex[i] == P){
					// Jump to next word!
					sPartitionIndex[i] = 0;
					sWordIndex[i]++;
				}

				// Select next word from vector of compressed words
				currWord = bitSets[i]->_words[sWordIndex[i]];
			} // end while: done aligning this BitSet

			// Might this BitSet be out of bounds?
			if (sWordIndex[i] > bitSets[i]->_words.size()){
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

				sPartitionIndex[i] += blocks_num_partitions(currFillLengthRemaining + sPartitionOffset[i]);
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
				sPartitionIndex[i] += blocks_num_partitions(currFillLengthRemaining + sPartitionOffset[i]);
				sBlockIndex[i] += currFillLengthRemaining;
				sPartitionOffset[i] = 0;
			} else {
				// At literal
				if (DEBUGGING) cout << "Encountered literal in BitSet " << i << ": " << toBitString(extract_partition(currWord, sPartitionIndex[i])) << endl;
				currMergedLiteral |= extract_partition(currWord, sPartitionIndex[i]);
				mergedLiteral = true;

				// Safe to skip one block
				sBlockIndex[i]++;
				sPartitionOffset[i] = 0;
				sPartitionIndex[i]++;
			}

			assert(sPartitionIndex[i] <= P);
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
				result->addFill(false, postponedZeroFillSize, rBlockIndex - postponedZeroFillSize);
				postponedZeroFillSize = 0;
			}

			// Postpone addition of 1-fill. It's more efficient to add large 1-fills
			// at once, in stead of in parts.
			postponedOneFillSize += largestOneFillSize;
			rBlockIndex += largestOneFillSize;

			if (DEBUGGING) cout << "Total length of postponed 1-fill: " << postponedOneFillSize <<  " block(s)" << endl;
		} else if (mergedLiteral){
			// No 1-fill available, store merged literal
			if (DEBUGGING) cout << "Adding literal to result: " << toBitString(currMergedLiteral) << endl;

			// First, check whether there is a postponed 0-fill or 1-fill floating around
			if (postponedOneFillSize != 0){
				if (DEBUGGING) cout << "Before adding literal: processing postponed 1-fill of length " << postponedOneFillSize << endl;
				result->addFill(true, postponedOneFillSize, rBlockIndex - postponedOneFillSize);
				postponedOneFillSize = 0;
			} else if (postponedZeroFillSize != 0){
				if (DEBUGGING) cout << "Before adding literal: processing postponed 0-fill of length " << postponedZeroFillSize << endl;
				result->addFill(false, postponedZeroFillSize, rBlockIndex - postponedZeroFillSize);
				postponedZeroFillSize = 0;
			}

			// Add literal to the result
			result->addLiteral(currMergedLiteral, rBlockIndex);
			rBlockIndex++;
		} else if (shortestZeroFill != -1) {
			// No 1-fill or literals seen, but a 0-fill is available
			if (DEBUGGING) cout << "Adding 0-fill of length " << shortestZeroFill << " to result (postponed) "<< endl;

			// First, check whether there is a postponed 1-fill floating around
			if (postponedOneFillSize != 0){
				if (DEBUGGING) cout << "Before adding 0-fill: processing postponed 1-fill of length " << postponedOneFillSize << endl;
				result->addFill(true, postponedOneFillSize, rBlockIndex - postponedOneFillSize);
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
				result->addFill(true, postponedOneFillSize, rBlockIndex - postponedOneFillSize);
				postponedOneFillSize = 0;
			} else if (postponedZeroFillSize != 0){
				if (DEBUGGING) cout << "Processing postponed 0-fill of length " << postponedZeroFillSize << endl;
				result->addFill(false, postponedZeroFillSize, rBlockIndex - postponedZeroFillSize);
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
	if (rBlockIndex > 0){
		result->decompressLastBlock();
	}

	result->_plainBlockIndex = maxPlainBlockIndex;
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

	if (DEBUGGING){
		cout << "Result multiway-OR:" << endl;
		cout << result->toString() << endl;
		cout << "done multiway!" << endl;
	}
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
	int currFillLength;
	for(unsigned int i = 0; i < _words.size(); i++){
		currWord = _words[i];

		if (i == _words.size() - 1){
			maxP = _lastUsedPartition;
		} else {
			maxP = P-1;
		}

		unsigned int p = 0;
		while (p <= maxP){
			if (is_fill(currWord, p)){
				currFillLength = fill_length(currWord, p);
				count += currFillLength;
				p += blocks_num_partitions(currFillLength);
			} else {
				// literal
				count++;
				p++;
			}
		}
	}

	return count;
}

/**
 * \brief Removes the last partition. This will effectively remove one or more blocks
 *
 * This function will update _compressedWords and _lastUsedPartition
 */
template<unsigned int P> void PWAHBitSet<P>::popLastPartition(){
	const bool DEBUGGING = false;
	if (_VERIFY){
		assert(_words.size() > 0);
		assert(_words.size() > 1 || _lastUsedPartition > 0);
	}

	if (DEBUGGING) cout << "PWAHBitSet::popLastPartition: " << toBitString(_words.back()) << endl;
	if (_lastUsedPartition == 0){
		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- last used partition = 0, number of words = " << _words.size() << " => removing last word" << endl;
		_words.pop_back();
		_lastUsedPartition = P - 1;
		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- done! Last used partition = " << _lastUsedPartition << ", number of words = " << _words.size() << endl;
	} else {
		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- clearing partition " << _lastUsedPartition << endl;
		_words[_words.size() - 1] = clear_partition(_words.back(), _lastUsedPartition);

		if (DEBUGGING) cout << "PWAHBitSet::popLastPartition -- decreasing _lastUsedPartition" << endl;
		_lastUsedPartition--;
	}

	if (DEBUGGING) cout << "PWAHBitSet::popLastPartition result: " << toBitString(_words.back()) << endl;
}

template<unsigned int P> void PWAHBitSet<P>::decompressLastBlock(){
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock()" << endl;
	if (_VERIFY){
		assert(_words.size() > 0);
		assert(_words.size() > 1 || _lastUsedPartition > 0);
		assert(extract_partition(_words[0], 0) == 0);
	}

	if (is_fill(_words.back(), _lastUsedPartition)){
		// Might be an extended fill...
		if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- decompressing block from fill..." << endl;
		const bool thisIsOneFill = is_onefill(_words.back(), _lastUsedPartition);
		const long word = _words.back();
		unsigned short fillHead = _lastUsedPartition;

		if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- scanning and clearing partitions..." << endl;
		for (int i = _lastUsedPartition; i >= 0; i--){
			if (is_fill(word, i) && is_onefill(word, i) == thisIsOneFill){
				// Part of the same fill => pop partition
				_words[_words.size() - 1] = clear_partition(_words.back(), i);
				fillHead = i;
			} else {
				break;
			}
		}
		const long fillLength = fill_length(word, fillHead);
		if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- detected fill of length " << fillLength << ", covering " << blocks_num_partitions(fillLength) << " partitions" << endl;

		if (fillHead == 0){
			// Fill started at partition 0. Since the entire fill was removed, remove
			// the last element from the vector with words
			if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- fill started at partition 0, popping off last word from vector..." << endl;
			_words.pop_back();
			_lastUsedPartition = P - 1;
		} else {
			_lastUsedPartition = fillHead - 1;
		}
		if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- new _lastUsedPartition = " << _lastUsedPartition << endl;

		if (fillLength > 1){
			// Re-add slightly smaller fill
			if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- re-adding fill of length " << (fillLength - 1) << endl;
			addFill(thisIsOneFill, fillLength - 1, -1);
		}

		if (thisIsOneFill){
			// Set plain block to all ones.
			P == 1 ? _words[0] |= 0b0111111111111111111111111111111111111111111111111111111111111111 :
			P == 2 ? _words[0] |= 0b0000000000000000000000000000000001111111111111111111111111111111 :
			P == 4 ? _words[0] |= 0b0000000000000000000000000000000000000000000000000111111111111111 :
					 _words[0] |= 0b0000000000000000000000000000000000000000000000000000000001111111;

		} // else: 0-fill. Don't need to change anything wrt plain block, all zeroes anyway.
	} else {
		// Literal word: creating a copy suffices.
		if (DEBUGGING) cout << "PWAHBitSet::decompressLastBlock -- last partition contains literal" << endl;
		_words[0] |= extract_partition(_words.back(), _lastUsedPartition);

		popLastPartition();
	}
}

template<unsigned int P> PWAHBitSet<P>* PWAHBitSet<P>::constructByOr(const PWAHBitSet<P>* first, const PWAHBitSet<P>* second){
	throw string("PWAHBitSet::constructByOr not implemented");
}

template<unsigned int P> const long PWAHBitSet<P>::memoryUsage(){
	long res = _words.size() * 64;
	return res;
}

template<unsigned int P> BitSetIterator* PWAHBitSet<P>::iterator(){
	return new PWAHBitSetIterator<P>(this);
}

template<unsigned int P> const int PWAHBitSet<P>::blocksize(){
	return _blockSize;
}


template<unsigned int P> const string PWAHBitSet<P>::toString(){
	stringstream res;

	res << "Last bit set: " << _lastBitIndex << endl;
	res << "Words (last used partition=" << _lastUsedPartition << ", plain block index = " << _plainBlockIndex << "):" << endl;
	if (_words.size() > 0){
		for (unsigned int i = 0; i < _words.size(); i++){
			res << toBitString(_words[i]) << endl;
		}
	} else {
		res << "(none)";
	}

	return res.str();
}


/**
 * \brief Adds an extended fill (using multiple partitions) to the compressed word.
 *
 * This method does not extend existing fills and should not be called directly, but only
 * through addFill(...). Furthermore, this method assumes that at least two partitions of
 * the current word are unused
 */
template<> void PWAHBitSet<1>::addExtendedFillPartitions(bool oneFill, int numBlocks, int firstBlockIndex){
	throw string("Extended fills are not supported in PWAHBitSet<1>");
}
template<> void PWAHBitSet<2>::addExtendedFillPartitions(bool oneFill, int numBlocks, int firstBlockIndex){
	throw string("Extended fills are not supported in PWAHBitSet<2>");
}
template<> void PWAHBitSet<4>::addExtendedFillPartitions(bool oneFill, int numBlocks, int firstBlockIndex){
	const bool DEBUGGING = false;
	if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- adding " << (oneFill ? "1" : "0") << "-fill of " << numBlocks << " blocks" << endl;

	const short nextAvailPartition = (_lastUsedPartition + 1) % 4;

	assert(numBlocks > _maxBlocksPerFill); // extended fill should cover >= 2 partitions
	assert(nextAvailPartition < 3); // at least 2 partitions should be available

	/**
	 * Note that the PWAHBitSet<4> is only capable of storing an extended fill
	 * covering 2 or 3 partitions. Would an extended fill require 4 partitions,
	 * an integer overflow for the 'numBlocks' parameter would have occurred.
	 */
	const short numPartitions = blocks_num_partitions(numBlocks);
	const short numAvailPartitions = 4 - nextAvailPartition;
	if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- fill of " << numBlocks << " blocks requires " << numPartitions << " partitions" << endl;

	// Number of partitions available in the target word equals 4 - nextAvailPartition
	if (numAvailPartitions < numPartitions){
		// Not enough partitions available in the current word, split the extended fill
		// into two pieces.
		const int part1 = pow(2, numAvailPartitions * 14) - 1;
		if (part1 > _maxBlocksPerFill){
			addExtendedFillPartitions(oneFill, part1, firstBlockIndex);
		} else {
			addFillPartition(oneFill, part1, firstBlockIndex);
		}

		const int part2 = numBlocks - part1;
		if (part2 > _maxBlocksPerFill){
			addExtendedFillPartitions(oneFill, part2, firstBlockIndex);
		} else {
			addFillPartition(oneFill, part2, firstBlockIndex);
		}

		if (DEBUGGING){
			cout << "PWAHBitSet<4>::addExtendedFill -- split add done!" << endl;
			cout << "PWAHBitSet<4>::addExtendedFill -- first word: " << toBitString(_words[_words.size() - 2]) << endl;
			cout << "PWAHBitSet<4>::addExtendedFill -- second word: " << toBitString(_words[_words.size() - 1]) << endl;
		}
	} else {
		// Sufficient amount of partitions available in the current word, now
		// do some dirty bit shifting!

		/**
		 * Explanation of what's going on below:
		 *
		 * Suppose a 1-fill of length 70,000 blocks is to be added. In binary notation,
		 * that plain number (just the number) would be stored as:
		 * 0b10001000101110000
		 *
		 * In this PWAHBitSet<4>, the block size equals 15. That means 14 bits are available
		 * to store a fill length. Therefore, we'll have to cut the 70,000 in two pieces.
		 * Furthermore, a 1-bit should be added to indicate the fact this fill represents a
		 * sequence of 1-bits.
		 *
		 * The result should equal two partitions of 15 bits, each starting with a 1-bit to indicate
		 * the 1-fill:
		 *
		 * first partition:  0b101000101110000
		 * second partition: 0b100000000000100
		 *
		 * Compare the result with the original number:
		 * second & first partition: 0b100000000000100|101000101110000
		 * original number:          0b000000000000100|x01000101110000
		 *
		 * The first partition contains the first 14 bits of the original fill length, prepended
		 * by a 1-bit. The second partition contains the remaining 3 bits of the original fill
		 * length, again prepended by a 1-bit.
		 *
		 * Note that this example only covers the case in which a fill covers 2 partitions. Obviously,
		 * it is possible for a fill to cover 3 partitions. However, a fill covering 4 partitions would
		 * have yielded an integer overflow in the 'numBlocks' parameter.
		 */

		if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- binary notation of " << numBlocks << ": " << toBitString(numBlocks) << endl;

		// first 14 bits (least significant)
		int part1 = numBlocks & 0b00000000000000000011111111111111;

		// last 14 bits (most significant)
		int part2 = numBlocks & 0b00001111111111111100000000000000;
		part2 >>= 14;

		// Add the two fill partitions
		addFillPartition(oneFill, part1, firstBlockIndex);
		addFillPartition(oneFill, part2, -1);

		if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- part1: " << toBitString(part1) << endl;
		if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- part2: " << toBitString(part2) << endl;
		if (numPartitions == 3){
			// Add third partition

			// last 4 bits (most significant)
			int part3 = numBlocks & 0b11110000000000000000000000000000;
			part3 >>= 18;

			addFillPartition(oneFill, part3, -1);
			if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- part3: " << toBitString(part3) << endl;
		}

		if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- resulting last compressed word: " << toBitString(_words.back()) << endl;
		if (_VERIFY){
			int length = fill_length(_words.back(), nextAvailPartition);
			assert(length == numBlocks);
			if (DEBUGGING) cout << "PWAHBitSet<4>::addExtendedFill -- successfully double checked length: " << length << endl;
		}
	}
}
template<> void PWAHBitSet<8>::addExtendedFillPartitions(bool oneFill, int numBlocks, int firstBlockIndex){
	// Determine the number of partitions required to store this extended fill
	assert(numBlocks > _maxBlocksPerFill);
	const bool DEBUGGING = false;
	const unsigned short numPartitions = blocks_num_partitions(numBlocks);
	const unsigned short nextAvailPartition = (_lastUsedPartition + 1) % 8;
	const unsigned short numAvailablePartitions = 8 - nextAvailPartition;

	if (DEBUGGING) cout << "PWAHBitSet::addExtendedFillPartitions -- adding extended " << (oneFill ? "1" : "0") << "-fill of " << numBlocks << " blocks (firstBlockIndex=" << firstBlockIndex << "), covering " << numPartitions << " partitions, starting at partition " << nextAvailPartition << endl;
	if (numAvailablePartitions < numPartitions){
		// Not enough partitions available in this word
		if (DEBUGGING) cout << "PWAHBitSet::addExtendedFillPartitions -- only " << numAvailablePartitions << " partitions available in current word, splitting fill..." << endl;

		// How many blocks can be stored in the available partitions?
		int numBlocksInFirstWord = pow(2, numAvailablePartitions * 6) - 1;
		if (numBlocksInFirstWord > _maxBlocksPerFill){
			addExtendedFillPartitions(oneFill, numBlocksInFirstWord, firstBlockIndex);
		} else {
			addFillPartition(oneFill, numBlocksInFirstWord, firstBlockIndex);
		}
		numBlocks -= numBlocksInFirstWord;
		firstBlockIndex += numBlocksInFirstWord;

		if (numBlocks < _maxBlocksPerFill){
			// Remaining number of blocks not sufficient to build an extended fill,
			// add regular fill
			addFillPartition(oneFill, numBlocks, firstBlockIndex);
		} else {
			addExtendedFillPartitions(oneFill, numBlocks, firstBlockIndex);
		}

		if (DEBUGGING){
			cout << "PWAHBitSet::addExtendedFillPartitions -- done adding split extended fill, resulting words: " << endl;
			cout << toBitString(_words[_words.size() - 2]) << endl;
			cout << toBitString(_words[_words.size() - 1]) << endl;
		}
	} else {
		// Number of available partition sufficient to store the specified number of blocks
		if (DEBUGGING) cout << "PWAHBitSet::addExtendedFill -- " << numAvailablePartitions << " partitions available in current word, go!" << endl;

		long currBits;
		long mask6 = 0b0000000000000000000000000000000000000000000000000000000000111111;
		for (int i = 0; i < numPartitions; i++){
			currBits = (numBlocks >> i * 6) & mask6;
			if (i == 0){
				addFillPartition(oneFill, currBits, -1);
				updateIndex(numBlocks, firstBlockIndex);
			} else {
				addFillPartition(oneFill, currBits, -1);
			}
		}

		if (_VERIFY){
			assert(fill_length(_words.back(), nextAvailPartition) == numBlocks);
		}

		if (DEBUGGING) cout << "PWAHBitSet::addExtendedFill -- done adding extended fill, result: " << toBitString(_words.back()) << endl;
	}


}

template<> string PWAHBitSet<1>::bsImplementationName(){
	return "PWAHBitSet<1>";
}
template<> string PWAHBitSet<2>::bsImplementationName(){
	return "PWAHBitSet<2>";
}
template<> string PWAHBitSet<4>::bsImplementationName(){
	return "PWAHBitSet<4>";
}
template<> string PWAHBitSet<8>::bsImplementationName(){
	return "PWAHBitSet<8>";
}

/**
 * Tell the compiler which instantiations to generate
 */
template class PWAHBitSet<1>;
template class PWAHBitSet<2>;
template class PWAHBitSet<4>;
template class PWAHBitSet<8>;
