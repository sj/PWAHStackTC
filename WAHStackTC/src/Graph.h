/*
 * Graph.h
 *
 *  Created on: Apr 9, 2010
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef GRAPH_H_
#define GRAPH_H_
#include <vector>
#include <string>
using namespace std;

class Graph {
private:
	vector<vector<int> > _vertices; // adjacency list

public:
	Graph();
	virtual ~Graph();

	int getNumberOfVertices();
	int countNumberOfEdges();
	vector<int>* getChildren(unsigned int vertexIndex);

	static Graph parseChacoFile(string filename);

	static vector<string> split(const std::string &s, char delim);
	static vector<string> &split(const string &s, char delim, vector<string> &elems);
};

#endif /* GRAPH_H_ */
