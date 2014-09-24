#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# map has no layers
@raises(IndexError)
def test_map_query_throw1():
    m = mapnik.Map(256,256)
    m.zoom_to_box(mapnik.Box2d(-1,-1,0,0))
    m.query_point(0,0,0)

# only positive indexes
@raises(IndexError)
def test_map_query_throw2():
    m = mapnik.Map(256,256)
    m.query_point(-1,0,0)

# map has never been zoomed (nodata)
@raises(RuntimeError)
def test_map_query_throw3():
    m = mapnik.Map(256,256)
    m.query_point(0,0,0)

if 'shape' in mapnik.DatasourceCache.plugin_names():
    # map has never been zoomed (even with data)
    @raises(RuntimeError)
    def test_map_query_throw4():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/agg_poly_gamma_map.xml')
        m.query_point(0,0,0)

    # invalid coords in general (do not intersect)
    @raises(RuntimeError)
    def test_map_query_throw5():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/agg_poly_gamma_map.xml')
        m.zoom_all()
        m.query_point(0,9999999999999999,9999999999999999)

    def test_map_query_works1():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/wgs842merc_reprojection.xml')
        merc_bounds = mapnik.Box2d(-20037508.34,-20037508.34,20037508.34,20037508.34)
        m.maximum_extent = merc_bounds
        m.zoom_all()
        fs = m.query_point(0,-11012435.5376, 4599674.6134) # somewhere in kansas
        feat = fs.next()
        eq_(feat.attributes['NAME_FORMA'],u'United States of America')

    def test_map_query_works2():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/merc2wgs84_reprojection.xml')
        wgs84_bounds = mapnik.Box2d(-179.999999975,-85.0511287776,179.999999975,85.0511287776)
        m.maximum_extent = wgs84_bounds
        # caution - will go square due to evil aspect_fix_mode backhandedness
        m.zoom_all()
        #mapnik.render_to_file(m,'works2.png')
        # validate that aspect_fix_mode modified the bbox reasonably
        e = m.envelope()
        assert_almost_equal(e.minx, -179.999999975, places=7)
        assert_almost_equal(e.miny, -167.951396161, places=7)
        assert_almost_equal(e.maxx, 179.999999975, places=7)
        assert_almost_equal(e.maxy, 192.048603789, places=7)
        fs = m.query_point(0,-98.9264, 38.1432) # somewhere in kansas
        feat = fs.next()
        eq_(feat.attributes['NAME'],u'United States')

    def test_map_query_in_pixels_works1():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/wgs842merc_reprojection.xml')
        merc_bounds = mapnik.Box2d(-20037508.34,-20037508.34,20037508.34,20037508.34)
        m.maximum_extent = merc_bounds
        m.zoom_all()
        fs = m.query_map_point(0,55,100) # somewhere in middle of us
        feat = fs.next()
        eq_(feat.attributes['NAME_FORMA'],u'United States of America')

    def test_map_query_in_pixels_works2():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/merc2wgs84_reprojection.xml')
        wgs84_bounds = mapnik.Box2d(-179.999999975,-85.0511287776,179.999999975,85.0511287776)
        m.maximum_extent = wgs84_bounds
        # caution - will go square due to evil aspect_fix_mode backhandedness
        m.zoom_all()
        # validate that aspect_fix_mode modified the bbox reasonably
        e = m.envelope()
        assert_almost_equal(e.minx, -179.999999975, places=7)
        assert_almost_equal(e.miny, -167.951396161, places=7)
        assert_almost_equal(e.maxx, 179.999999975, places=7)
        assert_almost_equal(e.maxy, 192.048603789, places=7)
        fs = m.query_map_point(0,55,100) # somewhere in Canada
        feat = fs.next()
        eq_(feat.attributes['NAME'],u'Canada')

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
