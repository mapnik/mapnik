#encoding: utf8

from nose.tools import *
import os,sys
from utilities import execution_path, run_all
import mapnik
from binascii import unhexlify

try:
    import json
except ImportError:
    import simplejson as json

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

wkts = [
    [1,"POINT(30 10)"],
    [1,"POINT(30 10)"],
    [1,"POINT(30.1 10.1)"],
    [1,"LINESTRING(30 10,10 30,40 40)"],
    [1,"POLYGON((30 10,10 20,20 40,40 40,30 10))"],
    [1,"POLYGON((35 10,10 20,15 40,45 45,35 10),(20 30,35 35,30 20,20 30))"],
    [4,"MULTIPOINT((10 40),(40 30),(20 20),(30 10))"],
    [2,"MULTILINESTRING((10 10,20 20,10 40),(40 40,30 30,40 20,30 10))"],
    [2,"MULTIPOLYGON(((30 20,10 40,45 40,30 20)),((15 5,40 10,10 20,5 10,15 5)))"],
    [2,"MULTIPOLYGON(((40 40,20 45,45 30,40 40)),((20 35,45 20,30 5,10 10,10 30,20 35),(30 20,20 25,20 15,30 20)))"],
    [3,"GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))"],
    [1,"POLYGON((-178.32319 71.518365,-178.321586 71.518439,-178.259635 71.510688,-178.304862 71.513129,-178.32319 71.518365),(-178.32319 71.518365,-178.341544 71.517524,-178.32244 71.505439,-178.215323 71.478034,-178.193473 71.47663,-178.147757 71.485175,-178.124442 71.481879,-178.005729 71.448615,-178.017203 71.441413,-178.054191 71.428778,-178.047049 71.425727,-178.033439 71.417792,-178.026236 71.415107,-178.030082 71.413459,-178.039908 71.40766,-177.970878 71.39643,-177.779837 71.333197,-177.718375 71.305243,-177.706412 71.3039,-177.68212 71.304877,-177.670279 71.301825,-177.655387 71.293158,-177.587577 71.285956,-177.548575 71.294867,-177.531119 71.296332,-177.51409 71.293402,-177.498649 71.284735,-177.506217 71.268622,-177.486991 71.258734,-177.459708 71.249884,-177.443412 71.237006,-177.445914 71.222663,-177.457755 71.209357,-177.507804 71.173774,-177.581168 71.147589,-177.637626 71.117011,-177.684134 71.110968,-177.751883 71.092963,-177.819266 71.084662,-177.877677 71.052558,-177.930472 71.041449,-178.206595 71.038398,-178.310111 71.013617,-178.875907 70.981024,-178.980277 70.95069,-179.342093 70.908026,-179.336234 70.911078,-179.322257 70.921698,-179.364493 70.930243,-179.457511 70.915534,-179.501212 70.919684,-179.666007 70.965461,-179.853385 70.979438,-179.888785 70.993598,-179.907523 70.996772,-179.999989 70.992011,-179.999989 71.024848,-179.999989 71.058661,-179.999989 71.126166,-179.999989 71.187018,-179.999989 71.224189,-179.999989 71.27497,-179.999989 71.312079,-179.999989 71.356024,-179.999989 71.410041,-179.999989 71.487799,-179.999989 71.536689,-179.862845 71.538642,-179.912223 71.555854,-179.900748 71.558478,-179.798819 71.569098,-179.757438 71.583197,-179.735953 71.586432,-179.715445 71.583258,-179.697501 71.577338,-179.678702 71.573676,-179.610831 71.585211,-179.372062 71.569098,-179.326774 71.555487,-179.306815 71.557563,-179.287162 71.562934,-179.24285 71.569098,-179.204642 71.583197,-179.074576 71.600043,-178.395438 71.539008,-178.32319 71.518365))"],
    [2,"MULTIPOLYGON(((-178.32319 71.518365,-178.321586 71.518439,-178.259635 71.510688,-178.304862 71.513129,-178.32319 71.518365)),((-178.32319 71.518365,-178.341544 71.517524,-178.32244 71.505439,-178.215323 71.478034,-178.193473 71.47663,-178.147757 71.485175,-178.124442 71.481879,-178.005729 71.448615,-178.017203 71.441413,-178.054191 71.428778,-178.047049 71.425727,-178.033439 71.417792,-178.026236 71.415107,-178.030082 71.413459,-178.039908 71.40766,-177.970878 71.39643,-177.779837 71.333197,-177.718375 71.305243,-177.706412 71.3039,-177.68212 71.304877,-177.670279 71.301825,-177.655387 71.293158,-177.587577 71.285956,-177.548575 71.294867,-177.531119 71.296332,-177.51409 71.293402,-177.498649 71.284735,-177.506217 71.268622,-177.486991 71.258734,-177.459708 71.249884,-177.443412 71.237006,-177.445914 71.222663,-177.457755 71.209357,-177.507804 71.173774,-177.581168 71.147589,-177.637626 71.117011,-177.684134 71.110968,-177.751883 71.092963,-177.819266 71.084662,-177.877677 71.052558,-177.930472 71.041449,-178.206595 71.038398,-178.310111 71.013617,-178.875907 70.981024,-178.980277 70.95069,-179.342093 70.908026,-179.336234 70.911078,-179.322257 70.921698,-179.364493 70.930243,-179.457511 70.915534,-179.501212 70.919684,-179.666007 70.965461,-179.853385 70.979438,-179.888785 70.993598,-179.907523 70.996772,-179.999989 70.992011,-179.999989 71.024848,-179.999989 71.058661,-179.999989 71.126166,-179.999989 71.187018,-179.999989 71.224189,-179.999989 71.27497,-179.999989 71.312079,-179.999989 71.356024,-179.999989 71.410041,-179.999989 71.487799,-179.999989 71.536689,-179.862845 71.538642,-179.912223 71.555854,-179.900748 71.558478,-179.798819 71.569098,-179.757438 71.583197,-179.735953 71.586432,-179.715445 71.583258,-179.697501 71.577338,-179.678702 71.573676,-179.610831 71.585211,-179.372062 71.569098,-179.326774 71.555487,-179.306815 71.557563,-179.287162 71.562934,-179.24285 71.569098,-179.204642 71.583197,-179.074576 71.600043,-178.395438 71.539008,-178.32319 71.518365)))"]
]

