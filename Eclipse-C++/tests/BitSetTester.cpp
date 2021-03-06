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

#include "BitSetTester.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "../src/datastructures/bitsets/pwah/PWAHBitSet.h"
#include "../src/datastructures/bitsets/wah/WAHBitSet.h"
#include "../src/datastructures/bitsets/DynamicBitSet.h"
#include "../src/datastructures/Graph.h"
#include "../src/algorithms/PWAHStackTC.h"
#include <math.h>
#include "../src/datastructures/bitsets/interval/IntervalBitSet.h"
#include "GraphTestInfo.h"
using namespace std;


template<class B> BitSetTester<B>::~BitSetTester() {}


TYPED_TEST(BitSetTester, TestGraphsFromDataset) {
	vector<GraphTestInfo> graphs = GraphTestInfo::getJin2008Graphs();

	for (unsigned int i = 0; i < graphs.size(); i++){
		Graph g = Graph::parseChacoFile(graphs[i].getFullFilename());
		PWAHStackTC<TypeParam>* pwahstc_b = new PWAHStackTC<TypeParam>(g);
		PWAHStackTC<WAHBitSet>* pwahstc_wah = new PWAHStackTC<WAHBitSet>(g);

		pwahstc_b->computeTransitiveClosure(false, false, 0);
		pwahstc_wah->computeTransitiveClosure(false, false, 0);

		//cout << pwahstc_b->algorithmName() <<  " uses " << pwahstc_b->memoryUsedByBitSets() << " bits" << endl;
		//cout << pwahstc_wah->algorithmName() << " uses " << pwahstc_wah->memoryUsedByBitSets() << " bits" << endl;

		const vector<TypeParam*> ref_b = pwahstc_b->getComponentSuccessors();
		const vector<WAHBitSet*> wah = pwahstc_wah->getComponentSuccessors();

		EXPECT_EQ(ref_b.size(), wah.size());

		if (ref_b.size() != wah.size()){
			//cout << "Number of bitsets doesn't: " << ref_b.size() << " versus " << wah.size() << endl;
		} else {
			//cout << "Sizes match: " << ref_b.size() << " bit sets" << endl;
			for (unsigned int i = 0; i < ref_b.size(); i++){
				if (ref_b[i] == NULL) continue;

				BitSetTester<TypeParam>::compare(ref_b[i], wah[i]);
			}
		}
		//cout << "done" << endl;
		delete pwahstc_b;
		delete pwahstc_wah;
	}
}

TYPED_TEST(BitSetTester, TestMultiOr) {
	const unsigned int numBitSets = 10;
	TypeParam* b_result = new TypeParam();
	//cout << "BitSetTester::testOr -- testing multi OR on " << numBitSets << " bitsets of type " << b_result->bsImplementationName() << endl;
	TypeParam** b_bitsets = new TypeParam*[numBitSets];
	WAHBitSet** wah_bitsets = new WAHBitSet*[numBitSets];
	WAHBitSet* wah_result = new WAHBitSet();

	for (unsigned int i = 0; i < numBitSets; i++){
		//cout << "BitSetTester::testOr -- randomising BitSets " << i << "... ";
		//cout.flush();

		wah_bitsets[i] = new WAHBitSet();
		b_bitsets[i] = new TypeParam();
		BitSetTester<TypeParam>::randomise(b_bitsets[i], wah_bitsets[i], 3000, 15000);

		BitSetTester<TypeParam>::compare(b_bitsets[i], wah_bitsets[i]);
		//cout << "done" << endl;
	}

	TypeParam::multiOr(b_bitsets, numBitSets, b_result);
	WAHBitSet::multiOr(wah_bitsets, numBitSets, wah_result);

	//cout << "Results of OR operation:" << endl;
	//cout << printBitSets(pwbs_res, dbs_res);

	BitSetTester<TypeParam>::compare(b_result, wah_result);

	delete[] b_bitsets;
	delete[] wah_bitsets;
	delete wah_result;
	delete b_result;
}

