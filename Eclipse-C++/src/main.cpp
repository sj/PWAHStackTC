#include <string>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <map>
#include "datastructures/bitsets/pwah/PWAHBitSet.h"
#include "datastructures/Graph.h"
#include "datastructures/bitsets/wah/WAHBitSet.h"
#include "algorithms/PWAHStackTC.h"
#include "datastructures/bitsets/DynamicBitSet.h"
#include "util/PerformanceTimer.h"
#include "algorithms/TransitiveClosureAlgorithm.h"
#include "datastructures/bitsets/interval/IntervalBitSet.h"
#include <sys/resource.h>
#include <fstream>
#include <assert.h>
#include <cstdio>
#include "gtest-includes.h"
#include "debug-build.h"
using namespace std;

/** \mainpage PWAHStackTC: an algorithm / data structure for reachability queries on graphs
 *
 * http://www.sjvs.nl/pwahstacktc
 *
 * Introduction
 * ---
 * PWAHStackTC is an algorithm and data structure to query and store reachability information on large directed graphs. It uses
 * an algorithm first proposed by Esko Nuutila [Nuutila1995] and a data structure for bit vector compression designed by
 * Sebastiaan van Schaik. See [VanSchaik2010] and [VanSchaikDeMoor2011] for a comprehensive description and experimental evaluation
 * of the data structure and algorithm.
 *
 * This implementation is licenced under the GNU General Public Licence (GPL), see http://www.gnu.org/licenses/gpl.html
 * <br><br>
 *
 *
 *
 * Comparing PWAHStackTC to your implementation
 * ---
 * By default, the executable will run an experiment by parsing a provided graph, constructing the reachability data structure,
 * and performing 1,000,000 random reachability queries. Its performance (time required to construct data structure,
 * time required to performing random queries) and memory usage are printed. For example:
 *
 *     PWAHStackTC --filename=myfile.graph
 * <br>
 *
 *
 * Compilation instructions
 * ---
 * PWAHStackTC requires a 64-bit operating system to compile and run, because it relies on the `long` type comprising 64 bits. For
 * unit testing, it relies on the Google Test (gtest) framework, which is available from https://code.google.com/p/googletest/.
 *
 * The easiest way to compile the source code, is to enter the `Release` directory and use `make` to build:
 *
 *     cd Release
 *     make clean
 *     make all
 *
 * <br>
 *
 *
 * Usage instructions
 * ---
 * After compiling the executable, run it with the option --help:
 *
 *     PWAHStackTC --filename=myfile.graph
 *
 *
 * ### References ###
 * Nuutila1995: Esko Nuutila. Efficient Transitive Closure Computation in Large Digraphs. PhD thesis, Finnish Academy of Technology, 1995. http://www.cs.hut.fi/~enu/tc.html
 *
 * VanSchaik2010: Sebastiaan J. van Schaik. Answering reachability queries on large directed graphs, introducing a new data structure using bit vector compression. MSc. thesis, Utrecht University, 2010.
 *
 * VanSchaikDeMoor2011: Sebastiaan J. van Schaik and Oege de Moor. A memory efficient reachability data structure through bit vector compression. In SIGMOD ’11: Proceedings of the 37th SIGMOD international conference on Management of data, pages 913-924, New York, NY, USA, 2011. ACM. DOI: 10.1145/1989323.1989419. See: http://dx.doi.org/10.1145/1989323.1989419
 *
 *
 */


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

	cerr << "--source-vertices=mysources.txt" << endl;
	cerr << "--target-vertices=mytargets.txt" << endl;
	cerr << "--reachable-sources-targets=myresult.txt" << endl;
	cerr << "  Reads sets of source and target vertex indices (whitespace/newline separated) from the " << endl;
	cerr << "  provided files, and will write all reachable pairs to 'myresult.txt'. " << endl;
	cerr << "  The implementation is optimised for single:many queries and many:many queries." << endl;
	cerr << "  (this functionality uses PWAH-8 and ignores all other options, except for --reflexive)" << endl << endl;

	cerr << "--run-unit-tests" << endl;
	cerr << "  Will call the Google Test framework (GTest) to run predefined unit tests" << endl;
	cerr << "  (only available in Debug builds)" << endl << endl;

	cerr << "For more information:" << endl;
	cerr << "Sebastiaan J. van Schaik. Answering reachability queries on large directed graphs, introducing a new data structure using bit vector compression. MSc. thesis, Utrecht University, 2010." << endl;
	cerr << "Sebastiaan J. van Schaik and Oege de Moor. A memory efficient reachability data structure through bit vector compression. In SIGMOD '11: Proceedings of the 37th SIGMOD international conference on Management of data, pages 913-924, New York, NY, USA, 2011. ACM." << endl;
}

