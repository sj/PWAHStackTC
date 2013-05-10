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

#include "Graph.h"
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "PerformanceTimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <assert.h>
#include "StaticBitSet.h"
using namespace std;

Graph::Graph() {}
Graph::~Graph() {}

unsigned int Graph::getNumberOfVertices(){
	return _vertices.size();
}

vector<int>* Graph::getChildren(unsigned int vertexIndex){
	return &_vertices[vertexIndex];
}

unsigned int Graph::countNumberOfEdges(){
	int numEdges = 0;
	for (unsigned int v = 0; v < this->getNumberOfVertices(); v++){
		numEdges += _vertices[v].size();
	}
	return numEdges;
}

double Graph::computeLocalClusteringCoefficient(int vertexIndex){
	StaticBitSet neighbourhood = StaticBitSet(_vertices.size());
	int neighbourhoodSize = 0;

	// Populate neighbourhood
	for (unsigned int i = 0; i < _vertices[vertexIndex].size(); i++){
		if (_vertices[vertexIndex][i] == vertexIndex) continue; // ignore self-loops
		if (neighbourhood.get(_vertices[vertexIndex][i])) continue; // ignore duplicate edges

		neighbourhood.set(_vertices[vertexIndex][i]);
		neighbourhoodSize++;
	}

	if (neighbourhoodSize > 0){
		// Loop over adjacent vertices a_i and check whether a_i is connected
		// to an other adjacent vertex a_j
		int currNeighbour;
		int numEdgesInNeighbourhood = 0;
		for (unsigned int i = 0; i < _vertices[vertexIndex].size(); i++){
			if (_vertices[vertexIndex][i] == vertexIndex) continue; // ignore self-loops
			if (!neighbourhood.get(_vertices[vertexIndex][i])) continue; // ignore duplicate edges

			currNeighbour = _vertices[vertexIndex][i];

			// Loop over adjacent vertices of the neighbour
			for (unsigned int j = 0; j < _vertices[currNeighbour].size(); j++){
				if (neighbourhood.get(_vertices[currNeighbour][j])){
					numEdgesInNeighbourhood++;
				}
			}
		}

		double localC = (double) numEdgesInNeighbourhood / (neighbourhoodSize * neighbourhoodSize);
		if (localC > 1){
			cerr << "vertex " << vertexIndex << " local clustering coefficient=" << localC << ", numEdgesInNeighbourhood=" << numEdgesInNeighbourhood << ", neighbourhoodSize=" << neighbourhoodSize << endl;
			assert(localC <= 1);
		}

		return localC;
	} else {
		return 0;
	}
}

double Graph::computeAverageLocalClusteringCoefficient(){
	double sum = 0;

	for (unsigned int i = 0; i < _vertices.size(); i++){
		sum += computeLocalClusteringCoefficient(i);
	}

	return (double) sum / _vertices.size();
}


