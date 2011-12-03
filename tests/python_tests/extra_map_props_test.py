#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

def test_non_member_known_attributes():
    m = mapnik.Map(256,256)
    mapnik.load_map(m,'../data/good_maps/extra_known_map_attributes.xml')
    attr = m.extra_attributes
    eq_(len(attr),2)
    eq_(attr['font-directory'],'.')
    eq_(attr['minimum-version'],'0.0.0')

def test_arbitrary_parameters_attached_to_map():
    m = mapnik.Map(256,256)
    mapnik.load_map(m,'../data/good_maps/extra_arbitary_map_parameters.xml')
    attr = m.extra_attributes
    eq_(len(attr),0)

    eq_(len(m.params),3)
    eq_(m.params['key'],'value2')
    eq_(m.params['key3'],'value3')
    eq_(m.params['unicode'],u'iván')


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
