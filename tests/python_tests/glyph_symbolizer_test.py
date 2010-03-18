#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, save_data

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    

def test_arrows_symbolizer():
    srs = '+init=epsg:32630'
    lyr = mapnik2.Layer('arrows')
    lyr.datasource = mapnik2.Shapefile(
        file = '../data/shp/arrows.shp',
        )
    lyr.srs = srs
    _map = mapnik2.Map(256,256, srs)
    style = mapnik2.Style()
    rule = mapnik2.Rule()
    rule.filter = mapnik2.Expression("[value] > 0 and [azimuth]>-1") #XXX Need to mention an attribute in the expression
    sym = mapnik2.GlyphSymbolizer("DejaVu Sans Condensed", "A")
    sym.angle_offset = -90
    sym.min_value = 1
    sym.max_value = 100
    sym.min_size = 1
    sym.max_size = 100
    sym.allow_overlap = True
    # Assigning a colorizer to the RasterSymbolizer tells the later
    # that it should use it to colorize the raw data raster
    sym.colorizer = mapnik2.RasterColorizer()
    for value, color in [
        (  0, "#0044cc"),
        ( 10, "#00cc00"),
        ( 20, "#ffff00"),
        ( 30, "#ff7f00"),
        ( 40, "#ff0000"),
    ]:
        sym.colorizer.append_band(value, mapnik2.Color(color))
    rule.symbols.append(sym)
    style.rules.append(rule)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(mapnik2.Box2d(0,0,8,8))

    im = mapnik2.Image(_map.width,_map.height)
    mapnik2.render(_map, im)
    save_data('test_arrows_symbolizer.png', im.tostring('png'))
    imdata = im.tostring()
    assert len(imdata) > 0
    # we have features with 20 as a value so check that they're colored
    assert '\xff\xff\xff\x00' in imdata

