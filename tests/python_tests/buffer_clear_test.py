import sys
import os, mapnik
from timeit import Timer, time
from nose.tools import *
from utilities import execution_path, run_all

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_clearing_image_data():
    im = mapnik.Image(256,256)
    # make sure it equals itself
    bytes = im.tostring()
    eq_(im.tostring(),bytes)
    # set background, then clear
    im.background = mapnik.Color('green')
    eq_(im.tostring()!=bytes,True)
    # clear image, should now equal original
    im.clear()
    eq_(im.tostring(),bytes)

def make_map():
    ds = mapnik.MemoryDatasource()
    context = mapnik.Context()
    context.push('Name')
    pixel_key = 1
    f = mapnik.Feature(context,pixel_key)
    f['Name'] = str(pixel_key)
    f.add_geometries_from_wkt('POLYGON ((0 0, 0 256, 256 256, 256 0, 0 0))')
    ds.add_feature(f)
    s = mapnik.Style()
    r = mapnik.Rule()
    symb = mapnik.PolygonSymbolizer()
    r.symbols.append(symb)
    s.rules.append(r)
    lyr = mapnik.Layer('Places')
    lyr.datasource = ds
    lyr.styles.append('places_labels')
    width,height = 256,256
    m = mapnik.Map(width,height)
    m.append_style('places_labels',s)
    m.layers.append(lyr)
    m.zoom_all()
    return m

if mapnik.has_grid_renderer():
    def test_clearing_grid_data():
        g = mapnik.Grid(256,256)
        utf = g.encode()
        # make sure it equals itself
        eq_(g.encode(),utf)
        m = make_map()
        mapnik.render_layer(m,g,layer=0,fields=['__id__','Name'])
        eq_(g.encode()!=utf,True)
        # clear grid, should now match original
        g.clear()
        eq_(g.encode(),utf)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
