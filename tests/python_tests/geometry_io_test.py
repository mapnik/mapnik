#encoding: utf8

from nose.tools import *
import os,sys
from utilities import execution_path
from utilities import Todo
import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

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


def compare_wkb_from_wkt(wkt,num=None):

    # create a Path from geometry(s)
    paths = mapnik.Path.from_wkt(wkt)

    # add geometry(s) to feature from wkt
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)

    # ensure both have same result
    # compare number of geometry parts
    if num:
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
    if num:
        eq_(len(paths2),num)
    eq_(paths2.to_wkb(mapnik.wkbByteOrder.XDR),paths.to_wkb(mapnik.wkbByteOrder.XDR))
    for idx,path in enumerate(paths2):
        eq_(f.geometries()[idx].to_wkb(mapnik.wkbByteOrder.XDR),path.to_wkb(mapnik.wkbByteOrder.XDR))

def compare_wkt_from_wkt(wkt,num=None):
    # create a Path from geometry(s)
    paths = mapnik.Path.from_wkt(wkt)

    # add geometry(s) to feature from wkt
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)

    # compare to original, which may not have significant digits
    if '.' not in wkt:
        eq_(f.geometries().to_wkt().upper().replace('.0',''),wkt)
    else:
        eq_(f.geometries().to_wkt().upper(),wkt)

    # ensure both have same result
    if num:
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
    if num:
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

@raises(IndexError)
def test_geometry_index_error():
    wkt = 'Point (0 0)'
    paths = mapnik.Path.from_wkt(wkt)
    paths[3]
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)
    f.geometries()[3]

@raises(IndexError)
def test_geometry_index_error2():
    wkt = 'Point (0 0)'
    f = mapnik.Feature(1)
    f.add_geometries_from_wkt(wkt)
    f.geometries()[3]

def test_wkt_rounding():
    raise Todo("fixme or remove test")
    # currently fails because we use output precision of 6 - should we make configurable? https://github.com/mapnik/mapnik/issues/1009
    # if precision is set to 15 still fails due to very subtle rounding issues
    wkt = "POLYGON((7.904185417583761 54.180426336712856,7.89918053477129 54.178168035931542,7.897715691021261 54.182318426556606,7.893565300396205 54.183111883587891,7.89039147227129 54.187567449994106,7.885874870708761 54.190680242962827,7.879893425396261 54.193915106244049,7.894541862896233 54.194647528119134,7.900645378521233 54.190680242962827,7.904185417583761 54.180426336712856))"
    compare_wkt_from_wkt(wkt,1)

def test_wkt_collection_flattening():
    raise Todo("fixme or remove test")
    # currently fails as the MULTIPOLYGON inside will be returned as multiple polygons - not a huge deal - should we worry?
    wkt = "GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),MULTIPOLYGON(((40 40,20 45,45 30,40 40)),((20 35,45 20,30 5,10 10,10 30,20 35),(30 20,20 25,20 15,30 20))),LINESTRING(2 3,3 4))"
    compare_wkt_from_wkt(wkt,4)

# skip since this data is not checked into tests
#def test_wkt_natural_earth():
#    '''
#    wget http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/physical/10m-land.zip
#    unzip 10m-land.zip
#    ogr2ogr -F CSV -lco GEOMETRY=AS_WKT 10m-land.csv 10m_land.shp
#    mv 10m-land.csv tests/data/csv/
#    '''
#    lines = open('../data/csv/10m-land.csv').readlines()
#    for line in lines:
#        wkt = lines[1][lines[1].index('"',0)+1:lines[1].index('"',1)]
#        wkt = wkt.replace(' (','(',1)
#        try:
#            compare_wkb_from_wkt(wkt)
#        except RuntimeError, e:
#            raise RuntimeError('%s %s' % (e, wkt))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
