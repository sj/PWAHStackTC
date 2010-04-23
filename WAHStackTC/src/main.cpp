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
using namespace std;

int main(int argc, char* argv[]) {
	cout.setf(ios::fixed,ios::floatfield);
	cout.precision(4);

	string defFilename;
	//defFilename = "../../Datasets/nuutila32.graph";
	//defFilename = "../../Datasets/Semmle graphs/java/depends.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/depends.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/depends.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/successor.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/successors.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/calls.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/categorypagelinks.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/pagelinks.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/child.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/samba/setflow.graph";
	defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/agrocyc.graph";

	typedef map<string,string> mapType;
	map<string, string> cmdLineArgs;
	cmdLineArgs["numruns"] = "1000";
	cmdLineArgs["filename"] = defFilename;
	cmdLineArgs["reflexitive"] = "no";

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

	const bool reflexitive = (cmdLineArgs["reflexitive"] != "no");
	const string filename = cmdLineArgs["filename"];
	const int numRuns = atoi(cmdLineArgs["numruns"].c_str());
	const int numQueries = 1000000;
	double totalConstructionTime = 0;
	double totalQueryTime = 0;

	try {
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
		double tmp;

		for (int i = 0; i < numRuns; i++){
			timer.reset();

			WAHStackTC* wstc = new WAHStackTC(graph);

			if (!reflexitive){
				cout << "Computing transitive closure... ";
			} else {
				cout << "Computing REFLEXITIVE transitive closure... ";
			}
			cout.flush();
			wstc->computeTransitiveClosure(reflexitive, false);

			tmp = timer.reset();
			totalConstructionTime += tmp;
			cout << "done, that took " << tmp << " msecs" << endl;
			cout.flush();

			//cout << wstc.tcToString();
			cout << "Number of components (vertices in condensation graph): " << wstc->getNumberOfComponents() << endl;
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
	} catch (string str){
		cerr << "Exception: " << str << endl;
		cerr.flush();
	}
	return 0;
}
