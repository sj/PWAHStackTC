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

#include "PWAHStackTC.h"
#include "../datastructures/Graph.h"
#include "../datastructures/bitsets/DynamicBitSet.h"
#include <iostream>
#include <sstream>
#include <algorithm> // for sorting vectors
#include "../util/PerformanceTimer.h"
#include "../datastructures/bitsets/StaticBitSet.h"
#include "../datastructures/DynamicStack.h"
#include "../datastructures/bitsets/pwah/PWAHBitSet.h"
#include "../datastructures/bitsets/wah/WAHBitSet.h"
#include <stdexcept>
#include "../datastructures/bitsets/interval/IntervalBitSet.h"
#include <assert.h>
#include <climits>
using namespace std;

template<class B> PWAHStackTC<B>::PWAHStackTC(Graph& graph) {
	_graph = &graph;
}

template<class B> PWAHStackTC<B>::~PWAHStackTC() {
	// Free allocated memory for WAHBitSet objects representing successor sets
	for (unsigned int i = 0; i < _componentSuccessors.size(); i++) delete _componentSuccessors[i];

	delete[] _vertexComponents;
	delete _vertexSelfLoop;
}

template<class B> void PWAHStackTC<B>::computeTransitiveClosure(bool reflexive, bool storeComponentMembers, int minOutDegreeForMultiOR){
	unsigned int numVertices = _graph->getNumberOfVertices();

	_cStack = new DynamicStack(numVertices);
	_vStack = new DynamicStack(numVertices);
	_componentSizes.clear();
	_componentSuccessors.clear();
	_componentVertices.clear();
	_vertexComponents = new int[numVertices];
	_vertexCandidateComponentRoot = new unsigned int[numVertices];
	_visited = DynamicBitSet(numVertices);
	_vertexSelfLoop = new StaticBitSet(numVertices);
	_vertexDFSSeqNo = new int[numVertices];
	_lastDFSSeqNo = -1;
	_lastComponentIndex = -1;
	_reflexive = reflexive;
	_storeComponentVertices = storeComponentMembers;
	_minOutDegreeForMultiOR = minOutDegreeForMultiOR;
	_mergeTimer.resetAndStop();

	for (unsigned int v = 0; v < numVertices; v++){
		if (!_visited.get(v)){
			dfsVisit(v);
		}
	}

	delete _cStack;
	delete _vStack;
	delete[] _vertexCandidateComponentRoot;
	delete[] _vertexDFSSeqNo;
}

