/*
 * DynamicStack.h
 *
 *  Created on: May 18, 2010
 *      Author: bas
 */

#ifndef DynamicStack_H_
#define DynamicStack_H_
#include <vector>
using namespace std;

class DynamicStack {
private:
	vector<int> _elements;
	int _lastIndexSet;

public:
	DynamicStack(unsigned int capacity);
	virtual ~DynamicStack();

	void push(int value);
	int peek();
	int pop();
	int* multiPop(unsigned int num);
	unsigned int size();
};

#endif /* DynamicStack_H_ */
