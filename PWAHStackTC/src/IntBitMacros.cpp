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
 * Macro to check the bit at position pos of variable var
 */
#define I_GET_BIT(var, pos) ((var) & (1 << (pos)))

/**
 * Macro to set the bit at position pos of variable var
 */
#define I_SET_BIT(var, pos) ((var) |= 1 << (pos))

/**
 * Macro to clear the bit at position pos of variable var
 */
#define I_CLEAR_BIT(var, pos) ((var) &= ~(1 << (pos)))

/**
 * Macro to determine whether a var represents a FILL or LITERAL
 */
#define IS_FILL(var) (I_GET_BIT(var,31))

/**
 * Macro to determine the length of a FILL
 */
#define FILL_LENGTH(var) ((var) & 0b00111111111111111111111111111111)

/**
 * Macro to determine whether a var represents a 1-fill
 */
#define IS_ONEFILL(var) (IS_FILL(var) && I_GET_BIT(var,30))

/**
 * Macro to determine whether a var represents a 0-fill
 */
#define IS_ZEROFILL(var) (IS_FILL(var) && !I_GET_BIT(var,30))

/**
 * Define a 1-fill, disguised as a literal word
 */
#define LITERAL_ONEFILL 0b01111111111111111111111111111111

/**
 * Define a 0-fill, disguised as a literal word
 */
#define LITERAL_ZEROFILL 0b00000000000000000000000000000000


/**
 * Macro to determine whether a var is a literal concealing a 1-fill
 */
#define IS_LITERAL_ONEFILL(var) ((var) == LITERAL_ONEFILL)

/**
 * Macro to determine whether a var is a literal concealing a 0-fill
 */
#define IS_LITERAL_ZEROFILL(var) ((var) == LITERAL_ZEROFILL)
