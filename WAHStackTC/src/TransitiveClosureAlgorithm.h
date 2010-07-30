/*
 * TransitiveClosureAlgorithm.h
 *
 *  Created on: Jul 6, 2010
 *      Author: bas
 */

#ifndef TRANSITIVECLOSUREALGORITHM_H_
#define TRANSITIVECLOSUREALGORITHM_H_
#include <string>
#include <vector>
#include "BitSet.h"
using namespace std;

class TransitiveClosureAlgorithm {
public:
	TransitiveClosureAlgorithm();
	virtual ~TransitiveClosureAlgorithm();

	virtual void computeTransitiveClosure(bool reflexitive, bool storeComponentMembers, int minOutDegreeForMultiOR) = 0;
	virtual string tcToString() = 0;
	virtual long countNumberOfEdgesInTC() = 0;
	virtual long countNumberOfEdgesInCondensedTC() = 0;
	virtual	long countNumberOfEdgesInCondensedTC(bool ignoreSelfLoops, bool ignoreSingletonSelfLoops) = 0;
	virtual int getNumberOfComponents() = 0;
	virtual void writeToChacoFile(string filename) = 0;
	virtual bool reachable(int src, int dst) = 0;
	virtual long memoryUsedByBitSets() = 0;
	virtual void reportStatistics() = 0;
	virtual string algorithmName() = 0;
	virtual long memoryUsedByIntervalLists() = 0;
	virtual double computeAverageLocalClusteringCoefficient() = 0;
};

#endif /* TRANSITIVECLOSUREALGORITHM_H_ */
