#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'sqlite' in mapnik2.DatasourceCache.instance().plugin_names():
    
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
    
    def test_attachdb_with_sql_join():
        ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
            table='(select * from world_merc INNER JOIN business on world_merc.iso3 = business.ISO3 limit 100)',
            attachdb='busines@business.sqlite'
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature.id(),1)
        expected = {
          1995:0,
          1996:0,
          1997:0,
          1998:0,
          1999:0,
          2000:0,
          2001:0,
          2002:0,
          2003:0,
          2004:0,
          2005:0,
          2006:0,
          2007:0,
          2008:0,
          2009:0,
          2010:0,
          # this appears to be sqlites way of
          # automatically handling clashing column names
          'ISO3:1':'ATG',
          'OGC_FID':1,
          'area':44,
          'fips':u'AC',
          'iso2':u'AG',
          'iso3':u'ATG',
          'lat':17.078,
          'lon':-61.783,
          'name':u'Antigua and Barbuda',
          'pop2005':83039,
          'region':19,
          'subregion':29,
          'un':28
        }
        for k,v in expected.items():
            try:
                eq_(feature[str(k)],v)
            except:
                #import pdb;pdb.set_trace()
                print 'invalid key/v %s/%s for: %s' % (k,v,feature) 
    
    def test_subqueries():
        ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
            table='world_merc',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['OGC_FID'],1)
        eq_(feature['fips'],u'AC')
        eq_(feature['iso2'],u'AG')
        eq_(feature['iso3'],u'ATG')
        eq_(feature['un'],28)
        eq_(feature['name'],u'Antigua and Barbuda')
        eq_(feature['area'],44)
        eq_(feature['pop2005'],83039)
        eq_(feature['region'],19)
        eq_(feature['subregion'],29)
        eq_(feature['lon'],-61.783)
        eq_(feature['lat'],17.078)
    
        ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
            table='(select * from world_merc)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['OGC_FID'],1)
        eq_(feature['fips'],u'AC')
        eq_(feature['iso2'],u'AG')
        eq_(feature['iso3'],u'ATG')
        eq_(feature['un'],28)
        eq_(feature['name'],u'Antigua and Barbuda')
        eq_(feature['area'],44)
        eq_(feature['pop2005'],83039)
        eq_(feature['region'],19)
        eq_(feature['subregion'],29)
        eq_(feature['lon'],-61.783)
        eq_(feature['lat'],17.078)
        
        ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
            table='(select OGC_FID,GEOMETRY from world_merc)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['OGC_FID'],1)
        eq_(len(feature),1)
    
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


    def test_empty_db():
        ds = mapnik2.SQLite(file='../data/sqlite/empty.db', 
            table='empty',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature,None)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
