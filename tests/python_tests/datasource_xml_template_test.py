#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from nose.tools import *
from utilities import execution_path
import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_datasource_template_is_working():
    m = mapnik.Map(256,256)
    try:
        mapnik.load_map(m,'../data/good_maps/datasource.xml')
    except RuntimeError, e:
        if "Required parameter 'type'" in str(e):
            raise RuntimeError(e)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
