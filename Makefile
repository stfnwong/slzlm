# Prefix Trie and Lempel Ziv
#
# OUTPUT DIRS
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
TEST_DIR=test
ASM_DIR=asm
TEST_BIN_DIR=$(BIN_DIR)
PROGRAM_DIR=programs

# Pybind stuff
PYTHON_VERSION=3.11
PYTHON_DIR=/usr/include/python$(PYTHON_VERSION)/
PYTHON_CFLAGS=$(shell python$$PYTHON_VERSION-config --cflags)
PYTHON_LDFLAGS=$(shell python$$PYTHON_VERSION-config --ldflags)

# TODO: need a way to be able to compile the C++ stuff standalone to get compilation 
# time down for testing...

# Tool options
CXX=g++
OPT=-O0
#CXXFLAGS=-Wall -g2 -std=c++17 -D_REENTRANT $(OPT) -fPIC -shared $(PYTHON_CFLAGS)
CXXFLAGS=$(PYTHON_CFLAGS) -Wall -g2 -std=c++17 -D_REENTRANT  -fPIC -shared $(OPT)
TESTFLAGS=
LDFLAGS=$(PYTHON_LDFLAGS) -lpython3.11
LIBS= 
TEST_LIBS=

# style for assembly output
ASM_STYLE=intel

# Object targets
INCS=-I$(SRC_DIR) $(shell python$$PYTHON_VERSION-config --includes)
#INCS=-I$(SRC_DIR) -I$(PYTHON_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.hpp)
# Unit tests 
TEST_SOURCES  = $(wildcard $(TEST_DIR)/*.cpp)
# Tools (program entry points)
PROGRAM_SOURCES = $(wildcard $(PROGRAM_DIR)/*.cpp)


# Objects
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp  $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCS)

# Objects, but output as assembly
$(ASSEM_OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -S $< -o $(ASM_DIR)/$@.S -masm=$(ASM_STYLE)


# Unit tests 
TEST_OBJECTS  := $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)
$(TEST_OBJECTS): $(OBJ_DIR)/%.o : $(TEST_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@ 

# ==== TEST TARGETS ==== #
TESTS=test_map_trie test_array_trie test_lzw

$(TESTS): $(TEST_OBJECTS) $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o $(INCS) \
		-o $(TEST_BIN_DIR)/$@ $(LIBS) $(TEST_LIBS)


# ==== PROGRAM TARGETS ==== #
PROGRAMS = trie encode
PROGRAM_OBJECTS := $(PROGRAM_SOURCES:$(PROGRAM_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(PROGRAM_OBJECTS): $(OBJ_DIR)/%.o : $(PROGRAM_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(PROGRAMS): $(OBJECTS) $(PROGRAM_OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o \
		$(INCS) -o $(BIN_DIR)/$@ $(LIBS)




# ==== PYTHON TARGETS ==== #
python:
	python setup.py build_ext 


# Main targets 
#
.PHONY: all test programs clean python


all : test programs python

test : $(OBJECTS) $(TESTS)

programs : $(PROGRAMS)

assem : $(ASSEM_OBJECTS)

clean:
	rm -fv *.o $(OBJ_DIR)/*.o 
	# Clean test programs
	rm -fv $(TEST_BIN_DIR)/test_*
	@find . -name '*.so' -exec rm -rf {} \;
	@find . -name '__pycache__' -exec rm -rf {} \;


print-%:
	@echo $* = $($*)