geojson = [
    [1,'{"type":"Point","coordinates":[30,10]}'],
    [1,'{"type":"Point","coordinates":[30.0,10.0]}'],
    [1,'{"type":"Point","coordinates":[30.1,10.1]}'],
    [1,'{"type":"LineString","coordinates":[[30.0,10.0],[10.0,30.0],[40.0,40.0]]}'],
    [1,'{"type":"Polygon","coordinates":[[[30.0,10.0],[10.0,20.0],[20.0,40.0],[40.0,40.0],[30.0,10.0]]]}'],
    [1,'{"type":"Polygon","coordinates":[[[35.0,10.0],[10.0,20.0],[15.0,40.0],[45.0,45.0],[35.0,10.0]],[[20.0,30.0],[35.0,35.0],[30.0,20.0],[20.0,30.0]]]}'],
    [4,'{"type":"MultiPoint","coordinates":[[10.0,40.0],[40.0,30.0],[20.0,20.0],[30.0,10.0]]}'],
    [2,'{"type":"MultiLineString","coordinates":[[[10.0,10.0],[20.0,20.0],[10.0,40.0]],[[40.0,40.0],[30.0,30.0],[40.0,20.0],[30.0,10.0]]]}'],
    [2,'{"type":"MultiPolygon","coordinates":[[[[30.0,20.0],[10.0,40.0],[45.0,40.0],[30.0,20.0]]],[[[15.0,5.0],[40.0,10.0],[10.0,20.0],[5.0,10.0],[15.0,5.0]]]]}'],
    [2,'{"type":"MultiPolygon","coordinates":[[[[40.0,40.0],[20.0,45.0],[45.0,30.0],[40.0,40.0]]],[[[20.0,35.0],[45.0,20.0],[30.0,5.0],[10.0,10.0],[10.0,30.0],[20.0,35.0]],[[30.0,20.0],[20.0,25.0],[20.0,15.0],[30.0,20.0]]]]}'],
    [3,'{"type":"GeometryCollection","geometries":[{"type":"Polygon","coordinates":[[[1.0,1.0],[2.0,1.0],[2.0,2.0],[1.0,2.0],[1.0,1.0]]]},{"type":"Point","coordinates":[2.0,3.0]},{"type":"LineString","coordinates":[[2.0,3.0],[3.0,4.0]]}]}'],
    [1,'{"type":"Polygon","coordinates":[[[-178.32319,71.518365],[-178.321586,71.518439],[-178.259635,71.510688],[-178.304862,71.513129],[-178.32319,71.518365]],[[-178.32319,71.518365],[-178.341544,71.517524],[-178.32244,71.505439],[-178.215323,71.478034],[-178.193473,71.47663],[-178.147757,71.485175],[-178.124442,71.481879],[-178.005729,71.448615],[-178.017203,71.441413],[-178.054191,71.428778],[-178.047049,71.425727],[-178.033439,71.417792],[-178.026236,71.415107],[-178.030082,71.413459],[-178.039908,71.40766],[-177.970878,71.39643],[-177.779837,71.333197],[-177.718375,71.305243],[-177.706412,71.3039],[-177.68212,71.304877],[-177.670279,71.301825],[-177.655387,71.293158],[-177.587577,71.285956],[-177.548575,71.294867],[-177.531119,71.296332],[-177.51409,71.293402],[-177.498649,71.284735],[-177.506217,71.268622],[-177.486991,71.258734],[-177.459708,71.249884],[-177.443412,71.237006],[-177.445914,71.222663],[-177.457755,71.209357],[-177.507804,71.173774],[-177.581168,71.147589],[-177.637626,71.117011],[-177.684134,71.110968],[-177.751883,71.092963],[-177.819266,71.084662],[-177.877677,71.052558],[-177.930472,71.041449],[-178.206595,71.038398],[-178.310111,71.013617],[-178.875907,70.981024],[-178.980277,70.95069],[-179.342093,70.908026],[-179.336234,70.911078],[-179.322257,70.921698],[-179.364493,70.930243],[-179.457511,70.915534],[-179.501212,70.919684],[-179.666007,70.965461],[-179.853385,70.979438],[-179.888785,70.993598],[-179.907523,70.996772],[-179.999989,70.992011],[-179.999989,71.024848],[-179.999989,71.058661],[-179.999989,71.126166],[-179.999989,71.187018],[-179.999989,71.224189],[-179.999989,71.27497],[-179.999989,71.312079],[-179.999989,71.356024],[-179.999989,71.410041],[-179.999989,71.487799],[-179.999989,71.536689],[-179.862845,71.538642],[-179.912223,71.555854],[-179.900748,71.558478],[-179.798819,71.569098],[-179.757438,71.583197],[-179.735953,71.586432],[-179.715445,71.583258],[-179.697501,71.577338],[-179.678702,71.573676],[-179.610831,71.585211],[-179.372062,71.569098],[-179.326774,71.555487],[-179.306815,71.557563],[-179.287162,71.562934],[-179.24285,71.569098],[-179.204642,71.583197],[-179.074576,71.600043],[-178.395438,71.539008],[-178.32319,71.518365]]]}'],
    [2,'{"type":"MultiPolygon","coordinates":[[[[-178.32319,71.518365],[-178.321586,71.518439],[-178.259635,71.510688],[-178.304862,71.513129],[-178.32319,71.518365]]],[[[-178.32319,71.518365],[-178.341544,71.517524],[-178.32244,71.505439],[-178.215323,71.478034],[-178.193473,71.47663],[-178.147757,71.485175],[-178.124442,71.481879],[-178.005729,71.448615],[-178.017203,71.441413],[-178.054191,71.428778],[-178.047049,71.425727],[-178.033439,71.417792],[-178.026236,71.415107],[-178.030082,71.413459],[-178.039908,71.40766],[-177.970878,71.39643],[-177.779837,71.333197],[-177.718375,71.305243],[-177.706412,71.3039],[-177.68212,71.304877],[-177.670279,71.301825],[-177.655387,71.293158],[-177.587577,71.285956],[-177.548575,71.294867],[-177.531119,71.296332],[-177.51409,71.293402],[-177.498649,71.284735],[-177.506217,71.268622],[-177.486991,71.258734],[-177.459708,71.249884],[-177.443412,71.237006],[-177.445914,71.222663],[-177.457755,71.209357],[-177.507804,71.173774],[-177.581168,71.147589],[-177.637626,71.117011],[-177.684134,71.110968],[-177.751883,71.092963],[-177.819266,71.084662],[-177.877677,71.052558],[-177.930472,71.041449],[-178.206595,71.038398],[-178.310111,71.013617],[-178.875907,70.981024],[-178.980277,70.95069],[-179.342093,70.908026],[-179.336234,70.911078],[-179.322257,70.921698],[-179.364493,70.930243],[-179.457511,70.915534],[-179.501212,70.919684],[-179.666007,70.965461],[-179.853385,70.979438],[-179.888785,70.993598],[-179.907523,70.996772],[-179.999989,70.992011],[-179.999989,71.024848],[-179.999989,71.058661],[-179.999989,71.126166],[-179.999989,71.187018],[-179.999989,71.224189],[-179.999989,71.27497],[-179.999989,71.312079],[-179.999989,71.356024],[-179.999989,71.410041],[-179.999989,71.487799],[-179.999989,71.536689],[-179.862845,71.538642],[-179.912223,71.555854],[-179.900748,71.558478],[-179.798819,71.569098],[-179.757438,71.583197],[-179.735953,71.586432],[-179.715445,71.583258],[-179.697501,71.577338],[-179.678702,71.573676],[-179.610831,71.585211],[-179.372062,71.569098],[-179.326774,71.555487],[-179.306815,71.557563],[-179.287162,71.562934],[-179.24285,71.569098],[-179.204642,71.583197],[-179.074576,71.600043],[-178.395438,71.539008],[-178.32319,71.518365]]]]}']
]

