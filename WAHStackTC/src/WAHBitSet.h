/*
 * WAHBitSet.h
 *
 *  Created on: Apr 10, 2010
 *      Author: bas
 */

#ifndef WAHBITSET_H_
#define WAHBITSET_H_
#include <vector>
#include <string>
#include "BitSet.h"
#include "DynamicBitSet.h"
#include "WAHBitSetIterator.h"
using namespace std;

class WAHBitSet : public BitSet {
private:
	int _lastBitIndex;
	vector<int> _compressedBits;
	int _plainWord;
	int _plainWordBlockSeq;
	bool _empty;

	void addOneFill(int numBlocks);
	void addZeroFill(int numBlocks);
	void addLiteral(int value);
	void init();
	void decompressLastWord();

public:
	static const bool DEBUGGING = false;
	static const unsigned int WORDSIZE = 32;
	static const unsigned int BLOCKSIZE = WORDSIZE - 1;

	// Maximum number of blocks in a 0-fill or 1-fill
	static const int MAX_BLOCKS_IN_FILL = 1073741823; // 2^30 - 1

	// Literal word (32 bits) consisting of only ones. That's 31 ones,
	// succeeded by a 0.
	static const int BLOCK_ONES = 0b01111111111111111111111111111111;

	// Block (31 bits) of zeroes
	static const int BLOCK_ZEROES = 0b00000000000000000000000000000000;

	// 1-fill: first two bits set to 1
	static const int EMPTY_ONEFILL = 0b11000000000000000000000000000000;

	// 0-fill: first bit set to 1, second to 0
	static const int EMPTY_ZEROFILL = 0b10000000000000000000000000000000;

	// 1-fill of length 1
	static const int SIMPLE_ONEFILL = 0b11000000000000000000000000000001;

	// 0-fill of length 1
	static const int SIMPLE_ZEROFILL = 0b10000000000000000000000000000001;

	WAHBitSet(DynamicBitSet* dynamicBitSet);
	WAHBitSet();
	virtual ~WAHBitSet();


	void setBits(int blockSeq, int value);
	void set(int bitIndex);
	void set(int bitIndex, bool value);
	const bool get(int bitIndex);
	const string toString();
	const unsigned int size();
	bool isEmpty();

	static int generateRandomLiteralBlock();
	static string toBitString(int value);
	static WAHBitSet* constructByOr(const WAHBitSet* first, const WAHBitSet* second);
	static void multiOr(WAHBitSet** bitSets, unsigned int numBitSets, WAHBitSet* result);

	void clear();
	const int blocksize();
	BitSetIterator* iterator();

	static WAHBitSet constructFailingExample1();
	static WAHBitSet constructFailingExample2();

	long memoryUsage();

	friend class WAHBitSetIterator;
	friend class WAHBitSetTester;
	friend class BitSetTester;
};

#endif /* WAHBITSET_H_ */
