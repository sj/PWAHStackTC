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
#include "StaticBitSet.h"
#include "DynamicStack.h"
#include <stdexcept>
using namespace std;

WAHStackTC::WAHStackTC(Graph& graph) {
	_graph = &graph;
}

WAHStackTC::~WAHStackTC() {
	// Free allocated memory for WAHBitSet objects representing successor sets
	for (unsigned int i = 0; i < _componentSuccessors.size(); i++) delete _componentSuccessors[i];

	delete[] _vertexComponents;
	delete _vertexSelfLoop;
}

void WAHStackTC::computeTransitiveClosure(bool reflexitive, bool storeComponentMembers, int minOutDegreeForMultiOR){
	unsigned int numVertices = _graph->getNumberOfVertices();
	_cStack = new DynamicStack(numVertices);
	_vStack = new DynamicStack(numVertices);
	_componentSizes.clear();
	_componentSuccessors.clear();
	_componentVertices.clear();
	_savedCStackSize = new unsigned int[numVertices];
	if (_storeComponentVertices) _savedVStackSize = new unsigned int[numVertices];
	_vertexComponents = new int[numVertices];
	_vertexCandidateComponentRoot = new unsigned int[numVertices];
	_visited = DynamicBitSet(numVertices);
	_vertexSelfLoop = new StaticBitSet(numVertices);
	_vertexDFSSeqNo = new int[numVertices];
	_lastDFSSeqNo = -1;
	_lastComponentIndex = -1;
	_reflexitive = reflexitive;
	_storeComponentVertices = storeComponentMembers;
	_minOutDegreeForMultiOR = minOutDegreeForMultiOR;

	for (unsigned int v = 0; v < numVertices; v++){
		if (!_visited.get(v)){
			dfsVisit(v);
		}
	}

	delete _cStack;
	delete _vStack;
	delete[] _savedCStackSize;
	if (_storeComponentVertices) delete[] _savedVStackSize;
	delete[] _vertexCandidateComponentRoot;
	delete[] _vertexDFSSeqNo;
}