vector<string> &Graph::split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while(getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> Graph::split(const string &s, char delim) {
	vector<string> elems;
	return split(s, delim, elems);
}

float Graph::computeAvgOutDegree(){
	long totalOutDegree = -1;
	for (unsigned int v = 0; v < getNumberOfVertices(); v++){
		totalOutDegree += getChildren(v)->size();
	}
	return (float) totalOutDegree / getNumberOfVertices();
}

float Graph::computeAvgInDegree(){
	long totalInDegree = -1;
	for (unsigned int v = 0; v < getNumberOfVertices(); v++){
		totalInDegree += _vertexIndegree[v];
	}
	return (float) totalInDegree / getNumberOfVertices();
}

int Graph::findMaxOutDegreeVertex(){
	if (getNumberOfVertices() == 0) return -1;

	unsigned int maxOutDegree = getChildren(0)->size();
	unsigned int maxOutDegreeVertex = 0;
	for (unsigned int v = 1; v < getNumberOfVertices(); v++){
		if (getChildren(v)->size() > maxOutDegree){
			maxOutDegree = getChildren(v)->size();
			maxOutDegreeVertex = v;
		}
	}
	return maxOutDegreeVertex;
}

int Graph::findMinOutDegreeVertex(){
	if (getNumberOfVertices() == 0) return -1;

	unsigned int minOutDegree = getChildren(0)->size();
	unsigned int minOutDegreeVertex = 0;
	for (unsigned int v = 1; v < getNumberOfVertices(); v++){
		if (getChildren(v)->size() < minOutDegree){
			minOutDegree = getChildren(v)->size();
			minOutDegreeVertex = v;
		}
	}
	return minOutDegreeVertex;
}

int Graph::findMinInDegreeVertex(){
	if (getNumberOfVertices() == 0) return -1;

	int minInDegree = _vertexIndegree[0];
	int minInDegreeVertex = 0;
	for (unsigned int v = 1; v < getNumberOfVertices(); v++){
		if (_vertexIndegree[v] < minInDegree){
			minInDegree = getChildren(v)->size();
			minInDegreeVertex = v;
		}
	}
	return minInDegreeVertex;
}

int Graph::computeMinInDegree(){
	int v = findMinInDegreeVertex();
	if (v == -1) return -1;

	return _vertexIndegree[v];
}

int Graph::findMaxInDegreeVertex(){
	if (getNumberOfVertices() == 0) return -1;

	unsigned int maxInDegree = getChildren(0)->size();
	unsigned int maxInDegreeVertex = 0;
	for (unsigned int v = 1; v < getNumberOfVertices(); v++){
		if (_vertexIndegree[v] > maxInDegree){
			maxInDegree = _vertexIndegree[v];
			maxInDegreeVertex = v;
		}
	}
	return maxInDegreeVertex;
}

unsigned int Graph::vertexInDegree(int v){
	return _vertexIndegree[v];
}

int Graph::computeMinOutDegree(){
	int v = findMinOutDegreeVertex();
	if (v == -1) return -1;

	return getChildren(v)->size();
}

int Graph::computeMaxOutDegree(){
	int v = findMaxOutDegreeVertex();
	if (v == -1) return -1;

	return getChildren(v)->size();
}

int Graph::computeMaxInDegree(){
	int v = findMaxInDegreeVertex();
	if (v == -1) return -1;

	return _vertexIndegree[v];
}

Graph Graph::parseChacoFile(string filename){
	const bool debug = false;
	Graph graph;

	PerformanceTimer timer = PerformanceTimer::start();
	ifstream input_file(filename.c_str());
	char buffer[2621440];

	input_file.clear();
	input_file.seekg(0);
	bool firstLine = true;
	signed int numVertices = -1;
	signed int numEdges = -1;
	int lineNo = 0;
	int currVertexIndex, currNeighbourIndex;
	int edgeCount = 0;

	while (!(input_file.eof())){
		stringstream eMsg;
		input_file.getline(buffer,sizeof(buffer), '\n');
		if ((input_file.rdstate() & ifstream::eofbit) != 0){
			// EOF
			break;
		} else if ((input_file.rdstate() & ifstream::failbit) != 0){
			// 'failbit' was set, read failed because line didn't fit in buffer
			eMsg << "Reading line " << (lineNo + 1) << " failed, buffer too small?";
			throw eMsg.str();
		}

		lineNo++;
		string line(buffer);
		//std::cout << "Parsing line: " << line << std::endl;



		vector<string> neighbours = split(line, ' ');
		if (firstLine){
			// Parse first line consisting of two integers: number
			// of vertices and the number of edges
			firstLine = false;
			if (neighbours.size() != 2){
				eMsg << "First line of graph file contains " << neighbours.size() << " elements? Expecting exactly 2." << endl;
				eMsg << "Contents of first line: " << line;
				throw eMsg.str();
			}
			numVertices = atoi(neighbours[0].c_str());
			numEdges = atoi(neighbours[1].c_str());

			graph._vertices = vector<vector<int> >(numVertices);
			graph._vertexIndegree = vector<int>(numVertices);
		} else {
			// Parse 'regular' lines: line number indicates vertex
			// index, integers on the line indicate adjacent vertex
			// indices (counting from 1)

			if (lineNo - 1 > numVertices){
				stringstream eMsg;
				eMsg << "Graph file contains at least " << lineNo << " vertex specifications, while it should contain only " << numVertices << "?";
				throw eMsg.str();
			}

			currVertexIndex = lineNo - 2;
			for (unsigned int i = 0; i < neighbours.size(); i++){
				currNeighbourIndex = atoi(neighbours[i].c_str()) - 1;
				if (debug) cout << currVertexIndex << " has neighbour " << currNeighbourIndex << endl;
				graph._vertices[currVertexIndex].push_back(currNeighbourIndex);
				graph._vertexIndegree[currNeighbourIndex]++;
				edgeCount++;
			}
		}
	}

	if (lineNo - 1 != numVertices){
		stringstream eMsg;
		eMsg << "Graph file contains only " << lineNo << " vertex specifications, whilst it should contain " << numVertices << "?";
		throw eMsg.str();
	}

	if (edgeCount != numEdges){
		stringstream eMsg;
		eMsg << "Graph file contains " << edgeCount << " edges, whilst it should contain " << numEdges << "?";
		throw eMsg.str();
	}

	input_file.close();


	return graph;
}



