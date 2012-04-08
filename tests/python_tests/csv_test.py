#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob
from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'csv' in mapnik.DatasourceCache.instance().plugin_names():

    def get_csv_ds(filename):
        return mapnik.Datasource(type='csv',file=os.path.join('../data/csv/',filename),quiet=True)

    def test_broken_files(visual=False):
        broken = glob.glob("../data/csv/fails/*.*")
        broken.extend(glob.glob("../data/csv/warns/*.*"))

        # Add a filename that doesn't exist 
        broken.append("../data/csv/fails/does_not_exist.csv")

        for csv in broken:
            throws = False
            if visual:
                try:
                    ds = mapnik.Datasource(type='csv',file=csv,strict=True,quiet=True)
                    print '\x1b[33mfailed\x1b[0m',csv
                except Exception:
                    print '\x1b[1;32m✓ \x1b[0m', csv

    def test_good_files(visual=False):
        good_files = glob.glob("../data/csv/*.*")
        good_files.extend(glob.glob("../data/csv/warns/*.*"))

        for csv in good_files:
            if visual:
                try:
                    ds = mapnik.Datasource(type='csv',file=csv,quiet=True)
                    print '\x1b[1;32m✓ \x1b[0m', csv
                except Exception:
                    print '\x1b[33mfailed\x1b[0m',csv

    def test_type_detection(**kwargs):
        ds = get_csv_ds('nypd.csv')
        eq_(ds.fields(),['Precinct','Phone','Address','City','geo_longitude','geo_latitude','geo_accuracy'])
        eq_(ds.field_types(),['str','str','str','str','float','float','str'])
        feat = ds.featureset().next()
        attr = {'City': u'New York, NY', 'geo_accuracy': u'house', 'Phone': u'(212) 334-0711', 'Address': u'19 Elizabeth Street', 'Precinct': u'5th Precinct', 'geo_longitude': -70, 'geo_latitude': 40}
        eq_(feat.attributes,attr)
        eq_(len(ds.all_features()),2)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_skipping_blank_rows(**kwargs):
        ds = get_csv_ds('blank_rows.csv')
        eq_(ds.fields(),['x','y','name'])
        eq_(ds.field_types(),['int','int','str'])
        eq_(len(ds.all_features()),2)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_empty_rows(**kwargs):
        ds = get_csv_ds('empty_rows.csv')
        eq_(len(ds.fields()),10)
        eq_(len(ds.field_types()),10)
        eq_(ds.fields(),['x', 'y', 'text', 'date', 'integer', 'boolean', 'float', 'time', 'datetime', 'empty_column'])
        eq_(ds.field_types(),['int', 'int', 'str', 'str', 'int', 'str', 'float', 'str', 'str', 'str'])
        fs = ds.featureset()
        feat = fs.next()
        attr = {'x': 0, 'empty_column': u'', 'text': u'a b', 'float': 1.0, 'datetime': u'1971-01-01T04:14:00', 'y': 0, 'boolean': u'True', 'time': u'04:14:00', 'date': u'1971-01-01', 'integer': 40}
        eq_(feat.attributes,attr)
        while feat:
            eq_(len(feat),10)
            eq_(feat['empty_column'],u'')
            feat = fs.next()
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_slashes(**kwargs):
        ds = get_csv_ds('has_attributes_with_slashes.csv')
        eq_(len(ds.fields()),3)
        fs = ds.all_features()
        eq_(fs[0].attributes,{'x':0,'y':0,'name':u'a/a'})
        eq_(fs[1].attributes,{'x':1,'y':4,'name':u'b/b'})
        eq_(fs[2].attributes,{'x':10,'y':2.5,'name':u'c/c'})
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_wkt_field(**kwargs):
        ds = get_csv_ds('wkt.csv')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['type','WKT'])
        eq_(ds.field_types(),['str','str'])
        fs = ds.all_features()
        #import pdb;pdb.set_trace()
        eq_(len(fs[0].geometries()),1)
        eq_(fs[0].geometries()[0].type(),mapnik.DataGeometryType.Point)
        eq_(len(fs[1].geometries()),1)
        eq_(fs[1].geometries()[0].type(),mapnik.DataGeometryType.LineString)
        eq_(len(fs[2].geometries()),1)
        eq_(fs[2].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[3].geometries()),1) # one geometry, two parts
        eq_(fs[3].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        # tests assuming we want to flatten geometries
        # ideally we should not have to:
        # https://github.com/mapnik/mapnik/issues?labels=multigeom+robustness&sort=created&direction=desc&state=open&page=1
        eq_(len(fs[4].geometries()),4)
        eq_(fs[4].geometries()[0].type(),mapnik.DataGeometryType.Point)
        eq_(len(fs[5].geometries()),2)
        eq_(fs[5].geometries()[0].type(),mapnik.DataGeometryType.LineString)
        eq_(len(fs[6].geometries()),2)
        eq_(fs[6].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[7].geometries()),2)
        eq_(fs[7].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Collection)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_handling_of_missing_header(**kwargs):
        ds = get_csv_ds('missing_header.csv')
        eq_(len(ds.fields()),6)
        eq_(ds.fields(),['one','two','x','y','_4','aftermissing'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['_4'],'missing')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_handling_of_headers_that_are_numbers(**kwargs):
        ds = get_csv_ds('numbers_for_headers.csv')
        eq_(len(ds.fields()),5)
        eq_(ds.fields(),['x','y','1990','1991','1992'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['1990'],1)
        eq_(feat['1991'],2)
        eq_(feat['1992'],3)

        eq_(mapnik.Expression("[1991]=2").evaluate(feat),True)

    def test_quoted_numbers(**kwargs):
        ds = get_csv_ds('points.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','label'])
        fs = ds.all_features()
        eq_(fs[0]['label'],"0,0")
        eq_(fs[1]['label'],"5,5")
        eq_(fs[2]['label'],"0,5")
        eq_(fs[3]['label'],"5,0")
        eq_(fs[4]['label'],"2.5,2.5")
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_windows_newlines(**kwargs):
        ds = get_csv_ds('windows_newlines.csv')
        eq_(len(ds.fields()),3)
        feats = ds.all_features()
        eq_(len(feats),1)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],1)
        eq_(feat['y'],10)
        eq_(feat['z'],9999.9999)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_mac_newlines(**kwargs):
        ds = get_csv_ds('windows_newlines.csv')
        eq_(len(ds.fields()),3)
        feats = ds.all_features()
        eq_(len(feats),1)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],1)
        eq_(feat['y'],10)
        eq_(feat['z'],9999.9999)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_tabs(**kwargs):
        ds = get_csv_ds('tabs_in_csv.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','z'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],-122)
        eq_(feat['y'],48)
        eq_(feat['z'],0)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_separator_pipes(**kwargs):
        ds = get_csv_ds('pipe_delimiters.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','z'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['z'],'hello')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_separator_semicolon(**kwargs):
        ds = get_csv_ds('semicolon_delimiters.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','z'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['z'],'hello')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_that_null_and_bool_keywords_are_empty_strings(**kwargs):
        ds = get_csv_ds('nulls_and_booleans_as_strings.csv')
        eq_(len(ds.fields()),4)
        eq_(ds.fields(),['x','y','null','boolean'])
        eq_(ds.field_types(),['int','int','str','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['null'],'null')
        eq_(feat['boolean'],'true')
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['null'],'')
        eq_(feat['boolean'],'false')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = get_csv_ds('lon_lat.csv')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['lon','lat'])
        eq_(ds.field_types(),['int','int'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    def test_that_leading_zeros_mean_strings(**kwargs):
        ds = get_csv_ds('leading_zeros.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','fips'])
        eq_(ds.field_types(),['int','int','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['fips'],'001')
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['fips'],'003')
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['fips'],'005')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    def test_advanced_geometry_detection(**kwargs):
        ds = get_csv_ds('point_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)
        ds = get_csv_ds('poly_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)
        ds = get_csv_ds('multi_poly_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)
        ds = get_csv_ds('line_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.LineString)

if __name__ == "__main__":
    setup()
    [eval(run)(visual=True) for run in dir() if 'test_' in run]