template<class B> void PWAHStackTC<B>::dfsVisit(unsigned int vertexIndex){
	const bool debug = false;

	if (debug) cout << "Visiting vertex " << vertexIndex << endl;

	// Save the current height of the two stacks - we'll need this information later when we're
	// constructing a strongly connected component (if this vertex turns out to be the root of
	// a SCC)
	const unsigned int savedVStackSize = _vStack->size();
	const unsigned int savedCStackSize = _cStack->size();

	_visited.set(vertexIndex);
	_vStack->push(vertexIndex);
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

		if (((unsigned int)_vStack->peek()) != vertexIndex || _vertexSelfLoop->get(vertexIndex)){
			// This component has size > 1 or has an explicit self-loop.
			if (!_storeComponentVertices && !_reflexive){
				explicitlyStoreSelfLoop = true;
			} // else: no need to explicitly store self-loop
		} // else: no self loop

		B* successors = NULL;
		unsigned int numAdjacentComponents = _cStack->size() - savedCStackSize;
		_componentSizes.push_back(0);

		// Merge operations
		if (numAdjacentComponents == 0){
			// No adjacent components! Nothing to merge!
			if (explicitlyStoreSelfLoop){
				successors = new B();
				successors->set(newComponentIndex);
			} else {
				// else: no need to store a WAHBitSet for this component!
			}
		} else {
			// Number of adjacent components > 0
			successors = new B();

			const bool use_multiway_or = _minOutDegreeForMultiOR >= 0 && (numAdjacentComponents > (unsigned int)_minOutDegreeForMultiOR);

			if (!use_multiway_or){
				if (explicitlyStoreSelfLoop){
					// When using regular OR, a self loop should be recorded by pushing the
					// new component index on the stack with adjacent components. In case of
					// multi-way or, the self loop will be added later on in the process
					_cStack->push(newComponentIndex);
					numAdjacentComponents++;
				}
			}

			// Pop adjacent component indices from the stack. Note that the multiPop method will
			// return a pointer to the first value which is popped off the stack. This pointer
			// points to memory which belongs to the stack, but is no longer in use because the
			// values where just popped off. Subsequent calls to push() will reuse these blocks
			// of memory again, effectively overwriting the values which were popped off the stack!
			// Ergo: never call a push() after having completely finished processing all elements
			// returned by the multiPop(...)
			int* adjacentComponentIndices = _cStack->multiPop(numAdjacentComponents);
			int* uniqueAdjacentComponentIndices = new int[numAdjacentComponents];

			// Sort the component indices in increasing order. Ergo: in order of detection.
			// This serves two purposes:
			//  1) the WAHBitSet is only capable of setting bits in increasing order
			//  2) it enables us to find duplicates, effectively decreasing the number of required OR-operations
			sort(adjacentComponentIndices, adjacentComponentIndices + numAdjacentComponents);

			// Prepare array of successor lists in case of multi-way OR
			// Note the +1: one slot is allocated in case we'll need to sneak in an extra WAHBitSet
			// for the MultiWay OR to work.
			B** adjacentComponentsSuccessors = NULL;
			if (use_multiway_or) adjacentComponentsSuccessors = new B*[numAdjacentComponents + 1];

			// Loop over the adjacent component indices to obtain the successor lists and
			// remove duplicates
			unsigned int numUniqueAdjacentComponents = 0;
			unsigned int numUniqueNonNullAdjacentComponents = 0;

			int lastAdjacentComponentIndex = -1;
			for (unsigned int i = 0; i < numAdjacentComponents; i++){
				if (lastAdjacentComponentIndex == adjacentComponentIndices[i]) continue; // skip duplicate

				if (use_multiway_or && _componentSuccessors[adjacentComponentIndices[i]] != NULL){
					adjacentComponentsSuccessors[numUniqueNonNullAdjacentComponents++] = _componentSuccessors[adjacentComponentIndices[i]];
				}
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
				_mergeTimer.resume();
				B* adjacentComponentBits = new B();

				int lastIndex = -1;
				for (unsigned int i = 0; i < numUniqueAdjacentComponents; i++){
					if (uniqueAdjacentComponentIndices[i] == lastIndex) continue;
					adjacentComponentBits->set(uniqueAdjacentComponentIndices[i]);
					lastIndex = uniqueAdjacentComponentIndices[i];
				}

				// Sneak the extra WAHBitSet into the list of WAHBitSet objects
				// (remember the +1 in the size specification of this array!)
				adjacentComponentsSuccessors[numUniqueNonNullAdjacentComponents] = adjacentComponentBits;

				// Note the numAdjacentComponents + 1. The +1 indicates the extra WAHBitSet sneaked
				// into the list
				B::multiOr(adjacentComponentsSuccessors, numUniqueNonNullAdjacentComponents + 1, successors);

				delete adjacentComponentBits;

				if (explicitlyStoreSelfLoop){
					successors->set(newComponentIndex);
				} else {
					successors->set(newComponentIndex, false);
				}
				_mergeTimer.pause();
			} else {
				// Simple OR
				_mergeTimer.resume();
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

							if (_componentSuccessors[adjacentComponentIndex] != NULL){
								if (debug) cout << "Merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... ";
								if (debug) cout.flush();

								B* oldSuccessors = successors;
								successors = B::constructByOr(successors, _componentSuccessors[adjacentComponentIndex]);
								delete oldSuccessors;
								oldSuccessors = NULL;

								if (debug) cout << "done!" << endl;
							}
						} else {
							// adjacent component index already in the successors list, skip merge
							if (debug) cout << "Not merging successor list of component " <<  newComponentIndex << " with " << adjacentComponentIndex << "... " << endl;
						}
					}
				}
				_mergeTimer.pause();
			}

			// Clean up dynamically allocated memory used for merge operations
			delete[] uniqueAdjacentComponentIndices;
			if (use_multiway_or) delete[] adjacentComponentsSuccessors;
		} // END merge operations


		// Store vertices when necessary
		if (_storeComponentVertices){
			// Prepare storing a list of vertices of this component
			_componentVertices.push_back(new int[_vStack->size() - savedVStackSize]);
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

		_componentSuccessors.push_back(successors);
	} // else: vertex is not a component root


	if (debug) cout << "Done visiting " << vertexIndex << ", returning DFS call" << endl;
}

