/*
 * BitSetTester.cpp
 *
 *  Created on: Jun 7, 2010
 *      Author: bas
 */

#include "BitSetTester.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "PWAHBitSet.h"
#include "WAHBitSet.h"
#include "DynamicBitSet.h"
using namespace std;

BitSetTester::BitSetTester(BitSet* bs1, BitSet* bs2) {
	srand ( time(NULL) );
	_bs1 = bs1;
	_bs2 = bs2;
}

BitSetTester::~BitSetTester() {
	// TODO Auto-generated destructor stub
}

/**
 * \brief Returns a random number between 0 and 1.
 *
 * Note that this function uses modulo calculation to generate the random number, which will
 * result in a very slight bias.
 */
float BitSetTester::rand_float(){
	return (rand() % 1000) / 1000.0;
}

void BitSetTester::testOr(){
	PWAHBitSet<2>* pwbs1 = new PWAHBitSet<2>();
	PWAHBitSet<2>* pwbs2 = new PWAHBitSet<2>();
	PWAHBitSet<2>* pwbs3 = new PWAHBitSet<2>();
	PWAHBitSet<2>* pwbs_res = new PWAHBitSet<2>();
	WAHBitSet *dbs1, *dbs2, *dbs3, *dbs_tempres, *dbs_res;

	dbs1 = new WAHBitSet();
	dbs2 = new WAHBitSet();
	dbs3 = new WAHBitSet();
	dbs_res = new WAHBitSet();

	randomise(pwbs1, dbs1, 50000);
	randomise(pwbs2, dbs2, 60000);
	randomise(pwbs3, dbs3, 70000);

	PWAHBitSet<2>** pw_bitsets = new PWAHBitSet<2>*[3];
	pw_bitsets[0] = pwbs1;
	pw_bitsets[1] = pwbs2;
	pw_bitsets[2] = pwbs3;

	WAHBitSet** w_bitsets = new WAHBitSet*[3];
	w_bitsets[0] = dbs1;
	w_bitsets[1] = dbs2;
	w_bitsets[2] = dbs3;

	//cout << "Input to MultiOR:" << endl;
	//cout << printBitSets(pwbs1, pwbs2, pwbs3);
	//cout.flush();

	PWAHBitSet<2>::multiOr(pw_bitsets, 3, pwbs_res);
	WAHBitSet::multiOr(w_bitsets, 3, dbs_res);
	//dbs_tempres = WAHBitSet::constructByOr(dbs1, dbs2);
	//dbs_res = WAHBitSet::constructByOr(dbs_tempres, dbs3);

	//cout << "Results of OR operation:" << endl;
	//cout << printBitSets(pwbs_res, dbs_res);

	compare(pwbs_res, dbs_res);

	delete pwbs1;
	delete pwbs2;
	delete pwbs3;
	delete pwbs_res;

	delete dbs1;
	delete dbs2;
	delete dbs3;
	delete dbs_res;

}

void BitSetTester::testSetGet(){
	for (int i = 1; i < 100; i++){
		randomise(_bs1, _bs2, 102400);
		cout << "Done randomising pass " << i << endl;

		compare(_bs1, _bs2);
		cout << "Done testing get/set pass " << i << endl;
	}
	//cout << printBitSets(_bs1, _bs2);

	cout << "Done!" << endl;
}

