/*
 * StaticStack.cpp
 *
 *  Created on: May 18, 2010
 *      Author: bas
 */

#include "StaticStack.h"
#include <string>
#include <string.h>
using namespace std;

StaticStack::StaticStack(unsigned int capacity) {
	_lastIndexSet = -1;
	_capacity = capacity;
	_elements = new int[capacity];
	memset(_elements, 0, _capacity * sizeof(int));
}

StaticStack::~StaticStack() {
	delete[] _elements;
}

void StaticStack::push(int value){
	if (_lastIndexSet == _capacity - 1) throw string ("Stack capacity reached");
	_elements[++_lastIndexSet] = value;
}

int StaticStack::pop(){
	if (_lastIndexSet < 0) throw string ("No more elements left on the stack");
	return _elements[_lastIndexSet--];
}

int StaticStack::peek(){
	if (_lastIndexSet < 0) throw string ("No more elements left on the stack");
	return _elements[_lastIndexSet];
}

/**
 * \brief Pops multiple elements of the stack and returns a pointer to the first element
 *
 * Warning! This is a dangerous method. It will return a pointer to the first element of an array
 * of elements. This pointer points to memory allocated by the stack. It is not safe to use the
 * elements in the returned array and add elements to the stack at the same time, since the newly
 * added elements will overwrite the old values the pointer is pointing to.
 *
 */
int* StaticStack::multiPop(unsigned int num){
	if (num > _lastIndexSet + 1) throw string("Not enough elements on the stack to execute this multiPop");
	int* res = _elements + _lastIndexSet - num + 1;
	_lastIndexSet -= num;

	return res;
}

unsigned int StaticStack::size(){
	return _lastIndexSet + 1;
}
