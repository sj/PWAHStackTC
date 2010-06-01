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

template<unsigned int P>
class PWAHBitSet : public BitSet {
public:
	PWAHBitSet();
	virtual ~PWAHBitSet(){}

	void set(int bitIndex);
	void set(int bitIndex, bool value);
	void clear();
	bool get(int bitIndex);
	unsigned int size();

private:
	static const long _maxBlocksPerFill;
	static const int _blockSize;
	static const int _partitionOffsets[P];

	int _lastBitSet;
	int _lastUsedPartition; // partition index of last used partition
	int _plainBlockIndex; // the block index of the plain block
	long _plainBlock;
	vector<long> _compressedBlocks;

	inline static bool is_fill(long bits, unsigned short partitionIndex);
	inline static bool is_onefill(long bits, unsigned short partitionIndex);
	inline static bool is_zerofill(long bits, unsigned short partitionIndex);
	inline static bool is_literal(long bits, unsigned short partitionIndex);
	inline static long fill_length(long bits, unsigned short partitionIndex);

	void compressPlainBlock();
	void addOneFill(int numBlocks);
	void addZeroFill(int numBlocks);
	void addLiteral(long value);
	void addPartition(bool isFill, long value);
};



#endif /* PWAHBITSET_H_ */
