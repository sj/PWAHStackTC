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

#include "Validator.h"
#include <string>
#include <iostream>
#include "PerformanceTimer.h"
#include "Graph.h"
#include "PWAHStackTC.h"
#include <stdlib.h>
#include "PWAHBitSet.h"
#include "TransitiveClosureAlgorithm.h"
#include "IntervalBitSet.h"
using namespace std;

Validator::Validator() {}
Validator::~Validator() {}

void Validator::validate(){
	const int numTests = 11;
	const string baseDir = "/home/bas/afstuderen/Datasets";

	string filenames[numTests] = {
		"Semmle graphs/java/depends.graph",
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
		7756079, // java/depends.graph
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

	long expectedReflexiveNumEdges[numTests] = {
		7758212, // java/depends.graph
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
		double time1;
		long numEdges;
		PerformanceTimer timer;
		TransitiveClosureAlgorithm* tca;

		for (int i = 0; i < numTests; i++){
			string filename = baseDir + "/" + filenames[i];

			cout << "======= " << filename << " =======" << endl;
			cout << "Expecting " << expectedNumEdges[i] << " edges in TC, " << expectedReflexiveNumEdges[i] << " edges in reflexive TC..." << endl;
			Graph graph = Graph::parseChacoFile(filename);

			for (int bstype = 0; bstype < 5; bstype++){
				for (int reflexive = 0; reflexive <= 1; reflexive++){
					if (bstype == 0) tca = new PWAHStackTC<PWAHBitSet<2> >(graph);
					else if (bstype == 1) tca = new PWAHStackTC<PWAHBitSet<4> >(graph);
					else if (bstype == 2) tca = new PWAHStackTC<PWAHBitSet<8> >(graph);
					else if (bstype == 3) tca = new PWAHStackTC<WAHBitSet>(graph);
					else if (bstype == 4) tca = new PWAHStackTC<IntervalBitSet>(graph);
					else throw string("???");

					cout << "Computing " << (reflexive == 1 ? "reflexive " : "") << "TC using " << tca->algorithmName() << "... ";
					timer.reset();
					tca->computeTransitiveClosure((reflexive == 1), false, 0);
					time1 = timer.reset();
					cout << "done, that took " << time1 << " msecs." << endl;
					cout << "Memory in use by data structure: " << tca->memoryUsedByBitSets() << " bits." << endl;
					cout << "Validating number of edges in TC: ";
					cout.flush();

					numEdges = tca->countNumberOfEdgesInTC();
					cout << numEdges << ". ";
					if (reflexive == 1){
						if (numEdges != expectedReflexiveNumEdges[i]){
							cout << "MISMATCH!" << endl;
							numFail++;
						} else {
							cout << "OK!" << endl;
							numPass++;
						}
					} else {
						if (numEdges != expectedNumEdges[i]){
							cout << "MISMATCH!" << endl;
							numFail++;
						} else {
							cout << "OK!" << endl;
							numPass++;
						}
					}
					delete tca;
				}
			}

			cout << endl << endl;
		}

		cout << "Total number of successful tests: " << numPass << endl;
		if (numFail > 0){
			cerr << "Total number of failed tests: " << numFail << endl;
			exit(1);
		} else {
			cout << "Total number of failed tests: " << numFail << endl;
			//exit(0);
		}
	} catch (string e){
		cerr << "Error: " << e;
		exit(1);
	}
}
