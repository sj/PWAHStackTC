#include <string>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <map>
#include "Validator.h"
#include "PWAHBitSet.h"
#include "LongBitMacros.cpp"
#include "BitSetTester.h"
#include "Graph.h"
#include "WAHBitSet.h"
#include "WAHStackTC.h"
#include "DynamicBitSet.h"
#include "WAHBitSetTester.h"
#include "PerformanceTimer.h"
#include "TransitiveClosureAlgorithm.h"
#include "IntervalBitSet.h"
#include <sys/resource.h>

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

void bla(TransitiveClosureAlgorithm* tca){
	cout << "BLA! " << tca->countNumberOfEdgesInTC() << endl;
}

int main(int argc, char* argv[]) {
	cout.setf(ios::fixed,ios::floatfield);
	cout.precision(4);

	// Increase stack size
	const rlim_t sSize = 1024 * 1024 * 32; // 32 Megabytes
	struct rlimit rl;

	if (getrlimit(RLIMIT_STACK, &rl) == 0){
		// Successfully acquired stack limit
		if (sSize > rl.rlim_cur){
			// Increase stack size...
			rl.rlim_cur = sSize;
			if (setrlimit(RLIMIT_STACK, &rl) != 0){
				// Error...
				cerr << "Warning: could not increase stack size. WAHStackTC might run into trouble..." << endl;
			}
		}
	}


	runValidatorWhenRequested(argc, argv);

	try {
		//while (true) BitSetTester<IntervalBitSet>::testOr(5);

		//while (true) BitSetTester::testIterator(new IntervalBitSet(), true);

		//while (true) BitSetTester::testOr();

		//BitSetTester::diff();
		//BitSetTester::testSetGetIndex();
		//bt.testSetGet();

		//exit(1);

		//WAHBitSet* bs2 = new WAHBitSet();

		//BitSetTester tester = BitSetTester(bs1, bs2);
		//tester.testSetGet();
		//while(true){
		//	BitSetTester::testOr();
		//}

		//while (true) BitSetTester::testIterator(bs1, true);



		//BitSetTester::compareMemoryUsage();
	} catch (int e){
		cerr << "Exception: " << e << endl;;
	}
	//exit(1);

	string defFilename;
	//defFilename = "../../Datasets/nuutila32.graph";
	//defFilename = "../../Datasets/Semmle graphs/java/depends.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/depends.graph";
	defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/depends.graph";
	defFilename = "/tmp/test.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/callgraph.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/c++/callgraph.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/successors.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/child.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/polycalls.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/calls.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/child.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/java/subtype.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Pajek/patents.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/categorypagelinks.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/categorylinks.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/wiki/pagelinks.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Semmle graphs/samba/setflow.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/agrocyc.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/reactome.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/kegg.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/human.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/SigMod 08/real_data/xmark.graph";
	//defFilename = "/home/bas/afstuderen/Datasets/Pajek/patents.graph";
	defFilename = "/home/bas/temp/crashgraphs/285c287ce.graph";

	typedef map<string,string> mapType;
	map<string, string> cmdLineArgs;
	cmdLineArgs["num-runs"] = "1";
	cmdLineArgs["filename"] = defFilename;
	cmdLineArgs["reflexive"] = "unset";
	cmdLineArgs["run-validator"] = "unset";
	cmdLineArgs["bitset-implementation"] = "pwah-8";
	cmdLineArgs["no-details"] = "unset";
	cmdLineArgs["index-chunk-size"] = "-1";

	// By default: use multi-OR when a component has out-degree of at least 5
	cmdLineArgs["min-multi-or"] = "0";

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

	const bool nodetails = (cmdLineArgs["no-details"] != "unset");
	const bool reflexive = (cmdLineArgs["reflexive"] != "unset");
	const int minMultiOR = atoi(cmdLineArgs["min-multi-or"].c_str());
	const string filename = cmdLineArgs["filename"];
	const int numRuns = atoi(cmdLineArgs["num-runs"].c_str());
	const int indexChunkSize = atoi(cmdLineArgs["index-chunk-size"].c_str());
	const int numQueries = 1000000;
	double totalConstructionTime = 0;
	double totalQueryTime = 0;
	long memUsage = 0;

	if (cmdLineArgs["run-validator"] != "unset"){
		// Validation should have been performed earlier?!
		cerr << "Something's wrong with the validator!" << endl;
		exit(1);
	}

	try {
		PerformanceTimer timer = PerformanceTimer::start();
		if (!nodetails) cout << "Parsing graph file: " << filename << "... ";
		cout.flush();
		Graph graph = Graph::parseChacoFile(filename);

		if (!nodetails){
			cout << "done, that took " << timer.reset() << " msecs" << endl;
			cout << "Number of vertices: " << graph.getNumberOfVertices() << ", ";
			cout << "number of edges: " << graph.countNumberOfEdges() << endl;
			cout << "Min, max, average out degree: ";
			cout << graph.computeMinOutDegree() << ", " << graph.computeMaxOutDegree() << ", " << graph.computeAvgOutDegree() << endl;
			cout << "Min, max, average in degree: ";
			cout << graph.computeMinInDegree() << ", " << graph.computeMaxInDegree() << ", " << graph.computeAvgInDegree() << endl;
			cout << "Average local clustering coefficient: ";
			cout.flush();
			cout.precision(8);
			cout << graph.computeAverageLocalClusteringCoefficient() << endl;
			cout.precision(4);
		}

		double tmp;

		for (int i = 0; i < numRuns; i++){
			timer.reset();

			TransitiveClosureAlgorithm* tca;
			if (cmdLineArgs["bitset-implementation"] == "pwah-2"){
				tca = new WAHStackTC<PWAHBitSet<2> >(graph);
				PWAHBitSet<2>::setIndexChunkSize(indexChunkSize);
			} else if (cmdLineArgs["bitset-implementation"] == "pwah-4"){
				tca = new WAHStackTC<PWAHBitSet<4> >(graph);
				PWAHBitSet<4>::setIndexChunkSize(indexChunkSize);
			} else if (cmdLineArgs["bitset-implementation"] == "pwah-8"){
				tca = new WAHStackTC<PWAHBitSet<8> >(graph);
				PWAHBitSet<8>::setIndexChunkSize(indexChunkSize);
			} else if (cmdLineArgs["bitset-implementation"] == "wah"){
				tca = new WAHStackTC<WAHBitSet>(graph);
			} else if (cmdLineArgs["bitset-implementation"] == "interval"){
				tca = new WAHStackTC<IntervalBitSet>(graph);
			} else {
				cerr << "Invalid BitSet implementation specified on command line" << endl;
				exit(1);
			}

			if (!reflexive){
				cout << "Computing transitive closure using " << tca->algorithmName() << " ";
			} else {
				cout << "Computing REFLEXITIVE transitive closure using " << tca->algorithmName() << " ";
			}
			if (minMultiOR == -1){
				cout << "(NOT using multi-OR)... ";
			} else if (minMultiOR == 0){
				cout << "(ALWAYS using multi-OR)... ";
			} else {
				cout << "(using multi-OR when out-degree >= " << minMultiOR << ")... ";
			}
			cout.flush();
			tca->computeTransitiveClosure(reflexive, false, minMultiOR);

			tmp = timer.reset();
			totalConstructionTime += tmp;
			cout << "done, that took " << tmp << " msecs" << endl;
			cout.flush();

			cout << "Number of components (vertices in condensation graph): " << tca->getNumberOfComponents() << endl;

			if (!nodetails){
				if (!reflexive){
					cout << "Counting number of edges in condensed transitive closure... ";
				} else {
					cout << "Counting number of edges in REFLEXIVE condensed transitive closure... ";
				}
				cout.flush();
				cout << tca->countNumberOfEdgesInCondensedTC() << " edges" << endl;

				if (!reflexive){
					cout << "Counting number of edges in transitive closure... ";
				} else {
					cout << "Counting number of edges in REFLEXIVE transitive closure... ";
				}
				cout.flush();
				cout << tca->countNumberOfEdgesInTC() << " edges" << endl;
			}

/*
			cout << "Average local clustering coefficient of condensation graph: ";
			cout.flush();
			cout.precision(8);
			cout << tca->computeAverageLocalClusteringCoefficient() << endl;
			cout.precision(4);
*/

			cout << "Memory used by the " << tca->algorithmName() << " data structure: " << tca->memoryUsedByBitSets() << " bits" << endl;

			if (!nodetails){
				// Check how much memory an interval list would have used
				cout << "Memory used by equivalent interval lists: ";
				cout.flush();
				cout << tca->memoryUsedByIntervalLists() << " bits" << endl;
			}

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
				if (tca->reachable(rndSrc[i], rndDst[i])) numReachable++;
			}
			tmp = timer.reset();
			totalQueryTime += tmp;
			cout << "done, that took " << tmp << " msecs" << endl;
			if (!nodetails) cout << numReachable << " pairs turned out to be reachable." << endl;

			memUsage = tca->memoryUsedByBitSets();
			cout << "Number of bits required to store WAH compressed bitsets: " << memUsage << endl;
			delete[] rndDst;
			delete[] rndSrc;
			delete tca;
		}

		double avgConstructionTime = totalConstructionTime / ((double)numRuns);
		double avgQueryTime = totalQueryTime / ((double)numRuns);
		cout << "Average construction time over " << numRuns << " runs: " << avgConstructionTime << " msecs" << endl;
		cout << "Average query time over " << numRuns << " runs: " << avgQueryTime << " msecs for " << numQueries << " queries" << endl;

		// Machine readable
		cout << "AVG_CONSTR_TIME=" << avgConstructionTime << endl;
		cout << "AVG_QUERY_TIME=" << avgQueryTime << endl;
		cout << "MEM_USAGE=" << memUsage << endl;
		exit(0);
	} catch (string str){
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
