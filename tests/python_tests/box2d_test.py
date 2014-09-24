#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from nose.tools import *
from utilities import execution_path, run_all
import mapnik

def test_coord_init():
    c = mapnik.Coord(100, 100)

    eq_(c.x, 100)
    eq_(c.y, 100)

def test_coord_multiplication():
    c = mapnik.Coord(100, 100)
    c *= 2

    eq_(c.x, 200)
    eq_(c.y, 200)

def test_envelope_init():
    e = mapnik.Box2d(100, 100, 200, 200)

    assert_true(e.contains(100, 100))
    assert_true(e.contains(100, 200))
    assert_true(e.contains(200, 200))
    assert_true(e.contains(200, 100))

    assert_true(e.contains(e.center()))

    assert_false(e.contains(99.9, 99.9))
    assert_false(e.contains(99.9, 200.1))
    assert_false(e.contains(200.1, 200.1))
    assert_false(e.contains(200.1, 99.9))

    eq_(e.width(), 100)
    eq_(e.height(), 100)

    eq_(e.minx, 100)
    eq_(e.miny, 100)

    eq_(e.maxx, 200)
    eq_(e.maxy, 200)

    eq_(e[0],100)
    eq_(e[1],100)
    eq_(e[2],200)
    eq_(e[3],200)
    eq_(e[0],e[-4])
    eq_(e[1],e[-3])
    eq_(e[2],e[-2])
    eq_(e[3],e[-1])

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

def test_envelope_static_init():
    e = mapnik.Box2d.from_string('100 100 200 200')
    e2 = mapnik.Box2d.from_string('100,100,200,200')
    e3 = mapnik.Box2d.from_string('100 , 100 , 200 , 200')
    eq_(e,e2)
    eq_(e,e3)

    assert_true(e.contains(100, 100))
    assert_true(e.contains(100, 200))
    assert_true(e.contains(200, 200))
    assert_true(e.contains(200, 100))

    assert_true(e.contains(e.center()))

    assert_false(e.contains(99.9, 99.9))
    assert_false(e.contains(99.9, 200.1))
    assert_false(e.contains(200.1, 200.1))
    assert_false(e.contains(200.1, 99.9))

    eq_(e.width(), 100)
    eq_(e.height(), 100)

    eq_(e.minx, 100)
    eq_(e.miny, 100)

    eq_(e.maxx, 200)
    eq_(e.maxy, 200)

    eq_(e[0],100)
    eq_(e[1],100)
    eq_(e[2],200)
    eq_(e[3],200)
    eq_(e[0],e[-4])
    eq_(e[1],e[-3])
    eq_(e[2],e[-2])
    eq_(e[3],e[-1])

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

def test_envelope_multiplication():
    # no width then no impact of multiplication
    a = mapnik.Box2d(100, 100, 100, 100)
    a *= 5
    eq_(a.minx,100)
    eq_(a.miny,100)
    eq_(a.maxx,100)
    eq_(a.maxy,100)

    a = mapnik.Box2d(100.0, 100.0, 100.0, 100.0)
    a *= 5
    eq_(a.minx,100)
    eq_(a.miny,100)
    eq_(a.maxx,100)
    eq_(a.maxy,100)

    a = mapnik.Box2d(100.0, 100.0, 100.001, 100.001)
    a *= 5
    assert_almost_equal(a.minx, 99.9979, places=3)
    assert_almost_equal(a.miny, 99.9979, places=3)
    assert_almost_equal(a.maxx, 100.0030, places=3)
    assert_almost_equal(a.maxy, 100.0030, places=3)

    e = mapnik.Box2d(100, 100, 200, 200)
    e *= 2
    eq_(e.minx,50)
    eq_(e.miny,50)
    eq_(e.maxx,250)
    eq_(e.maxy,250)

    assert_true(e.contains(50, 50))
    assert_true(e.contains(50, 250))
    assert_true(e.contains(250, 250))
    assert_true(e.contains(250, 50))

    assert_false(e.contains(49.9, 49.9))
    assert_false(e.contains(49.9, 250.1))
    assert_false(e.contains(250.1, 250.1))
    assert_false(e.contains(250.1, 49.9))

    assert_true(e.contains(e.center()))

    eq_(e.width(), 200)
    eq_(e.height(), 200)

    eq_(e.minx, 50)
    eq_(e.miny, 50)

    eq_(e.maxx, 250)
    eq_(e.maxy, 250)

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

def test_envelope_clipping():
    e1 = mapnik.Box2d(-180,-90,180,90)
    e2 = mapnik.Box2d(-120,40,-110,48)
    e1.clip(e2)
    eq_(e1,e2)

    # madagascar in merc
    e1 = mapnik.Box2d(4772116.5490, -2744395.0631, 5765186.4203, -1609458.0673)
    e2 = mapnik.Box2d(5124338.3753, -2240522.1727, 5207501.8621, -2130452.8520)
    e1.clip(e2)
    eq_(e1,e2)

    # nz in lon/lat
    e1 = mapnik.Box2d(163.8062, -47.1897, 179.3628, -33.9069)
    e2 = mapnik.Box2d(173.7378, -39.6395, 174.4849, -38.9252)
    e1.clip(e2)
    eq_(e1,e2)

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
