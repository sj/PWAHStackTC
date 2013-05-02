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

/**
 * Check whether we're compiling on a target where 'long int' and pointers both use 64 bits
 * (see "Common Predefined Macros" of the C Preprocessor of GNU GCC)
 */
#ifndef __LP64__
#error This implementation depends on 64-bit longs, your platform does not seem to offer those. Aborting.
#endif


/**
 * Macro to check the bit at position pos of variable var
 */
#define L_GET_BIT(var, pos) ((var) & (1L << (pos)))

/**
 * Macro to set the bit at position pos of variable var
 */
#define L_SET_BIT(var, pos) ((var) |= 1L << (pos))

/**
 * Macro to clear the bit at position pos of variable var
 */
#define L_CLEAR_BIT(var, pos) ((var) &= ~(1L << (pos)))
