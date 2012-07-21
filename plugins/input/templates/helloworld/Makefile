CXX = clang++

CXXFLAGS = $(shell mapnik-config --cflags)

LIBS = $(shell mapnik-config --libs --ldflags --dep-libs)

SRC = $(wildcard *.cpp)

OBJ = $(SRC:.cpp=.o)

BIN = hello.input

all : $(SRC) $(BIN)

$(BIN) : $(OBJ)
	$(CXX) -shared $(OBJ) $(LIBS) -o $@

.cpp.o :
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY : clean

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

deploy : all
	cp hello.input $(shell mapnik-config --input-plugins)

install: clean all deploy