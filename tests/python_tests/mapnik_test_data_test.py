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
            if 'topo' in filepath:
                kwargs = {'type': 'ogr','file': filepath}
                kwargs['layer_by_index'] = 0
                ds = mapnik.Datasource(**kwargs)
            else:
                ext = os.path.splitext(filepath)[1]
                if plugin_mapping.get(ext):
                   for plugin in plugin_mapping[ext]:
                      kwargs = {'type': plugin,'file': filepath}
                      if plugin is 'ogr':
                          kwargs['layer_by_index'] = 0
                      print kwargs
                      ds = mapnik.Datasource(**kwargs)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
