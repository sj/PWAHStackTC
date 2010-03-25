//============================================================================
// Name        : boost-benchmarks.cpp
// Author      : Sebastiaan J. van Schaik
// Version     :
// Copyright   : 
// Description :
//============================================================================

#include <sys/time.h>
#include <time.h>
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


long difftimeMsecs(timeval time1, timeval time2){
	long msecs1 = time1.tv_sec * 1000 + time1.tv_usec / 1000;
	long msecs2 = time2.tv_sec * 1000 + time2.tv_usec / 1000;
	return msecs2 - msecs1;
}

void go(string filename){


	typedef property < vertex_name_t, char >Name;
	typedef property < vertex_index_t, std::size_t, Name > Index;
	typedef adjacency_list < listS, listS, directedS, Index > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_t;

	graph_t G;
	graph_t G2;
	std::vector < vertex_t > vertices;

	//string filename = "/home/bas/Documents-local/Semmle graphs/graphs/wiki/categorylinks.graph";
	//std::ifstream input_file("/home/bas/temp/nuutila32.graph");
	//std::ifstream input_file("/tmp/graphs/successor.graph");
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




  /*std::vector < vertex_t > verts(4);
  for (int i = 0; i < 4; ++i)
    verts[i] = add_vertex(Index(i, Name('a' + i)), G2);
  add_edge(verts[1], verts[2], G2);
  add_edge(verts[1], verts[3], G2);
  add_edge(verts[2], verts[1], G2);
  add_edge(verts[3], verts[2], G2);
  add_edge(verts[3], verts[0], G2);
  std::cout << "Graph G2:" << std::endl;
  print_graph(G2, get(vertex_name, G2));*/


	//std::cout << "Graph G:" << std::endl;
	cout << "Computing transitive closure... ";
	cout.flush();
	gettimeofday(&tv1, NULL);
	adjacency_list <> TC;
	transitive_closure(G, TC);
	gettimeofday(&tv2, NULL);
	cout << "done! That took " << difftimeMsecs(tv1, tv2) << " msecs" << endl;

	cout << "Transitive closure contains " << num_edges(TC) << " edges.";

  //std::cout << std::endl << "Graph G+:" << std::endl;
  //char name[] = "abcd";
  //print_graph(TC, name);
  //print_graph(TC);
  //std::cout << std::endl;

  //std::ofstream out("tc-out.dot");
  //write_graphviz(out, TC, make_label_writer(name));

	sleep(20);
  return;
}

int main(int, char *[])
{
	try {
		/*go("/home/bas/Documents-local/Semmle graphs/graphs/wiki/categorylinks.graph");
		go("/home/bas/Documents-local/Semmle graphs/graphs/c++/callgraph.graph");*/
		//OOM: go("/home/bas/Documents-local/Semmle graphs/graphs/wiki/categorypagelinks.graph");
		go("/home/bas/Documents-local/Semmle graphs/graphs/java/calls.graph");
	} catch (string e){
		cerr << "Exception: " << e << endl;
	}
}

/*

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

	return 0;
}*/
