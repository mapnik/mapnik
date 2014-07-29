#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'shape' in mapnik.DatasourceCache.plugin_names():

    # Shapefile initialization
    def test_shapefile_init():
        s = mapnik.Shapefile(file='../../demo/data/boundaries')

        e = s.envelope()

        assert_almost_equal(e.minx, -11121.6896651, places=7)
        assert_almost_equal(e.miny, -724724.216526, places=6)
        assert_almost_equal(e.maxx, 2463000.67866, places=5)
        assert_almost_equal(e.maxy, 1649661.267, places=3)

    # Shapefile properties
    def test_shapefile_properties():
        s = mapnik.Shapefile(file='../../demo/data/boundaries', encoding='latin1')
        f = s.features_at_point(s.envelope().center()).features[0]

        eq_(f['CGNS_FID'], u'6f733341ba2011d892e2080020a0f4c9')
        eq_(f['COUNTRY'], u'CAN')
        eq_(f['F_CODE'], u'FA001')
        eq_(f['NAME_EN'], u'Quebec')
        # this seems to break if icu data linking is not working
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['Shape_Area'], 1512185733150.0)
        eq_(f['Shape_Leng'], 19218883.724300001)

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Shapefile(file='../data/shp/world_merc')
        eq_(len(ds.fields()),11)
        eq_(ds.fields(),['FIPS', 'ISO2', 'ISO3', 'UN', 'NAME', 'AREA', 'POP2005', 'REGION', 'SUBREGION', 'LON', 'LAT'])
        eq_(ds.field_types(),['str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)

    def test_dbf_logical_field_is_boolean():
        ds = mapnik.Shapefile(file='../data/shp/long_lat')
        eq_(len(ds.fields()),7)
        eq_(ds.fields(),['LONG', 'LAT', 'LOGICAL_TR', 'LOGICAL_FA', 'CHARACTER', 'NUMERIC', 'DATE'])
        eq_(ds.field_types(),['str', 'str', 'bool', 'bool', 'str', 'float', 'str'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        feat = ds.all_features()[0]
        eq_(feat.id(),1)
        eq_(feat['LONG'],'0')
        eq_(feat['LAT'],'0')
        eq_(feat['LOGICAL_TR'],True)
        eq_(feat['LOGICAL_FA'],False)
        eq_(feat['CHARACTER'],'254')
        eq_(feat['NUMERIC'],32)
        eq_(feat['DATE'],'20121202')

    # created by hand in qgis 1.8.0
    def test_shapefile_point2d_from_qgis():
        ds = mapnik.Shapefile(file='../data/shp/points/qgis.shp')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['id','name'])
        eq_(ds.field_types(),['int','str'])
        eq_(len(ds.all_features()),3)

    # ogr2ogr tests/data/shp/3dpoint/ogr_zfield.shp tests/data/shp/3dpoint/qgis.shp -zfield id
    def test_shapefile_point_z_from_qgis():
        ds = mapnik.Shapefile(file='../data/shp/points/ogr_zfield.shp')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['id','name'])
        eq_(ds.field_types(),['int','str'])
        eq_(len(ds.all_features()),3)

    def test_shapefile_multipoint_from_qgis():
        ds = mapnik.Shapefile(file='../data/shp/points/qgis_multi.shp')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['id','name'])
        eq_(ds.field_types(),['int','str'])
        eq_(len(ds.all_features()),1)

    # pointzm from arcinfo
    def test_shapefile_point_zm_from_arcgis():
        ds = mapnik.Shapefile(file='../data/shp/points/poi.shp')
        eq_(len(ds.fields()),7)
        eq_(ds.fields(),['interst_id', 'state_d', 'cnty_name', 'latitude', 'longitude', 'Name', 'Website'])
        eq_(ds.field_types(),['str', 'str', 'str', 'float', 'float', 'str', 'str'])
        eq_(len(ds.all_features()),17)

    # copy of the above with ogr2ogr that makes m record 14 instead of 18
    def test_shapefile_point_zm_from_ogr():
        ds = mapnik.Shapefile(file='../data/shp/points/poi_ogr.shp')
        eq_(len(ds.fields()),7)
        eq_(ds.fields(),['interst_id', 'state_d', 'cnty_name', 'latitude', 'longitude', 'Name', 'Website'])
        eq_(ds.field_types(),['str', 'str', 'str', 'float', 'float', 'str', 'str'])
        eq_(len(ds.all_features()),17)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
