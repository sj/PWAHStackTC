/*
 * GraphTestInfo.cpp
 *
 *  Created on: 12 May 2013
 *      Author: bas
 */

#include "GraphTestInfo.h"

const string GraphTestInfo::basedir = "../datasets";

GraphTestInfo::GraphTestInfo(string filename, unsigned int numVertices, unsigned int numEdges, unsigned int numStronglyConnectedComponents, unsigned int numEdgesInTC, unsigned int numEdgesInReflexiveTC):
		filename(filename), numVertices(numVertices), numEdges(numEdges), numStronglyConnectedComponents(numStronglyConnectedComponents), numEdgesInTC(numEdgesInTC), numEdgesInReflexiveTC(numEdgesInReflexiveTC)
{

}

string GraphTestInfo::getFullFilename() const {
	return basedir + "/" + filename;
}

vector<GraphTestInfo> GraphTestInfo::getAllGraphs(){
	vector<GraphTestInfo> res;

	//							FILENAME							numVertices		numEdges	numSSC		numEdgesTC	numEdgesReflTC
	res.push_back(GraphTestInfo("Semmle/Adempiere/depends.graph", 	6332, 			79804,		4679, 	7756079, 	7758212));

	return res;

	/**
	string filenames[numTests] = {
			"Semmle/adempiere/depends.graph",
			"Semmle/adempiere/polycalls.graph",
			"Semmle/wikipedia/pagelinks.graph",
			"Semmle/imagemagick/successor.graph",
			//"Semmle/samba/setflow.graph",
			"SigMod 08/real_data/agrocyc.graph",
			"SigMod 08/real_data/amaze.graph",
			"SigMod 08/real_data/anthra.graph",
			"SigMod 08/real_data/ecoo.graph",
			"SigMod 08/real_data/hpycyc.graph",
			"SigMod 08/real_data/human.graph"
	};

	long expectedNumEdges[numTests] = {
			7756079, // java/depends.graph
			438508149, // polycalls.graph
			12479685213, // pagelinks.graph
			2309714078, // c++/successor.graph
			//10968067063, // samba/setflow.graph
			2731596, // agrocyc.graph
			93685094, // amaze.graph
			2440124, // anthra.graph
			2398250, // ecoo.graph
			1113356, // hpycyc.graph
			2804552 // human.graph
	};

	long expectedReflexiveNumEdges[numTests] = {
			7758212, // java/depends.graph
			438552734, // polycalls.graph (NOT VERIFIED!)
			12479732188, // wiki/pagelinks.graph (NOT VERIFIED!)
			2310250306, // c++/successor.graph (NOT VERIFIED!)
			//10968568966, // samba/setflow.graph (NOT VERIFIED!)
			2744279, // agrocyc.graph
			93688747, // amaze.graph
			2452621, // anthra.graph
			2410868, // ecoo.graph
			1118124, // hpycyc.graph
			2843362 // human.graph
	};**/
}
