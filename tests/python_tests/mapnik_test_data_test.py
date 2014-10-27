#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik
from glob import glob

default_logging_severity = mapnik.logger.get_severity()

def setup():
    mapnik.logger.set_severity(mapnik.severity_type.None)
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def teardown():
    mapnik.logger.set_severity(default_logging_severity)

plugin_mapping = {
    '.csv' : ['csv'],
    '.json': ['geojson','ogr'],
    '.tif' : ['gdal'],
    #'.tif' : ['gdal','raster'],
    '.kml' : ['ogr'],
    '.gpx' : ['ogr'],
    '.vrt' : ['gdal']
}

def test_opening_data():
    # https://github.com/mapbox/mapnik-test-data
    # cd tests/data
    # git clone --depth 1 https://github.com/mapbox/mapnik-test-data
    if os.path.exists('../data/mapnik-test-data/'):
        files = glob('../data/mapnik-test-data/data/*/*.*')
        for filepath in files:
            ext = os.path.splitext(filepath)[1]
            if plugin_mapping.get(ext):
                #print 'testing opening %s' % filepath
                if 'topo' in filepath:
                    kwargs = {'type': 'ogr','file': filepath}
                    kwargs['layer_by_index'] = 0
                    try:
                        ds = mapnik.Datasource(**kwargs)
                    except Exception, e:
                        print 'could not open, %s: %s' % (kwargs,e)
                else:
                   for plugin in plugin_mapping[ext]:
                      kwargs = {'type': plugin,'file': filepath}
                      if plugin is 'ogr':
                          kwargs['layer_by_index'] = 0
                      try:
                          ds = mapnik.Datasource(**kwargs)
                      except Exception, e:
                          print 'could not open, %s: %s' % (kwargs,e)
            #else:
            #    print 'skipping opening %s' % filepath

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
