/*
 * Validator.cpp
 *
 *  Created on: May 19, 2010
 *      Author: bas
 */

#include "Validator.h"
#include <string>
#include <iostream>
#include "PerformanceTimer.h"
#include "Graph.h"
#include "WAHStackTC.h"
#include <stdlib.h>
using namespace std;

Validator::Validator() {}
Validator::~Validator() {}

void Validator::validate(){
	const int numTests = 10;
	const string baseDir = "/home/bas/afstuderen/Datasets";

	string filenames[numTests] = {
		"Semmle graphs/java/polycalls.graph",
		"Semmle graphs/wiki/pagelinks.graph",
		"Semmle graphs/c++/successor.graph",
		"Semmle graphs/samba/setflow.graph",
		"SigMod 08/real_data/agrocyc.graph",
		"SigMod 08/real_data/amaze.graph",
		"SigMod 08/real_data/anthra.graph",
		"SigMod 08/real_data/ecoo.graph",
		"SigMod 08/real_data/hpycyc.graph",
		"SigMod 08/real_data/human.graph"
	};

	long expectedNumEdges[numTests] = {
		438508149, // polycalls.graph
		12479685213, // pagelinks.graph
		2309714078, // c++/successor.graph
		10968067063, // samba/setflow.graph
		2731596, // agrocyc.graph
		93685094, // amaze.graph
		2440124, // anthra.graph
		2398250, // ecoo.graph
		1113356, // hpycyc.graph
		2804552 // human.graph
	};

	long expectedReflexitiveNumEdges[numTests] = {
		438552734, // polycalls.graph (NOT VERIFIED!)
		12479732188, // wiki/pagelinks.graph (NOT VERIFIED!)
		2310250306, // c++/successor.graph (NOT VERIFIED!)
		10968568966, // samba/setflow.graph (NOT VERIFIED!)
		2744279, // agrocyc.graph
		93688747, // amaze.graph
		2452621, // anthra.graph
		2410868, // ecoo.graph
		1118124, // hpycyc.graph
		2843362 // human.graph
	};

	try {
		int numFail = 0;
		int numPass = 0;
		double time1, time2;
		long numEdges;
		PerformanceTimer timer;
		WAHStackTC<WAHBitSet>* wstc;

		for (int i = 0; i < numTests; i++){
			string filename = baseDir + "/" + filenames[i];

			cout << "======= " << filename << " =======" << endl;
			cout << "Expecting " << expectedNumEdges[i] << " edges in TC, " << expectedReflexitiveNumEdges[i] << " edges in reflexitive TC..." << endl;
			Graph graph = Graph::parseChacoFile(filename);

			wstc = new WAHStackTC<WAHBitSet>(graph);
			timer.reset();
			wstc->computeTransitiveClosure(false, false, 5);
			time1 = timer.reset();
			cout << "Computation of TC took " << time1 << " msecs. Number of edges in TC: ";
			numEdges = wstc->countNumberOfEdgesInTC();
			cout << numEdges << ". ";
			if (numEdges != expectedNumEdges[i]){
				cout << "MISMATCH!" << endl;
				numFail++;
			} else {
				cout << "OK!" << endl;
				numPass++;
			}
			delete wstc;

			wstc = new WAHStackTC<WAHBitSet>(graph);
			timer.reset();
			wstc->computeTransitiveClosure(true, false, 5);
			time2 = timer.reset();
			cout << "Computation of reflexitive TC took " << time2 << " msecs. Number of edges in reflexitive TC: ";
			numEdges = wstc->countNumberOfEdgesInTC();
			cout << numEdges << ". ";
			if (numEdges != expectedReflexitiveNumEdges[i]){
				cout << "MISMATCH!" << endl;
				numFail++;
			} else {
				cout << "OK!" << endl;
				numPass++;
			}
			delete wstc;

			cout << endl << endl;
		}

		cout << "Total number of successful tests: " << numPass << endl;
		if (numFail > 0){
			cerr << "Total number of failed tests: " << numFail << endl;
			exit(1);
		} else {
			cout << "Total number of failed tests: " << numFail << endl;
			exit(0);
		}
	} catch (string e){
		cerr << "Error: " << e;
		exit(1);
	}
}
