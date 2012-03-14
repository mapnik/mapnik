#encoding: utf8
#!/usr/bin/env python

from nose.tools import *

from utilities import execution_path, save_data, contains_word

import os, mapnik

def test_renders_with_agg():
    sym = mapnik.GlyphSymbolizer("DejaVu Sans Condensed",
                                  mapnik.Expression("'í'"))
    sym.allow_overlap = True
    sym.angle = mapnik.Expression("[azimuth]+90") #+90 so the top of the glyph points upwards
    sym.size = mapnik.Expression("[value]")
    sym.color = mapnik.Expression("'#ff0000'")

    _map = create_map_and_append_symbolyzer(sym)
    im = mapnik.Image(_map.width,_map.height)
    mapnik.render(_map, im)
    save_data('agg_glyph_symbolizer.png', im.tostring('png'))
    assert contains_word('\xff\x00\x00\xff', im.tostring())

def test_renders_with_cairo():
    if not mapnik.has_pycairo():
        return
    sym = mapnik.GlyphSymbolizer("DejaVu Sans Condensed",
                                  mapnik.Expression("'í'"))
    sym.allow_overlap = True
    sym.angle = mapnik.Expression("[azimuth]+90") #+90 so the top of the glyph points upwards
    sym.size = mapnik.Expression("[value]")
    sym.color = mapnik.Expression("'#ff0000'")
    _map = create_map_and_append_symbolyzer(sym)

    from cStringIO import StringIO
    import cairo
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 256, 256)
    mapnik.render(_map, surface)
    im = mapnik.Image.from_cairo(surface)
    save_data('cairo_glyph_symbolizer.png', im.tostring('png'))
    assert contains_word('\xff\x00\x00\xff', im.tostring())

def test_load_save_load_map():
    map = mapnik.Map(256,256)
    in_map = "../data/good_maps/glyph_symbolizer.xml"
    mapnik.load_map(map, in_map)
    style = map.find_style('arrows')
    sym = style.rules[0].symbols[0]
    assert isinstance(sym, mapnik.GlyphSymbolizer)
    assert sym.angle_mode == mapnik.angle_mode.AZIMUTH

    out_map = mapnik.save_map_to_string(map).decode('utf8')
    map = mapnik.Map(256,256)
    mapnik.load_map_from_string(map, out_map.encode('utf8'))
    assert 'GlyphSymbolizer' in out_map
    # make sure non-ascii characters are well supported since most interesting
    # glyphs for symbology are usually in that range
    assert u'í' in out_map, out_map

#
# Utilities and setup code
#

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    
def create_map_and_append_symbolyzer(sym):
    srs = '+init=epsg:32630'
    lyr = mapnik.Layer('arrows')
    lyr.datasource = mapnik.Shapefile(
        file = '../data/shp/arrows.shp',
        )
    lyr.srs = srs
    _map = mapnik.Map(256,256, srs)
    style = mapnik.Style()
    rule = mapnik.Rule()
    rule.symbols.append(sym)

    # put a test symbolizer to see what is the azimuth being read
    ts = mapnik.TextSymbolizer(mapnik.Expression('[azimuth]'),
                               "DejaVu Sans Book",
                               10,
                               mapnik.Color("black"))
    ts.allow_overlap = True
    rule.symbols.append(ts)

    style.rules.append(rule)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(mapnik.Box2d(0,0,8,8))
    return _map

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]