TYPED_TEST(BitSetTester, TestSetGet) {
	TypeParam* tested_bitset = new TypeParam();
	WAHBitSet* ref_bitset = new WAHBitSet();

	for (int i = 1; i < 50; i++){
		BitSetTester<TypeParam>::randomise(tested_bitset, ref_bitset, 1000, 2500);
		//cout << "Done randomising pass " << i << endl;

		BitSetTester<TypeParam>::compare(tested_bitset, ref_bitset);
		//cout << "Done testing get/set pass " << i << endl;
	}
	//cout << printBitSets(_bs1, _bs2);

	//cout << "Done!" << endl;
}

TYPED_TEST(BitSetTester, TestIncrementalSetGet) {
	TypeParam* tested_bitset = new TypeParam();
	WAHBitSet* ref_bitset = new WAHBitSet();

	for (int i = 1; i < 50; i++){
		BitSetTester<TypeParam>::randomise(tested_bitset, ref_bitset, 1000, 2500);

		// Force use of incr_get (will only work on PWAHBitSet classes) to see whether it behaves
		for (unsigned int b = 0; b < tested_bitset->size(); b++){
			EXPECT_EQ(tested_bitset->incr_get(b), ref_bitset->get(b));
		}
	}
}

TYPED_TEST(BitSetTester, TestIterator) {
	TypeParam* bitset = new TypeParam();
	const unsigned int minNumRandomBits = 10000;
	//cout << "BitSetTester::testIterator() -- starting..." << endl;
	BitSetTester<TypeParam>::randomise(bitset, minNumRandomBits, minNumRandomBits * 5);
	//cout << bitset->toString() << endl;

	EXPECT_GE(bitset->size(), minNumRandomBits);
	BitSetIterator* it = bitset->iterator();
	bool value;
	for (unsigned int i = 0; i < bitset->size(); i++){
		value = bitset->get(i);
		if (value){
			//cout << "BitSetTester::testIterator() -- bit " << i << " should be set..." << endl;
			if (it->next() != (int)i){
				cout.flush();
				cerr << "BitSet causing problems:" << endl;
				cerr << bitset->toString() << endl;
				cerr << "Bit with index " << i << " " << (value ? "should be set, but is not according to iterator" : "should be 0, but is 1 according to iterator") << endl;
				cerr.flush();
				throw string("iterator does not agree with bitset?!");
			}
		}
	}

	int nextVal = it->next();
	if (nextVal != -1){
		cerr << "Iterator returns bonus bit " << nextVal << endl;
		cerr.flush();
		throw string("iterator has more elements?!");
	}

	delete it;
	//cout << "BitSetTester::testIterator() -- done" << endl;
}

/**
 * Helper method to compare two BitSet implementations using GTest EXPECT_EQ calls
 */
template<class B> void BitSetTester<B>::compare(BitSet* bitset1, BitSet* bitset2){
	EXPECT_EQ(bitset1->size(), bitset2->size());

	if (bitset1->size() != bitset2->size()){
		cerr << printBitSets(bitset1, bitset2) << endl;
		stringstream str;
		str << "Sizes don't match: ";
		str << "bitset1->size() = " << bitset1->size() << ", ";
		str << "bitset2->size() = " << bitset2->size() << endl;
		cout.flush();
		cerr << str.str();
		cerr.flush();
		throw str.str();
	}
	//cout << "Comparing BitSets of size " << bitset1->size() << endl;

	bool val1, val2;
	for (unsigned int i = 0; i < bitset1->size(); i++){
		val1 = bitset1->get(i);
		val2 = bitset2->get(i);

		EXPECT_EQ(val1, val2);

		if (val1 != val2){
			cout.flush();
			cerr << printBitSets(bitset1, bitset2) << endl;

			stringstream str;
			str << "BitSets disagree on bit " << i << ": ";
			str << "bitset1[" << i << "] = " << (val1 ? "true" : "false") << ", ";
			str << "bitset2[" << i << "] = " << (val2 ? "true" : "false");

			cerr << str.str() << endl;
			throw str.str();
		}

		//cout << "Bit " << i << ": " << (val1 ? "set" : "not set") << endl;
	}

	//cout << "Done comparing!" << endl;
}


/**
 * \brief Randomises the bits in 'bitset' and 'bitset2'
 *
 * The first BitSet is used as 'base' for defining block sizes. The second BitSet can be NULL
 */
