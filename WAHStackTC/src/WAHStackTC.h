/*
 * WAHStackTC.h
 *
 *  Created on: Apr 19, 2010
 *      Author: bas
 */

#ifndef WAHSTACKTC_H_
#define WAHSTACKTC_H_
#include "Graph.h"
#include "WAHBitSet.h"
#include "DynamicBitSet.h"
#include <vector>
#include <stack>
#include "PerformanceTimer.h"
using namespace std;

class WAHStackTC {
private:
	Graph* _graph;
	vector<WAHBitSet*> _componentSuccessors; // stores the successor list (transitive closure) for each component
	//vector<DynamicBitSet*> _componentSuccessors; // stores the successor list (transitive closure) for each component
	int* _vertexComponents; // stores the component index of each vertex
	bool _storeComponentVertices; // determines whether or not to store a list of vertices in each component
	vector<int*> _componentVertices; // stores the vertices in each component
	vector<unsigned int> _componentSizes; // stores the size of strongly connected components
	DynamicBitSet _visited; // records visited vertices
	stack<unsigned int> _vStack, _cStack; // vertex and component stacks
	unsigned int* _savedVStackSize; // size of stack _vStack upon detection of a vertex
	unsigned int* _savedCStackSize; // size of stack _cStack upon detection of a vertex
	unsigned int* _vertexCandidateComponentRoot; // candidate component root for each vertex
	DynamicBitSet _vertexSelfLoop; // records vertex self loops
	int* _vertexDFSSeqNo; // record DFS sequence number of every vertex
	int _lastDFSSeqNo;
	int _lastComponentIndex;
	bool _reflexitive;
	int _minOutDegreeForMultiOR;

	bool componentHasSelfLoop(int componentIndex);
	void dfsVisit(unsigned int vertexIndex);

public:
	WAHStackTC(Graph& graph);
	virtual ~WAHStackTC();

	void computeTransitiveClosure(bool reflexitive, bool storeComponentMembers, int minOutDegreeForMultiOR);
	string tcToString();
	long countNumberOfEdgesInTC();
	long countNumberOfEdgesInCondensedTC();
	int getNumberOfComponents();
	void writeToChacoFile(string filename);
	bool reachable(int src, int dst);
	long memoryUsedByBitSets();
	void reportStatistics();
};

#endif /* WAHSTACKTC_H_ */
