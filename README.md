# LZ


So this [gzip thing](https://aclanthology.org/2023.findings-acl.426/) got in the news lately and I thought it would be interesting to write an LZ compressor and decompressor.


## C++ Requirements 
- C++17
- GNU Make
- Catch2 for tests

Build everything by invoking `make all`. Test binaries end up in `bin/`. There is a 
script `test/run_tests.sh` that globs every file in `bin/` that starts with `test.*` 
and executes this. Run this and make sure none of the output is red. Also actually read
the tests to see what is isn't being tested as there are no guarantees that the tests
are in any way comprehensive.



## Python Requirements
To make it easier to see how this works I decided I would try to write a `python` interface
so that I can make notebooks. This is why there are `poetry` files in this project. 

As of this writing, to get the python packages built do 

`python setup.py build_ext -i`

This creates the shared object file in the root folder. NOTE: this is a shit place to 
put this, and anyway it fails in the test because obviously we can't find it.


TODO: There is loads of real packaging to do for this, and I probably have to move away
from my usual handy-dandy Makefile and use `cmake` or something.
