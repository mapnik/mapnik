#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path
from Queue import Queue
import threading

import os, mapnik2
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
    ds = mapnik2.SQLite(file=DB,table=TABLE)
    fs = ds.all_features()

if 'sqlite' in mapnik2.DatasourceCache.instance().plugin_names():
    
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
        cur.execute("Select count(*) from idx_%s_GEOMETRY" % TABLE.replace("'",""))
        conn.commit()
        eq_(cur.fetchone()[0],TOTAL)
        cur.close()

        ds = mapnik2.SQLite(file=DB,table=TABLE)
        fs = ds.all_features()
        eq_(len(fs),TOTAL)
        os.unlink(index)
        ds = mapnik2.SQLite(file=DB,table=TABLE,use_spatial_index=False)
        fs = ds.all_features()
        eq_(len(fs),TOTAL)
        eq_(os.path.exists(index),False)

        ds = mapnik2.SQLite(file=DB,table=TABLE,use_spatial_index=True)
        fs = ds.all_features()
        for feat in fs:
            query = mapnik2.Query(feat.envelope())
            selected = ds.features(query)
            eq_(len(selected.features)>=1,True)

        eq_(os.path.exists(index),True)
        os.unlink(index)


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
