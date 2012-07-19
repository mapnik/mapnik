CXX = g++

CXXFLAGS = $(shell mapnik-config --cflags) -I/usr/include/python2.7 -fPIC

LIBS = $(shell mapnik-config --libs --ldflags --dep-libs) -lboost_python

SRC = $(wildcard *.cpp)

OBJ = $(SRC:.cpp=.o)

BIN = python.input

all : $(SRC) $(BIN)

$(BIN) : $(OBJ)
	$(CXX) -shared $(OBJ) $(LIBS) -o $@

.cpp.o :
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY : clean test

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

deploy : all
	cp python.input $(shell mapnik-config --input-plugins)

install: clean all deploy

test: deploy
	python test.py

