#encoding: utf8

from nose.tools import *
import os
from utilities import execution_path
import mapnik

wkts = [
    [1,"POINT(30 10)"],
    [1,"LINESTRING(30 10,10 30,40 40)"],
    [1,"POLYGON((30 10,10 20,20 40,40 40,30 10))"],
    [1,"POLYGON((35 10,10 20,15 40,45 45,35 10),(20 30,35 35,30 20,20 30))"],
    [4,"MULTIPOINT((10 40),(40 30),(20 20),(30 10))"],
    [2,"MULTILINESTRING((10 10,20 20,10 40),(40 40,30 30,40 20,30 10))"],
    [2,"MULTIPOLYGON(((30 20,10 40,45 40,30 20)),((15 5,40 10,10 20,5 10,15 5)))"],
    [2,"MULTIPOLYGON(((40 40,20 45,45 30,40 40)),((20 35,45 20,30 5,10 10,10 30,20 35),(30 20,20 25,20 15,30 20)))"],
    [3,"GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))"]
]


def compare_wkb_from_wkt(wkt,num):
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)
    eq_(len(f.geometries()),num)

    paths = mapnik.Path.from_wkt(wkt)
    eq_(len(paths),num)

    eq_(f.geometries()[0].to_wkb(),paths[0].to_wkb())

    paths2 = mapnik.Path()
    for path in paths:
        paths2.add_wkb(path.to_wkb())

    eq_(len(paths2),num)
    eq_(f.geometries()[0].to_wkb(),paths2[0].to_wkb())

def test_wkb():
    for wkt in wkts:
        try:
            compare_wkb_from_wkt(wkt[1],wkt[0])
        except RuntimeError, e:
            raise RuntimeError('%s %s' % (e, wkt))    

def compare_wkt_from_wkt(wkt,num):
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)
    eq_(len(f.geometries()),num)

    paths = mapnik.Path.from_wkt(wkt)
    eq_(len(paths),num)

    eq_(f.geometries()[0].to_wkt(),paths[0].to_wkt())

    paths2 = mapnik.Path()
    for path in paths:
        paths2.add_wkb(path.to_wkb())

    eq_(len(paths2),num)
    eq_(f.geometries()[0].to_wkt(),paths2[0].to_wkt())
    eq_(f.geometries().to_wkt().upper().replace('.0',''),wkt)

def test_wkt():
    for wkt in wkts:
        try:
            compare_wkt_from_wkt(wkt[1],wkt[0])
        except RuntimeError, e:
            raise RuntimeError('%s %s' % (e, wkt))

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