geojson_nulls = [
  '{ "type": "Feature", "properties": { }, "geometry": null }',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "Point", "coordinates": [] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "LineString", "coordinates": [ [] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "Polygon", "coordinates": [ [ [] ] ] } }',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiPoint", "coordinates": [ [] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiPoint", "coordinates": [ [],[] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiLineString", "coordinates": [ [] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiLineString", "coordinates": [ [ [] ] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiPolygon", "coordinates": [ [] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiPolygon", "coordinates": [ [ [] ] ] }}',
  '{ "type": "Feature", "properties": { }, "geometry": { "type": "MultiPolygon", "coordinates": [ [ [ [] ] ] ] }}',
]

wkbs = [
    [ 0, "Point EMPTY", '010400000000000000'],
    [ 0, "MULTIPOINT EMPTY", '010400000000000000'],
    [ 0, "LINESTRING EMPTY", '010200000000000000'],
    [ 0, "MULTILINESTRING EMPTY", '010500000000000000'],
    [ 0, "Polygon EMPTY", '010300000000000000'],
    [ 0, "MULTIPOLYGON EMPTY", '010600000000000000'],
    [ 0, "TRIANGLE EMPTY", '011100000000000000'],

    [ 0, "CircularString EMPTY", '010800000000000000'],
    [ 0, "CurvePolygon EMPTY", '010A00000000000000'],
    [ 0, "CompoundCurve EMPTY", '010900000000000000'],
    [ 0, "MultiCurve EMPTY", '010B00000000000000'],

    [ 0, "MultiSurface EMPTY", '010C00000000000000'],
    [ 0, "PolyhedralSurface EMPTY", '010F00000000000000'],
    [ 0, "TIN EMPTY", '011000000000000000'],
    [ 0, "GEOMETRYCOLLECTION EMPTY", '010700000000000000'],
    [ 2, "GEOMETRYCOLLECTION(MULTILINESTRING((10 10,20 20,10 40),(40 40,30 30,40 20,30 10)),LINESTRING EMPTY)", '010700000002000000010500000002000000010200000003000000000000000000244000000000000024400000000000003440000000000000344000000000000024400000000000004440010200000004000000000000000000444000000000000044400000000000003e400000000000003e40000000000000444000000000000034400000000000003e400000000000002440010200000000000000'
    ],
    [ 0, "GEOMETRYCOLLECTION(LINESTRING EMPTY,LINESTRING EMPTY)", '010700000000000000'],
    [ 0, "GEOMETRYCOLLECTION(POINT EMPTY,POINT EMPTY)", '010700000000000000'],
    [ 1, "GEOMETRYCOLLECTION(POINT EMPTY,POINT(0 0))", '010700000002000000010400000000000000010100000000000000000000000000000000000000'],
    [ 1, "GEOMETRYCOLLECTION(POINT EMPTY,MULTIPOINT(0 0))", '010700000002000000010400000000000000010400000001000000010100000000000000000000000000000000000000'],
    [ 0, "LINESTRING EMPTY", '010200000000000000' ],
    [ 1, "Point(0 0)", '010100000000000000000000000000000000000000' ]
]

