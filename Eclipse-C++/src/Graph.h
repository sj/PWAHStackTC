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

	double computeAverageLocalClusteringCoefficient();
	double computeLocalClusteringCoefficient(int vertexIndex);
	int computeMaxInDegree();
	int findMaxInDegreeVertex();
	float computeAvgInDegree();
	int computeMinInDegree();
	int findMinInDegreeVertex();

	unsigned int vertexInDegree(int v);

	vector<int>* getChildren(unsigned int vertexIndex);

	static Graph parseChacoFile(string filename);

	static vector<string> split(const string &s, char delim);
	static vector<string> &split(const string &s, char delim, vector<string> &elems);
};

#endif /* GRAPH_H_ */
