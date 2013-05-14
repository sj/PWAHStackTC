PWAHStackTC's Debug build tree depends on Google Test (gtest.h) for unit testing. See:
  https://code.google.com/p/googletest/

It was tested with Google Test 1.6.0.


For Debian and Ubuntu, installing Google Test is simple (but not at simple as you'd expect). As root (or prepend sudo), run:
> apt-get install libgtest-dev cmake build-essential
> cd /usr/src/gtest
> cmake .
> make
> cd /usr/lib
> ln -s /usr/src/gtest/libgtest.a
> ln -s /usr/src/gtest/libgtest_main.a