void BitSetTester::testIterator(BitSet* bitset, bool randomiseBitset){
	cout << "BitSetTester::testIterator() -- starting..." << endl;
	if (randomiseBitset) randomise(bitset, 15000);

	BitSetIterator* it = bitset->iterator();
	bool value;
	for (unsigned int i = 0; i < bitset->size(); i++){
		value = bitset->get(i);
		if (value){
			//cout << "BitSetTester::testIterator() -- bit " << i << " should be set..." << endl;
			if (it->next() != i){
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
	cout << "BitSetTester::testIterator() -- done" << endl;
}

void BitSetTester::compare(BitSet* bitset1, BitSet* bitset2){
	if (bitset1->size() != bitset2->size()){
		cerr << printBitSets(bitset1, bitset2) << endl;
		stringstream str;
		str << "Sizes don't match: ";
		str << "bitset1->size() = " << bitset1->size() << ", ";
		str << "bitset2->size() = " << bitset2->size() << endl;
		throw str.str();
	}
	cout << "Comparing BitSets of size " << bitset1->size() << endl;

	bool val1, val2;
	for (unsigned int i = 0; i < bitset1->size(); i++){
		val1 = bitset1->get(i);
		val2 = bitset2->get(i);

		if (val1 != val2){
			cerr << printBitSets(bitset1, bitset2) << endl;

			stringstream str;
			str << "BitSets disagree on bit " << i << ": ";
			str << "bitset1[" << i << "] = " << (val1 ? "true" : "false") << ", ";
			str << "bitset2[" << i << "] = " << (val2 ? "true" : "false");
			throw str.str();
		}

		//cout << "Bit " << i << ": " << (val1 ? "set" : "not set") << endl;
	}

	cout << "Done comparing!" << endl;
}

/**
 * \brief Randomises the bits in 'bitset' and 'bitset2'
 *
 * The first BitSet is used as 'base' for defining block sizes. The second BitSet can be NULL
 */
void BitSetTester::randomise(BitSet* bitset1, BitSet* bitset2, int maxBits){
	const bool DEBUGGING = false;

	bitset1->clear();
	if (bitset2 != NULL) bitset2->clear();

	// Probability of constructing a fill-bit
	float fillProb = rand_float();

	// Probability a fill bit is a 1-fill
	float oneFillProb = rand_float();

	// Fill the bit set with a random number of bits, somewhere between 0 and a big number
	//int numBlocks = (rand() % 10000000) / 31;
	int numBits = rand() % maxBits;
	int numBlocks = numBits / bitset1->blocksize() + 1;
	int offset;

	if (DEBUGGING) cout << "BitSetTester::randomise -- Setting " << numBlocks << " blocks consisting of " << bitset1->blocksize() << " bits each." << endl;
	for (int block = 0; block < numBlocks; block++){
		offset = block * bitset1->blocksize();

		if (rand_float() <= fillProb){
			// Construct a block in such a way that a fill will occur
			bool onefill = rand_float() <= oneFillProb;
			if (DEBUGGING) cout << "BitSetTester::randomise -- Constructing block " << block << ": will be stored as " << (onefill ? "1" : "0") << "-fill word" << endl;

			for (int bit = offset; bit < offset + bitset1->blocksize(); bit++){
				bitset1->set(bit, onefill);
				if (bitset2 != NULL) bitset2->set(bit, onefill);
			}
		} else {
			// Construct a block with random bits
			if (DEBUGGING) cout << "BitSetTester::randomise -- Constructing block " << block << ": will be stored as literal word" << endl;
			for (int bit = offset; bit < offset + bitset1->blocksize(); bit++){
				bool value = rand_float() < 0.5;
				bitset1->set(bit, value);
				if (bitset2 != NULL) bitset2->set(bit, value);
			}
		}
	}

	if (DEBUGGING) cout << "BitSetTester::randomise -- done randomising!" << endl;
}

string BitSetTester::printBitSets(BitSet* bitset1, BitSet* bitset2){
	return printBitSets(bitset1, bitset2, NULL);
}

string BitSetTester::printBitSets(BitSet* bitset1, BitSet* bitset2, BitSet* bitset3){
	stringstream res;
	res << "BitSet 1:" << endl;
	res << bitset1->toString() << endl << endl;
	res << "BitSet 2:" << endl;
	res << bitset2->toString() << endl;
	if (bitset3 != NULL){
		res << "BitSet 3:" << endl;
		res << bitset3->toString() << endl;
	}
	return res.str();
}

void BitSetTester::randomise(BitSet* bitset1, int maxBits){
	randomise(bitset1, NULL, maxBits);
}
