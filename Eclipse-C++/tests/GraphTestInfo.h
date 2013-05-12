/*
 * GraphTestInfo.h
 *
 *  Created on: 12 May 2013
 *      Author: bas
 */

#ifndef GRAPHTESTINFO_H_
#define GRAPHTESTINFO_H_

#include <string>
#include <vector>
using namespace std;

struct GraphTestInfo {
	const static string basedir;
	string filename;
	unsigned int numVertices;
	unsigned int numEdges;
	unsigned int numStronglyConnectedComponents;
	unsigned int numEdgesInTC;
	unsigned int numEdgesInReflexiveTC;


	GraphTestInfo(string filename, unsigned int numEdges, unsigned int numVertices, unsigned int numEdgesInTC,
			unsigned int numEdgesInReflexiveTC, unsigned int numStronglyConnectedComponents);

	string getFullFilename() const;
	static vector<GraphTestInfo> getAllGraphs();
};

#endif /* GRAPHTESTINFO_H_ */
