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
#include "WAHStackTC.h"
#include "DynamicBitSet.h"
#include "WAHBitSetTester.h"
#include "PerformanceTimer.h"
#include <string>
#include <stdlib.h>
#include <sstream>
using namespace std;

int main() {



	try {
		/**WAHBitSetTester::testIterator();
		cout << "done testing iterator" << endl;
		exit(1);**/

		// string filename = "../../Datasets/nuutila32.graph";
		//string filename = "../../Datasets/Semmle graphs/java/depends.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/depends.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/depends.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/successor.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/successors.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/calls.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/categorypagelinks.graph";
		string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/pagelinks.graph";
		//string filename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/child.graph";
		PerformanceTimer timer = PerformanceTimer::start();
		cout << "Parsing graph file... ";
		cout.flush();

		Graph graph = Graph::parseChacoFile(filename);
		cout << "done, that took " << timer.reset() << " msecs";
		cout << "Number of vertices: " << graph.getNumberOfVertices() << endl;
		cout << "Number of edges: " << graph.countNumberOfEdges() << endl;

		WAHStackTC* wstc = new WAHStackTC(graph);

		cout << "Computing transitive closure... ";
		cout.flush();
		wstc->computeTransitiveClosure();
		cout << "done, that took " << timer.reset() << " msecs" << endl;
		cout.flush();

		//cout << wstc.tcToString();
		cout << "Counting number of edges in TC... ";
		cout.flush();
		cout << "Transitive closure contains "<< wstc->countNumberOfEdgesInTC() << " edges" << endl;

		delete wstc;
		exit(1);
	} catch (string str){
		cerr << "Exception: " << str << endl;
		cerr.flush();
	}
	return 0;
}
