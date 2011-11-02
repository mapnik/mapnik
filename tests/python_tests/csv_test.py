#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob
from nose.tools import *
from utilities import execution_path

import os, mapnik2


def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'csv' in mapnik2.DatasourceCache.instance().plugin_names():

    def get_csv_ds(filename):
        return mapnik2.Datasource(type='csv',file=os.path.join('../data/csv/',filename),quiet=True)

    def test_broken_files(visual=False):
        broken = glob.glob("../data/csv/fails/*.*")
        broken.extend(glob.glob("../data/csv/warns/*.*"))
    
        # Add a filename that doesn't exist 
        broken.append("../data/csv/fails/does_not_exist.csv")
    
        for csv in broken:
            throws = False
            if visual:
                try:
                    ds = mapnik2.Datasource(type='csv',file=csv,strict=True,quiet=True)
                    print '\x1b[33mfailed\x1b[0m',csv
                except Exception:
                    print '\x1b[1;32m✓ \x1b[0m', csv
    
    def test_good_files(visual=False):
        good_files = glob.glob("../data/csv/*.*")
        good_files.extend(glob.glob("../data/csv/warns/*.*"))
    
        for csv in good_files:
            if visual:
                try:
                    ds = mapnik2.Datasource(type='csv',file=csv,quiet=True)
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

    def test_skipping_blank_rows(**kwargs):
        ds = get_csv_ds('blank_rows.csv')
        eq_(ds.fields(),['x','y','name'])
        eq_(ds.field_types(),['int','int','str'])
        eq_(len(ds.all_features()),2)

    def test_empty_rows(**kwargs):
        ds = get_csv_ds('empty_rows.csv')
        eq_(len(ds.fields()),10)
        eq_(len(ds.field_types()),10)
        eq_(ds.fields(),['x', 'y', 'text', 'date', 'integer', 'boolean', 'float', 'time', 'datetime', 'empty_column'])
        eq_(ds.field_types(),['int', 'int', 'str', 'str', 'int', 'bool', 'float', 'str', 'str', 'str'])
        fs = ds.featureset()
        feat = fs.next()
        attr = {'x': 0, 'empty_column': None, 'text': u'a b', 'float': 1.0, 'datetime': u'1971-01-01T04:14:00', 'y': 0, 'boolean': True, 'time': u'04:14:00', 'date': u'1971-01-01', 'integer': 40}
        eq_(feat.attributes,attr)
        while feat:
            eq_(len(feat),10)
            eq_(feat['empty_column'],None)
            feat = fs.next()
    
    def test_slashes(**kwargs):
        ds = get_csv_ds('has_attributes_with_slashes.csv')
        eq_(len(ds.fields()),3)
        fs = ds.all_features()
        eq_(fs[0].attributes,{'x':0,'y':0,'name':u'a/a'})
        eq_(fs[1].attributes,{'x':1,'y':4,'name':u'b/b'})
        eq_(fs[2].attributes,{'x':10,'y':2.5,'name':u'c/c'})

    def test_wkt_field(**kwargs):
        ds = get_csv_ds('wkt.csv')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['type','WKT'])
        eq_(ds.field_types(),['str','str'])
        fs = ds.all_features()
        #import pdb;pdb.set_trace()
        eq_(len(fs[0].geometries()),1)
        eq_(fs[0].geometries()[0].type(),mapnik2.GeometryType.Point)
        eq_(len(fs[1].geometries()),1)
        eq_(fs[1].geometries()[0].type(),mapnik2.GeometryType.LineString)
        eq_(len(fs[2].geometries()),1)
        eq_(fs[2].geometries()[0].type(),mapnik2.GeometryType.Polygon)
        eq_(len(fs[3].geometries()),1) # one geometry, two parts
        eq_(fs[3].geometries()[0].type(),mapnik2.GeometryType.Polygon)
        # tests assuming we want to flatten geometries
        # ideally we should not have to:
        # https://github.com/mapnik/mapnik/issues?labels=multigeom+robustness&sort=created&direction=desc&state=open&page=1
        eq_(len(fs[4].geometries()),4)
        eq_(fs[4].geometries()[0].type(),mapnik2.GeometryType.Point)
        eq_(len(fs[5].geometries()),2)
        eq_(fs[5].geometries()[0].type(),mapnik2.GeometryType.LineString)
        eq_(len(fs[6].geometries()),2)
        eq_(fs[6].geometries()[0].type(),mapnik2.GeometryType.Polygon)
        eq_(len(fs[7].geometries()),2)
        eq_(fs[7].geometries()[0].type(),mapnik2.GeometryType.Polygon)
        

    def test_handling_of_missing_header(**kwargs):
        ds = get_csv_ds('missing_header.csv')
        eq_(len(ds.fields()),6)
        eq_(ds.fields(),['one','two','x','y','_4','aftermissing'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['_4'],'missing')

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
        
        eq_(mapnik2.Expression("[1991]=2").evaluate(feat),True)

if __name__ == "__main__":
    setup()
    [eval(run)(visual=True) for run in dir() if 'test_' in run]
