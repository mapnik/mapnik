#!/usr/bin/env python

from nose.tools import *

import mapnik

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

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
