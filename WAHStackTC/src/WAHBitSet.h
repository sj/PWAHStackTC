/*
 * WAHBitSet.h
 *
 *  Created on: Apr 10, 2010
 *      Author: bas
 */

#ifndef WAHBITSET_H_
#define WAHBITSET_H_
#include <vector>
using namespace std;

class WAHBitSet {
private:
	vector<int> _compressedBits;
	int _plainWord;
	int _plainWordOffset;

public:
	static const bool DEBUGGING = true;
	static const int WORDSIZE = 32;
	static const int BLOCKSIZE = WORDSIZE - 1;

	// Maximum number of blocks in a 0-fill or 1-fill
	static const int MAX_BLOCKS_IN_FILL = 1073741824; // 2^30

	// Block (31 bits) of ones, preceded by a zero
	static const int BLOCK_ONES = 0b01111111111111111111111111111111;

	// Block (31 bits) of zeroes
	static const int BLOCK_ZEROES = 0b00000000000000000000000000000000;

	// 1-fill: first two bits set to 1
	static const int ONEFILL = 0b11000000000000000000000000000000;

	// 0-fill: first bit set to 1, second to 0
	static const int ZEROFILL = 0b10000000000000000000000000000000;

	// 1-fill of length 1
	static const int SIMPLE_ONEFILL = 0b11000000000000000000000000000001;

	// 0-fill of length 1
	static const int SIMPLE_ZEROFILL = 0b10000000000000000000000000000001;

	WAHBitSet();
	WAHBitSet(int initialCapacity);
	virtual ~WAHBitSet();

	void set(int bitIndex);
	void set(int bitIndex, bool value);
	bool get(int bitIndex);
	string toString();

	static string toBitString(int value);
};

//const int WAHBitSet::SIMPLE_ZEROFILL;
//const int WAHBitSet::SIMPLE_ONEFILL;
//WAHBitSet::SIMPLE_ONEFILL = 0b11000000000000000000000000000001;

#endif /* WAHBITSET_H_ */
