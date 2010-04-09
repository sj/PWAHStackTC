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
	vector<vector<int> > _vertices;

public:
	Graph();
	virtual ~Graph();

	int getNumberOfVertices();

	static Graph* parseChacoFile(string& filename);
};

#endif /* GRAPH_H_ */
