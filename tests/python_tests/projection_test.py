#!/usr/bin/env python

from nose.tools import *

import mapnik, pickle

# Tests that exercise map projections.

# Trac Ticket #128
def test_wgs84_inverse_forward():
    p = mapnik.Projection('+init=epsg:4326')

    c = mapnik.Coord(3.01331418311, 43.3333092669)
    e = mapnik.Envelope(-122.54345245, 45.12312553, 68.2335581353, 48.231231233)

    # It appears that the y component changes very slightly, is this OK?
    eq_(p.inverse(c), c)
    eq_(p.forward(c), c)

    eq_(p.inverse(e), e)
    eg_(p.forward(e), e)

    eq_(c.inverse(p), c)
    eq_(c.forward(p), c)

    eq_(e.inverse(p), e)
    eq_(e.forward(p), e)