def test_path_geo_interface():
    path = mapnik.Path()
    path.add_wkt('POINT(0 0)')
    eq_(path.__geo_interface__,{u'type': u'Point', u'coordinates': [0, 0]})

def test_wkb_parsing():
    path = mapnik.Path()
    count = 0
    for wkb in wkbs:
        count += wkb[0]
        try :
            path.add_wkb(unhexlify(wkb[2]))
        except RuntimeError:
            pass
    eq_(count,len(path))

def test_geojson_parsing():
    path = mapnik.Path()
    count = 0
    for json in geojson:
        count += json[0]
        try :
            path.add_geojson(json[1])
        except RuntimeError:
            pass
    eq_(count,len(path))

reader = mapnik.WKTReader()

def compare_wkb_from_wkt(wkt,num=None):

    # create a Path from geometry(s)
    # easy api, but slower
    #paths = mapnik.Path.from_wkt(wkt)
    # fast api
    paths = reader.read(wkt);

    # add geometry(s) to feature from wkt
    f = mapnik.Feature(mapnik.Context(),1)
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
    # easy api, but slower
    #paths = mapnik.Path.from_wkt(wkt)
    # fast api
    paths = reader.read(wkt);

    # add geometry(s) to feature from wkt
    f = mapnik.Feature(mapnik.Context(),1)
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