template<class B> string PWAHStackTC<B>::tcToString(){
	stringstream stream;
	stream << "== Vertices and components ==" << endl;
	for (unsigned int v = 0; v < _graph->getNumberOfVertices(); v++){
		stream << "Vertex " << v << ": component " << _vertexComponents[v] << endl;
	}
	stream << endl;
	stream << "== Component successors ==" << endl;
	for (unsigned int c = 0; c < _componentSuccessors.size(); c++){
		stream << "Successors of component " << c << ": ";
		BitSet* successors = _componentSuccessors[c];
		if (successors != NULL){
			for (unsigned int i = 0; i <= c; i++){
				if (successors->get(i)) stream << i << " ";
			}
		}
		stream << endl;
	}
	return stream.str();
}

/**
 * \brief Counts the number of edges in the condensed transitive closure. This includes all kinds of self-loops
 *
 * Edges in the condensed transitive closure are basically all edges between components. Duplicate edges
 * between components (i.e. when multiple vertices from component A are connected to one or more vertices
 * in component B) are ignored.
 */
template<class B> long PWAHStackTC<B>::countNumberOfEdgesInCondensedTC(){
	return countNumberOfEdgesInCondensedTC(false, false);
}

/**
 * \brief Counts the number of edges in the condensed transitive closure. Optionally, some kinds of
 * self-loops are not counted
 *
 * Edges in the condensed transitive closure are basically all edges between components. Duplicate edges
 * between components (i.e. when multiple vertices from component A are connected to one or more vertices
 * in component B) are ignored.
 *
 * \param ignoreSelfLoops Ignore all self-loops in the condensed transitive closure (note that
 * a component has a self loop when the number of vertices in the component > 1!)
 * \param ignoreSingletonSelfLoops Ignore all self-loops of singleton components (components consisting
 * of only one vertex)
 */
template<class B> long PWAHStackTC<B>::countNumberOfEdgesInCondensedTC(bool ignoreSelfLoops, bool ignoreSingletonSelfLoops){
	unsigned long count = 0;
	int currSuccessorIndex;

	if (ignoreSelfLoops) ignoreSingletonSelfLoops = true;

	for (unsigned int c = 0; c < _componentSizes.size(); c++){
		if (_componentSuccessors[c] != NULL){
			BitSetIterator* iter = _componentSuccessors[c]->iterator();

			currSuccessorIndex = iter->next();
			while (currSuccessorIndex != -1){

				// The number of if-statements can be reduced, but is preserved like this for
				// sake of clarity.
				if ((unsigned int)currSuccessorIndex == c){
					// Self-loop!
					if (!ignoreSelfLoops){
						// Self-loops need to be processed
						if (_componentSizes[c] == 1 && !ignoreSingletonSelfLoops){
							// Trivial component (consisting of only 1 vertex)
							count++;
						} else if (_componentSizes[c] > 1){
							// Non-trivial component: count self loop
							count++;
						}
					} // else: all self-loops are ignored
				} else {
					// Not a self-loop
					count++;
				}

				currSuccessorIndex = iter->next();
			}
			delete iter;
		}

		if (!ignoreSelfLoops){
			if (_reflexive || _storeComponentVertices){
				// Self-loops were not stored explicitly
				if (this->componentHasSelfLoop(c)) count++;
			}
		}
	}
	return count;
}

