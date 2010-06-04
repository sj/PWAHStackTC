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
#include <iostream>
#include <map>
#include "Validator.h"
#include "PWAHBitSet.h"
#include "LongBitMacros.cpp"
using namespace std;

void runValidatorWhenRequested(int argc, char* argv[]){
	if (argc == 2){
		string arg = argv[1];
		if (arg == "--run-validator"){
			Validator::validate();
			exit(0);
		}
	}
}

int main(int argc, char* argv[]) {
	cout.setf(ios::fixed,ios::floatfield);
	cout.precision(4);
	runValidatorWhenRequested(argc, argv);

	try {
		long test = 0b1000000000000000000000000000000000000000000000000000000000000000;
		if (L_GET_BIT(test, 0)){
			cout << "0 = set" << endl;
		} else {
			cout << "0 = not set" << endl;
		}
		if (L_GET_BIT(test, 63)){
			cout << "63 = set" << endl;
		} else {
			cout << "63 = not set" << endl;
		}
		cout << test << endl;
		PWAHBitSet<8> pwbs = PWAHBitSet<8>();
		pwbs.set(7);
		//cout << pwbs.toString() << endl;

		cout << "fill length: " << PWAHBitSet<2>::fill_length(0b1110000000000000000000000000000111000000000000000000000000000010,0) << endl;
	} catch (int e){
		cerr << "Exception: " << e << endl;;
	}
	//exit(1);

	string defFilename;
	//defFilename = "../../Datasets/nuutila32.graph";
	//defFilename = "../../Datasets/Semmle graphs/java/depends.graph";
	defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/depends.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/depends.graph";
	defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/successor.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/callgraph.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/successors.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/child.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/polycalls.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Pajek/patents.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/categorypagelinks.graph";
	defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/pagelinks.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/child.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/samba/setflow.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/agrocyc.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/kegg.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Pajek/patents.graph";

	typedef map<string,string> mapType;
	map<string, string> cmdLineArgs;
	cmdLineArgs["num-runs"] = "1";
	cmdLineArgs["filename"] = defFilename;
	cmdLineArgs["reflexitive"] = "unset";
	cmdLineArgs["run-validator"] = "unset";

	// By default: use multi-OR when a component has out-degree of at least 5
	cmdLineArgs["min-multi-or"] = "5";

	for (int i = 1; i < argc; i++){
		string currArg = argv[i];
		size_t found = string::npos;

		// Iterate over all possible keys in map
	    // Using a const_iterator since we are not going to change the values.
		for(mapType::const_iterator it = cmdLineArgs.begin(); it != cmdLineArgs.end(); ++it)
		{
			string fullArg = "--" + it->first;
			found = currArg.find(fullArg);
			if (found != string::npos){
				if (currArg.find(fullArg + "=") != string::npos){
					// record value
					cmdLineArgs[it->first] = currArg.substr(fullArg.size() + 1);
				} else {
					// no --argument=value, just --argument
					cmdLineArgs[it->first] = "";
				}
				break;
			}
		}

		if (found == string::npos){
			// Commandline argument invalid
			cerr << "Invalid commandline argument: " << currArg << endl;
			exit(1);
		}
	}

	const bool reflexitive = (cmdLineArgs["reflexitive"] != "unset");
	const int minMultiOR = atoi(cmdLineArgs["min-multi-or"].c_str());
	const string filename = cmdLineArgs["filename"];
	const int numRuns = atoi(cmdLineArgs["num-runs"].c_str());
	const int numQueries = 1000000;
	double totalConstructionTime = 0;
	double totalQueryTime = 0;

	if (cmdLineArgs["run-validator"] != "unset"){
		// Validation should have been performed earlier?!
		cerr << "Something's wrong with the validator!" << endl;
		exit(1);
	}

	try {
		//WAHBitSetTester::testMultiOr();
		//exit(1);
		/**WAHBitSetTester::testIterator();
		cout << "done testing iterator" << endl;
		exit(1);**/




		PerformanceTimer timer = PerformanceTimer::start();
		cout << "Parsing graph file: " << filename << "... ";
		cout.flush();
		Graph graph = Graph::parseChacoFile(filename);
		cout << "done, that took " << timer.reset() << " msecs" << endl;
		cout << "Number of vertices: " << graph.getNumberOfVertices() << ", ";
		cout << "number of edges: " << graph.countNumberOfEdges() << endl;
		cout << "Min, max, average out degree: ";
		cout << graph.computeMinOutDegree() << ", " << graph.computeMaxOutDegree() << ", " << graph.computeAvgOutDegree() << endl;
		cout << "Min, max, average in degree: ";
		cout << graph.computeMinInDegree() << ", " << graph.computeMaxInDegree() << ", " << graph.computeAvgInDegree() << endl;



		double tmp;

		for (int i = 0; i < numRuns; i++){
			timer.reset();

			WAHStackTC<WAHBitSet>* wstc = new WAHStackTC<WAHBitSet>(graph);

			if (!reflexitive){
				cout << "Computing transitive closure ";
			} else {
				cout << "Computing REFLEXITIVE transitive closure ";
			}
			if (minMultiOR == -1){
				cout << "(NOT using multi-OR)... ";
			} else if (minMultiOR == 0){
				cout << "(ALWAYS using multi-OR)... ";
			} else {
				cout << "(using multi-OR when out-degree >= " << minMultiOR << ")... ";
			}
			cout.flush();
			wstc->computeTransitiveClosure(reflexitive, false, minMultiOR);

			tmp = timer.reset();
			totalConstructionTime += tmp;
			cout << "done, that took " << tmp << " msecs" << endl;
			cout.flush();

			//cout << wstc.tcToString();
			cout << "Number of components (vertices in condensation graph): " << wstc->getNumberOfComponents() << endl;

			if (!reflexitive){
				cout << "Counting number of edges in condensed transitive closure... ";
			} else {
				cout << "Counting number of edges in REFLEXITIVE condensed transitive closure... ";
			}
			cout.flush();
			cout << wstc->countNumberOfEdgesInCondensedTC() << " edges" << endl;

			if (!reflexitive){
				cout << "Counting number of edges in transitive closure... ";
			} else {
				cout << "Counting number of edges in REFLEXITIVE transitive closure... ";
			}
			cout.flush();
			cout << wstc->countNumberOfEdgesInTC() << " edges" << endl;

			if (RAND_MAX < graph.getNumberOfVertices()){
				cerr << "Warning! RAND_MAX=" << RAND_MAX << ", whilst number of vertices = " << graph.getNumberOfVertices() << ". Not every vertex can possibly be reached." << endl;
			}
			cout << "Generating " << numQueries << " random sources and destinations... ";
			timer.reset();
			int* rndDst = new int[numQueries];
			int* rndSrc = new int[numQueries];
			srand ( time(NULL) );

			for (int i = 0; i < numQueries; i++){
				// modulo computation does not yield an exactly uniform distribution,
				// but it's good enough for this purpose...
				rndSrc[i] = rand() % graph.getNumberOfVertices();
				rndDst[i] = rand() % graph.getNumberOfVertices();
			}
			cout << "done, that took " << timer.reset() << " msecs" << endl;

			cout << "Performing " << numQueries << " random queries... ";
			cout.flush();
			timer.reset();
			long numReachable = 0;
			for (int i = 0; i < numQueries; i++){
				if (wstc->reachable(rndSrc[i], rndDst[i])) numReachable++;
			}
			tmp = timer.reset();
			totalQueryTime += tmp;
			cout << "done, that took " << tmp << " msecs" << endl;
			cout << numReachable << " pairs turned out to be reachable." << endl;
			cout << "Number of bits required to store WAH compressed bitsets: " << wstc->memoryUsedByBitSets() << endl;
			delete[] rndDst;
			delete[] rndSrc;
			delete wstc;
		}

		double avgConstructionTime = totalConstructionTime / ((double)numRuns);
		double avgQueryTime = totalQueryTime / ((double)numRuns);
		cout << "Average construction time over " << numRuns << " runs: " << avgConstructionTime << " msecs" << endl;
		cout << "Average query time over " << numRuns << " runs: " << avgQueryTime << " msecs for " << numQueries << " queries" << endl;

		// Machine readable
		cout << "AVG_CONSTR_TIME=" << avgConstructionTime << endl;
		cout << "AVG_QUERY_TIME=" << avgQueryTime << endl;
		exit(1);
	} catch (int str){
		cerr << "Exception: " << str << endl;
		cerr.flush();
	}
	return 0;
}

/**
 * Tell the compiler which instantiations of PWAHBitSet to generate
 */
template class PWAHBitSet<1>;
template class PWAHBitSet<2>;
template class PWAHBitSet<4>;
template class PWAHBitSet<8>;
