/*
 * WAHStackTC.cpp
 *
 *  Created on: Apr 19, 2010
 *      Author: bas
 */

#include "WAHStackTC.h"
#include "Graph.h"
#include "DynamicBitSet.h"
#include <iostream>
#include <sstream>
#include <algorithm> // for sorting vectors
using namespace std;

WAHStackTC::WAHStackTC(Graph& graph) {
	_graph = &graph;
}

WAHStackTC::~WAHStackTC() {}

void WAHStackTC::computeTransitiveClosure(){
	unsigned int numVertices = _graph->getNumberOfVertices();
	_cStack = stack<unsigned int>();
	_vStack = stack<unsigned int>();
	_componentSizes.clear();
	_componentSuccessors.clear();
	_savedStackSize = vector<unsigned int>(numVertices);
	_vertexComponents = vector<int>(numVertices);
	_vertexCandidateComponentRoot = vector<unsigned int>(numVertices);
	_visited = DynamicBitSet(numVertices);
	_vertexSelfLoop = DynamicBitSet(numVertices);
	_vertexDFSSeqNo = vector<unsigned int>(numVertices);
	_lastDFSSeqNo = -1;
	_lastComponentIndex = -1;

	for (unsigned int v = 0; v < numVertices; v++){
		if (!_visited.get(v)) dfsVisit(v);
	}

	cout << "done!" << endl;
}

void WAHStackTC::dfsVisit(unsigned int vertexIndex){
	const bool debug = false;

	if (debug) cout << "Visiting vertex " << vertexIndex << endl;
	_visited.set(vertexIndex);
	_vStack.push(vertexIndex);
	_savedStackSize[vertexIndex] = _cStack.size();
	_vertexComponents[vertexIndex] = -1;
	_vertexDFSSeqNo[vertexIndex] = ++_lastDFSSeqNo;

	// At first, the candidate component root for this vertex is this vertex itself
	_vertexCandidateComponentRoot[vertexIndex] = vertexIndex;

	vector<int>* children = _graph->getChildren(vertexIndex);
	unsigned int currChildIndex;
	bool childWasVisitedBefore;
	for (unsigned int i = 0; i < children->size(); i++){
		currChildIndex = children->at(i);
		if (debug) cout << "Child index " << currChildIndex << " of vertex " << vertexIndex << endl;
		if (currChildIndex == vertexIndex){
			// Vertex loops back to itself
			_vertexSelfLoop.set(vertexIndex);
		} else {
			childWasVisitedBefore = true;
			if (!_visited.get(currChildIndex)){
				// Child was not visited earlier. Keep that in mind and visit the child.
				childWasVisitedBefore = false;
				dfsVisit(currChildIndex);
			}

			if (_vertexComponents[currChildIndex] == -1){
				if (debug) cout << "Child " << currChildIndex << " of vertex " << vertexIndex << " not yet a member of a component" << endl;
				// Child is not yet a member of a wrapped-up component, which means
				// it belongs to the same component as this vertex belongs to!

				// Check whether the child has discovered a better candidate
				// component root
				int childCandidateComponentRoot = _vertexCandidateComponentRoot[currChildIndex];
				int thisCandidateComponentRoot = _vertexCandidateComponentRoot[vertexIndex];
				if (_vertexDFSSeqNo[childCandidateComponentRoot] < _vertexDFSSeqNo[thisCandidateComponentRoot]){
					// Candidate root of the child vertex has a lower DFS sequence number:
					// adept the candidate root!
					_vertexCandidateComponentRoot[vertexIndex] = childCandidateComponentRoot;
				}
			} else {
				// Child is a member of a component!
				if (debug) cout << "Child " << currChildIndex << " of vertex " << vertexIndex << " is a member of component " << _vertexComponents[currChildIndex] << endl;
				if (!childWasVisitedBefore){
					// Child was first visited by this vertex.

					// Register the component of the child as being adjacent to the
					// component of the current vertex by pushing it on top of the stack
					if (debug) cout << "component " << _vertexComponents[currChildIndex] << " is adjacent to vertex " << vertexIndex << endl;
					_cStack.push(_vertexComponents[currChildIndex]);
				} else {
					// Child was not visited directly by this vertex. However, this might
					// be a forward edge, which means that the child was visited by another child
					// of this vertex.
					// In the latter case, the DFS sequence no of the child would be higher
					// than the DFS sequence no of this vertex.
					if (_vertexDFSSeqNo[currChildIndex] < _vertexDFSSeqNo[vertexIndex]){
						// Not a forward edge, register the component of the child
						// as adjacent
						if (debug) cout << "component " << _vertexComponents[currChildIndex] << " is adjacent to vertex " << vertexIndex << endl;
						_cStack.push(_vertexComponents[currChildIndex]);
					} // else: forward edge! Ignore.
				}
			}
		}
	} // end foreach child

	if (_vertexCandidateComponentRoot[vertexIndex] == vertexIndex){
		// This vertex is a component root!
		int newComponentIndex = ++_lastComponentIndex;
		if (debug) cout << "Vertex " << vertexIndex << " is component root, newComponentIndex=" << newComponentIndex << endl;
		_componentSizes.push_back(0);

		WAHBitSet successors;
		if (_vStack.top() != vertexIndex || _vertexSelfLoop.get(vertexIndex)){
			// This component has size > 1 or has an explicit self-loop. Include the
			// component index as successor of itself
			_cStack.push(newComponentIndex);
		}

		// Pop off all adjacent components from the stack
		vector<int> dupAdjacentComponentIndices;
		while (_cStack.size() > _savedStackSize[vertexIndex]){
			dupAdjacentComponentIndices.push_back(_cStack.top());
			_cStack.pop();
		}


		// Eliminate duplicates in the vector using a temporary BitSet
		vector<int> adjacentComponentIndices;
		DynamicBitSet tmpAdjacentComponentBits;
		int adjacentComponentIndex;
		if (debug) cout << "Successors of new component: ";
		for (unsigned int i = 0; i < dupAdjacentComponentIndices.size(); i++){
			adjacentComponentIndex = dupAdjacentComponentIndices[i];
			if (tmpAdjacentComponentBits.get(adjacentComponentIndex)) continue; // duplicate
			if (debug) cout << adjacentComponentIndex << " ";
			tmpAdjacentComponentBits.set(adjacentComponentIndex);
			adjacentComponentIndices.push_back(adjacentComponentIndex);
		}
		if (debug) cout << endl;

		sort(adjacentComponentIndices.begin(), adjacentComponentIndices.end());

		for (unsigned int i = 0; i < adjacentComponentIndices.size(); i++){
			adjacentComponentIndex = adjacentComponentIndices[i];

			if (adjacentComponentIndex == newComponentIndex){
				// Self-loop: full union not needed
				successors.set(newComponentIndex);
			} else {
				if (!successors.get(adjacentComponentIndex)){
					// Merge this successor list with the successor list of the adjacent component
					successors.set(adjacentComponentIndex);
					if (debug) cout << "Merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... ";
					if (debug) cout.flush();
					//cout << successors.toString() << endl;
					//cout << _componentSuccessors[adjacentComponentIndex].toString() << endl;
					successors = WAHBitSet::constructByOr(successors, _componentSuccessors[adjacentComponentIndex]);
					if (debug) cout << "done!" << endl;
					//cout << successors.toString() << endl;
				} else {
					// adjacent component index already in the successors list, skip merge
					if (debug) cout << "Not merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... " << endl;
				}
			}
		}

		_componentSuccessors.push_back(successors);


		if (debug) cout << "Adding vertices to new component... ";
		if (debug) cout.flush();
		unsigned int currStackVertexIndex;
		do {
			currStackVertexIndex = _vStack.top();
			_vStack.pop();
			_vertexComponents[currStackVertexIndex] = newComponentIndex;
			_componentSizes[newComponentIndex]++;
		} while (vertexIndex != currStackVertexIndex);
		if (debug) cout << "done!";
	} // else: vertex is not a component root


	if (debug) cout << "Done visiting " << vertexIndex << ", returning DFS call" << endl;
}

