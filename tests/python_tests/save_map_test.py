#!/usr/bin/env python

from nose.tools import *
from utilities import Todo
from utilities import execution_path
import tempfile

import os, sys, glob, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test():
    # TODO: Write a better test
    # 1. Construct map in memory
    # 2. Save map as XML
    # 3. Load map to a second object
    # 4. Compare both map objects
    map = mapnik.Map(256, 256)

    raise Todo("map comparison is currently broken due to lacking relative paths support (#324,#340")

    def compare_map(in_map):

        mapnik.load_map(map, in_map)

        (handle, test_map) = tempfile.mkstemp(suffix='.xml', prefix='mapnik-temp-map1-')
        os.close(handle)

        (handle, test_map2) = tempfile.mkstemp(suffix='.xml', prefix='mapnik-temp-map2-')
        os.close(handle)

        if os.path.exists(test_map):
            os.remove(test_map)

        mapnik.save_map(map, test_map)
        new_map = mapnik.Map(256, 256)

        mapnik.load_map(new_map, test_map)
        open(test_map2,'w').write(mapnik.save_map_to_string(new_map))

        diff = ' diff %s %s' % (os.path.abspath(test_map),os.path.abspath(test_map2))
        try:
            eq_(open(test_map).read(),open(test_map2).read())
        except AssertionError, e:
            raise AssertionError('serialized map "%s" not the same after being reloaded, \ncompare with command:\n\n$%s' % (in_map,diff))

        if os.path.exists(test_map):
            os.remove(test_map)
        else:
            # Fail, the map wasn't written
            return False

    for m in glob.glob("../data/good_maps/*.xml"):
        compare_map(m)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
