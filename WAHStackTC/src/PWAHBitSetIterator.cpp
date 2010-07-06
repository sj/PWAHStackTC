/*
 * PWAHBitSetIterator.cpp
 *
 *  Created on: Jun 16, 2010
 *      Author: bas
 */

#include "PWAHBitSetIterator.h"
#include "LongBitMacros.cpp"
#include "PWAHBitSet.h"

template<unsigned int P> PWAHBitSetIterator<P>::PWAHBitSetIterator(PWAHBitSet<P>* pwbs) {
	_bitSet = pwbs;
	reset();
}

template<unsigned int P> PWAHBitSetIterator<P>::~PWAHBitSetIterator() {}

template<unsigned int P> bool PWAHBitSetIterator<P>::hasNext(){
	throw string("not implemented");
}

template<unsigned int P> void PWAHBitSetIterator<P>::reset(){
	_lastBlockIndex = 0;
	_lastBlockBitIndex = -1;
	_lastWordIndex = 0;
	_lastPartitionBlockIndex = 0;
	_lastWordPartitionIndex = 0;

	_atStart = true;
	_atEnd = false;
}

template<unsigned int P> int PWAHBitSetIterator<P>::next(){
	const bool DEBUGGING = false;

	if (_atEnd){
		return -1;
	}

	_atStart = false;

	long currWord, currFillLengthRemaining, currPartition;
	while(true){
		if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- scanning: lastBlockIndex=" << _lastBlockIndex << ", lastBlockBitIndex=" << _lastBlockBitIndex << ", lastWordIndex=" << _lastWordIndex << ", lastPartitionBlockIndex=" << _lastPartitionBlockIndex << ", lastWordPartitionIndex=" << _lastWordPartitionIndex << endl;
		if (PWAHBitSet<P>::_VERIFY){
			// Check whether _lastWordIndex and _lastBlockIndex are consistent with _plainBlockIndex
			if (_lastBlockIndex > _bitSet->_plainBlockIndex + 1){
				cerr << "lastBlockIndex = " << _lastBlockIndex << ", while BitSet plainBlockIndex=" << _bitSet->_plainBlockIndex << endl;
				cerr << _bitSet->toString() << endl;
				cerr.flush();
				throw string("Unexpected state");
			}
		}

		if (_lastBlockIndex < _bitSet->_plainBlockIndex){
			if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- checking block with index " << _lastBlockIndex << " within compressed word with wordindex " << _lastWordIndex << endl;
			currWord = _bitSet->_compressedWords[_lastWordIndex];
		} else if (_lastBlockIndex == _bitSet->_plainBlockIndex){
			if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- checking plain block with block index " << _lastBlockIndex << endl;
			_lastWordPartitionIndex = 0;
			_lastWordIndex = _bitSet->_compressedWords.size();
			currWord = _bitSet->_plainBlock;
		} else {
			if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- blockindex " << _lastBlockIndex << " out of bounds: at end!"<< endl;
			_atEnd = true;
			return -1;
		}

		if (PWAHBitSet<P>::is_onefill(currWord, _lastWordPartitionIndex)){
			// Partition contains 1-fill
			currFillLengthRemaining = PWAHBitSet<P>::fill_length(currWord, _lastWordPartitionIndex);
			if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- encountered 1-fill of length " << currFillLengthRemaining << " covering " << PWAHBitSet<P>::blocks_num_partitions(currFillLengthRemaining) << " partition(s)" << endl;

			_lastBlockBitIndex++;
			if (_lastBlockBitIndex >= _bitSet->_blockSize){
				// No more bits in this block, is there a next block in this 1-fill word?
				_lastBlockBitIndex = 0;
				_lastPartitionBlockIndex++;
				_lastBlockIndex++;

				if (_lastPartitionBlockIndex >= currFillLengthRemaining){
					// No more blocks in this 1-fill partition, skip to the next partition
					_lastWordPartitionIndex += PWAHBitSet<P>::blocks_num_partitions(currFillLengthRemaining);
					_lastPartitionBlockIndex = 0;

					// Enter next iteration of while-loop to find the next word
					_lastBlockBitIndex = -1;
				} else {
					// There are more blocks in this 1-fill word
					_lastBlockBitIndex = 0;

					// Return the absolute position of the 1-bit
					return _lastBlockIndex * _bitSet->_blockSize;
				}
			} else {
				// new bit index still within this block

				// Return the absolute position of the 1-bit
				return _lastBlockIndex * _bitSet->_blockSize + _lastBlockBitIndex;
			}
		} else if (PWAHBitSet<P>::is_zerofill(currWord, _lastWordPartitionIndex)){
			// Partition contains 0-fill, not interesting! Skip to next partition
			currFillLengthRemaining = PWAHBitSet<P>::fill_length(currWord, _lastWordPartitionIndex);
			if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- encountered 0-fill of length " << currFillLengthRemaining << ", covering " << PWAHBitSet<P>::blocks_num_partitions(currFillLengthRemaining) << " partition(s)" << endl;

			_lastWordPartitionIndex += PWAHBitSet<P>::blocks_num_partitions(currFillLengthRemaining);
			_lastPartitionBlockIndex = 0;
			_lastBlockIndex += currFillLengthRemaining;
			_lastBlockBitIndex = -1;
		} else {
			// Literal: find the next 1-bit
			currPartition = PWAHBitSet<P>::extract_partition(currWord, _lastWordPartitionIndex);
			if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- encountered literal: " << PWAHBitSet<P>::toBitString(currPartition) << endl;

			while(true){
				// Iterate over bits in this block
				_lastBlockBitIndex++;
				if (DEBUGGING) cout << "PWAHBitSetIterator::next() -- checking bit " << _lastBlockBitIndex << " of literal" << endl;

				if (_lastBlockBitIndex >= _bitSet->_blockSize){
					// No more 1-bits in this partition
					break;
				} else {
					if (L_GET_BIT(currPartition, _lastBlockBitIndex)){
						// 1-bit!
						return _lastBlockIndex * _bitSet->_blockSize + _lastBlockBitIndex;
					} // else: 0-bit, not interesting
				}
			}

			// Skip to next partition
			_lastWordPartitionIndex++;
			_lastPartitionBlockIndex = 0;
			_lastBlockBitIndex = -1;
			_lastBlockIndex++;
		}

		if (_lastWordPartitionIndex == P){
			// Next word
			_lastWordPartitionIndex = 0;
			_lastWordIndex++;
		}
	}
}

/**
 * Tell the compiler which instantiations to generate
 */
template class PWAHBitSetIterator<2>;
template class PWAHBitSetIterator<4>;
template class PWAHBitSetIterator<8>;
template class PWAHBitSetIterator<1>;
