/*
 * StaticStack.h
 *
 *  Created on: May 18, 2010
 *      Author: bas
 */

#ifndef STATICSTACK_H_
#define STATICSTACK_H_

class StaticStack {
private:
	int* _elements;
	int _capacity;
	int _lastIndexSet;

public:
	StaticStack(unsigned int capacity);
	virtual ~StaticStack();

	void push(int value);
	int peek();
	int pop();
	int* multiPop(unsigned int num);
	unsigned int size();
};

#endif /* STATICSTACK_H_ */
