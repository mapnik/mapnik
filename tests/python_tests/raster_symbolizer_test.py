#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    

def test_dataraster_coloring():
    srs = '+init=epsg:32630'
    lyr = mapnik2.Layer('dataraster')
    lyr.datasource = mapnik2.Gdal(
        file = '../data/raster/dataraster.tif',
        band = 1,
        )
    lyr.srs = srs
    _map = mapnik2.Map(256,256, srs)
    style = mapnik2.Style()
    rule = mapnik2.Rule()
    sym = mapnik2.RasterSymbolizer()
    # Assigning a colorizer to the RasterSymbolizer tells the later
    # that it should use it to colorize the raw data raster
    sym.colorizer = mapnik2.RasterColorizer()
    for value, color in [
        (  0, "#0044cc"),
        ( 10, "#00cc00"),
        ( 20, "#ffff00"),
        ( 30, "#ff7f00"),
        ( 40, "#ff0000"),
        ( 50, "#ff007f"),
        ( 60, "#ff00ff"),
        ( 70, "#cc00cc"),
        ( 80, "#990099"),
        ( 90, "#660066"),
        ( 200, "transparent"),
    ]:
        sym.colorizer.append_band(value, mapnik2.Color(color))
    rule.symbols.append(sym)
    style.rules.append(rule)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(lyr.envelope())

    im = mapnik2.Image(_map.width,_map.height)
    mapnik2.render(_map, im)
    imdata = im.tostring()
    assert len(imdata) > 0
    # we have some values in the [20,30) interval so check that they're colored
    assert '\xff\xff\xff\x00' in imdata

    # save a png somewhere so we can see it
    if 'MAPNIK_TEST_IMAGE_PATH' in os.environ:
        f = open(os.environ['MAPNIK_TEST_IMAGE_PATH'],'wb')
        f.write(im.tostring('png'))
        f.close()

def test_dataraster_query_point():
    srs = '+init=epsg:32630'
    lyr = mapnik2.Layer('dataraster')
    lyr.datasource = mapnik2.Gdal(
        file = '../data/raster/dataraster.tif',
        band = 1,
        )
    lyr.srs = srs
    _map = mapnik2.Map(256,256, srs)
    _map.layers.append(lyr)

    # point inside raster extent with valid data
    x, y = 427417, 4477517
    features = _map.query_point(0,x,y).features
    assert len(features) == 1
    feat = features[0]
    center = feat.envelope().center()
    assert center.x==x and center.y==y, center
    value = feat['value']
    assert value == 21.0, value

    # point outside raster extent
    features = _map.query_point(0,-427417,4477517).features
    assert len(features) == 0

    # point inside raster extent with nodata
    features = _map.query_point(0,126850,4596050).features
    assert len(features) == 0
