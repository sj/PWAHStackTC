/*
 * BitSetIterator.h
 *
 *  Created on: Jun 4, 2010
 *      Author: bas
 */

#ifndef BITSETITERATOR_H_
#define BITSETITERATOR_H_

class BitSetIterator {
public:
	BitSetIterator();
	virtual ~BitSetIterator();

	virtual int next() = 0;
	virtual bool hasNext() = 0;
	virtual void reset() = 0;
};

#endif /* BITSETITERATOR_H_ */
