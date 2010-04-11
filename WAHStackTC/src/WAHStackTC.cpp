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
#include <string>
using namespace std;

int main() {
	string filename = "../../Datasets/Semmle graphs/java/depends.graph";
	/**Graph* graph = Graph::parseChacoFile(filename);
	cout << "Number of vertices: " << graph->getNumberOfVertices() << endl;

		delete graph;**/

	try {
		WAHBitSet bitset = WAHBitSet(100);
		/*bitset.set(0);
		bitset.set(1);
		bitset.set(3);*/
		for (int i = 0; i < 34; i++) bitset.set(i);
		bitset.set(38);

		cout << bitset.toString();
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
