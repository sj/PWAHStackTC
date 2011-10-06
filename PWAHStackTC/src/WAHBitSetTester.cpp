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

#include "WAHBitSetTester.h"
#include "WAHBitSet.h"
#include "DynamicBitSet.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <bitset>
#include <sstream>
#include "PerformanceTimer.h"
#include "WAHBitSetIterator.h"
#include "BitSetTester.h"
#include <string>
using namespace std;

#define RAND_FLOAT() ((rand() % 1000) / 1000.0)

WAHBitSetTester::WAHBitSetTester() {}
WAHBitSetTester::~WAHBitSetTester() {}

void WAHBitSetTester::testIterator(){
	srand ( time(NULL) );

	while (true){
		WAHBitSet* wbs = new WAHBitSet();
		int maxBits = 5000;
		BitSetTester<WAHBitSet>::randomise(wbs, maxBits);

		cout << wbs->toString() << endl;
		WAHBitSetIterator iter(wbs);
		int nextIndex = iter.next();

		stringstream message;
		for (int i = 0; i < maxBits; i++){
			if (wbs->get(i)){
				// Bit is set, check whether iterator agrees
				if (nextIndex != i){
					message << "Bit with index " << i << " is set, but iterator doesn't think so?";
					throw message.str();
				}
				nextIndex = iter.next();
			} else {
				// Bit is not set
				if (nextIndex == i){
					message << "Bit with index " << i << " is not set, but iterator does think so?";
					throw message.str();
				}
			}
		}

		delete wbs;
	}
}

void WAHBitSetTester::testMultiOr(){
	srand ( time(NULL) );
	PerformanceTimer timer = PerformanceTimer::start();

	int numSourceBitSets = 20;
	int numBits = 560;

	cout << "go" << endl;
	while (true){
		WAHBitSet** bitsets = new WAHBitSet*[numSourceBitSets];

		for (int i = 0; i < numSourceBitSets; i++){
			bitsets[i] = new WAHBitSet();
			BitSetTester<WAHBitSet>::randomise(bitsets[i], numBits);
			cout << "BitSet " << i << endl;
			cout << bitsets[i]->toString() << endl << endl;
		}

		// Compute simple or
		WAHBitSet* simpleOrRes = new WAHBitSet();
		for (int i = 0; i < numSourceBitSets; i++){
			simpleOrRes = WAHBitSet::constructByOr(simpleOrRes, bitsets[i]);
		}

		// Multi or
		WAHBitSet* multiOrRes = new WAHBitSet();
		WAHBitSet::multiOr(bitsets, numSourceBitSets, multiOrRes);

		cout << endl << endl << "Simple OR result:" << endl;
		cout << simpleOrRes->toString() << endl << endl;

		cout << "Multi OR result:" << endl;
		cout << multiOrRes->toString() << endl << endl;

		// Compare results
		if (simpleOrRes->_compressedBits.size() != multiOrRes->_compressedBits.size()){
			throw string("Sizes do not match!");
		}
		for (unsigned int i = 0; i < simpleOrRes->_compressedBits.size(); i++){
			if (simpleOrRes->_compressedBits[i] != multiOrRes->_compressedBits[i]){
				throw string("Compressed bits mismatch");
			}
		}

		cout << "ALL OK!" << endl;

		delete simpleOrRes;
		delete multiOrRes;
		delete[] bitsets;
	}

	cout << "array done" << endl;
	WAHBitSet** bitsets = new WAHBitSet*[2];
	bitsets[0] = new WAHBitSet();
	bitsets[0]->addZeroFill(2); // 0-fill of size 2
	bitsets[0]->addLiteral(2147483647); // 1-fill as plain word
	bitsets[0]->decompressLastWord();

	cout << "done init 0:" << endl;
	cout << bitsets[0]->toString() << endl;

	bitsets[1] = new WAHBitSet();
	bitsets[1]->addLiteral(581261521); // 10001011000110101010010101000100
	bitsets[1]->decompressLastWord();

	cout << "done init 1" << endl;
	cout << bitsets[1]->toString() << endl;

	WAHBitSet* resultMulti = new WAHBitSet();

	cout << "starting multior" << endl;
	WAHBitSet::multiOr(bitsets, 2, resultMulti);
	cout << "done multior" << endl;

	cout << "starting simpleor" << endl;
	WAHBitSet* resultSimple = WAHBitSet::constructByOr(bitsets[0], bitsets[1]);
	cout << "done simpleor" << endl;

	cout << "Multi-or result:" << endl;
	cout << resultMulti->toString() << endl;

	cout << endl << endl << "single or result:" << endl;
	cout << resultSimple->toString() << endl;

	cout << "done!" << endl;
}


void WAHBitSetTester::testOr(){
	srand ( time(NULL) );
	PerformanceTimer timer = PerformanceTimer::start();

	while(true){
		//WAHBitSet wahbs1, wahbs2;
		cout << "Creating 2 random DynamicBitSet objects... ";
		cout.flush();
		DynamicBitSet* dbs1 = new DynamicBitSet(); //(wahbs1);
		DynamicBitSet* dbs2 = new DynamicBitSet(); //(wahbs2);
		BitSetTester<WAHBitSet>::randomise(dbs1, NULL, 100000);
		BitSetTester<WAHBitSet>::randomise(dbs2, NULL, 100000);
		cout << "done, that took " << timer.reset() << " msecs" << endl;
		//wahbs1 = WAHBitSet::constructFailingExample1();
		//wahbs2 = WAHBitSet::constructFailingExample2();
		cout << "Creating 2 WAHBitSet objects from generated DynamicBitSet objects... ";
		cout.flush();
		WAHBitSet wahbs1(dbs1);
		WAHBitSet wahbs2(dbs2);
		cout << "done, that took " << timer.reset() << " msecs" << endl;

		/**cout << "Comparing DynamicBitSets and WAHBitSets... ";
		cout.flush();
		if (!wahbs1.equals(dbs1)){
			throw string("bitset1 mismatch");
		}
		if (!wahbs2.equals(dbs2)){
			throw string("bitset2 mismatch");
		}
		cout << "done, that took " << timer.reset() << " msecs" << endl;**/


		cout << "Merging two bitsets of sizes " << wahbs1.size() << " and " << wahbs2.size() << "..." << endl;
		cout.flush();
		timer.reset();
		WAHBitSet* wahres = WAHBitSet::constructByOr(&wahbs1, &wahbs2);
		cout << "WAHBitSets merged, that took " << timer.reset() << " msecs" << endl;
		DynamicBitSet* dbsres = DynamicBitSet::constructByOr(dbs1, dbs2);
		cout << "DynamicBitSets merged, that took " << timer.reset() << " msecs" << endl;

		cout << "Comparing merge results... ";
		cout.flush();
		if (!wahres->equals(dbsres)){
			cerr << "=== INPUT 1 (compressed) ===" << endl;
			cerr << wahbs1.toString() << endl << endl;
			cerr << "=== INPUT 2 (compressed) ===" << endl;
			cerr << wahbs2.toString() << endl << endl << endl;

			cerr << "=== INPUT 1 (plain) ===" << endl;
			cerr << dbs1->toString() << endl << endl;
			cerr << "=== INPUT 2 (plain) ===" << endl;
			cerr << dbs2->toString() << endl;
			throw string("MERGE FAIL!");
		}
		cout << "done, that took " << timer.reset() << " msecs" << endl << endl;
		delete dbs1;
		delete dbs2;
		delete wahres;
		delete dbsres;

		//cout << wahres.toString() << endl;
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

