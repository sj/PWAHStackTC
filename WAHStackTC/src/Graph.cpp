/*
 * Graph.cpp
 *
 *  Created on: Apr 9, 2010
 *      Author: Sebastiaan J. van Schaik
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
using namespace std;

Graph::Graph() {
	// TODO Auto-generated constructor stub
}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

int Graph::getNumberOfVertices(){
	return _vertices.size();
}

vector<int>* Graph::getChildren(unsigned int vertexIndex){
	return &_vertices[vertexIndex];
}

int Graph::countNumberOfEdges(){
	int numEdges = 0;
	for (int v = 0; v < this->getNumberOfVertices(); v++){
		numEdges += _vertices[v].size();
	}
	return numEdges;
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

Graph Graph::parseChacoFile(string filename){
	Graph graph;

	PerformanceTimer timer = PerformanceTimer::start();
	cout << "Parsing " << filename << "... ";
	ifstream input_file(filename.c_str());
	char buffer[131072];

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
			for (int i = 0; i < neighbours.size(); i++){
				currNeighbourIndex = atoi(neighbours[i].c_str()) - 1;
				cout << currVertexIndex << " has neighbour " << currNeighbourIndex << endl;
				graph._vertices[currVertexIndex].push_back(currNeighbourIndex);
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

	cout << "done, that took " << timer.reset() << " msecs" << endl;
	cout << "Number of vertices: " << numVertices << ", number of edges: " << numEdges << endl;
	input_file.close();


	return graph;
}



