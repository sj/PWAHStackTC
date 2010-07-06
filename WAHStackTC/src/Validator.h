/*
 * Validator.h
 *
 *  Created on: May 19, 2010
 *      Author: bas
 */

#ifndef VALIDATOR_H_
#define VALIDATOR_H_
#include "TransitiveClosureAlgorithm.h"
#include "Graph.h"

class Validator {
public:
	Validator();
	virtual ~Validator();

	static void validate();
};

#endif /* VALIDATOR_H_ */
