#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_arbitrary_parameters_attached_to_map():
    m = mapnik.Map(256,256)
    mapnik.load_map(m,'../data/good_maps/extra_arbitary_map_parameters.xml')
    eq_(len(m.parameters),5)
    eq_(m.parameters['key'],'value2')
    eq_(m.parameters['key3'],'value3')
    eq_(m.parameters['unicode'],u'iván')
    eq_(m.parameters['integer'],10)
    eq_(m.parameters['decimal'],.999)
    m2 = mapnik.Map(256,256)
    for k,v in m.parameters:
        m2.parameters.append(mapnik.Parameter(k,v))
    eq_(len(m2.parameters),5)
    eq_(m2.parameters['key'],'value2')
    eq_(m2.parameters['key3'],'value3')
    eq_(m2.parameters['unicode'],u'iván')
    eq_(m2.parameters['integer'],10)
    eq_(m2.parameters['decimal'],.999)
    map_string = mapnik.save_map_to_string(m)
    m3 = mapnik.Map(256,256)
    mapnik.load_map_from_string(m3,map_string)
    eq_(len(m3.parameters),5)
    eq_(m3.parameters['key'],'value2')
    eq_(m3.parameters['key3'],'value3')
    eq_(m3.parameters['unicode'],u'iván')
    eq_(m3.parameters['integer'],10)
    eq_(m3.parameters['decimal'],.999)


def test_serializing_arbitrary_parameters():
    m = mapnik.Map(256,256)
    m.parameters.append(mapnik.Parameter('width',m.width))
    m.parameters.append(mapnik.Parameter('height',m.height))

    m2 = mapnik.Map(1,1)
    mapnik.load_map_from_string(m2,mapnik.save_map_to_string(m))
    eq_(m2.parameters['width'],m.width)
    eq_(m2.parameters['height'],m.height)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
