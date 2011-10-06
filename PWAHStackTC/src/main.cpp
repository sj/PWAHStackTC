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
#include "PWAHStackTC.h"
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

void printUsage(){
	cerr << "Usage of PWAHStackTC:" << endl;
	cerr << "  PWAHStackTC --arg1=val1 --arg2=val2" << endl << endl << endl;

	cerr << "Command line parameters:" << endl << endl;

	cerr << "--help" << endl;
	cerr << "--usage" << endl;
	cerr << "  Prints this usage information and exits." << endl << endl;

	cerr << "--filename=myfile.graph" << endl;
	cerr << "  Use myfile.graph as input file (expected format: chaco)." << endl << endl;

	cerr << "--num-runs=5" << endl;
	cerr << "  Runs the algorithm multiple (in this example: 5) times." << endl << endl;

	cerr << "--reflexive" << endl;
	cerr << "  Computes the reflexive transitive closure." << endl << endl;

	cerr << "--bitset-implementation=pwah-8" << endl;
	cerr << "  Specifies the bitset implementation to use. Valid implementations:" << endl;
	cerr << "  pwah-2, pwah-4, pwah-8, wah, interval (default=pwah-8)." << endl << endl;

	cerr << "--no-details" << endl;
	cerr << "  Decreases verbosity." << endl << endl;

	cerr << "--index-chuck-size=1024" << endl;
	cerr << "  Sets the chunk size of the PWAH index (default: disable index)." << endl << endl;

	cerr << "For more information:" << endl;
	cerr << "Sebastiaan J. van Schaik. Answering reachability queries on large directed graphs, introducing a new data structure using bit vector compression. MSc. thesis, Utrecht University, 2010." << endl;
	cerr << "Sebastiaan J. van Schaik and Oege de Moor. A memory efficient reachability data structure through bit vector compression. In SIGMOD '11: Proceedings of the 37th SIGMOD international conference on Management of data, pages 913-924, New York, NY, USA, 2011. ACM." << endl;

}

int main(int argc, char* argv[]) {
	cout.setf(ios::fixed,ios::floatfield);
	cout.precision(4);

	// Increase stack size
	const rlim_t sSize = 1024 * 1024 * 128; // 128 Megabytes
	struct rlimit rl;

	if (getrlimit(RLIMIT_STACK, &rl) == 0){
		// Successfully acquired stack limit
		if (sSize > rl.rlim_cur){
			// Increase stack size...
			rl.rlim_cur = sSize;
			if (setrlimit(RLIMIT_STACK, &rl) != 0){
				// Error...
				cerr << "Warning: could not increase stack size. PWAHStackTC might run into trouble..." << endl;
			}
		}
	}


	typedef map<string,string> mapType;
	map<string, string> cmdLineArgs;
	cmdLineArgs["num-runs"] = "1";
	cmdLineArgs["filename"] = "";
	cmdLineArgs["reflexive"] = "unset";
	cmdLineArgs["help"] = "unset";
	cmdLineArgs["usage"] = "unset";
	cmdLineArgs["bitset-implementation"] = "pwah-8";
	cmdLineArgs["no-details"] = "unset";
	cmdLineArgs["index-chunk-size"] = "-1";
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
			printUsage();
			exit(1);
		}
	}

	if (cmdLineArgs["filename"] == ""){
		cerr << "No input file provided, please use command-line option --filename=myfile.graph" << endl;
		printUsage();
		exit (1);
	}

	if (cmdLineArgs["usage"] != "unset" || cmdLineArgs["help"] != "unset"){
		printUsage();
		exit(0);
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

	try {
		PerformanceTimer timer = PerformanceTimer::start();
		if (!nodetails) cout << "Parsing graph file: " << filename << "... ";
		cout.flush();
		Graph graph = Graph::parseChacoFile(filename);

		if (!nodetails){
			cout << "done, that took " << timer.reset() << " msecs" << endl;
			cout << "Number of vertices: " << graph.getNumberOfVertices() << ", ";
			cout << "number of edges: " << graph.countNumberOfEdges() << endl;
		}

		double tmp;

		for (int i = 0; i < numRuns; i++){
			timer.reset();

			TransitiveClosureAlgorithm* tca;
			if (cmdLineArgs["bitset-implementation"] == "pwah-2"){
				tca = new PWAHStackTC<PWAHBitSet<2> >(graph);
				PWAHBitSet<2>::setIndexChunkSize(indexChunkSize);
			} else if (cmdLineArgs["bitset-implementation"] == "pwah-4"){
				tca = new PWAHStackTC<PWAHBitSet<4> >(graph);
				PWAHBitSet<4>::setIndexChunkSize(indexChunkSize);
			} else if (cmdLineArgs["bitset-implementation"] == "pwah-8"){
				tca = new PWAHStackTC<PWAHBitSet<8> >(graph);
				PWAHBitSet<8>::setIndexChunkSize(indexChunkSize);
			} else if (cmdLineArgs["bitset-implementation"] == "wah"){
				tca = new PWAHStackTC<WAHBitSet>(graph);
			} else if (cmdLineArgs["bitset-implementation"] == "interval"){
				tca = new PWAHStackTC<IntervalBitSet>(graph);
			} else {
				cerr << "Invalid BitSet implementation specified on command line: '" << cmdLineArgs["bitset-implementation"] << "'" << endl;
				printUsage();
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

			cout << "Memory used by the reachability data structures of the " << tca->algorithmName() << " algorithm: " << tca->memoryUsedByBitSets() << " bits" << endl;

			if (!nodetails){
				// Check how much memory an interval list would have used
				cout << "Memory used by equivalent interval lists: ";
				cout.flush();
				cout << tca->memoryUsedByIntervalLists() << " bits" << endl;
			}

			cout << "Total memory used by the " << tca->algorithmName() << " algorithm: " << tca->totalMemoryUsage() << " bits" << endl;
			cout << tca->getStatistics() << endl;

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

			memUsage = tca->totalMemoryUsage();
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
