//============================================================================
// Name        : WAHStackTC.cpp
// Author      : Sebastiaan J. van Schaik
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "Graph.h"
#include "WAHBitSet.h"
#include "DynamicBitSet.h"
#include "WAHBitSetTester.h"
#include "PerformanceTimer.h"
#include <string>
#include <stdlib.h>
#include <sstream>
using namespace std;

int main() {
	string filename = "../../Datasets/Semmle graphs/java/depends.graph";
	/**Graph* graph = Graph::parseChacoFile(filename);
	cout << "Number of vertices: " << graph->getNumberOfVertices() << endl;

		delete graph;**/

	try {
		/*DynamicBitSet dynaBitSet;
		dynaBitSet.set(1);
		dynaBitSet.set(3);
		dynaBitSet.set(5);
		cout << dynaBitSet.toString() << endl;*/
		//WAHBitSetTester::testOr();

		//WAHBitSetTester::test(10000);
		//exit(1);

		WAHBitSet bitset1, bitset2;
		for (int i = 0; i < 3; i++){
			int randomBlock1 = WAHBitSet::generateRandomLiteralBlock();
			int randomBlock2 = WAHBitSet::generateRandomLiteralBlock();
			//cout << "Adding " << WAHBitSet::toBitString(randomBlock) << endl;
			//bitset1.setBits(i, randomBlock1);
			bitset2.setBits(i, randomBlock2);
		}
		bitset1.clear();
		for (int i = 0; i < 4; i++) bitset1.setBits(i, 0xFFFFFFFF);
		//for (int i = 0; i < 2 * 31; i++) bitset1.set(i);
		bitset1.setBits(5, WAHBitSet::generateRandomLiteralBlock());
		bitset1.setBits(6, WAHBitSet::generateRandomLiteralBlock());

		cout << "===== BitSet 1 =====" << endl << bitset1.toString() << endl;


		cout << "===== BitSet 2 =====" << endl << bitset2.toString() << endl;

		WAHBitSet merge = WAHBitSet::constructByOr(bitset1, bitset2);
		cout << "Result:" << endl;
		cout << merge.toString() << endl;


		//PerformanceTimer timer = PerformanceTimer::start();
		//WAHBitSetTester::test(1000);
		//cout << "That took " << timer.reset() << " msecs" << endl;
		/**WAHBitSet wahBitset;
		wahBitset.constructFailingExample();
		cout << wahBitset.toString() << endl;

		DynamicBitSet dynaBitset;
		dynaBitset.constructFailingExample();
		cout << dynaBitset.toString() << endl;

		for (int i = 0; i < 64 * 3; i++){
			cout << "Checking bit with index " << i << "..." << endl;
			if (dynaBitset.get(i) != wahBitset.get(i)){
				stringstream stream;
				stream << "Inconsistency at bit index " << i << ": ";
				if (dynaBitset.get(i)) stream << "DynamicBitSet seems to think it is set, WAHBitSet says otherwise?";
				else stream << "WAHBitSet seems to think it is set, DynamicBitSet says otherwise?";
				throw stream.str();
			}
		}
		cout << "done!" << endl;**/
		/*WAHBitSet bitset = WAHBitSet();
		//bitset.set(0);
		//bitset.set(3);
		//bitset.set(1);
		//bitset.set(3);
		for (int i = 0; i < 34; i++){
			//if (i == 11) continue;
			bitset.set(i);
		}
		bitset.set(38);

		cout << endl << endl << "Resulting compressed BitSet:" << endl;
		cout << bitset.toString();*/
		/*for (int i = 0; i < 100; i++){
			string val = "not set";
			if (bitset.get(i)) val = "set";
			cout << i << ": " << val << endl;
		}*/
	} catch (string str){
		cerr << "Exception: " << str << endl;
	}
	return 0;
}
