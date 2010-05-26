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
	vector<int> _vertexIndegree; // indegree of vertices

public:
	Graph();
	virtual ~Graph();

	unsigned int getNumberOfVertices();
	unsigned int countNumberOfEdges();

	float computeAvgOutDegree();
	int computeMaxOutDegree();
	int findMaxOutDegreeVertex();
	int computeMinOutDegree();
	int findMinOutDegreeVertex();

	int computeMaxInDegree();
	int findMaxInDegreeVertex();
	float computeAvgInDegree();
	int computeMinInDegree();
	int findMinInDegreeVertex();

	unsigned int vertexInDegree(int v);

	vector<int>* getChildren(unsigned int vertexIndex);

	static Graph parseChacoFile(string filename);

	static vector<string> split(const std::string &s, char delim);
	static vector<string> &split(const string &s, char delim, vector<string> &elems);
};

#endif /* GRAPH_H_ */
