/*
 * PWAHStackTCTest.cpp
 *
 *  Created on: 11 May 2013
 *      Author: bas
 */

#ifndef NDEBUG
#include "PWAHStackTCTest.h"

#include "../src/algorithms/PWAHStackTC.h"
#include "../src/datastructures/bitsets/BitSet.h"
#include "GraphTestInfo.h"
#include "../src/datastructures/bitsets/pwah/PWAHBitSet.h"
#include "../src/datastructures/bitsets/wah/WAHBitSet.h"
#include <vector>
#include <iostream>

TEST_F(PWAHStackTCTest, TestGraphsFromDataset){
	vector<GraphTestInfo> graphtestinfo = GraphTestInfo::getAllGraphs();
	TransitiveClosureAlgorithm* tca;

	try {

	for (unsigned int graph_i = 0; graph_i < graphtestinfo.size(); graph_i++){
		const GraphTestInfo gti = graphtestinfo[graph_i];
		const string filename = gti.getFullFilename();

		//cout << "======= " << filename << " =======" << endl;
		//cout << "Expecting " << gti.numEdgesInTC << " edges in TC, " << gti.numEdgesInReflexiveTC << " edges in reflexive TC..." << endl;
		Graph graph;
		ASSERT_NO_THROW(
			graph = Graph::parseChacoFile(filename);
		);

		EXPECT_EQ(graph.getNumberOfVertices(), gti.numVertices);

		for (int bstype = 0; bstype < 5; bstype++){
			for (int reflexive = 0; reflexive <= 1; reflexive++){
				if (bstype == 0) tca = new PWAHStackTC<PWAHBitSet<2> >(graph);
				else if (bstype == 1) tca = new PWAHStackTC<PWAHBitSet<4> >(graph);
				else if (bstype == 2) tca = new PWAHStackTC<PWAHBitSet<8> >(graph);
				else if (bstype == 3) tca = new PWAHStackTC<WAHBitSet>(graph);
				else if (bstype == 4) tca = new PWAHStackTC<IntervalBitSet>(graph);
				else throw string("???");

				//cout << "Computing " << (reflexive == 1 ? "reflexive " : "") << "TC using " << tca->algorithmName() << "... " << endl;

				tca->computeTransitiveClosure((reflexive == 1), false, 0);
				//cout << "Memory in use by data structure: " << tca->memoryUsedByBitSets() << " bits." << endl;
				//cout << "Validating number of edges in TC: ";
				//cout.flush();

				EXPECT_EQ(tca->getNumberOfComponents(), gti.numStronglyConnectedComponents);

				const unsigned int numEdges = tca->countNumberOfEdgesInTC();
				//cout << numEdges << ". ";
				if (reflexive == 1){
					EXPECT_EQ(numEdges, gti.numEdgesInReflexiveTC);
				} else {
					EXPECT_EQ(numEdges, gti.numEdgesInTC);
				}

				delete tca;
			}
		}
	}

	} catch (string exc){
		cerr << exc << endl;
		exit(1);
	}
}

void PWAHStackTCTest::SetUp(){

}

void PWAHStackTCTest::TearDown(){

}

#endif /* ifndef NDEBUG */