string WAHStackTC::tcToString(){
	stringstream stream;
	stream << "== Vertices and components ==" << endl;
	for (int v = 0; v < _graph->getNumberOfVertices(); v++){
		stream << "Vertex " << v << ": component " << _vertexComponents[v] << endl;
	}
	stream << endl;
	stream << "== Component successors ==" << endl;
	for (int c = 0; c < _componentSuccessors.size(); c++){
		stream << "Successors of component " << c << ": ";
		WAHBitSet successors = _componentSuccessors[c];
		for (int i = 0; i <= c; i++){
			if (successors.get(i)) stream << i << " ";
		}
		stream << endl;
	}
	return stream.str();
}

long WAHStackTC::countNumberOfEdgesInTC(){
	// TODO: optimise by keeping track of cardinality
	vector<int> componentVertexSuccessorCount(_componentSizes.size());

	for (int c = 0; c < _componentSizes.size(); c++){
		int count = 0;
		WAHBitSet successors = _componentSuccessors[c];
		for (int i = 0; i <= c; i++){
			if (successors.get(i)) count += _componentSizes[i];
		}
		componentVertexSuccessorCount[c] = count;
	}

	long edgeCount = 0;
	for (int v = 0; v < _graph->getNumberOfVertices(); v++){
		edgeCount += componentVertexSuccessorCount[_vertexComponents[v]];
	}
	return edgeCount;
}
