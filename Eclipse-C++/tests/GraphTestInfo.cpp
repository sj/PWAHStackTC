/*
 * GraphTestInfo.cpp
 *
 *  Created on: 12 May 2013
 *      Author: bas
 */

#include "GraphTestInfo.h"

const string GraphTestInfo::basedir = "../../datasets";

GraphTestInfo::GraphTestInfo(string filename, unsigned int numVertices, unsigned int numEdges, unsigned int numStronglyConnectedComponents, unsigned int numEdgesInTC, unsigned int numEdgesInReflexiveTC):
		filename(filename), numVertices(numVertices), numEdges(numEdges), numStronglyConnectedComponents(numStronglyConnectedComponents), numEdgesInTC(numEdgesInTC), numEdgesInReflexiveTC(numEdgesInReflexiveTC)
{

}

string GraphTestInfo::getFullFilename() const {
	return basedir + "/" + filename;
}

vector<GraphTestInfo> GraphTestInfo::getAllGraphs(){
	vector<GraphTestInfo> res;

	//							FILENAME						numVertices		numEdges	numSSC		numEdgesTC	numEdgesReflTC
	res.push_back(GraphTestInfo("Jin2008/agrocyc.graph", 		13969,			17694,		12684, 		2731596,	2744279));
	res.push_back(GraphTestInfo("Jin2008/amaze.graph", 			11877,			28700,		3710, 		93685094,	93688747));
	res.push_back(GraphTestInfo("Jin2008/anthra.graph", 		13736,			17307,		12499, 		2440124,	2452621));
	res.push_back(GraphTestInfo("Jin2008/ecoo.graph", 			13800,			17308,		12620, 		2398250,	2410868));
	res.push_back(GraphTestInfo("Jin2008/hpycyc.graph", 		5565,			8474,		4771, 		1113356,	1118124));
	res.push_back(GraphTestInfo("Jin2008/human.graph", 			40051,			43879,		38811, 		2804552,	2843362));


	//							FILENAME										numVertices		numEdges	numSSC		numEdgesTC		numEdgesReflTC
	res.push_back(GraphTestInfo("Semmle/Adempiere/depends.graph", 				6332, 			79804,		4679, 		7756079, 		7758212));
	//res.push_back(GraphTestInfo("Semmle/Adempiere/polycalls.graph", 			53206, 			595198,		44839, 		438508149, 		438552734));
	//res.push_back(GraphTestInfo("Semmle/Wikipedia/pagelinks.graph", 			137830,			2949220,	47242, 		12479685213,	12479732188));
	res.push_back(GraphTestInfo("Semmle/Wikipedia/categorypagelinks.graph", 	75946,			181084,		75936, 		2262118,		2338037));

	return res;
}
