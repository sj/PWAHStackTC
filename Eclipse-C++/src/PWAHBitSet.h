/**
 * Copyright 2010-2011 Sebastiaan J. van Schaik
 *
 * This file is part of PWAHStackTC.
 *
 * PWAHStackTC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PWAHStackTC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PWAHStackTC. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PWAHBITSET_H_
#define PWAHBITSET_H_
#include <cstring>
#include <iostream>
#include <vector>
#include "BitSet.h"
#include "BitSetTester.h"
using namespace std;

template<unsigned int P> class PWAHBitSetIterator;

template<unsigned int P>
class PWAHBitSet : public BitSet {
public:
	PWAHBitSet();
	virtual ~PWAHBitSet(){}

	void set(unsigned int bitIndex);
	void set(unsigned int bitIndex, bool value);
	void clear();
	const bool get(unsigned int bitIndex, bool disableIndex);
	const bool get(unsigned int bitIndex);

	const bool incr_get(unsigned int bitIndex);
	void reset_incr_get();

	const unsigned int size();
	static void multiOr(PWAHBitSet** bitSets, unsigned int numBitSets, PWAHBitSet* result);
	static PWAHBitSet* constructByOr(const PWAHBitSet* first, const PWAHBitSet* second);
	static string toBitString(long value);
	static long fill_length(long bits, unsigned short partitionIndex);
	static void setIndexChunkSize(int chunkSize);
	const long memoryUsage();
	string bsImplementationName();

	BitSetIterator* iterator();
	const string toString();
	const int blocksize();

	friend class PWAHBitSetIterator<P>;
	friend class BitSetTester<PWAHBitSet<P> >;

private:
	static const long _maxBlocksPerFill;
	static const int _blockSize;
	static const int _partitionOffsets[P];
	static const bool _VERIFY = false;
	static int _indexChunkSize; // default: don't use indices

	long _plainBlockIndex; // the block index of the plain block
	long _lastBitIndex;
	unsigned short _lastUsedPartition; // partition index of last used partition within the vector of words


	vector<long> _words;
	vector<int> _indexWord;
	vector<unsigned short> _indexPartition;
	vector<int> _indexPartitionOffset;

	unsigned int _incr_get_WordIndex;
	unsigned int _incr_get_PartitionIndex;
	long _incr_get_BlockIndex;

	inline static bool is_fill(long bits, unsigned short partitionIndex);
	static bool is_onefill(long bits, unsigned short partitionIndex);
	static bool is_zerofill(long bits, unsigned short partitionIndex);
	inline static bool is_literal(long bits, unsigned short partitionIndex);
	inline static bool is_literal_onefill(long bits, unsigned short partitionIndex);
	inline static bool is_literal_zerofill(long bits, unsigned short partitionIndex);
	inline static long extract_partition(long bits, unsigned short partitionIndex);
	inline static long clear_partition(long bits, unsigned short partitionIndex);
	static short blocks_num_partitions(int numBlocks);

	void compressPlainPartition();
	void popLastPartition();
	void addFill(bool oneFill, int numBlocks, int firstBlockIndex);
	void addExtendedFillPartitions(bool oneFill, int numBlocks, int firstBlockIndex);
	void addLiteral(long value, int blockIndex);
	void addFillPartition(bool oneFill, long numBlocks, int firstBlockIndex);
	void addPartition(bool isFill, long value);
	void decompressLastBlock();
	void updateIndex(int numBlocks, int firstBlockIndex);
	void setIndexEntry(unsigned int chunkIndex, int indexWord, int indexPartition, int indexPartitionOffset);
	int countNumberOfBlocks();

	const bool _get(unsigned int bitIndex, bool disableIndex, unsigned int initialWordIndex, unsigned int initialPartitionIndex, long initialBlockIndex, bool update_incr_get_indices);
};





#endif /* PWAHBITSET_H_ */
