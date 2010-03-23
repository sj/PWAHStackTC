//============================================================================
// Name        : boost-benchmarks.cpp
// Author      : Sebastiaan J. van Schaik
// Version     :
// Copyright   : 
// Description :
//============================================================================

#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>
#include <iostream>
using namespace std;
using namespace boost;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while(std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	return split(s, delim, elems);
}

void go(){


	typedef property < vertex_name_t, char >Name;
	typedef property < vertex_index_t, std::size_t, Name > Index;
	typedef adjacency_list < listS, listS, directedS, Index > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_t;

	graph_t G;

	std::ifstream input_file("/home/bas/temp/nuutila32.graph");
	int counter = 0;
	char buffer[4096];
	char delim = ' ';
	//delim << ' ';

	input_file.clear();
	input_file.seekg(0);
	bool firstLine = true;
	signed int numVertices = -1;
	signed int numEdges = -1;
	int lineNo = 1;
	int currVertexIndex, currNeighbourIndex;

	while (!(input_file.eof())){
		stringstream eMsg;
		input_file.getline(buffer,sizeof(buffer), '\n');
		if ((input_file.rdstate() & ifstream::eofbit) != 0){
			// EOF
			break;
		} else if ((input_file.rdstate() & ifstream::failbit) != 0){
			// 'failbit' was set, read failed because line didn't fit in buffer
			eMsg << "Reading line " << lineNo << " failed, buffer too small?";
			throw eMsg.str();
		}

		string line(buffer);
		std::cout << "Parsing line: " << line << std::endl;

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
		} else {
			// Parse 'regular' lines: line number indicates vertex
			// index, integers on the line indicate adjacent vertex
			// indices (counting from 1)
			currVertexIndex = lineNo - 2;
			for (int i = 0; i < neighbours.size(); i++){
				currNeighbourIndex = atoi(neighbours[i].c_str()) - 1;
				cout << currVertexIndex << " has neighbour " << currNeighbourIndex << endl;
			}
		}
		lineNo++;
	}
	cout << "Number of vertices: " << numVertices << endl;
	cout << "Number of edges: " << numEdges << endl;
	input_file.close();

	return;
  /*std::vector < vertex_t > verts(4);
  for (int i = 0; i < 4; ++i)
    verts[i] = add_vertex(Index(i, Name('a' + i)), G);
  add_edge(verts[1], verts[2], G);
  add_edge(verts[1], verts[3], G);
  add_edge(verts[2], verts[1], G);
  add_edge(verts[3], verts[2], G);
  add_edge(verts[3], verts[0], G);*/


  std::cout << "Graph G:" << std::endl;
  print_graph(G, get(vertex_name, G));

  adjacency_list <> TC;
  transitive_closure(G, TC);

  std::cout << std::endl << "Graph G+:" << std::endl;
  char name[] = "abcd";
  print_graph(TC, name);
  std::cout << std::endl;

  std::ofstream out("tc-out.dot");
  write_graphviz(out, TC, make_label_writer(name));

  return;
}

int main(int, char *[])
{
	try {
		go();
	} catch (string e){
		cerr << "Exception: " << e << endl;
	}
}

/*

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

	return 0;
}*/
