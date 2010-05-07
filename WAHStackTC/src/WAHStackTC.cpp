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
}

void WAHStackTC::computeTransitiveClosure(bool reflexitive, bool storeComponentMembers){
	unsigned int numVertices = _graph->getNumberOfVertices();
	_cStack = stack<unsigned int>();
	_vStack = stack<unsigned int>();
	_componentSizes.clear();
	_componentSuccessors.clear();
	_componentVertices.clear();
	_savedCStackSize = new unsigned int[numVertices];
	_savedVStackSize = new unsigned int[numVertices];
	_vertexComponents = new int[numVertices];
	_vertexCandidateComponentRoot = new unsigned int[numVertices];
	_visited = DynamicBitSet(numVertices);
	_vertexSelfLoop = DynamicBitSet(numVertices);
	_vertexDFSSeqNo = new int[numVertices];
	_lastDFSSeqNo = -1;
	_lastComponentIndex = -1;
	_reflexitive = reflexitive;
	_storeComponentVertices = storeComponentMembers;

	for (unsigned int v = 0; v < numVertices; v++){
		if (!_visited.get(v)) dfsVisit(v);
	}


	delete[] _savedCStackSize;
	delete[] _savedVStackSize;
	delete[] _vertexCandidateComponentRoot;
	delete[] _vertexDFSSeqNo;
}

void WAHStackTC::dfsVisit(unsigned int vertexIndex){
	const bool debug = false;

	if (debug) cout << "Visiting vertex " << vertexIndex << endl;
	_visited.set(vertexIndex);
	_savedVStackSize[vertexIndex] = _vStack.size();
	_vStack.push(vertexIndex);
	_savedCStackSize[vertexIndex] = _cStack.size();
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
		if (_vStack.top() != vertexIndex || _vertexSelfLoop.get(vertexIndex)){
			// This component has size > 1 or has an explicit self-loop.
			if (!_storeComponentVertices && !_reflexitive){
				_cStack.push(newComponentIndex);
			} // else: no need to explicitly store self-loop
		}

		// Make an overestimation of the number of adjacent components
		int* adjacentComponentIndices = new int[_cStack.size() - _savedCStackSize[vertexIndex]];
		DynamicBitSet tmpAdjacentComponentBits;

		// Pop off all adjacent components from the stack and remove duplicates
		unsigned int numAdjacentComponents = 0;
		while (_cStack.size() > _savedCStackSize[vertexIndex]){
			if (tmpAdjacentComponentBits.get(_cStack.top())) continue; // skip duplicate
			adjacentComponentIndices[numAdjacentComponents++] = _cStack.top();
			_cStack.pop();
		}

		sort(adjacentComponentIndices, adjacentComponentIndices + numAdjacentComponents);

		unsigned int adjacentComponentIndex;
		for (unsigned int i = 0; i < numAdjacentComponents; i++){
			adjacentComponentIndex = adjacentComponentIndices[i];

			if (adjacentComponentIndex == newComponentIndex){
				// No need for a full merge, just set the relevant bit.
				successors->set(newComponentIndex);
			} else {
				if (!successors->get(adjacentComponentIndex)){
					// Merge this successor list with the successor list of the adjacent component
					successors->set(adjacentComponentIndex);
					if (debug) cout << "Merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... ";
					if (debug) cout.flush();

					WAHBitSet* oldSuccessors = successors;
					successors = WAHBitSet::constructByOr(successors, _componentSuccessors[adjacentComponentIndex]);
					delete oldSuccessors;
					oldSuccessors = NULL;

					if (debug) cout << "done!" << endl;
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

		if (_storeComponentVertices){
			// Prepare storing a list of vertices of this component
			_componentVertices.push_back(new int[_vStack.size() - _savedVStackSize[vertexIndex]]);
		}

		unsigned int currStackVertexIndex;
		unsigned int i = 0;
		do {
			currStackVertexIndex = _vStack.top();
			_vStack.pop();
			_vertexComponents[currStackVertexIndex] = newComponentIndex;
			_componentSizes[newComponentIndex]++;

			if (_storeComponentVertices){
				_componentVertices[newComponentIndex][i++] = currStackVertexIndex;
			}
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

long WAHStackTC::countNumberOfEdgesInCondensedTC(){
	long count = 0;
	int currSuccessorIndex;


	for (unsigned int c = 0; c < _componentSizes.size(); c++){
		WAHBitSetIterator iter(_componentSuccessors[c]);
		currSuccessorIndex = iter.next();
		while (currSuccessorIndex != -1){
			count++;
			currSuccessorIndex = iter.next();
		}

		if (_reflexitive || _storeComponentVertices){
			// Self-loops were not stored explicitly
			if (this->componentHasSelfLoop(c)) count++;
		}
	}
	return count;
}

bool WAHStackTC::componentHasSelfLoop(int componentIndex){
	if (_reflexitive || _storeComponentVertices){
		// Self-loops were not stored explicitly, since their existence can be shown
		// implicitly.

		if (!_reflexitive){
			// Some components may have self-loops, others not
			if (_componentSizes[componentIndex] > 1){
				// Self-loop implied
				return true;
			} else {
				// Component contains only one vertex
				if (_vertexSelfLoop.get(_componentVertices[componentIndex][0])){
					// That vertex has a self-loop
					return true;
				} // else: vertex doesn't have a a self-loop, therefore, the component has no self-loop
			}
		} else {
			// Reflexitive transitive closure: all components have self-loops
			return true;
		}
	} else {
		// Self-loops were stored explicitly, this function should not be used
		throw string("WAHStackTC::componentHasSelfLoop can not be used when self-loops are stored explicitly!");
	}
	return false;
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

		if (_reflexitive || _storeComponentVertices){
			// Self-loops were not stored explicitly, since their existence can be shown
			// implicitly.
			if (this->componentHasSelfLoop(c)) count += _componentSizes[c];
		} // else: self-loops were stored explicitly, no reason to count them twice...

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

	if (src == dst) return _vertexSelfLoop.get(src);

	int srcComponent = _vertexComponents[src];
	int dstComponent = _vertexComponents[dst];
	if (srcComponent == dstComponent){
		// Source and destination are within the same component!
		// Determine whether the component contains a self-loop
		if (_reflexitive || _componentSizes[src] > 1){
			return true;
		} else if (_componentSizes[src] == 1 && _storeComponentVertices){
			return _vertexSelfLoop.get(src);
		} // else: unable to determine which vertex is the only member of the component...
	}

	return _componentSuccessors[srcComponent]->get(dstComponent);
}

long WAHStackTC::memoryUsedByBitSets(){
	long totalBits = 0;
	for (unsigned int i = 0; i < _componentSizes.size(); i++){
		totalBits += _componentSuccessors[i]->memoryUsage();
	}
	return totalBits;
}
void WAHStackTC::reportStatistics(){
	cout << "Number of vertices: " << _graph->getNumberOfVertices() << endl;
	cout << "Number of edges: " << _graph->countNumberOfEdges() << endl;
	cout << "Number of strongly connected components: " << _componentSizes.size() << endl;

	cout << "Number of bits required to store all WAH bitsets: " << this->memoryUsedByBitSets() << endl;
}

/**
 * Stores the transitive closure in a Chaco-formatted file. Can be huge!
 */
void WAHStackTC::writeToChacoFile(string filename){

}

int WAHStackTC::getNumberOfComponents(){
	return _componentSizes.size();
}
