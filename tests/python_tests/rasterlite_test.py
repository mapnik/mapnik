#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all, contains_word, get_unique_colors

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))


if 'rasterlite' in mapnik.DatasourceCache.plugin_names():

    def test_rasterlite():
        ds = mapnik.Rasterlite(
            file = '../data/rasterlite/globe.sqlite',
            table = 'globe'
            )
        e = ds.envelope()

        assert_almost_equal(e.minx,-180, places=5)
        assert_almost_equal(e.miny, -90, places=5)
        assert_almost_equal(e.maxx, 180, places=5)
        assert_almost_equal(e.maxy,  90, places=5)
        eq_(len(ds.fields()),0)
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        fs = ds.features(query)
        feat = fs.next()
        eq_(feat.id(),1)
        eq_(feat.attributes,{})

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
