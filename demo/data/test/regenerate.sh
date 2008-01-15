#!/bin/sh
rm -f road.shp road.shx road.dbf
ogr2ogr -f "ESRI Shapefile" road raw/road.gml
mv road/* ./
rmdir road
