#!/bin/sh
rm -f textspacing.shp textspacing.shx textspacing.dbf
ogr2ogr -f "ESRI Shapefile" textspacing raw/textspacing.gml
mv textspacing/* ./
rmdir textspacing

rm -f overlap.shp overlap.shx overlap.dbf
ogr2ogr -f "ESRI Shapefile" overlap raw/overlap.gml
mv overlap/* ./
rmdir overlap

rm -f displacement.shp displacement.shx displacement.dbf
ogr2ogr -f "ESRI Shapefile" displacement raw/displacement.gml
mv displacement/* ./
rmdir displacement

rm -f charplacement.shp charplacement.shx charplacement.dbf
ogr2ogr -f "ESRI Shapefile" charplacement raw/charplacement.gml
mv charplacement/* ./
rmdir charplacement
