/*
 * Graph.cpp
 *
 *  Created on: Apr 9, 2010
 *      Author: Sebastiaan J. van Schaik
 */

#include "Graph.h"


Graph::Graph() {
	// TODO Auto-generated constructor stub
}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

int Graph::getNumberOfVertices(){
	return _vertices.size();
}

/**
Graph* Graph::parseChacoFile(string& filename){
	Graph* graph = new Graph();

	cout << "Parsing " << filename << "... ";
	std::ifstream input_file(filename.c_str());
	char buffer[131072];

	input_file.clear();
	input_file.seekg(0);
	bool firstLine = true;
	signed int numVertices = -1;
	signed int numEdges = -1;
	int lineNo = 0;
	int currVertexIndex, currNeighbourIndex;
	int edgeCount = 0;

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

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



		std::vector<std::string> neighbours = split(line, ' ');
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
			vertices = std::vector<vertex_t>(numVertices);

			for (int i = 0; i < numVertices; i++){
				vertices[i] = add_vertex(i,G);
			}
		} else {
			// Parse 'regular' lines: line number indicates vertex
			// index, integers on the line indicate adjacent vertex
			// indices (counting from 1)

			if (lineNo - 1 > numVertices){
				stringstream eMsg;
				eMsg << "Graph file contains at least " << lineNo << " vertex specifications, whilst it should contain only " << numVertices << "?";
				throw eMsg.str();
			}

			currVertexIndex = lineNo - 2;
			for (int i = 0; i < neighbours.size(); i++){
				currNeighbourIndex = atoi(neighbours[i].c_str()) - 1;
				//cout << currVertexIndex << " has neighbour " << currNeighbourIndex << endl;
				add_edge(vertices[currVertexIndex], vertices[currNeighbourIndex], G);
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

	gettimeofday(&tv2, NULL);
	cout << "done, that took " << difftimeMsecs(tv1,tv2) << " msecs" << endl;
	cout << "Number of vertices: " << numVertices << ", number of edges: " << numEdges << endl;
	input_file.close();


	return graph;
}
**/