void WAHStackTC::dfsVisit(unsigned int vertexIndex){
	const bool debug = false;

	if (debug) cout << "Visiting vertex " << vertexIndex << endl;
	_visited.set(vertexIndex);
	if (_storeComponentVertices) _savedVStackSize[vertexIndex] = _vStack->size();
	_vStack->push(vertexIndex);
	_savedCStackSize[vertexIndex] = _cStack->size();
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
			_vertexSelfLoop->set(vertexIndex);
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
					_cStack->push(_vertexComponents[currChildIndex]);
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
						_cStack->push(_vertexComponents[currChildIndex]);
					} // else: forward edge! Ignore.
				}
			}
		}
	} // end foreach child

	if (_vertexCandidateComponentRoot[vertexIndex] == vertexIndex){
		// This vertex is a component root!
		unsigned int newComponentIndex = ++_lastComponentIndex;
		bool explicitlyStoreSelfLoop = false;

		if (debug) cout << "Vertex " << vertexIndex << " is component root, newComponentIndex=" << newComponentIndex << endl;
		_componentSizes.push_back(0);

		WAHBitSet* successors = new WAHBitSet();
		if (_vStack->peek() != vertexIndex || _vertexSelfLoop->get(vertexIndex)){
			// This component has size > 1 or has an explicit self-loop.
			if (!_storeComponentVertices && !_reflexitive){
				explicitlyStoreSelfLoop = true;
			} // else: no need to explicitly store self-loop
		}

		const bool use_multiway_or = false; //(_cStack.size() - _savedCStackSize[vertexIndex] > _minOutDegreeForMultiOR) && _minOutDegreeForMultiOR >= 0;

		if (!use_multiway_or){
			if (explicitlyStoreSelfLoop){
				// When using regular OR, a self loop should be recorded by pushing the
				// new component index on the stack with adjacent components
				_cStack->push(newComponentIndex);
			}
		}

		// Pop adjacent component indices from the stack. Note that the multiPop method will
		// return a pointer to the first value which is popped off the stack. This pointer
		// points to memory which belongs to the stack, but is no longer in use because the
		// values where just popped off. Subsequent calls to push() will reuse these blocks
		// of memory again, effectively overwriting the values which were popped off the stack!
		// Ergo: never call a push() after having completely finished processing all elements
		// returned by the multiPop(...)
		unsigned int numAdjacentComponents = _cStack->size() - _savedCStackSize[vertexIndex];
		int* adjacentComponentIndices = _cStack->multiPop(numAdjacentComponents);
		int* uniqueAdjacentComponentIndices = new int[numAdjacentComponents];

		// Sort the component indices. This serves two purposes:
		//  1) the WAHBitSet is only capable of setting bits in increasing order
		//  2) it enables us to find duplicates, effectively decreasing the number of required OR-operations
		sort(adjacentComponentIndices, adjacentComponentIndices + numAdjacentComponents);

		// Prepare array of successor lists in case of multi-way OR
		// Note the +1: one slot is allocated in case we'll need to sneak in an extra WAHBitSet
		// for the MultiWay OR to work.
		WAHBitSet** adjacentComponentsSuccessors;
		if (use_multiway_or) adjacentComponentsSuccessors = new WAHBitSet*[numAdjacentComponents + 1];

		// Loop over the adjacent component indices to obtain the successor lists and
		// remove duplicates
		unsigned int numUniqueAdjacentComponents = 0;
		int lastAdjacentComponentIndex = -1;
		for (unsigned int i = 0; i < numAdjacentComponents; i++){
			if (lastAdjacentComponentIndex == adjacentComponentIndices[i]) continue; // skip duplicate

			if (use_multiway_or) adjacentComponentsSuccessors[numUniqueAdjacentComponents] = _componentSuccessors[adjacentComponentIndices[i]];
			uniqueAdjacentComponentIndices[numUniqueAdjacentComponents++] = adjacentComponentIndices[i];

			lastAdjacentComponentIndex = adjacentComponentIndices[i];
		}

		// Make sure this pointer is never used again, because it points to values within a globally
		// used stack...
		adjacentComponentIndices = NULL;
		numAdjacentComponents = -1;

		if (use_multiway_or){
			// Multi way merge

			// First, construct an extra (sparse) WAHBitSet with the bits representing the indices
			// of directly adjacent components set. Note that the array 'adjacentComponentIndices'
			// should be sorted for this to work, since bits of the WAHBitSet can only be set in
			// increasing order.
			WAHBitSet* adjacentComponentBits = new WAHBitSet();
			int lastIndex = -1;
			for (unsigned int i = 0; i < numUniqueAdjacentComponents; i++){
				if (uniqueAdjacentComponentIndices[i] == lastIndex) continue;
				adjacentComponentBits->set(uniqueAdjacentComponentIndices[i]);
				lastIndex = uniqueAdjacentComponentIndices[i];
			}

			// Sneak the extra WAHBitSet into the list of WAHBitSet objects
			// (remember the +1 in the size specification of this array!)
			adjacentComponentsSuccessors[numUniqueAdjacentComponents] = adjacentComponentBits;

			// Note the numAdjacentComponents + 1. The +1 indicates the extra WAHBitSet sneaked
			// into the list
			WAHBitSet::multiOr(adjacentComponentsSuccessors, numUniqueAdjacentComponents + 1, successors);

			delete adjacentComponentBits;

			if (explicitlyStoreSelfLoop){
				successors->set(newComponentIndex);
			}
		} else {
			// Simple OR
			unsigned int adjacentComponentIndex;
			for (unsigned int i = 0; i < numUniqueAdjacentComponents; i++){
				adjacentComponentIndex = uniqueAdjacentComponentIndices[i];

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
		}

		delete[] uniqueAdjacentComponentIndices;
		if (use_multiway_or) delete[] adjacentComponentsSuccessors;

		_componentSuccessors.push_back(successors);


		if (debug) cout << "Adding vertices to new component... ";
		if (debug) cout.flush();

		if (_storeComponentVertices){
			// Prepare storing a list of vertices of this component
			_componentVertices.push_back(new int[_vStack->size() - _savedVStackSize[vertexIndex]]);
		}

		unsigned int currStackVertexIndex;
		unsigned int i = 0;
		do {
			currStackVertexIndex = _vStack->pop();
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
				if (_vertexSelfLoop->get(_componentVertices[componentIndex][0])){
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

	if (src == dst) return _vertexSelfLoop->get(src);

	int srcComponent = _vertexComponents[src];
	int dstComponent = _vertexComponents[dst];
	if (srcComponent == dstComponent){
		// Source and destination are within the same component!
		// Determine whether the component contains a self-loop
		if (_reflexitive || _componentSizes[src] > 1){
			return true;
		} else if (_componentSizes[src] == 1 && _storeComponentVertices){
			return _vertexSelfLoop->get(src);
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
