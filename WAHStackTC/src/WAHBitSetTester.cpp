/*
 * WAHBitSetTester.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: bas
 */

#include "WAHBitSetTester.h"
#include "WAHBitSet.h"
#include "DynamicBitSet.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <bitset>
#include <sstream>
using namespace std;

#define RAND_FLOAT() ((rand() % 1000) / 1000.0)

WAHBitSetTester::WAHBitSetTester() {}
WAHBitSetTester::~WAHBitSetTester() {}

void WAHBitSetTester::testOr(){
	srand ( time(NULL) );

	WAHBitSet bs1, bs2;
	randomize(bs1);
	randomize(bs2);

	WAHBitSet res = WAHBitSet::constructByOr(bs1, bs2);

	cout << res.toString() << endl;
}

void WAHBitSetTester::randomize(WAHBitSet& bitset, int maxBits){
	bitset.clear();

	// Probability of constructing a fill-bit
	float fillProb = RAND_FLOAT();

	// Probability a fill bit is a 1-fill
	float oneFillProb = RAND_FLOAT();

	// Fill the bit set with a random number of bits, somewhere between 0 and a big number
	//int numBlocks = (rand() % 10000000) / 31;
	int numBits = 4 * 31 + (rand() % maxBits);
	int numBlocks = numBits / 31 + 1;
	int offset;

	for (int block = 0; block < numBlocks; block++){
		offset = block * 31;

		if (RAND_FLOAT() <= fillProb){
			// Construct a block in such a way that a fill will occur
			if (RAND_FLOAT() <= oneFillProb){
				// 1-fill
				for (int bit = offset; bit < offset + 31; bit++) bitset.set(bit);
			} // else: 0-fill, do nothing
		} else {
			// Construct a block with random bits
			if (DEBUGGING) cout << "Constructing block " << block << ": will be stored as literal word" << endl;
			for (int bit = offset; bit < offset + 31; bit++) if (RAND_FLOAT() < 0.5) bitset.set(bit);
		}
	}
}



void WAHBitSetTester::test(int runs){
	srand ( time(NULL) );
	int runCount = 0;
	while (true){
		// Probability of constructing a fill-bit
		float fillProb = RAND_FLOAT();

		// Probability a fill bit is a 1-fill
		float oneFillProb = RAND_FLOAT();

		// Fill the bit set with a random number of bits, somewhere between 0 and 10,000,000
		//int numBlocks = (rand() % 10000000) / 31;
		int numBits = 4 * 31 + (rand() % 10000);
		int numBlocks = numBits / 31 + 1;
		int offset;

		if (DEBUGGING) cout << "Generating random bitset consisting of " << numBits << " bits (numBlocks=" << numBlocks << ", fillProb=" << fillProb << ", oneFillProb=" << oneFillProb << ")" << endl;

		WAHBitSet wahBitset;
		DynamicBitSet dynaBitset;

		for (int block = 0; block < numBlocks; block++){
			offset = block * 31;

			if (RAND_FLOAT() <= fillProb){
				// Construct a block in such a way that a fill will occur
				if (RAND_FLOAT() <= oneFillProb){
					// 1-fill
					if (DEBUGGING) cout << "Constructing block " << block << ": suitable for compression as 1-fill" << endl;
					for (int bit = offset; bit < offset + 31; bit++){
						wahBitset.set(bit);
						dynaBitset.set(bit);
					}
				} else {
					// else: 0-fill, do nothing
					if (DEBUGGING) cout << "Constructing block " << block << ": suitable for compression as 0-fill" << endl;
					for (int bit = offset; bit < offset + 31; bit++){
						wahBitset.set(bit, false);
						dynaBitset.set(bit, false);
					}
				}
			} else {
				// Construct a block with random bits
				if (DEBUGGING) cout << "Constructing block " << block << ": will be stored as literal word" << endl;
				for (int bit = offset; bit < offset + 31; bit++){
					if (RAND_FLOAT() < 0.5){
						wahBitset.set(bit);
						dynaBitset.set(bit);
					} else {
						dynaBitset.set(bit,false);
					}
				}
			}
		}

		if (DEBUGGING){
			cout << endl << "Resulting compressed BitSet:" << endl;
			cout << wahBitset.toString() << endl;

			cout << endl << "Resulting dynamic BitSet:" << endl;
			cout << dynaBitset.toString() << endl;
		}

		// Compare
		for (int i = 0; i < numBits; i++){
			if (dynaBitset.get(i) != wahBitset.get(i)){
				cout << endl << "Resulting compressed BitSet:" << endl;
				cout << wahBitset.toString() << endl;

				cout << endl << "Resulting dynamic BitSet:" << endl;
				cout << dynaBitset.toString() << endl;

				stringstream stream;
				stream << "Inconsistency at bit index " << i << ": ";
				if (dynaBitset.get(i)) stream << "DynamicBitSet seems to think it is set, WAHBitSet says otherwise?";
				else stream << "WAHBitSet seems to think it is set, DynamicBitSet says otherwise?";
				throw stream.str();
			}
		}

		cout << ".";
		cout.flush();
		if (runCount > 0 && runCount % 100 == 0){
			cout <<  " " << runCount << " tests passed" << endl;
			cout.flush();
		}

		runCount++;
		if (runs > 0 && runCount >= runs){
			cout <<  " " << runCount << " tests passed" << endl;
			break;
		}
	}
}