void runUnitTestsWhenRequested(int argc, char* argv[]){
	if (argc >= 2){
		string arg = argv[1];

		if (arg == "--run-unit-tests"){
#ifndef __OPTIMIZE__
			try {
				::testing::InitGoogleTest(&argc, argv);
				const int test_result = RUN_ALL_TESTS();
				exit(test_result);
			} catch (string str){
				cerr << "Exception: " << str << endl;
				exit(1);
			}
#else
			cerr << "Error: unit tests are only available in Debug builds. Please use --help switch for usage information." << endl;
			exit(1);
#endif
		}
	}
}


vector<unsigned int> _read_int_tokens(string filename){
	vector<unsigned int> res;
	ifstream in_file(filename.c_str());
	string line = "";
	if (in_file.is_open()){
		while(std::getline(in_file,line)){
			string token;

			stringstream lineStream(line);
			while(lineStream >> token)	{
				unsigned int int_token = atoi(token.c_str());
				res.push_back(int_token);
			}
		}
	} else {
		cerr << "Can't read '" << filename << "'. Aborting." << endl;
		exit(1);
	}
	in_file.close();

	return res;
}

void doSourcesTargetsReachability(string filename_graph, string filename_sources, string filename_targets, string filename_out, bool reflexive){
	vector<unsigned int> source_vertices = _read_int_tokens(filename_sources);
	vector<unsigned int> target_vertices = _read_int_tokens(filename_targets);
	long num_pairs = source_vertices.size() * target_vertices.size();

	PerformanceTimer total_timer;
	PerformanceTimer timer;
	total_timer.reset();

	cout << "Determining reachability for " << source_vertices.size() << " source and " << target_vertices.size() << " target vertices (" << num_pairs << " pairs):" << endl;
	cout << " - parsing graph: ";
	fflush(stdout);

	timer.reset();
	Graph graph = Graph::parseChacoFile(filename_graph);
	cout << "done in " << timer.reset() << " milliseconds, found " << graph.getNumberOfVertices() << " vertices." << endl;

	PWAHStackTC<PWAHBitSet<8> >* tc = new PWAHStackTC<PWAHBitSet<8> >(graph);

	vector<vector<unsigned int> > reachable;
	reachable.reserve(source_vertices.size());


	cout << " - computing transitive closure: ";
	fflush(stdout);
	timer.reset();
	tc->computeTransitiveClosure(reflexive, false, 0);
	cout << "done in " << timer.reset() << " millisecs" << endl;

	try {
		cout << " - performing reachability queries: ";
		fflush(stdout);
		timer.reset();
		tc->reachablepairs(source_vertices, target_vertices, reachable);
		const long time_reachable_micro = timer.currRunTimeMicro();
		const long time_reachable = time_reachable_micro / (double) 1000;
		cout << "done in " << time_reachable << " millisecs / " << time_reachable_micro << " microsecs (amortised " << (time_reachable_micro / (double)(num_pairs)) << " microseconds per query)" << endl;
	} catch (string& exc){
		cerr << "Exception: " << exc << endl;
		exit(1);
	}
	cout << " - writing reachable pairs to '" << filename_out << ": ";
	fflush(stdout);

	assert(reachable.size() == source_vertices.size());
	ofstream out_file (filename_out.c_str(), ios::trunc);
	if (out_file.is_open())
	for (unsigned int i = 0; i < reachable.size(); i++){
		const unsigned int source_vertex = source_vertices[i];

		for (unsigned int j = 0; j < reachable[i].size(); j++){
			const unsigned int target_vertex = reachable[i][j];
			out_file << source_vertex << " " << target_vertex << "\n";
		}
	}
	out_file.close();

	cout << "done! Total time: " << total_timer.reset() << " milliseconds." << endl;


	cout << " - verifying result: ";
	fflush(stdout);

	timer.reset();
	unsigned int num_pairs_reachable = 0;
	for (unsigned int i = 0; i < reachable.size(); i++){
		const unsigned int source_vertex = source_vertices[i];

		for (unsigned int j = 0; j < reachable[i].size(); j++){
			const unsigned int target_vertex = reachable[i][j];

			if (!tc->reachable(source_vertex, target_vertex)){
				cerr << "ERROR: " << target_vertex << " is not actually reachable from " << source_vertex << "!?" << endl;
				exit(1);
			}

			num_pairs_reachable++;
		}
	}


	// The other way around: are there reachable pairs which were reported not reachable?
	PerformanceTimer timer_distinct_queries;
	timer_distinct_queries.reset();
	unsigned int dist_num_pairs_reachable = 0;
	for (unsigned int i = 0; i < source_vertices.size(); i++){
		const unsigned int source_vertex = source_vertices[i];

		for (unsigned int j = 0; j < target_vertices.size(); j++){
			const unsigned int target_vertex = target_vertices[j];

			if (tc->reachable(source_vertex, target_vertex)) dist_num_pairs_reachable++;
		}
	}
	long time_distinct_queries = timer_distinct_queries.currRunTime();

	if (dist_num_pairs_reachable != num_pairs_reachable){
		cerr << "ERROR! Distinct reachability queries yield " << dist_num_pairs_reachable << " reachable source/target pairs, rather than " << num_pairs_reachable << " pairs!" << endl;
		exit(1);
	}


	const long num_pairs_unreachable = num_pairs - num_pairs_reachable;
	cout << "done, verification took " << timer.reset() << " milliseconds - verified " << dist_num_pairs_reachable << " reachable and " << num_pairs_unreachable << " unreachable source/target pairs! Performing " << num_pairs << " distinct queries took " << time_distinct_queries << " milliseconds." << endl;
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

	runUnitTestsWhenRequested(argc, argv);

#ifdef DEBUGGING
	cerr << "RUNNING PWAHSTACKTC IN DEBUGGING MODE - DON'T USE THIS FOR EXPERIMENTAL EVALUATION!" << endl;
	sleep(2);
#endif


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
	cmdLineArgs["source-vertices"] = "unset";
	cmdLineArgs["target-vertices"] = "unset";
	cmdLineArgs["reachable-sources-targets"] = "unset";

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
	const string source_vertices = cmdLineArgs["source-vertices"];
	const string target_vertices = cmdLineArgs["target-vertices"];
	const string reachable_sources_targets = cmdLineArgs["reachable-sources-targets"];
	const int numRuns = atoi(cmdLineArgs["num-runs"].c_str());
	const int indexChunkSize = atoi(cmdLineArgs["index-chunk-size"].c_str());
	const int numQueries = 1000000;
	double totalConstructionTime = 0;
	double totalQueryTime = 0;
	long memUsage = 0;

	if (source_vertices != "" || target_vertices != "" ||  reachable_sources_targets != ""){
		if (source_vertices == "" || target_vertices == "" || reachable_sources_targets == ""){
			cerr << "Need all three of: --source-vertices, --target-vertices, --reachable-sources-targets" << endl << endl;
			printUsage();
			exit(1);
		}
		doSourcesTargetsReachability(filename, source_vertices, target_vertices,reachable_sources_targets, reflexive);
		exit(0);
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
				cout << "Computing REFLEXIVE transitive closure using " << tca->algorithmName() << " ";
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
