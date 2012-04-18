#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path
from Queue import Queue
import threading

import os, mapnik
import sqlite3

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

NUM_THREADS = 10
TOTAL = 245
DB = '../data/sqlite/world.sqlite'
TABLE= 'world_merc'

def create_ds():
    ds = mapnik.SQLite(file=DB,table=TABLE)
    fs = ds.all_features()

if 'sqlite' in mapnik.DatasourceCache.instance().plugin_names():

    def test_rtree_creation():

        index = DB +'.index'
        if os.path.exists(index):
            os.unlink(index)

        threads = []
        for i in range(NUM_THREADS):
            t = threading.Thread(target=create_ds)
            t.start()
            threads.append(t)

        for i in threads:
            i.join()

        eq_(os.path.exists(index),True)
        conn = sqlite3.connect(index)
        cur = conn.cursor()
        try:
            cur.execute("Select count(*) from idx_%s_GEOMETRY" % TABLE.replace("'",""))
            conn.commit()
            eq_(cur.fetchone()[0],TOTAL)
        except sqlite3.OperationalError:
            # don't worry about testing # of index records if 
            # python's sqlite module does not support rtree
            pass
        cur.close()

        ds = mapnik.SQLite(file=DB,table=TABLE)
        fs = ds.all_features()
        eq_(len(fs),TOTAL)
        os.unlink(index)
        ds = mapnik.SQLite(file=DB,table=TABLE,use_spatial_index=False)
        fs = ds.all_features()
        eq_(len(fs),TOTAL)
        eq_(os.path.exists(index),False)

        ds = mapnik.SQLite(file=DB,table=TABLE,use_spatial_index=True)
        fs = ds.all_features()
        for feat in fs:
            query = mapnik.Query(feat.envelope())
            selected = ds.features(query)
            eq_(len(selected.features)>=1,True)

        eq_(os.path.exists(index),True)
        os.unlink(index)

    def test_geometry_round_trip():
        test_db = '/tmp/mapnik-sqlite-point.db'

        # create test db
        conn = sqlite3.connect(test_db)
        cur = conn.cursor()
        cur.execute('''
             CREATE TABLE IF NOT EXISTS "point_table"
             (id INTEGER PRIMARY KEY AUTOINCREMENT, geometry BLOB, "name" varchar)
             ''')
        conn.commit()
        cur.close()

        # add a point as wkb to match how an ogr created db looks
        cur = conn.cursor()
        wkb = mapnik.Path.from_wkt('POINT(-122 48)').to_wkb(mapnik.wkbByteOrder.XDR)
        values = (None,sqlite3.Binary(wkb),"test point")
        cur.execute('''INSERT into "point_table" (id,geometry,name) values (?,?,?)''',values)
        conn.commit()
        cur.close()

        # ensure we can read this data back out properly with mapnik
        ds = mapnik.Datasource(**{'type':'sqlite','file':test_db, 'table':'point_table'})
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat.id(),1)
        eq_(feat['name'],'test point')
        geoms = feat.geometries()
        eq_(len(geoms),1)
        eq_(geoms.to_wkt(),'Point(-122.0 48.0)')

        # ensure it matches data read with just sqlite
        cur = conn.cursor()
        cur.execute('''SELECT * from point_table''')
        conn.commit()
        result = cur.fetchone()
        feat_id = result[0]
        eq_(feat_id,1)
        name = result[2]
        eq_(name,'test point')
        geom_wkb_blob = result[1]
        eq_(str(geom_wkb_blob),geoms.to_wkb(mapnik.wkbByteOrder.XDR))
        new_geom = mapnik.Path.from_wkb(str(geom_wkb_blob))
        eq_(new_geom.to_wkt(),geoms.to_wkt())

        # cleanup
        cur.close()
        os.unlink(test_db)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
