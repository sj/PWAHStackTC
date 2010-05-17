/*
 * IntMacros.cpp
 *
 *  Created on: Apr 20, 2010
 *      Author: bas
 */


/**
 * Macro to check the bit at position pos of variable var
 */
#define GET_BIT(var, pos) ((var) & (1 << (pos)))

/**
 * Macro to set the bit at position pos of variable var
 */
#define SET_BIT(var, pos) ((var) |= 1 << (pos))

/**
 * Macro to clear the bit at position pos of variable vat
 */
#define CLEAR_BIT(var, pos) ((var) &= ~(1 << (pos)))

/**
 * Macro to determine whether a var represents a FILL or LITERAL
 */
#define IS_FILL(var) (GET_BIT(var,31))

/**
 * Macro to determine the length of a FILL
 */
#define FILL_LENGTH(var) ((var) & 0b00111111111111111111111111111111)

/**
 * Macro to determine whether a var represents a 1-fill
 */
#define IS_ONEFILL(var) (IS_FILL(var) && GET_BIT(var,30))

/**
 * Macro to determine whether a var represents a 0-fill
 */
#define IS_ZEROFILL(var) (IS_FILL(var) && !GET_BIT(var,30))

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
