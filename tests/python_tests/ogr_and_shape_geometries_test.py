#!/usr/bin/env python

from nose.tools import *

from utilities import execution_path, run_all

import os, sys, glob, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# TODO - fix truncation in shapefile...
polys = ["POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))",
         "POLYGON ((35 10, 10 20, 15 40, 45 45, 35 10),(20 30, 35 35, 30 20, 20 30))",
         "MULTIPOLYGON (((30 20, 10 40, 45 40, 30 20)),((15 5, 40 10, 10 20, 5 10, 15 5)))"
         "MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20)))"
        ]

plugins = mapnik.DatasourceCache.plugin_names()
if 'shape' in plugins and 'ogr' in plugins:

    def ensure_geometries_are_interpreted_equivalently(filename):
        ds1 = mapnik.Ogr(file=filename,layer_by_index=0)
        ds2 = mapnik.Shapefile(file=filename)
        fs1 = ds1.featureset()
        fs2 = ds2.featureset()
        count = 0;
        import itertools
        for feat1,feat2 in itertools.izip(fs1, fs2):
            count += 1
            eq_(feat1.attributes,feat2.attributes)
            # TODO - revisit this: https://github.com/mapnik/mapnik/issues/1093
            # eq_(feat1.to_geojson(),feat2.to_geojson())
            #eq_(feat1.geometries().to_wkt(),feat2.geometries().to_wkt())
            #eq_(feat1.geometries().to_wkb(mapnik.wkbByteOrder.NDR),feat2.geometries().to_wkb(mapnik.wkbByteOrder.NDR))
            #eq_(feat1.geometries().to_wkb(mapnik.wkbByteOrder.XDR),feat2.geometries().to_wkb(mapnik.wkbByteOrder.XDR))

    def test_simple_polys():
        ensure_geometries_are_interpreted_equivalently('../data/shp/wkt_poly.shp')

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
