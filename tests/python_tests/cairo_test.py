#!/usr/bin/env python

import os
import mapnik
from nose.tools import *
from utilities import execution_path

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if mapnik.has_pycairo() and 'sqlite' in mapnik.DatasourceCache.plugin_names():

    def _pycairo_surface(type,sym):
            import cairo
            test_cairo_file = '/tmp/test.%s' % type
            m = mapnik.Map(256,256)
            mapnik.load_map(m,'../data/good_maps/%s_symbolizer.xml' % sym)
            if hasattr(cairo,'%sSurface' % type.upper()):
                surface = getattr(cairo,'%sSurface' % type.upper())(test_cairo_file, m.width,m.height)
                mapnik.render(m, surface)
                surface.finish()
                if os.path.exists(test_cairo_file):
                    os.remove(test_cairo_file)
                    return True
                else:
                    # Fail, the file wasn't written
                    return False
            else:
                print 'skipping cairo.%s test since surface is not available' % type.upper()
                return True

    def test_pycairo_svg_surface1():
        eq_(_pycairo_surface('svg','point'),True)

    def test_pycairo_svg_surface2():
        eq_(_pycairo_surface('svg','building'),True)

    def test_pycairo_svg_surface3():
        eq_(_pycairo_surface('svg','polygon'),True)

    def test_pycairo_pdf_surface1():
        eq_(_pycairo_surface('pdf','point'),True)

    def test_pycairo_pdf_surface2():
        eq_(_pycairo_surface('pdf','building'),True)

    def test_pycairo_pdf_surface3():
        eq_(_pycairo_surface('pdf','polygon'),True)

    def test_pycairo_ps_surface1():
        eq_(_pycairo_surface('ps','point'),True)

    def test_pycairo_ps_surface2():
        eq_(_pycairo_surface('ps','building'),True)

    def test_pycairo_ps_surface3():
        eq_(_pycairo_surface('ps','polygon'),True)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
