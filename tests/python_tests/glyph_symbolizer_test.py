#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, save_data, Todo

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    

def test_glyph_symbolizer():
    srs = '+init=epsg:32630'
    lyr = mapnik2.Layer('arrows')
    lyr.datasource = mapnik2.Shapefile(
        file = '../data/shp/arrows.shp',
        )
    lyr.srs = srs
    _map = mapnik2.Map(256,256, srs)
    style = mapnik2.Style()
    rule = mapnik2.Rule()
    sym = mapnik2.GlyphSymbolizer("DejaVu Sans Condensed", mapnik2.Expression("'A'"))
    sym.allow_overlap = True
    sym.angle = mapnik2.Expression("[azimuth]-90")
    sym.value = mapnik2.Expression("[value]")
    sym.size = mapnik2.Expression("[value]")
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
    save_data('test_glyph_symbolizer.png', im.tostring('png'))
    imdata = im.tostring()
    assert len(imdata) > 0

    raise Todo("Implement the process methods of the agg/cairo renderers for GlyphSymbolizer")

    # we have features with 20 as a value so check that they're colored
    assert '\xff\xff\xff\x00' in imdata

def test_load_save_map():
    raise Todo("Implement XML de/serialization for GlyphSymbolizer")

    map = mapnik2.Map(256,256)
    in_map = "../data/good_maps/glyph_symbolizer.xml"
    mapnik2.load_map(map, in_map)

    out_map = mapnik2.save_map_to_string(map)
    assert 'GlyphSymbolizer' in out_map
    assert 'RasterSymbolizer' in out_map
    assert 'RasterColorizer' in out_map
    assert 'ColorBand' in out_map
