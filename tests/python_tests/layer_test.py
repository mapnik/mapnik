#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
import mapnik

# Map initialization
def test_layer_init():
    l = mapnik.Layer('test')
    eq_(l.name,'test')
    eq_(l.srs,'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')
    eq_(l.envelope(),mapnik.Box2d())
    eq_(l.clear_label_cache,False)
    eq_(l.cache_features,False)
    eq_(l.visible(1),True)
    eq_(l.active,True)
    eq_(l.datasource,None)
    eq_(l.queryable,False)
    eq_(l.minzoom,0.0)
    eq_(l.maxzoom > 1e+6,True)
    eq_(l.group_by,"")
    eq_(l.maximum_extent,None)
    eq_(l.buffer_size,0.0)
    eq_(len(l.styles),0)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
