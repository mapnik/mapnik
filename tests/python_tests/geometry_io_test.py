#encoding: utf8

from nose.tools import *
import os
from utilities import execution_path
import mapnik

wkts = [
    [1,"POINT(30 10)"],
    [1,"POINT(30.0 10.0)"],
    [1,"POINT(30.1 10.1)"],
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

    # create a Path from geometry(s)
    paths = mapnik.Path.from_wkt(wkt)

    # add geometry(s) to feature from wkt
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)

    # ensure both have same result
    # compare number of geometry parts
    eq_(len(paths),num)
    eq_(len(f.geometries()),num)
    # compare collection off all geometries
    eq_(paths.to_wkb(mapnik.wkbByteOrder.XDR),f.geometries().to_wkb(mapnik.wkbByteOrder.XDR))
    # compare all parts
    for idx,path in enumerate(paths):
        eq_(f.geometries()[idx].to_wkb(mapnik.wkbByteOrder.XDR),path.to_wkb(mapnik.wkbByteOrder.XDR))

    # compare round trip
    paths2 = mapnik.Path()
    for path in paths:
        paths2.add_wkb(path.to_wkb(mapnik.wkbByteOrder.XDR))

    # ensure result
    eq_(len(paths2),num)
    eq_(paths2.to_wkb(mapnik.wkbByteOrder.XDR),paths.to_wkb(mapnik.wkbByteOrder.XDR))
    for idx,path in enumerate(paths2):
        eq_(f.geometries()[idx].to_wkb(mapnik.wkbByteOrder.XDR),path.to_wkb(mapnik.wkbByteOrder.XDR))

def compare_wkt_from_wkt(wkt,num):
    # create a Path from geometry(s)
    paths = mapnik.Path.from_wkt(wkt)

    # add geometry(s) to feature from wkt
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)

    # compare to original, which may not have significant digits
    if '.0' not in wkt:
        eq_(f.geometries().to_wkt().upper().replace('.0',''),wkt)
    else:
        eq_(f.geometries().to_wkt().upper(),wkt)

    # ensure both have same result
    eq_(len(paths),num)
    eq_(len(f.geometries()),num)
    eq_(paths.to_wkt(),f.geometries().to_wkt())
    for idx,path in enumerate(paths):
        eq_(f.geometries()[idx].to_wkt(),path.to_wkt())

    # compare round trip
    paths2 = mapnik.Path()
    for path in paths:
        paths2.add_wkt(path.to_wkt())

    # ensure result
    eq_(len(paths2),num)
    eq_(paths2.to_wkt(),paths.to_wkt())
    for idx,path in enumerate(paths2):
        eq_(f.geometries()[idx].to_wkb(mapnik.wkbByteOrder.XDR),path.to_wkb(mapnik.wkbByteOrder.XDR))

def test_wkt_simple():
    for wkt in wkts:
        try:
            compare_wkt_from_wkt(wkt[1],wkt[0])
        except RuntimeError, e:
            raise RuntimeError('%s %s' % (e, wkt))

def test_wkb_simple():
    for wkt in wkts:
        try:
            compare_wkb_from_wkt(wkt[1],wkt[0])
        except RuntimeError, e:
            raise RuntimeError('%s %s' % (e, wkt))

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