template<class B> bool PWAHStackTC<B>::componentHasSelfLoop(int componentIndex){
	if (_reflexive || _storeComponentVertices){
		// Self-loops were not stored explicitly, since their existence can be shown
		// implicitly.

		if (!_reflexive){
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
		throw string("PWAHStackTC::componentHasSelfLoop can not be used when self-loops are stored explicitly!");
	}
	return false;
}

template<class B> long PWAHStackTC<B>::countNumberOfEdgesInTC(){
	/**
	// Brute force without using iterators:
	int compA, compB;
	long counter = 0;
	for (int i = 0; i < _graph->getNumberOfVertices(); i++){
		compA = _vertexComponents[i];
		for (int j = 0; j < _graph->getNumberOfVertices(); j++){
			compB = _vertexComponents[j];

			if (_componentSuccessors[compA] != NULL && _componentSuccessors[compA]->get(compB)) counter++;
		}
	}

	return counter;**/

	// Use iterators:
	/**for (int i = 0; i < _componentSizes.size(); i++){
		if (_componentSuccessors[i] != NULL) BitSetTester::testIterator(_componentSuccessors[i], false);
	}**/

	int* componentVertexSuccessorCount = new int[_componentSizes.size()];
	int currSuccessorIndex;
	long count;

	for (unsigned int c = 0; c < _componentSizes.size(); c++){
		count = 0;

		if (_componentSuccessors[c] != NULL){
			BitSetIterator* iter = _componentSuccessors[c]->iterator();
			currSuccessorIndex = iter->next();
			while (currSuccessorIndex != -1){
				count += _componentSizes[currSuccessorIndex];
				currSuccessorIndex = iter->next();
			}
			delete iter;
		}

		if (_reflexive || _storeComponentVertices){
			// Self-loops were not stored explicitly, since their existence can be shown
			// implicitly.
			if (this->componentHasSelfLoop(c)) count += _componentSizes[c];
		} // else: self-loops were stored explicitly, no reason to count them twice...

		componentVertexSuccessorCount[c] = count;
	}

	count = 0;
	for (unsigned int v = 0; v < _graph->getNumberOfVertices(); v++){
		count += componentVertexSuccessorCount[_vertexComponents[v]];
	}

	delete[] componentVertexSuccessorCount;

	return count;
}

/**
 * Determines for each possible source/target pair whether they are reachable or not. Returns the number
 * or pairs that were reachable and stores the result in the reachable vector.
 *
 * This implementation is more efficient than actually doing individual queries using "reachable", as it will
 * only iterate over the reachability PWAHBitSet for components once (so if multiple sources are in the same
 * strongly connected component, this saves time), and it will iterate over the PWAHBitSet to determine the 
 * reachability of each target.
 *
 * Result stored in reachable:
 *   reachable[si1] = { t1, t2, t3, tn }
 *
 * Where 
 *  - si1 is the index of the source vertex in the sources vector (i.e., si1 is not a vertex index itself!)
 *  - t1, ..., tn are the reachable target vertices (i.e., t1 is a vertex index of the target vertex)
 *
 * Note that reachable_pairs is a vector of pointers to vectors {t1, ..., tn} : if source vertices share 
 * the same strongly connected component, they wil also share the same vector {t1, ..., tn} 
 *
 * The calling method is responsible for freeing the memory in the reachable array!
 * 
 */
template<> void PWAHStackTC<PWAHBitSet<8> >::reachablepairs(vector<unsigned int>& sources, vector<unsigned int>& targets, vector<vector<unsigned int> >& reachable){
	if (sources.size() == 0 || targets.size() == 0) return;
	reachable.reserve(sources.size());


	// First, find out the strongly connected components of the target vertices

	// Will hold, for each component index, the vertices in the 'targets' vector that belong to that component
	// (initialise with a NULL pointer for each component)
	vector<vector<unsigned int>* > target_component_vertices = vector<vector<unsigned int>* >(_componentSuccessors.size(), NULL);
	
	// Will hold a simple list of target component indices we're interested in
	vector<unsigned int> target_components;

	// Populate target_components and target_component_vertices. We need this because we will have to
	// return the vertex indices, rather than the component indices.
	for (std::vector<unsigned int>::iterator it_tgt = targets.begin(); it_tgt != targets.end(); ++it_tgt){
		const unsigned int target_vertex_index = *it_tgt;
		
		if (target_vertex_index >= _graph->getNumberOfVertices()) throw range_error("Target index out of bounds");

		const unsigned int target_comp_index = _vertexComponents[target_vertex_index];

		if (target_component_vertices[target_comp_index] == NULL){
			target_component_vertices[target_comp_index] = new vector<unsigned int>();
			target_components.push_back(target_comp_index);
		}
		target_component_vertices[target_comp_index]->push_back(target_vertex_index);
	}

	// Sort the component indices in target_components
	sort(target_components.begin(), target_components.end());

	// Stores for each source component whether we've stored its result in the 'reachable' vector already. This
	// happens when multiple source vertices share the same component.
	vector<unsigned int> components_in_reachable = vector<unsigned int>(_componentSuccessors.size(), UINT_MAX);

	for (unsigned int i = 0; i < sources.size(); i++){
		const unsigned int src_vertex_index = sources[i];
		
		if (src_vertex_index >= _graph->getNumberOfVertices()) throw range_error("Source index out of bounds");
		const unsigned int src_comp_index = _vertexComponents[src_vertex_index];

		if (components_in_reachable[src_comp_index] != UINT_MAX){
			// We already have a result for this source component (i.e., we've seen a source vertex from this
			// strongly connected component before). No need to iterate over the bitset, just copy
			// the previous result
			reachable.push_back(reachable[components_in_reachable[src_comp_index]]);
		} else {
			// Iterate over bit vectors to see which target components (and vertices) are reachable
			reachable.push_back(vector<unsigned int>());
			components_in_reachable[src_comp_index] = i;

			if (_componentSuccessors[src_comp_index] == NULL){
				// This component can't reach anything - leave result vector empty
				continue;
			}

			// Implementation of a single scan over a compressed bitset using the 'incr_get' method of
			// PWAHBitSet. This method requires that the queries to the PWAHBitSet are in increasing order
			// of bit index, but then guarantees only a single pass is made over the data structure, which
			// is of course much more efficient than scanning over the entire PWAHBitSet for every single
			// query.
			for (unsigned int j = 0; j < target_components.size(); j++){
				const unsigned int curr_target_component_index = target_components[j];

				bool target_reachable = false;
				bool decided = false;

				if (src_comp_index == curr_target_component_index){
					// Source and target lie in the same component. If the component has size > 1, then any
					// target vertex can be reached from any source within that same component. If the component
					// consists of a single vertex, then reachability requires:
					//		1) a self-loop  OR
					// 		2) a reflexive transitive closure

					if (_reflexive || _componentSizes[src_comp_index] > 1){
						// Always reachable
						target_reachable = true;
						decided = true;
					} else if (_storeComponentVertices) {
						// Self-loops were not stored explicitly, since their existence can be shown implicitly.
						target_reachable = this->componentHasSelfLoop(src_comp_index);
						decided = true;
					} else {
						// Self-loops were stored explicitly
						decided = false;
					}
				}

				if (!decided){
					target_reachable = _componentSuccessors[src_comp_index]->incr_get(curr_target_component_index);
					decided = true;
				}

				if (target_reachable){
					// add all target vertices of that component to the list of reachable vertices.
					reachable[i].insert(reachable[i].end(),
							target_component_vertices[curr_target_component_index]->begin(),
							target_component_vertices[curr_target_component_index]->end()
					);
				}
			}
		}
	}

	// Be nice and free some memory
	for (unsigned int i = 0; i < target_component_vertices.size(); i++){
		if (target_component_vertices[i] != NULL) delete target_component_vertices[i];
	}
}

/**
 * \brief Answers a reachability query for two vertices 'src' and 'dst'.
 */
template<class B> bool PWAHStackTC<B>::reachable(unsigned int src, unsigned int dst){
	const bool DEBUGGING = false;
	if (src >= _graph->getNumberOfVertices()) throw range_error("Source index out of bounds");
	if (dst >= _graph->getNumberOfVertices()) throw range_error("Source index out of bounds");
	if (DEBUGGING) cout << "PWAHStackTC::reachable " << src << " -> " << dst << " (vertex indices)" << endl;

	int srcComponent = _vertexComponents[src];
	int dstComponent = _vertexComponents[dst];

	if (DEBUGGING) cout << "PWAHStackTC::reachable " << srcComponent << " -> " << dstComponent << " (component indices)" << endl;

	if (_componentSuccessors[srcComponent] == NULL){
		// Source component didn't have any adjacent components
		return false;
	} else if (srcComponent == dstComponent){
		// Source and destination are within the same component!
		// Determine whether the component contains a self-loop
		if (_reflexive || _componentSizes[srcComponent] > 1){
			return true;
		} else if (_componentSizes[srcComponent] == 1 && _storeComponentVertices){
			return _vertexSelfLoop->get(src);
		} // else: unable to determine which vertex is the only member of the component...
	}

	return _componentSuccessors[srcComponent]->get(dstComponent);
}

/**
 * \brief Returns the total amount of memory (unit: bits) used by the reachability data structures in PWAHStackTC.
 *
 * @see totalMemoryUsage();
 */
template<class B> long PWAHStackTC<B>::memoryUsedByBitSets(){
	long totalBits = 0;
	for (unsigned int i = 0; i < _componentSizes.size(); i++){
		if (_componentSuccessors[i] != NULL) totalBits += _componentSuccessors[i]->memoryUsage();
	}
	return totalBits;
}

/**
 * \brief Returns the total amount of memory (unit: number of bits) used by PWAHStackTC to answer reachability queries.
 *
 * This amounts to the number of bits used by the backing data structure (e.g. PWAHBitSet<8>), plus
 * 32 bits for every vertex index (to store the mapping from vertex -> strongly connected component).
 */
template<class B> long PWAHStackTC<B>::totalMemoryUsage(){
	long byBitSets = memoryUsedByBitSets();
	return byBitSets + 32 * _graph->getNumberOfVertices();
}

template<class B> string PWAHStackTC<B>::getStatistics(){
	return "";
}

template<> long PWAHStackTC<IntervalBitSet>::countTotalNumberOfIntervals(){
	long count = 0;
	for(unsigned int i = 0; i < _componentSuccessors.size(); i++){
		if (_componentSuccessors[i] != NULL){
			count += _componentSuccessors[i]->numberOfIntervals();
		}
	}
	return count;
}

template<> int PWAHStackTC<IntervalBitSet>::countMaxNumberOfIntervals(){
	unsigned int max = 0;
	for(unsigned int i = 0; i < _componentSuccessors.size(); i++){
		if (_componentSuccessors[i] != NULL){
			if (max < _componentSuccessors[i]->numberOfIntervals()) max = _componentSuccessors[i]->numberOfIntervals();
		}
	}

	return max;
}

template<> float PWAHStackTC<IntervalBitSet>::countAverageNumberOfIntervals(bool countNulls){
	long total = this->countTotalNumberOfIntervals();

	if (countNulls){
		return total / ((float) _componentSuccessors.size());
	} else {
		// Count number of (non-null) interval lists
		int count = 0;
		for(unsigned int i = 0; i < _componentSuccessors.size(); i++){
			if (_componentSuccessors[i] != NULL){
				count++;
			}
		}

		return total / ((float) count);
	}
}

template<> string PWAHStackTC<IntervalBitSet>::getStatistics(){
	stringstream ss;
	//ss << "Number of vertices: " << _graph->getNumberOfVertices() << endl;
	/**ss << "Number of edges: " << _graph->countNumberOfEdges() << endl;
	ss << "Number of strongly connected components: " << _componentSizes.size() << endl;
	ss << "Number of bits required to store all WAH bitsets: " << this->memoryUsedByBitSets() << endl;**/
	ss << "Total number of intervals in all interval lists: " << this->countTotalNumberOfIntervals() << endl;
	ss << "Average number (including 0-count of NULL-lists) of intervals in interval lists: " << this->countAverageNumberOfIntervals(true) << endl;
	ss << "Average number (excluding 0-count of NULL-lists) of intervals in interval lists: " << this->countAverageNumberOfIntervals(false) << endl;
	ss << "Number of intervals in largest interval list: " << this->countMaxNumberOfIntervals();
	return ss.str();
}



template<class B> string PWAHStackTC<B>::algorithmName(){
	stringstream ss;
	B inst;
	ss << "PWAHStackTC<" << inst.bsImplementationName() << ">";
	return ss.str();
}

/**
 * Stores the transitive closure in a Chaco-formatted file. Can be huge!
 */
template<class B> void PWAHStackTC<B>::writeToChacoFile(string filename){

}

template<class B> int PWAHStackTC<B>::getNumberOfComponents(){
	return _componentSizes.size();
}

template<class B> long PWAHStackTC<B>::memoryUsedByIntervalLists(){
	long res = 0;
	for (unsigned int i = 0; i < _componentSuccessors.size(); i++){
		if (_componentSuccessors[i] != NULL){
			IntervalBitSet* ibs = new IntervalBitSet();
			IntervalBitSet::copy(_componentSuccessors[i], ibs);
			res += ibs->memoryUsage();
			delete ibs;
		}
	}
	return res;
}

template<class B> double PWAHStackTC<B>::computeAverageLocalClusteringCoefficient(){
	double total = 0;

	for (unsigned int i = 0; i < _componentSuccessors.size(); i++){
		total += computeLocalClusteringCoefficient(i);
	}

	return total / _componentSuccessors.size();
}

template<class B> double PWAHStackTC<B>::computeLocalClusteringCoefficient(int componentIndex){
	if (_componentSuccessors[componentIndex] == NULL) return 0;
	StaticBitSet neighbourhood = StaticBitSet(_componentSuccessors.size());
	int neighbourhoodSize = 0;

	// Register neighbourhood in StaticBitSet
	BitSetIterator* it = _componentSuccessors[componentIndex]->iterator();
	int currNeighbourIndex = it->next();
	while (it->next() >= 0){
		if (currNeighbourIndex != componentIndex){ // skip self-loops
			cout << "component " << componentIndex << ": " << currNeighbourIndex << " is neighbour" << endl;
			neighbourhood.set(currNeighbourIndex);
			neighbourhoodSize++;
		}
		currNeighbourIndex = it->next();
	}
	delete it;
	cout << "Neighbourhood of component " << componentIndex << " contains " << neighbourhoodSize << " components" << endl;
	if(neighbourhoodSize == 0) return 0;

	int numEdgesInNeighbourhood = 0;
	int currNeighbourNeighbourIndex;
	it = _componentSuccessors[componentIndex]->iterator();
	BitSetIterator* nIt;
	currNeighbourIndex = it->next();
	while (it->next() >= 0){
		// Process currSuccessorIndex, check whether it neighbours are also neighbours
		// of componentIndex

		if (_componentSuccessors[currNeighbourIndex] != NULL){
			cout << "Neighbourhood of " << componentIndex << ": iterating over neighbours of " << currNeighbourIndex << endl;
			nIt = _componentSuccessors[currNeighbourIndex]->iterator();
			currNeighbourNeighbourIndex = nIt->next();

			while (currNeighbourNeighbourIndex >= 0){
				if (neighbourhood.get(currNeighbourNeighbourIndex)){
					cout << "Neighbourhood of " << componentIndex << ": " << currNeighbourIndex << " has neighbour " << currNeighbourNeighbourIndex << ", which is also a neighbour of " << componentIndex << endl;
					numEdgesInNeighbourhood++;
				}
				currNeighbourNeighbourIndex = nIt->next();
			}
			delete nIt;
		}

		currNeighbourIndex = it->next();
	}
	delete it;

	double res = (double) numEdgesInNeighbourhood / (neighbourhoodSize * neighbourhoodSize);
	cout << componentIndex << " has local correlation coefficient of " << res << endl;
	assert(res <= 1);
	return res;
}

template<class B> const vector<B*> PWAHStackTC<B>::getComponentSuccessors() const {
	return _componentSuccessors;
}

/**
 * Instruct the compiler which templates to instantiate
 */
template class PWAHStackTC<WAHBitSet>;
template class PWAHStackTC<IntervalBitSet>;
template class PWAHStackTC<PWAHBitSet<1> >;
template class PWAHStackTC<PWAHBitSet<2> >;
template class PWAHStackTC<PWAHBitSet<4> >;
template class PWAHStackTC<PWAHBitSet<8> >;
