#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'sqlite' in mapnik.DatasourceCache.plugin_names():

    def test_attachdb_with_relative_file():
        # The point table and index is in the qgis_spatiallite.sqlite
        # database.  If either is not found, then this fails
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='point',
            attachdb='scratch@qgis_spatiallite.sqlite'
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['pkuid'],1)

    def test_attachdb_with_multiple_files():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='attachedtest',
            attachdb='scratch1@:memory:,scratch2@:memory:',
            initdb='''
                create table scratch1.attachedtest (the_geom);
                create virtual table scratch2.idx_attachedtest_the_geom using rtree(pkid,xmin,xmax,ymin,ymax);
                insert into scratch2.idx_attachedtest_the_geom values (1,-7799225.5,-7778571.0,1393264.125,1417719.375);
                '''
            )
        fs = ds.featureset()
        feature = None
        try :
            feature = fs.next()
        except StopIteration:
            pass
        # the above should not throw but will result in no features
        eq_(feature,None)

    def test_attachdb_with_absolute_file():
        # The point table and index is in the qgis_spatiallite.sqlite
        # database.  If either is not found, then this fails
        ds = mapnik.SQLite(file=os.getcwd() + '/../data/sqlite/world.sqlite',
            table='point',
            attachdb='scratch@qgis_spatiallite.sqlite'
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['pkuid'],1)

    def test_attachdb_with_index():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='attachedtest',
            attachdb='scratch@:memory:',
            initdb='''
                create table scratch.attachedtest (the_geom);
                create virtual table scratch.idx_attachedtest_the_geom using rtree(pkid,xmin,xmax,ymin,ymax);
                insert into scratch.idx_attachedtest_the_geom values (1,-7799225.5,-7778571.0,1393264.125,1417719.375);
                '''
            )

        fs = ds.featureset()
        feature = None
        try :
            feature = fs.next()
        except StopIteration:
            pass
        eq_(feature,None)

    def test_attachdb_with_explicit_index():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
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
        feature = None
        try:
            feature = fs.next()
        except StopIteration:
            pass
        eq_(feature,None)

    def test_attachdb_with_sql_join():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select * from world_merc INNER JOIN business on world_merc.iso3 = business.ISO3 limit 100)',
            attachdb='busines@business.sqlite'
            )
        eq_(len(ds.fields()),29)
        eq_(ds.fields(),['OGC_FID', 'fips', 'iso2', 'iso3', 'un', 'name', 'area', 'pop2005', 'region', 'subregion', 'lon', 'lat', 'ISO3:1', '1995', '1996', '1997', '1998', '1999', '2000', '2001', '2002', '2003', '2004', '2005', '2006', '2007', '2008', '2009', '2010'])
        eq_(ds.field_types(),['int', 'str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float', 'str', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int'])
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

    def test_attachdb_with_sql_join_count():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select * from world_merc INNER JOIN business on world_merc.iso3 = business.ISO3 limit 100)',
            attachdb='busines@business.sqlite'
            )
        eq_(len(ds.fields()),29)
        eq_(ds.fields(),['OGC_FID', 'fips', 'iso2', 'iso3', 'un', 'name', 'area', 'pop2005', 'region', 'subregion', 'lon', 'lat', 'ISO3:1', '1995', '1996', '1997', '1998', '1999', '2000', '2001', '2002', '2003', '2004', '2005', '2006', '2007', '2008', '2009', '2010'])
        eq_(ds.field_types(),['int', 'str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float', 'str', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int'])
        eq_(len(ds.all_features()),100)

    def test_attachdb_with_sql_join_count2():
        '''
        sqlite3 world.sqlite
        attach database 'business.sqlite' as business;
        select count(*) from world_merc INNER JOIN business on world_merc.iso3 = business.ISO3;
        '''
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select * from world_merc INNER JOIN business on world_merc.iso3 = business.ISO3)',
            attachdb='busines@business.sqlite'
            )
        eq_(len(ds.fields()),29)
        eq_(ds.fields(),['OGC_FID', 'fips', 'iso2', 'iso3', 'un', 'name', 'area', 'pop2005', 'region', 'subregion', 'lon', 'lat', 'ISO3:1', '1995', '1996', '1997', '1998', '1999', '2000', '2001', '2002', '2003', '2004', '2005', '2006', '2007', '2008', '2009', '2010'])
        eq_(ds.field_types(),['int', 'str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float', 'str', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int'])
        eq_(len(ds.all_features()),192)

    def test_attachdb_with_sql_join_count3():
        '''
        select count(*) from (select * from world_merc where 1=1) as world_merc INNER JOIN business on world_merc.iso3 = business.ISO3;
        '''
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select * from (select * from world_merc where !intersects!) as world_merc INNER JOIN business on world_merc.iso3 = business.ISO3)',
            attachdb='busines@business.sqlite'
            )
        eq_(len(ds.fields()),29)
        eq_(ds.fields(),['OGC_FID', 'fips', 'iso2', 'iso3', 'un', 'name', 'area', 'pop2005', 'region', 'subregion', 'lon', 'lat', 'ISO3:1', '1995', '1996', '1997', '1998', '1999', '2000', '2001', '2002', '2003', '2004', '2005', '2006', '2007', '2008', '2009', '2010'])
        eq_(ds.field_types(),['int', 'str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float', 'str', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int'])
        eq_(len(ds.all_features()),192)

    def test_attachdb_with_sql_join_count4():
        '''
        select count(*) from (select * from world_merc where 1=1) as world_merc INNER JOIN business on world_merc.iso3 = business.ISO3;
        '''
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select * from (select * from world_merc where !intersects! limit 1) as world_merc INNER JOIN business on world_merc.iso3 = business.ISO3)',
            attachdb='busines@business.sqlite'
            )
        eq_(len(ds.fields()),29)
        eq_(ds.fields(),['OGC_FID', 'fips', 'iso2', 'iso3', 'un', 'name', 'area', 'pop2005', 'region', 'subregion', 'lon', 'lat', 'ISO3:1', '1995', '1996', '1997', '1998', '1999', '2000', '2001', '2002', '2003', '2004', '2005', '2006', '2007', '2008', '2009', '2010'])
        eq_(ds.field_types(),['int', 'str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float', 'str', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int', 'int'])
        eq_(len(ds.all_features()),1)

    def test_attachdb_with_sql_join_count5():
        '''
        select count(*) from (select * from world_merc where 1=1) as world_merc INNER JOIN business on world_merc.iso3 = business.ISO3;
        '''
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select * from (select * from world_merc where !intersects! and 1=2) as world_merc INNER JOIN business on world_merc.iso3 = business.ISO3)',
            attachdb='busines@business.sqlite'
            )
        # nothing is able to join to business so we don't pick up business schema
        eq_(len(ds.fields()),12)
        eq_(ds.fields(),['OGC_FID', 'fips', 'iso2', 'iso3', 'un', 'name', 'area', 'pop2005', 'region', 'subregion', 'lon', 'lat'])
        eq_(ds.field_types(),['int', 'str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float'])
        eq_(len(ds.all_features()),0)

    def test_subqueries():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
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

        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
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

        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select OGC_FID,GEOMETRY from world_merc)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['OGC_FID'],1)
        eq_(len(feature),1)

        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select GEOMETRY,OGC_FID,fips from world_merc)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['OGC_FID'],1)
        eq_(feature['fips'],u'AC')

        # same as above, except with alias like postgres requires
        # TODO - should we try to make this work?
        #ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
        #    table='(select GEOMETRY,rowid as aliased_id,fips from world_merc) as table',
        #    key_field='aliased_id'
        #    )
        #fs = ds.featureset()
        #feature = fs.next()
        #eq_(feature['aliased_id'],1)
        #eq_(feature['fips'],u'AC')

        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',
            table='(select GEOMETRY,OGC_FID,OGC_FID as rowid,fips from world_merc)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['rowid'],1)
        eq_(feature['fips'],u'AC')

    def test_empty_db():
        ds = mapnik.SQLite(file='../data/sqlite/empty.db',
            table='empty',
            )
        fs = ds.featureset()
        feature = None
        try:
            feature = fs.next()
        except StopIteration:
            pass
        eq_(feature,None)

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.SQLite(file='../data/sqlite/empty.db',
            table='empty',
            )
        eq_(len(ds.fields()),25)
        eq_(ds.fields(),['OGC_FID', 'scalerank', 'labelrank', 'featurecla', 'sovereignt', 'sov_a3', 'adm0_dif', 'level', 'type', 'admin', 'adm0_a3', 'geou_dif', 'name', 'abbrev', 'postal', 'name_forma', 'terr_', 'name_sort', 'map_color', 'pop_est', 'gdp_md_est', 'fips_10_', 'iso_a2', 'iso_a3', 'iso_n3'])
        eq_(ds.field_types(),['int', 'int', 'int', 'str', 'str', 'str', 'float', 'float', 'str', 'str', 'str', 'float', 'str', 'str', 'str', 'str', 'str', 'str', 'float', 'float', 'float', 'float', 'str', 'str', 'float'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)

    def test_intersects_token1():
        ds = mapnik.SQLite(file='../data/sqlite/empty.db',
            table='(select * from empty where !intersects!)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature,None)

    def test_intersects_token1():
        ds = mapnik.SQLite(file='../data/sqlite/empty.db',
            table='(select * from empty where "a"!="b" and !intersects!)',
            )
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature,None)

    def test_intersects_token1():
        ds = mapnik.SQLite(file='../data/sqlite/empty.db',
            table='(select * from empty where "a"!="b" and !intersects!)',
            )
        fs = ds.featureset()
        feature = None
        try :
            feature = fs.next()
        except StopIteration:
            pass
        eq_(feature,None)

    # https://github.com/mapnik/mapnik/issues/1537
    # this works because key_field is manually set
    def test_db_with_one_text_column():
        # form up an in-memory test db
        wkb = '010100000000000000000000000000000000000000'
        ds = mapnik.SQLite(file=':memory:',
            table='test1',
            initdb='''
                create table test1 (alias TEXT,geometry BLOB);
                insert into test1 values ("test",x'%s');
                ''' % wkb,
            extent='-180,-60,180,60',
            use_spatial_index=False,
            key_field='alias'
        )
        eq_(len(ds.fields()),1)
        eq_(ds.fields(),['alias'])
        eq_(ds.field_types(),['str'])
        fs = ds.all_features()
        eq_(len(fs),1)
        feat = fs[0]
        #eq_(feat.id(),1)
        eq_(feat['alias'],'test')
        eq_(len(feat.geometries()),1)
        eq_(feat.geometries()[0].to_wkt(),'Point(0.0 0.0)')

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
