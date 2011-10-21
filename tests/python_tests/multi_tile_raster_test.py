#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, save_data, contains_word

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_multi_tile_policy():
    srs = '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'
    lyr = mapnik2.Layer('raster')
    lyr.datasource = mapnik2.Raster(
        file = '../data/raster_tiles/${x}/${y}.tif',
        lox = -180,
        loy = -90,
        hix = 180,
        hiy = 90,
        multi = 1,
        tile_size = 256,
        x_width = 2,
        y_width = 2
        )
    lyr.srs = srs
    _map = mapnik2.Map(256, 256, srs)
    style = mapnik2.Style()
    rule = mapnik2.Rule()
    sym = mapnik2.RasterSymbolizer()
    rule.symbols.append(sym)
    style.rules.append(rule)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(lyr.envelope())
    
    im = mapnik2.Image(_map.width, _map.height)
    mapnik2.render(_map, im)
    
    save_data('test_multi_tile_policy.png', im.tostring('png'))

    # test green chunk
    assert im.view(0,64,1,1).tostring() == '\x00\xff\x00\xff'
    assert im.view(127,64,1,1).tostring() == '\x00\xff\x00\xff'
    assert im.view(0,127,1,1).tostring() == '\x00\xff\x00\xff'
    assert im.view(127,127,1,1).tostring() == '\x00\xff\x00\xff'

    # test blue chunk
    assert im.view(128,64,1,1).tostring() == '\x00\x00\xff\xff'
    assert im.view(255,64,1,1).tostring() == '\x00\x00\xff\xff'
    assert im.view(128,127,1,1).tostring() == '\x00\x00\xff\xff'
    assert im.view(255,127,1,1).tostring() == '\x00\x00\xff\xff'

    # test red chunk
    assert im.view(0,128,1,1).tostring() == '\xff\x00\x00\xff'
    assert im.view(127,128,1,1).tostring() == '\xff\x00\x00\xff'
    assert im.view(0,191,1,1).tostring() == '\xff\x00\x00\xff'
    assert im.view(127,191,1,1).tostring() == '\xff\x00\x00\xff'

    # test magenta chunk
    assert im.view(128,128,1,1).tostring() == '\xff\x00\xff\xff'
    assert im.view(255,128,1,1).tostring() == '\xff\x00\xff\xff'
    assert im.view(128,191,1,1).tostring() == '\xff\x00\xff\xff'
    assert im.view(255,191,1,1).tostring() == '\xff\x00\xff\xff'

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
