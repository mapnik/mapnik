CXX = g++

INCLUDE = `pkg-config --cflags mapnik freetype2 cairomm-1.0`
LIBDIR  = `pkg-config --libs mapnik freetype2 cairomm-1.0`

DEFINES = 

CXXFLAGS = $(DEFINES) $(INCLUDE) -Wall

LIBS =  $(LIBDIR)

OBJ = rundemo.o

BIN = rundemo

all : $(BIN)

$(BIN) : $(OBJ)
	libtool $(CXX) $(OBJ) $(LIBS) -o $@

.c.o :
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean

clean: 
	rm -f $(OBJ)
	rm -f $(BIN)
