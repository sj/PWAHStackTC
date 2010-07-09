/*
 * VWAHBitSet.h
 *
 *  Created on: May 25, 2010
 *      Author: bas
 */

#ifndef PWAHBITSET_H_
#define PWAHBITSET_H_
#include <cstring>
#include <iostream>
#include <vector>
#include "BitSet.h"
using namespace std;

template<unsigned int P> class PWAHBitSetIterator;

template<unsigned int P>
class PWAHBitSet : public BitSet {
public:
	PWAHBitSet(int indexIntervalSize = -1);
	virtual ~PWAHBitSet(){}

	void set(int bitIndex);
	void set(int bitIndex, bool value);
	void clear();
	const bool get(int bitIndex);
	const unsigned int size();
	static void multiOr(PWAHBitSet** bitSets, unsigned int numBitSets, PWAHBitSet* result);
	static PWAHBitSet* constructByOr(const PWAHBitSet* first, const PWAHBitSet* second);
	static string toBitString(long value);
	static long fill_length(long bits, unsigned short partitionIndex);
	const long memoryUsage();
	string bsImplementationName();

	BitSetIterator* iterator();
	const string toString();
	const int blocksize();

	friend class PWAHBitSetIterator<P>;
	friend class BitSetTester;

private:
	static const long _maxBlocksPerFill;
	static const int _blockSize;
	static const int _partitionOffsets[P];
	static const bool _VERIFY = false;

	unsigned short _lastUsedPartition; // partition index of last used partition within the vector of words
	long _plainBlockIndex; // the block index of the plain block
	long _lastBitIndex;
	vector<long> _words;
	const int _indexChunkSize;
	vector<int> _indexWord;
	vector<unsigned short> _indexPartition;
	vector<int> _indexPartitionOffset;

	inline static bool is_fill(long bits, unsigned short partitionIndex);
	static bool is_onefill(long bits, unsigned short partitionIndex);
	static bool is_zerofill(long bits, unsigned short partitionIndex);
	inline static bool is_literal(long bits, unsigned short partitionIndex);
	inline static bool is_literal_onefill(long bits, unsigned short partitionIndex);
	inline static bool is_literal_zerofill(long bits, unsigned short partitionIndex);
	inline static long extract_partition(long bits, unsigned short partitionIndex);
	inline static long clear_partition(long bits, unsigned short partitionIndex);
	inline static short blocks_num_partitions(int numBlocks);

	void compressPlainPartition();
	void popLastPartition();
	void addFill(bool oneFill, int numBlocks, int firstBlockIndex);
	void addExtendedFillPartitions(bool oneFill, int numBlocks, int firstBlockIndex);
	void addLiteral(long value, int blockIndex);
	void addFillPartition(bool oneFill, int numBlocks, int firstBlockIndex);
	void addPartition(bool isFill, long value);
	void decompressLastBlock();
	void updateIndex(int numBlocks, int firstBlockIndex);
	void setIndexEntry(int chunkIndex, int indexWord, int indexPartition, int indexPartitionOffset);
	int countNumberOfBlocks();
};





#endif /* PWAHBITSET_H_ */
