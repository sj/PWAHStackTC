//============================================================================
// Name        : WAHStackTC.cpp
// Author      : Sebastiaan J. van Schaik
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "Graph.h"
#include <string>
using namespace std;

int main() {
	string filename = "../../Datasets/Semmle graphs/java/depends.graph";
	Graph* graph = Graph::parseChacoFile(filename);

	cout << "Number of vertices: " << graph->getNumberOfVertices() << endl;

	delete graph;
	return 0;
}
