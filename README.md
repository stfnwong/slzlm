# Shit LZ-based Langauge Models


So this [gzip thing](https://aclanthology.org/2023.findings-acl.426/) got in the news lately and I thought it would be interesting to write an LZ compressor and decompressor.

I gave this a title with the word _Language Model_ but its not really a language model.
Some people don't even think its that, see [here](https://kenschutte.com/gzip-knn-paper/).

## Quickstart 
The minimum required to start is 

- Clone this repo and move into the directory
- Type `make all`
- To check that it worked do `./tests/run_tests.sh` to make sure the `C++` stuff works, and `pytest` to make sure the `python` stuff works.


# Header formats
This is a description of the header formats used by the various compressors. 

### LZW

| Offset | Length | Meaning |
| ------ | ------ | ------- |
| 0      | 4      | Offset to 24-bit symbol table |
| 4      | 4      | Offset to 32-bit symbol table | 
| 8      | 4      | Total number of codes | 


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

Poetry may be the least worst package manager out there but I stil hate it. To create the
venv [install poetry](https://python-poetry.org/docs/) using that link or your package 
manager and then do 

`poetry install`

Add `-vvv` to the end of the above command to also see a lot of text. The build doesn't
work with poetry yet and perhaps never will, so don't do anything else.


As of this writing, to get the python packages built do 

`python setup.py build_ext -i`

This creates the shared object file in the root folder. NOTE: this is a shit place to 
put this, and anyway it fails in the test because obviously we can't find it.


TODO: There is loads of real packaging to do for this, and I probably have to move away
from my usual handy-dandy Makefile and use `cmake` or something.
