/*
 * DynamicBitSet.h
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#ifndef DYNAMICBITSET_H_
#define DYNAMICBITSET_H_
#include <vector>
#include <string>
using namespace std;

class DynamicBitSet {
private:
	vector<long> _vec;

public:
	DynamicBitSet(int initialCapacity);
	DynamicBitSet();
	virtual ~DynamicBitSet();

	void set(int bitIndex);
	void set(int bitIndex, bool value);
	bool get(int bitIndex);
	string toString();
	string toBitString(long value);

	void constructFailingExample();
};

#endif /* DYNAMICBITSET_H_ */
