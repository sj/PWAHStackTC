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

#include "DynamicStack.h"
#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <assert.h>
using namespace std;

DynamicStack::DynamicStack(int initialCapacity) {
	assert(initialCapacity > 0);
	_lastIndexSet = -1;
	_capacity = initialCapacity;
	_elements = new int[initialCapacity];
}

DynamicStack::~DynamicStack() {
	delete[] _elements;
}

void DynamicStack::reserve(int minCapacity){
	if (minCapacity < _capacity) throw string("Cannot reduce capacity!");

	int* newElements = new int[minCapacity];
	memcpy(newElements, _elements, _capacity * sizeof(int));
	delete[] _elements;
	_elements = newElements;
	_capacity = minCapacity;
}

void DynamicStack::push(int value){
	if (_lastIndexSet >= 0 && _lastIndexSet >= _capacity - 1){
		reserve(_capacity * 2);
	}
	_elements[++_lastIndexSet] = value;
}

int DynamicStack::pop(){
	if (_lastIndexSet < 0) throw string ("No more elements left on the stack");
	return _elements[_lastIndexSet--];
}

int DynamicStack::peek(){
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
int* DynamicStack::multiPop(int num){
	if (num > _lastIndexSet + 1) throw string("Not enough elements on the stack to execute this multiPop");
	int* firstElem = (&_elements[0]) + _lastIndexSet - num + 1;
	_lastIndexSet -= num;

	return firstElem;
}

unsigned int DynamicStack::size(){
	return _lastIndexSet + 1;
}
