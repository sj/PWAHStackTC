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

#ifndef DynamicStack_H_
#define DynamicStack_H_
#include <vector>
using namespace std;

class DynamicStack {
private:
	int* _elements;
	int _capacity;
	int _lastIndexSet;
	void reserve(int minCapacity);

public:
	DynamicStack(int capacity);
	virtual ~DynamicStack();

	void push(int value);
	int peek();
	int pop();
	int* multiPop(int num);
	unsigned int size();
};

#endif /* DynamicStack_H_ */
