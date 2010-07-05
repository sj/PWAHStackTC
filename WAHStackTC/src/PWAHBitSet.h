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
	PWAHBitSet();
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

	BitSetIterator* iterator();
	const string toString();
	const int blocksize();

	friend class PWAHBitSetIterator<P>;

private:
	static const long _maxBlocksPerFill;
	static const int _blockSize;
	static const int _partitionOffsets[P];
	static const bool _VERIFY = true;

	int _lastUsedPartition; // partition index of last used partition within the last compressed word
	long _plainBlockIndex; // the block index of the plain block
	long _plainBlock;
	long _lastBitIndex;
	vector<long> _compressedWords;

	inline static bool is_fill(long bits, unsigned short partitionIndex);
	static bool is_onefill(long bits, unsigned short partitionIndex);
	static bool is_zerofill(long bits, unsigned short partitionIndex);
	inline static bool is_literal(long bits, unsigned short partitionIndex);
	inline static bool is_literal_onefill(long bits, unsigned short partitionIndex);
	inline static bool is_literal_zerofill(long bits, unsigned short partitionIndex);
	static long extract_partition(long bits, unsigned short partitionIndex);
	inline static long clear_partition(long bits, unsigned short partitionIndex);

	void compressPlainBlock();
	void popLastPartition();
	void addFill(bool oneFill, int numBlocks);
	void addExtendedFill(bool oneFill, int numBlocks);
	void addLiteral(long value);
	void addFillPartition(bool oneFill, int numBlocks);
	void addPartition(bool isFill, long value);
	void decompressLastBlock();
	int countNumberOfBlocks();
};





#endif /* PWAHBITSET_H_ */
