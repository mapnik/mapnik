#!/usr/bin/env python

from nose.tools import *

import mapnik
import random
import math
from utilities import run_all

# Tests that exercise map projections.

def test_normalizing_definition():
    p = mapnik.Projection('+init=epsg:4326')
    expanded = p.expanded()
    eq_('+proj=longlat' in expanded,True)


# Trac Ticket #128
def test_wgs84_inverse_forward():
    p = mapnik.Projection('+init=epsg:4326')

    c = mapnik.Coord(3.01331418311, 43.3333092669)
    e = mapnik.Box2d(-122.54345245, 45.12312553, 68.2335581353, 48.231231233)

    # It appears that the y component changes very slightly, is this OK?
    # so we test for 'almost equal float values'

    assert_almost_equal(p.inverse(c).y, c.y)
    assert_almost_equal(p.inverse(c).x, c.x)

    assert_almost_equal(p.forward(c).y, c.y)
    assert_almost_equal(p.forward(c).x, c.x)

    assert_almost_equal(p.inverse(e).center().y, e.center().y)
    assert_almost_equal(p.inverse(e).center().x, e.center().x)

    assert_almost_equal(p.forward(e).center().y, e.center().y)
    assert_almost_equal(p.forward(e).center().x, e.center().x)

    assert_almost_equal(c.inverse(p).y, c.y)
    assert_almost_equal(c.inverse(p).x, c.x)

    assert_almost_equal(c.forward(p).y, c.y)
    assert_almost_equal(c.forward(p).x, c.x)

    assert_almost_equal(e.inverse(p).center().y, e.center().y)
    assert_almost_equal(e.inverse(p).center().x, e.center().x)

    assert_almost_equal(e.forward(p).center().y, e.center().y)
    assert_almost_equal(e.forward(p).center().x, e.center().x)

def wgs2merc(lon,lat):
    x = lon * 20037508.34 / 180;
    y = math.log(math.tan((90 + lat) * math.pi / 360)) / (math.pi / 180);
    y = y * 20037508.34 / 180;
    return [x,y];

def merc2wgs(x,y):
    x = (x / 20037508.34) * 180;
    y = (y / 20037508.34) * 180;
    y = 180 / math.pi * (2 * math.atan(math.exp(y * math.pi/180)) - math.pi/2);
    if x > 180: x = 180;
    if x < -180: x = -180;
    if y > 85.0511: y = 85.0511;
    if y < -85.0511: y = -85.0511;
    return [x,y]

#echo -109 37 | cs2cs -f "%.10f" +init=epsg:4326 +to +init=epsg:3857
#-12133824.4964668211    4439106.7872505859 0.0000000000

## todo
# benchmarks
# better well known detection
# better srs matching with strip/trim
# python copy to avoid crash

def test_proj_transform_between_init_and_literal():
    one = mapnik.Projection('+init=epsg:4326')
    two = mapnik.Projection('+init=epsg:3857')
    tr1 = mapnik.ProjTransform(one,two)
    tr1b = mapnik.ProjTransform(two,one)
    wgs84 = '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'
    merc = '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over'
    src = mapnik.Projection(wgs84)
    dest = mapnik.Projection(merc)
    tr2 = mapnik.ProjTransform(src,dest)
    tr2b = mapnik.ProjTransform(dest,src)
    for x in xrange(-180,180,10):
        for y in xrange(-60,60,10):
            coord = mapnik.Coord(x,y)
            merc_coord1 = tr1.forward(coord)
            merc_coord2 = tr1b.backward(coord)
            merc_coord3 = tr2.forward(coord)
            merc_coord4 = tr2b.backward(coord)
            eq_(math.fabs(merc_coord1.x - merc_coord1.x) < 1,True)
            eq_(math.fabs(merc_coord1.x - merc_coord2.x) < 1,True)
            eq_(math.fabs(merc_coord1.x - merc_coord3.x) < 1,True)
            eq_(math.fabs(merc_coord1.x - merc_coord4.x) < 1,True)
            eq_(math.fabs(merc_coord1.y - merc_coord1.y) < 1,True)
            eq_(math.fabs(merc_coord1.y - merc_coord2.y) < 1,True)
            eq_(math.fabs(merc_coord1.y - merc_coord3.y) < 1,True)
            eq_(math.fabs(merc_coord1.y - merc_coord4.y) < 1,True)
            lon_lat_coord1 = tr1.backward(merc_coord1)
            lon_lat_coord2 = tr1b.forward(merc_coord2)
            lon_lat_coord3 = tr2.backward(merc_coord3)
            lon_lat_coord4 = tr2b.forward(merc_coord4)
            eq_(math.fabs(coord.x - lon_lat_coord1.x) < 1,True)
            eq_(math.fabs(coord.x - lon_lat_coord2.x) < 1,True)
            eq_(math.fabs(coord.x - lon_lat_coord3.x) < 1,True)
            eq_(math.fabs(coord.x - lon_lat_coord4.x) < 1,True)
            eq_(math.fabs(coord.y - lon_lat_coord1.y) < 1,True)
            eq_(math.fabs(coord.y - lon_lat_coord2.y) < 1,True)
            eq_(math.fabs(coord.y - lon_lat_coord3.y) < 1,True)
            eq_(math.fabs(coord.y - lon_lat_coord4.y) < 1,True)


if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
