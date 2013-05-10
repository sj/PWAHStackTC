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
	virtual bool reachable(unsigned int src, unsigned int dst) = 0;
	virtual long memoryUsedByBitSets() = 0;
	virtual long totalMemoryUsage() = 0;
	virtual string getStatistics() = 0;
	virtual string algorithmName() = 0;
	virtual long memoryUsedByIntervalLists() = 0;
	virtual double computeAverageLocalClusteringCoefficient() = 0;
};

#endif /* TRANSITIVECLOSUREALGORITHM_H_ */
