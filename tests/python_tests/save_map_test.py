#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all
import tempfile

import os, sys, glob, mapnik
from xml.etree import ElementTree

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def compare_map(xmlfile):
    have_inputs = True
    missing_plugins = set()
    e = ElementTree.parse(xmlfile)
    data_source_type_params = e.findall(".//Layer/Datasource/Parameter[@name=\"type\"]")
    if data_source_type_params is not None and len(data_source_type_params) > 0:
        for p in data_source_type_params:
            dstype = p.text
            if dstype not in mapnik.DatasourceCache.plugin_names():
                have_inputs = False
                missing_plugins.add(dstype)

    if not have_inputs:
        print 'Notice: skipping map comparison for %s due to missing input plugins: %s' % (os.path.basename(xmlfile), list(missing_plugins))
        return False

    m = mapnik.Map(256, 256)
    absolute_base = os.path.abspath(os.path.dirname(xmlfile))
    mapnik.load_map(m, xmlfile, False, absolute_base)
    (handle, test_map) = tempfile.mkstemp(suffix='.xml', prefix='mapnik-temp-map1-')
    os.close(handle)
    (handle, test_map2) = tempfile.mkstemp(suffix='.xml', prefix='mapnik-temp-map2-')
    os.close(handle)
    if os.path.exists(test_map):
        os.remove(test_map)
    mapnik.save_map(m, test_map)
    new_map = mapnik.Map(256, 256)
    mapnik.load_map(new_map, test_map, False, absolute_base)
    open(test_map2,'w').write(mapnik.save_map_to_string(new_map))
    diff = ' diff %s %s' % (os.path.abspath(test_map),os.path.abspath(test_map2))
    try:
        eq_(open(test_map).read(),open(test_map2).read())
    except AssertionError, e:
        raise AssertionError('serialized map "%s" not the same after being reloaded, \ncompare with command:\n\n$%s' % (xmlfile, diff))

    if os.path.exists(test_map):
        os.remove(test_map)
    else:
        # Fail, the map wasn't written
        return False

def test_compare_map():
    for m in glob.glob("../data/good_maps/*.xml"):
        compare_map(m)

if __name__ == "__main__":
    setup()
    run_all(eval(x) for x in dir() if x.startswith("test_"))
