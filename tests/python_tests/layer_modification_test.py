#!/usr/bin/env python

import os
from nose.tools import *
from utilities import execution_path, run_all
import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_adding_datasource_to_layer():
    map_string = '''<?xml version="1.0" encoding="utf-8"?>
<Map>

    <Layer name="world_borders">
        <StyleName>world_borders_style</StyleName>
        <StyleName>point_style</StyleName>
        <!-- leave datasource empty -->
        <!--
        <Datasource>
            <Parameter name="file">../data/shp/world_merc.shp</Parameter>
            <Parameter name="type">shape</Parameter>
        </Datasource>
        -->
    </Layer>

</Map>
'''
    m = mapnik.Map(256, 256)

    try:
        mapnik.load_map_from_string(m, map_string)

        # validate it loaded fine
        eq_(m.layers[0].styles[0],'world_borders_style')
        eq_(m.layers[0].styles[1],'point_style')
        eq_(len(m.layers),1)

        # also assign a variable reference to that layer
        # below we will test that this variable references
        # the same object that is attached to the map
        lyr = m.layers[0]

        # ensure that there was no datasource for the layer...
        eq_(m.layers[0].datasource,None)
        eq_(lyr.datasource,None)

        # also note that since the srs was black it defaulted to wgs84
        eq_(m.layers[0].srs,'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')
        eq_(lyr.srs,'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')

        # now add a datasource one...
        ds = mapnik.Shapefile(file='../data/shp/world_merc.shp')
        m.layers[0].datasource = ds

        # now ensure it is attached
        eq_(m.layers[0].datasource.describe()['name'],"shape")
        eq_(lyr.datasource.describe()['name'],"shape")

        # and since we have now added a shapefile in spherical mercator, adjust the projection
        lyr.srs = '+proj=merc +lon_0=0 +lat_ts=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs'

        # test that assignment
        eq_(m.layers[0].srs,'+proj=merc +lon_0=0 +lat_ts=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs')
        eq_(lyr.srs,'+proj=merc +lon_0=0 +lat_ts=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs')
    except RuntimeError, e:
        # only test datasources that we have installed
        if not 'Could not create datasource' in str(e):
            raise RuntimeError(e)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
