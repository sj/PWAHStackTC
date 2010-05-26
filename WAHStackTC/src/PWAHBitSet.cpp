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
 * Constructor for PWAHBitSet
 */
template<unsigned int P> PWAHBitSet<P>::PWAHBitSet():
			_plainBlockIndex(0), _compressedBlocks(vector<long>()){
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a fill
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_fill(long bits, unsigned short partitionIndex){
	return L_GET_BIT(bits, partitionIndex);
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 1-fill
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_onefill(long bits, unsigned short partitionIndex){
	return is_fill(bits, partitionIndex) && L_GET_BIT(bits, _partitionOffsets[partitionIndex]);
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of a 0-fill
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_zerofill(long bits, unsigned short partitionIndex){
	return is_fill(bits, partitionIndex) && !L_GET_BIT(bits, _partitionOffsets[partitionIndex]);
}

/**
 * \brief Checks whether the partition with index 'partitionIndex' within 'bits' consists of literal
 */
template<unsigned int P> inline bool PWAHBitSet<P>::is_literal(long bits, unsigned short partitionIndex){
	return !is_fill(bits, partitionIndex);
}

/**
 * \brief Sets the bit with index 'bitIndex' in this PWAHBitSet to '1'
 */
template<unsigned int P> void PWAHBitSet<P>::set(int bitIndex){
	cout << "Setting bit " << bitIndex << " (P=" << P << ", blockSize=" << _blockSize << ". Offset of first partition: " << _partitionOffsets[0] << ")" << endl;

	const int blockIndex = bitIndex / _blockSize;
	cout << "Bit belongs to block " << blockIndex << endl;
}

/**
 * Tell the compiler which instantiations to generate
 */
template class PWAHBitSet<1>;
template class PWAHBitSet<2>;
template class PWAHBitSet<4>;
template class PWAHBitSet<8>;