template<class B> void BitSetTester<B>::randomise(BitSet* bitset1, BitSet* bitset2, unsigned int minBits, unsigned int maxBits){
	const bool DEBUGGING = false;
	assert(minBits <= maxBits);

	srand ( time(NULL) );

	bitset1->clear();
	if (bitset2 != NULL) bitset2->clear();

	// Probability of constructing a fill-bit
	float fillProb = rand_float();

	// Probability a fill bit is a 1-fill
	float oneFillProb = rand_float();

	// Fill the bit set with a random number of bits, somewhere between 0 and a big number
	//int numBlocks = (rand() % 10000000) / 31;
	int blockSize = bitset1->blocksize() > 0 ? bitset1->blocksize() : 64;

	unsigned int numAdditionalBits = 0;
	if (maxBits - minBits > 0) numAdditionalBits = rand() % (maxBits - minBits);
	const unsigned int numBits = minBits + numAdditionalBits;

	int numBlocks = numBits / blockSize + 1;
	int offset;

	if (DEBUGGING) cout << "BitSetTester::randomise -- Setting " << numBlocks << " blocks consisting of " << bitset1->blocksize() << " bits each." << endl;
	for (int block = 0; block < numBlocks; block++){
		offset = block * blockSize;

		if (rand_float() <= fillProb){
			// Construct a block in such a way that a fill will occur
			bool onefill = rand_float() <= oneFillProb;
			if (DEBUGGING) cout << "BitSetTester::randomise -- Constructing block " << block << ": will be stored as " << (onefill ? "1" : "0") << "-fill word" << endl;

			for (int bit = offset; bit < offset + blockSize; bit++){
				bitset1->set(bit, onefill);
				if (bitset2 != NULL) bitset2->set(bit, onefill);
			}
		} else {
			// Construct a block with random bits
			if (DEBUGGING) cout << "BitSetTester::randomise -- Constructing block " << block << ": will be stored as literal word" << endl;
			for (int bit = offset; bit < offset + blockSize; bit++){
				bool value = rand_float() < 0.5;
				bitset1->set(bit, value);
				if (bitset2 != NULL) bitset2->set(bit, value);
			}
		}
	}

	if (DEBUGGING){
		cout << "BitSetTester::randomise -- done randomising!" << endl;
		cout << "BitSetTester::randomise -- result: " << endl;
		cout << bitset1->toString() << endl;
	}

	ASSERT_GE(bitset1->size(), minBits);
	if (bitset2 != NULL) ASSERT_GE(bitset2->size(), minBits);
}

template<class B> string BitSetTester<B>::printBitSets(BitSet* bitset1, BitSet* bitset2){
	BitSet** bitsets = new BitSet*[2];
	bitsets[0] = bitset1;
	bitsets[1] = bitset2;
	return printBitSets(bitsets, 2);
}

template<class B> string BitSetTester<B>::printBitSets(BitSet* bitset1, BitSet* bitset2, BitSet* bitset3){
	BitSet** bitsets = new BitSet*[3];
	bitsets[0] = bitset1;
	bitsets[1] = bitset2;
	bitsets[1] = bitset3;
	return printBitSets(bitsets, 3);
}

template<class B> string BitSetTester<B>::printBitSets(BitSet** bitSets, int numBitSets){
	stringstream res;

	for (int i = 0; i < numBitSets; i++){
		res << "BitSet " << i << ": " << bitSets[i]->bsImplementationName() << endl;
		res << bitSets[i]->toString() << endl << endl;
	}
	return res.str();
}

template<class B> void BitSetTester<B>::randomise(BitSet* bitset1, unsigned int minBits, unsigned int maxBits){
	randomise(bitset1, NULL, minBits, maxBits);
}

/**
 * \brief Returns a random number between 0 and 1.
 *
 * Note that this function uses modulo calculation to generate the random number, which will
 * result in a very slight bias.
 */
template<class B> float BitSetTester<B>::rand_float(){
	return (rand() % 1000) / 1000.0;
}


template class BitSetTester<IntervalBitSet>;
template class BitSetTester<WAHBitSet>;

template class BitSetTester<PWAHBitSet<2> >;
template class BitSetTester<PWAHBitSet<4> >;
template class BitSetTester<PWAHBitSet<8> >;
