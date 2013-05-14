/**
 * Copyright 2010-2011 Sebastiaan J. van Schaik
 *
 * This file is part of PWAHStackTC.
 *
 * PWAHStackTC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PWAHStackTC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PWAHStackTC. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WAHSTACKTC_H_
#define WAHSTACKTC_H_
#include "../datastructures/Graph.h"
#include "../datastructures/bitsets/wah/WAHBitSet.h"
#include "../datastructures/bitsets/DynamicBitSet.h"
#include <vector>
#include <stack>
#include "../util/PerformanceTimer.h"
#include "../datastructures/bitsets/StaticBitSet.h"
#include "../datastructures/DynamicStack.h"
#include "../datastructures/bitsets/pwah/PWAHBitSet.h"
#include "../datastructures/bitsets/BitSet.h"
#include "TransitiveClosureAlgorithm.h"
#include "../datastructures/bitsets/interval/IntervalBitSet.h"
#include "../gtest-includes.h"
using namespace std;

template <class B>
class PWAHStackTC : public TransitiveClosureAlgorithm {
private:
	Graph* _graph;
	vector<B*> _componentSuccessors; // stores the successor list (transitive closure) for each component
	//vector<DynamicBitSet*> _componentSuccessors; // stores the successor list (transitive closure) for each component
	int* _vertexComponents; // stores the component index of each vertex
	bool _storeComponentVertices; // determines whether or not to store a list of vertices in each component
	vector<int*> _componentVertices; // stores the vertices in each component
	vector<unsigned int> _componentSizes; // stores the size of strongly connected components
	DynamicBitSet _visited; // records visited vertices
	DynamicStack* _vStack; // stack for vertices
	DynamicStack* _cStack; // stack for components
	unsigned int* _savedVStackSize; // size of stack _vStack upon detection of a vertex
	unsigned int* _savedCStackSize; // size of stack _cStack upon detection of a vertex
	unsigned int* _vertexCandidateComponentRoot; // candidate component root for each vertex
	StaticBitSet* _vertexSelfLoop; // records vertex self loops
	int* _vertexDFSSeqNo; // record DFS sequence number of every vertex
	int _lastDFSSeqNo;
	int _lastComponentIndex;
	bool _reflexitive;
	int _minOutDegreeForMultiOR;
	PerformanceTimer _mergeTimer;

	bool componentHasSelfLoop(int componentIndex);
	void dfsVisit(unsigned int vertexIndex);

	FRIEND_TEST(PWAHStackTCTest, TestReflexive);


public:
	PWAHStackTC(Graph& graph);
	virtual ~PWAHStackTC();

	void computeTransitiveClosure(bool reflexive, bool storeComponentMembers, int minOutDegreeForMultiOR);
	string tcToString();
	long countNumberOfEdgesInTC();
	long countNumberOfEdgesInCondensedTC();
	long countNumberOfEdgesInCondensedTC(bool ignoreSelfLoops, bool ignoreSingletonSelfLoops);
	float countAverageNumberOfIntervals(bool countNulls = true);
	int countMaxNumberOfIntervals();
	long countTotalNumberOfIntervals();
	double computeLocalClusteringCoefficient(int componentIndex);
	double computeAverageLocalClusteringCoefficient();
	int getNumberOfComponents();
	void writeToChacoFile(string filename);
	bool reachable(unsigned int src, unsigned int dst);
	void reachablepairs(vector<unsigned int>& sources, vector<unsigned int>& targets, vector<vector<unsigned int> >& reachable_pairs);
	long memoryUsedByBitSets();
	long totalMemoryUsage();
	string getStatistics();
	string algorithmName();
	long memoryUsedByIntervalLists();
	const vector<B*> getComponentSuccessors() const;
};

#endif /* WAHSTACKTC_H_ */

