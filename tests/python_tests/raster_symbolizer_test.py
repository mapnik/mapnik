#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, save_data, contains_word

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
    sym.colorizer = mapnik2.RasterColorizer(mapnik2.COLORIZER_DISCRETE, mapnik2.Color("transparent"))
    
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
        sym.colorizer.add_stop(value, mapnik2.Color(color))
    rule.symbols.append(sym)
    style.rules.append(rule)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(lyr.envelope())

    im = mapnik2.Image(_map.width,_map.height)
    mapnik2.render(_map, im)
    # save a png somewhere so we can see it
    save_data('test_dataraster_coloring.png', im.tostring('png'))
    imdata = im.tostring()
    # we have some values in the [20,30) interval so check that they're colored
    assert contains_word('\xff\xff\x00\xff', imdata)

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

def test_load_save_map():
    map = mapnik2.Map(256,256)
    in_map = "../data/good_maps/raster_symbolizer.xml"
    mapnik2.load_map(map, in_map)

    out_map = mapnik2.save_map_to_string(map)
    assert 'RasterSymbolizer' in out_map
    assert 'RasterColorizer' in out_map
    assert 'stop' in out_map

def test_raster_with_alpha_blends_correctly_with_background():
    WIDTH = 500
    HEIGHT = 500

    map = mapnik2.Map(WIDTH, HEIGHT)
    WHITE = mapnik2.Color(255, 255, 255)
    map.background = WHITE

    style = mapnik2.Style()
    rule = mapnik2.Rule()
    symbolizer = mapnik2.RasterSymbolizer()
    #XXX: This fixes it, see http://trac.mapnik.org/ticket/759#comment:3
    #     (and remove comment when this test passes)
    #symbolizer.scaling="bilinear_old"

    rule.symbols.append(symbolizer)
    style.rules.append(rule)

    map.append_style('raster_style', style)

    map_layer = mapnik2.Layer('test_layer')
    filepath = '../data/raster/white-alpha.png'
    map_layer.datasource = mapnik2.Gdal(file=filepath)
    map_layer.styles.append('raster_style')
    map.layers.append(map_layer)

    map.zoom_all()

    mim = mapnik2.Image(WIDTH, HEIGHT)

    mapnik2.render(map, mim)
    save_data('test_raster_with_alpha_blends_correctly_with_background.png',
              mim.tostring('png'))
    imdata = mim.tostring()
    # All white is expected
    assert contains_word('\xff\xff\xff\xff', imdata)

def test_raster_warping():
    lyrSrs = "+init=epsg:32630"
    mapSrs = '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'
    lyr = mapnik2.Layer('dataraster', lyrSrs)
    lyr.datasource = mapnik2.Gdal(
        file = '../data/raster/dataraster.tif',
        band = 1,
        )
    sym = mapnik2.RasterSymbolizer()
    sym.colorizer = mapnik2.RasterColorizer(mapnik2.COLORIZER_DISCRETE, mapnik2.Color(255,255,0))
    rule = mapnik2.Rule()
    rule.symbols.append(sym)
    style = mapnik2.Style()
    style.rules.append(rule)
    _map = mapnik2.Map(256,256, mapSrs)
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    prj_trans = mapnik2.ProjTransform(mapnik2.Projection(mapSrs),
                                      mapnik2.Projection(lyrSrs)) 
    _map.zoom_to_box(prj_trans.backward(lyr.envelope()))

    im = mapnik2.Image(_map.width,_map.height)
    mapnik2.render(_map, im)
    # save a png somewhere so we can see it
    save_data('test_raster_warping.png', im.tostring('png'))
    imdata = im.tostring()
    assert contains_word('\xff\xff\x00\xff', imdata)

def test_raster_warping_does_not_overclip_source():
    lyrSrs = "+init=epsg:32630"
    mapSrs = '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'
    lyr = mapnik2.Layer('dataraster', lyrSrs)
    lyr.datasource = mapnik2.Gdal(
        file = '../data/raster/dataraster.tif',
        band = 1,
        )
    sym = mapnik2.RasterSymbolizer()
    sym.colorizer = mapnik2.RasterColorizer(mapnik2.COLORIZER_DISCRETE, mapnik2.Color(255,255,0))
    rule = mapnik2.Rule()
    rule.symbols.append(sym)
    style = mapnik2.Style()
    style.rules.append(rule)
    _map = mapnik2.Map(256,256, mapSrs)
    _map.background=mapnik2.Color('white')
    _map.append_style('foo', style)
    lyr.styles.append('foo')
    _map.layers.append(lyr)
    _map.zoom_to_box(mapnik2.Box2d(3,42,4,43))

    im = mapnik2.Image(_map.width,_map.height)
    mapnik2.render(_map, im)
    # save a png somewhere so we can see it
    save_data('test_raster_warping_does_not_overclip_source.png',
              im.tostring('png'))
    assert im.view(0,200,1,1).tostring()=='\xff\xff\x00\xff'
    
if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
