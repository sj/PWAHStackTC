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
using namespace std;

template<unsigned int P>
class PWAHBitSet  {
public:
	PWAHBitSet();
	virtual ~PWAHBitSet(){}
	void set(int bitIndex);

private:
	static const int _blockSize;
	static const int _partitionOffsets[P];
	vector<long> _compressedBlocks;
	long _plainBlock;
	int _plainBlockIndex; // the block index of the plain block

	inline static bool is_fill(long bits, unsigned short partitionIndex);
	inline static bool is_onefill(long bits, unsigned short partitionIndex);
	inline static bool is_zerofill(long bits, unsigned short partitionIndex);
	inline static bool is_literal(long bits, unsigned short partitionIndex);
};



#endif /* PWAHBITSET_H_ */
