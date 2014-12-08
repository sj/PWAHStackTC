Welcome to PWAHStackTC!

The source code is in an Eclipse CDT (C++) project in the "Eclipse-C++"
directory. To build the executable for an experimental evaluation of
PWAHStackTC, run "make" in the "Release" directory:
> cd Eclipse-C++/Release 
> make 
> ./PWAHStackTC --help 
> ./PWAHStackTC --filename=mygraph.graph

PWAHStackTC expects its input graphs in the following format:
 - line 1: number of vertices n and the number of edges m, separated by a space
 - following n lines: adjacency lists for vertex n, listing the indices of
   adjacent vertices. Note that indices start at 1. A vertex without any
   adjacent vertices is encoded as an empty line.

Do not use the build setup in the "Debug" directory, unless you know what
you're doing. Under no circumstances should a debug build be used for an
experimental evaluation.

A more extensive  guide can be found on our website: http://www.sj-vs.net/pwah

Source code documentation can be found in the "doc" subdirectory. A good place
to start looking is the index page of the PWAHStackTC source code documentation
(generated using Doxygen): doc/html/index.html

Enjoy!

Bas van Schaik
