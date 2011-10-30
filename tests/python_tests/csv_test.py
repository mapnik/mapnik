#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob
from nose.tools import *
from utilities import execution_path

import os, mapnik2


def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'csv' in mapnik2.DatasourceCache.instance().plugin_names():

    def test_broken_files(visual=False):
        broken = glob.glob("../data/csv/fails/*.*")
        broken.extend(glob.glob("../data/csv/warns/*.*"))
    
        # Add a filename that doesn't exist 
        broken.append("../data/csv/fails/does_not_exist.csv")
    
        for csv in broken:
            throws = False
            if visual:
                try:
                    ds = mapnik2.Datasource(type='csv',file=csv,strict=True,quiet=True)
                    print '\x1b[33mfailed\x1b[0m',csv
                except Exception:
                    print '\x1b[1;32m✓ \x1b[0m', csv
    
    def test_good_files(visual=False):
        good_files = glob.glob("../data/csv/*.*")
        good_files.extend(glob.glob("../data/csv/warns/*.*"))
    
        for csv in good_files:
            if visual:
                try:
                    ds = mapnik2.Datasource(type='csv',file=csv,quiet=True)
                    print '\x1b[1;32m✓ \x1b[0m', csv
                except Exception:
                    print '\x1b[33mfailed\x1b[0m',csv


if __name__ == "__main__":
    setup()
    [eval(run)(visual=True) for run in dir() if 'test_' in run]
