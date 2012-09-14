CXXFLAGS = $(shell mapnik-config --cflags)
LDFLAGS = $(shell mapnik-config --libs --dep-libs --ldflags)

OBJ = rundemo.o

BIN = rundemo

all : $(BIN)

$(BIN) : $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

.c.o :
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean

clean: 
	rm -f $(OBJ)
	rm -f $(BIN)
