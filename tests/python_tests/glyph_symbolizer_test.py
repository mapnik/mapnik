#encoding: utf8
#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, save_data, Todo, contains_word

import os, mapnik2

def test_glyph_symbolizer():
    sym = mapnik2.GlyphSymbolizer("DejaVu Sans Condensed",
                                  mapnik2.Expression("'í'"))
    sym.allow_overlap = True
    sym.angle = mapnik2.Expression("[azimuth]+90") #+90 so the top of the glyph points upwards
    sym.size = mapnik2.Expression("[value]")
    sym.color = mapnik2.Expression("'#ff0000'")

    _map = create_map_and_append_symbolyzer(sym)
    im = mapnik2.Image(_map.width,_map.height)
    mapnik2.render(_map, im)
    save_data('test_glyph_symbolizer.png', im.tostring('png'))
    assert contains_word('\xff\x00\x00\xff', im.tostring())

def test_load_save_map():
    raise Todo("Implement XML de/serialization for GlyphSymbolizer")

    map = mapnik2.Map(256,256)
    in_map = "../data/good_maps/glyph_symbolizer.xml"
    mapnik2.load_map(map, in_map)

    out_map = mapnik2.save_map_to_string(map)
    assert 'GlyphSymbolizer' in out_map
    assert 'RasterColorizer' in out_map

#
# Utilities and setup code
#

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    
def create_map_and_append_symbolyzer(sym):
    srs = '+init=epsg:32630'
    lyr = mapnik2.Layer('arrows')
    lyr.datasource = mapnik2.Shapefile(
        file = '../data/shp/arrows.shp',
        )
    lyr.srs = srs
    _map = mapnik2.Map(256,256, srs)
    style = mapnik2.Style()
    rule = mapnik2.Rule()
    #TODO Investigate why I need to add a filter which refers to the
    #     feature property that the symbolizer needs so they don't reach
    #     to it as nulls.
    rule.filter = mapnik2.Filter('[azimuth]>=0 and [value]>=0') #XXX
    rule.symbols.append(sym)

    ts = mapnik2.TextSymbolizer(mapnik2.Expression('[azimuth]'),
                               "DejaVu Sans Book",
                               10,
                               mapnik2.Color("black"))
    ts.allow_overlap = True
    rule.symbols.append(ts)

    style.rules.append(rule)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(mapnik2.Box2d(0,0,8,8))
    return _map

