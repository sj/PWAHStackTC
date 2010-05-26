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
