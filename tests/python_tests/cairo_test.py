#!/usr/bin/env python

import os
import mapnik2
from nose.tools import *
from utilities import execution_path,Todo

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def _pycairo_surface(type,sym):
    if mapnik2.has_pycairo():
        import cairo
        test_cairo_file = 'test.%s' % type
        m = mapnik2.Map(256,256)
        mapnik2.load_map(m,'../data/good_maps/%s_symbolizer.xml' % sym)
        surface = getattr(cairo,'%sSurface' % type.upper())(test_cairo_file, m.width,m.height)
        mapnik2.render(m, surface)
        surface.finish()        
        
        if os.path.exists(test_cairo_file):
            os.remove(test_cairo_file)
            return True
        else:
            # Fail, the file wasn't written
            return False

def test_pycairo_svg_surface():
    return _pycairo_surface('svg','point')

def test_pycairo_svg_surface():
    return _pycairo_surface('svg','building')
    
def test_pycairo_svg_surface():
    return _pycairo_surface('svg','polygon')

def test_pycairo_svg_surface():
    return _pycairo_surface('pdf','point')

def test_pycairo_svg_surface():
    return _pycairo_surface('pdf','building')
    
def test_pycairo_svg_surface():
    return _pycairo_surface('pdf','polygon')

def test_pycairo_svg_surface():
    return _pycairo_surface('ps','point')

def test_pycairo_svg_surface():
    return _pycairo_surface('ps','building')
    
def test_pycairo_svg_surface():
    return _pycairo_surface('ps','polygon')

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
