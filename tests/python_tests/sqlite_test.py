#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_attachdb_with_relative_file():
    # The point table and index is in the qgis_spatiallite.sqlite
    # database.  If either is not found, then this fails
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='point',
        attachdb='scratch@qgis_spatiallite.sqlite'
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['pkuid'],1)

def test_attachdb_with_multiple_files():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='attachedtest',
        attachdb='scratch1@:memory:,scratch2@:memory:',
        initdb='''
            create table scratch1.attachedtest (the_geom);
            create virtual table scratch2.idx_attachedtest_the_geom using rtree(pkid,xmin,xmax,ymin,ymax);
            insert into scratch2.idx_attachedtest_the_geom values (1,-7799225.5,-7778571.0,1393264.125,1417719.375);
            '''
        )
    fs = ds.featureset()
    feature = fs.next()
    # the above should not throw but will result in no features
    eq_(feature,None)

def test_attachdb_with_absolute_file():
    # The point table and index is in the qgis_spatiallite.sqlite
    # database.  If either is not found, then this fails
    ds = mapnik2.SQLite(file=os.getcwd() + '/../data/sqlite/world.sqlite', 
        table='point',
        attachdb='scratch@qgis_spatiallite.sqlite'
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['pkuid'],1)

def test_attachdb_with_index():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='attachedtest',
        attachdb='scratch@:memory:',
        initdb='''
            create table scratch.attachedtest (the_geom);
            create virtual table scratch.idx_attachedtest_the_geom using rtree(pkid,xmin,xmax,ymin,ymax);
            insert into scratch.idx_attachedtest_the_geom values (1,-7799225.5,-7778571.0,1393264.125,1417719.375);
            '''
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature,None)
    
def test_attachdb_with_explicit_index():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='attachedtest',
        index_table='myindex',
        attachdb='scratch@:memory:',
        initdb='''
            create table scratch.attachedtest (the_geom);
            create virtual table scratch.myindex using rtree(pkid,xmin,xmax,ymin,ymax);
            insert into scratch.myindex values (1,-7799225.5,-7778571.0,1393264.125,1417719.375);
            '''
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature,None)

def test_subqueries():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='world_merc',
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['OGC_FID'],1)
    eq_(feature['fips'],u'AC')

    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='(select * from world_merc)',
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['OGC_FID'],1)
    eq_(feature['fips'],u'AC')

    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='(select GEOMETRY,OGC_FID,fips from world_merc)',
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['OGC_FID'],1)
    eq_(feature['fips'],u'AC')

    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='(select GEOMETRY,rowid as aliased_id,fips from world_merc)',
        key_field='aliased_id'
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['aliased_id'],1)
    eq_(feature['fips'],u'AC')

    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='(select GEOMETRY,OGC_FID,OGC_FID as rowid,fips from world_merc)',
        )
    fs = ds.featureset()
    feature = fs.next()
    eq_(feature['rowid'],1)
    eq_(feature['fips'],u'AC')

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
