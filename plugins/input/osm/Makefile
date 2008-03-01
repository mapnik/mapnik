CXXFLAGS = `xml2-config --cflags` -I/usr/local/include/mapnik -I/usr/include/boost -I/usr/include/freetype2  -I/home/nick/mapnik-osm/agg/include  -g
LDFLAGS = `xml2-config --libs` -L/usr/local/lib  -lexpat
LDFLAGS2 = -L/usr/local/lib -lexpat `xml2-config --libs`
#LDFLAGS3 = -L/usr/local/lib -lmapnik -lexpat 
#LDFLAGS4 = `xml2-config --libs` -L/usr/local/lib  -lmapnik
LDFLAGS5 = `xml2-config --libs` -L/usr/local/lib -lmapnik -lexpat

OBJ = test.o LibxmlParser.o osm.o ExpatParser.o
OBJ2 = test2.o ExpatParser.o osm.o
test: $(OBJ) 
	g++ -o test $(OBJ) $(LDFLAGS) 
test2: $(OBJ2) 
	g++ -o test2 $(OBJ2) $(LDFLAGS2) 
render: render.o
	g++ -o render render.o $(LDFLAGS5) 