def compare_wkt_to_geojson(idx,wkt,num=None):
    paths = reader.read(wkt);
    # ensure both have same result
    if num:
        eq_(len(paths),num)
    gj = paths.to_geojson()
    eq_(len(gj) > 1,True)
    a = json.loads(gj)
    e = json.loads(geojson[idx][1])
    eq_(a,e)

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

def test_wkt_to_geojson():
    idx = -1
    for wkt in wkts:
        try:
            idx += 1
            compare_wkt_to_geojson(idx,wkt[1],wkt[0])
        except RuntimeError, e:
            raise RuntimeError('%s %s' % (e, wkt))

@raises(IndexError)
def test_geometry_index_error():
    wkt = 'Point (0 0)'
    # easy api, but slower
    #paths = mapnik.Path.from_wkt(wkt)
    # fast api
    paths = reader.read(wkt);
    paths[3]
    f = mapnik.Feature(mapnik.Context(),1)
    f.add_geometries_from_wkt(wkt)
    f.geometries()[3]

@raises(IndexError)
def test_geometry_index_error2():
    wkt = 'Point (0 0)'
    f = mapnik.Feature(mapnik.Context(),1)
    f.add_geometries_from_wkt(wkt)
    f.geometries()[3]

def test_wkt_rounding():
    # currently fails because we use output precision of 6 - should we make configurable? https://github.com/mapnik/mapnik/issues/1009
    # if precision is set to 15 still fails due to very subtle rounding issues
    wkt = "POLYGON((7.904185 54.180426,7.89918 54.178168,7.897715 54.182318,7.893565 54.183111,7.890391 54.187567,7.885874 54.19068,7.879893 54.193915,7.894541 54.194647,7.900645 54.19068,7.904185 54.180426))"
    compare_wkt_from_wkt(wkt,1)

def test_wkt_collection_flattening():
    wkt = 'GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),POLYGON((40 40,20 45,45 30,40 40)),POLYGON((20 35,45 20,30 5,10 10,10 30,20 35),(30 20,20 25,20 15,30 20)),LINESTRING(2 3,3 4))'
    # currently fails as the MULTIPOLYGON inside will be returned as multiple polygons - not a huge deal - should we worry?
    #wkt = "GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),MULTIPOLYGON(((40 40,20 45,45 30,40 40)),((20 35,45 20,30 5,10 10,10 30,20 35),(30 20,20 25,20 15,30 20))),LINESTRING(2 3,3 4))"
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

def test_creating_feature_from_geojson():
    json_feat = {
      "type": "Feature",
      "geometry": {"type": "Point", "coordinates": [-122,48]},
      "properties": {"name": "value"}
    }
    ctx = mapnik.Context()
    feat = mapnik.Feature.from_geojson(json.dumps(json_feat),ctx)
    eq_(feat.id(),1)
    eq_(feat['name'],u'value')

def test_handling_geojson_null_geoms():
    for json in geojson_nulls:
        ctx = mapnik.Context()
        out_json = mapnik.Feature.from_geojson(json,ctx).to_geojson()
        expected = '{"type":"Feature","id":1,"geometry":null,"properties":{}}'
        eq_(out_json,expected)
        # ensure it round trips
        eq_(mapnik.Feature.from_geojson(out_json,ctx).to_geojson(),expected)


if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
