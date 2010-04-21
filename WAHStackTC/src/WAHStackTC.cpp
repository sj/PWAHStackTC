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
#include "PerformanceTimer.h"
#include "WAHBitSetIterator.h"
#include <stdexcept>
using namespace std;

WAHStackTC::WAHStackTC(Graph& graph) {
	_graph = &graph;
}

WAHStackTC::~WAHStackTC() {
	// Free allocated memory for WAHBitSet objects representing successor sets
	for (unsigned int i = 0; i < _componentSuccessors.size(); i++) delete _componentSuccessors[i];

	delete[] _vertexComponents;


	cout << "Destructed WAHStackTC" << endl;
}

void WAHStackTC::computeTransitiveClosure(){
	unsigned int numVertices = _graph->getNumberOfVertices();
	_cStack = stack<unsigned int>();
	_vStack = stack<unsigned int>();
	_componentSizes.clear();
	_componentSuccessors.clear();
	_savedStackSize = new unsigned int[numVertices];
	_vertexComponents = new int[numVertices];
	_vertexCandidateComponentRoot = new unsigned int[numVertices];
	_visited = DynamicBitSet(numVertices);
	_vertexSelfLoop = DynamicBitSet(numVertices);
	_vertexDFSSeqNo = new int[numVertices];
	_lastDFSSeqNo = -1;
	_lastComponentIndex = -1;

	for (unsigned int v = 0; v < numVertices; v++){
		if (!_visited.get(v)) dfsVisit(v);
	}


	delete[] _savedStackSize;
	delete[] _vertexCandidateComponentRoot;
	delete[] _vertexDFSSeqNo;
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
		unsigned int newComponentIndex = ++_lastComponentIndex;
		if (debug) cout << "Vertex " << vertexIndex << " is component root, newComponentIndex=" << newComponentIndex << endl;
		_componentSizes.push_back(0);

		WAHBitSet* successors = new WAHBitSet();
		//DynamicBitSet* successors = new DynamicBitSet(newComponentIndex + 1);
		if (_vStack.top() != vertexIndex || _vertexSelfLoop.get(vertexIndex)){
			// This component has size > 1 or has an explicit self-loop. Include the
			// component index as successor of itself
			_cStack.push(newComponentIndex);
		}

		// Make an overestimation of the number of adjacent components
		int* adjacentComponentIndices = new int[_cStack.size() - _savedStackSize[vertexIndex]];
		DynamicBitSet tmpAdjacentComponentBits;

		// Pop off all adjacent components from the stack and remove duplicates
		unsigned int numAdjacentComponents = 0;
		while (_cStack.size() > _savedStackSize[vertexIndex]){
			if (tmpAdjacentComponentBits.get(_cStack.top())) continue; // skip duplicate
			adjacentComponentIndices[numAdjacentComponents++] = _cStack.top();
			_cStack.pop();
		}

		sort(adjacentComponentIndices, adjacentComponentIndices + numAdjacentComponents);
		//cout << "checkpoint c: " << _timer.currRunTimeMicro() << " usecs" << endl;

		unsigned int adjacentComponentIndex;
		for (unsigned int i = 0; i < numAdjacentComponents; i++){
			adjacentComponentIndex = adjacentComponentIndices[i];

			if (adjacentComponentIndex == newComponentIndex){
				// Self-loop: full union not needed
				successors->set(newComponentIndex);
			} else {
				if (!successors->get(adjacentComponentIndex)){
					// Merge this successor list with the successor list of the adjacent component
					successors->set(adjacentComponentIndex);
					if (debug) cout << "Merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... ";
					if (debug) cout.flush();
					//cout << successors.toString() << endl;
					//cout << _componentSuccessors[adjacentComponentIndex].toString() << endl;
					WAHBitSet* oldSuccessors = successors;
					successors = WAHBitSet::constructByOr(successors, _componentSuccessors[adjacentComponentIndex]);
					delete oldSuccessors;
					oldSuccessors = NULL;

					//successors = DynamicBitSet::constructByOr(successors, _componentSuccessors[adjacentComponentIndex]);
					if (debug) cout << "done!" << endl;
					//cout << successors.toString() << endl;
				} else {
					// adjacent component index already in the successors list, skip merge
					if (debug) cout << "Not merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... " << endl;
				}
			}
		}

		delete[] adjacentComponentIndices;

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
	for (unsigned int c = 0; c < _componentSuccessors.size(); c++){
		stream << "Successors of component " << c << ": ";
		BitSet* successors = _componentSuccessors[c];
		for (unsigned int i = 0; i <= c; i++){
			if (successors->get(i)) stream << i << " ";
		}
		stream << endl;
	}
	return stream.str();
}

long WAHStackTC::countNumberOfEdgesInTC(){
	int* componentVertexSuccessorCount = new int[_componentSizes.size()];
	int currSuccessorIndex;
	long count;

	for (unsigned int c = 0; c < _componentSizes.size(); c++){
		count = 0;

		WAHBitSet* successors = _componentSuccessors[c];
		WAHBitSetIterator iter(successors);
		currSuccessorIndex = iter.next();
		while (currSuccessorIndex != -1){
			count += _componentSizes[currSuccessorIndex];
			currSuccessorIndex = iter.next();
		}
		componentVertexSuccessorCount[c] = count;
	}

	count = 0;
	for (int v = 0; v < _graph->getNumberOfVertices(); v++){
		count += componentVertexSuccessorCount[_vertexComponents[v]];
	}

	delete[] componentVertexSuccessorCount;

	return count;
}

bool WAHStackTC::reachable(int src, int dst){
	if (src >= _graph->getNumberOfVertices()) throw range_error("Source index out of bounds");
	if (dst >= _graph->getNumberOfVertices()) throw range_error("Source index out of bounds");

	int srcComponent = _vertexComponents[src];
	int dstComponent = _vertexComponents[dst];
	return _componentSuccessors[srcComponent]->get(dstComponent);
}
