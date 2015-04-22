#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import eq_,assert_almost_equal,raises
from utilities import execution_path, run_all
import os, mapnik

try:
    import json
except ImportError:
    import simplejson as json

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'ogr' in mapnik.DatasourceCache.plugin_names():

    # Shapefile initialization
    def test_shapefile_init():
        ds = mapnik.Ogr(file='../../demo/data/boundaries.shp',layer_by_index=0)
        e = ds.envelope()
        assert_almost_equal(e.minx, -11121.6896651, places=7)
        assert_almost_equal(e.miny, -724724.216526, places=6)
        assert_almost_equal(e.maxx, 2463000.67866, places=5)
        assert_almost_equal(e.maxy, 1649661.267, places=3)
        meta = ds.describe()
        eq_(meta['geometry_type'],mapnik.DataGeometryType.Polygon)
        eq_('+proj=lcc' in meta['proj4'],True)

    # Shapefile properties
    def test_shapefile_properties():
        ds = mapnik.Ogr(file='../../demo/data/boundaries.shp',layer_by_index=0)
        f = ds.features_at_point(ds.envelope().center(), 0.001).features[0]
        eq_(ds.geometry_type(),mapnik.DataGeometryType.Polygon)

        eq_(f['CGNS_FID'], u'6f733341ba2011d892e2080020a0f4c9')
        eq_(f['COUNTRY'], u'CAN')
        eq_(f['F_CODE'], u'FA001')
        eq_(f['NAME_EN'], u'Quebec')
        eq_(f['Shape_Area'], 1512185733150.0)
        eq_(f['Shape_Leng'], 19218883.724300001)
        meta = ds.describe()
        eq_(meta['geometry_type'],mapnik.DataGeometryType.Polygon)
        # NOTE: encoding is latin1 but gdal >= 1.9 should now expose utf8 encoded features
        # See SHAPE_ENCODING for overriding: http://gdal.org/ogr/drv_shapefile.html
        # Failure for the NOM_FR field is expected for older gdal
        #eq_(f['NOM_FR'], u'Qu\xe9bec')
        #eq_(f['NOM_FR'], u'Québec')

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Ogr(file='../data/shp/world_merc.shp',layer_by_index=0)
        eq_(len(ds.fields()),11)
        eq_(ds.fields(),['FIPS', 'ISO2', 'ISO3', 'UN', 'NAME', 'AREA', 'POP2005', 'REGION', 'SUBREGION', 'LON', 'LAT'])
        eq_(ds.field_types(),['str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        ds.features(query)

    # disabled because OGR prints an annoying error: ERROR 1: Invalid Point object. Missing 'coordinates' member.
    #def test_handling_of_null_features():
    #    ds = mapnik.Ogr(file='../data/json/null_feature.geojson',layer_by_index=0)
    #    fs = ds.all_features()
    #    eq_(len(fs),1)

    # OGR plugin extent parameter
    def test_ogr_extent_parameter():
        ds = mapnik.Ogr(file='../data/shp/world_merc.shp',layer_by_index=0,extent='-1,-1,1,1')
        e = ds.envelope()
        eq_(e.minx,-1)
        eq_(e.miny,-1)
        eq_(e.maxx,1)
        eq_(e.maxy,1)
        meta = ds.describe()
        eq_(meta['geometry_type'],mapnik.DataGeometryType.Polygon)
        eq_('+proj=merc' in meta['proj4'],True)

    def test_ogr_reading_gpx_waypoint():
        ds = mapnik.Ogr(file='../data/gpx/empty.gpx',layer='waypoints')
        e = ds.envelope()
        eq_(e.minx,-122)
        eq_(e.miny,48)
        eq_(e.maxx,-122)
        eq_(e.maxy,48)
        meta = ds.describe()
        eq_(meta['geometry_type'],mapnik.DataGeometryType.Point)
        eq_('+proj=longlat' in meta['proj4'],True)

    def test_ogr_empty_data_should_not_throw():
        default_logging_severity = mapnik.logger.get_severity()
        mapnik.logger.set_severity(mapnik.severity_type.None)
        # use logger to silence expected warnings
        for layer in ['routes', 'tracks', 'route_points', 'track_points']:
            ds = mapnik.Ogr(file='../data/gpx/empty.gpx',layer=layer)
            e = ds.envelope()
            eq_(e.minx,0)
            eq_(e.miny,0)
            eq_(e.maxx,0)
            eq_(e.maxy,0)
        mapnik.logger.set_severity(default_logging_severity)
        meta = ds.describe()
        eq_(meta['geometry_type'],mapnik.DataGeometryType.Point)
        eq_('+proj=longlat' in meta['proj4'],True)

    # disabled because OGR prints an annoying error: ERROR 1: Invalid Point object. Missing 'coordinates' member.
    #def test_handling_of_null_features():
    #    ds = mapnik.Ogr(file='../data/json/null_feature.geojson',layer_by_index=0)
    #    fs = ds.all_features()
    #    eq_(len(fs),1)

    def test_geometry_type():
        ds = mapnik.Ogr(file='../data/csv/wkt.vrt',layer_by_index=0)
        e = ds.envelope()
        assert_almost_equal(e.minx, 1.0, places=1)
        assert_almost_equal(e.miny, 1.0, places=1)
        assert_almost_equal(e.maxx, 45.0, places=1)
        assert_almost_equal(e.maxy, 45.0, places=1)
        meta = ds.describe()
        eq_(meta['geometry_type'],mapnik.DataGeometryType.Point)
        eq_('+proj=longlat' in meta['proj4'],True)
        fs = ds.featureset()
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'Point', u'coordinates': [30, 10]}, u'type': u'Feature', u'id': 2, u'properties': {u'type': u'point', u'WKT': u'           POINT (30 10)'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'LineString', u'coordinates': [[30, 10], [10, 30], [40, 40]]}, u'type': u'Feature', u'id': 3, u'properties': {u'type': u'linestring', u'WKT': u'      LINESTRING (30 10, 10 30, 40 40)'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'Polygon', u'coordinates': [[[30, 10], [10, 20], [20, 40], [40, 40], [30, 10]]]}, u'type': u'Feature', u'id': 4, u'properties': {u'type': u'polygon', u'WKT': u'         POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'Polygon', u'coordinates': [[[35, 10], [10, 20], [15, 40], [45, 45], [35, 10]], [[20, 30], [35, 35], [30, 20], [20, 30]]]}, u'type': u'Feature', u'id': 5, u'properties': {u'type': u'polygon', u'WKT': u'         POLYGON ((35 10, 10 20, 15 40, 45 45, 35 10),(20 30, 35 35, 30 20, 20 30))'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'MultiPoint', u'coordinates': [[10, 40], [40, 30], [20, 20], [30, 10]]}, u'type': u'Feature', u'id': 6, u'properties': {u'type': u'multipoint', u'WKT': u'      MULTIPOINT ((10 40), (40 30), (20 20), (30 10))'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'MultiLineString', u'coordinates': [[[10, 10], [20, 20], [10, 40]], [[40, 40], [30, 30], [40, 20], [30, 10]]]}, u'type': u'Feature', u'id': 7, u'properties': {u'type': u'multilinestring', u'WKT': u' MULTILINESTRING ((10 10, 20 20, 10 40),(40 40, 30 30, 40 20, 30 10))'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'MultiPolygon', u'coordinates': [[[[30, 20], [10, 40], [45, 40], [30, 20]]], [[[15, 5], [40, 10], [10, 20], [5, 10], [15, 5]]]]}, u'type': u'Feature', u'id': 8, u'properties': {u'type': u'multipolygon', u'WKT': u'    MULTIPOLYGON (((30 20, 10 40, 45 40, 30 20)),((15 5, 40 10, 10 20, 5 10, 15 5)))'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'MultiPolygon', u'coordinates': [[[[40, 40], [20, 45], [45, 30], [40, 40]]], [[[20, 35], [45, 20], [30, 5], [10, 10], [10, 30], [20, 35]], [[30, 20], [20, 25], [20, 15], [30, 20]]]]}, u'type': u'Feature', u'id': 9, u'properties': {u'type': u'multipolygon', u'WKT': u'    MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20)))'}})
        feat = fs.next()
        actual = json.loads(feat.to_geojson())
        eq_(actual,{u'geometry': {u'type': u'GeometryCollection', u'geometries': [{u'type': u'Polygon', u'coordinates': [[[1, 1], [2, 1], [2, 2], [1, 2], [1, 1]]]}, {u'type': u'Point', u'coordinates': [2, 3]}, {u'type': u'LineString', u'coordinates': [[2, 3], [3, 4]]}]}, u'type': u'Feature', u'id': 10, u'properties': {u'type': u'collection', u'WKT': u'      GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))'}})

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
