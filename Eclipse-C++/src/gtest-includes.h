/*
 * gtest.h
 *
 *  Created on: 11 May 2013
 *      Author: bas
 */

/**
 * This file selectively includes the Google Test (GTest) header files. These are not included for Release builds
 * (when the compiler is optimising the build), but are included for Debug builds.
 */
#ifndef GTEST_INCLUDES_H_
#define GTEST_INCLUDES_H_

#ifdef __OPTIMIZE__
// Code compiled without debugging flags
#define FRIEND_TEST(foo,bar)
#define RUN_ALL_TESTS()
#else /* __OPTIMIZE__ */
// Code compiled with debugging flags - include Google Test code
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#endif /* __OPTIMIZE__ */


#endif /* GTEST_INCLUDES_H_ */
