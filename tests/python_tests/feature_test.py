#!/usr/bin/env python
# -*- coding: utf-8 -*-
import itertools
import unittest
from nose.tools import *

import mapnik
from binascii import unhexlify

def test_default_constructor():
    f = mapnik.Feature(mapnik.Context(),1)
    eq_(f is not None,True)

def test_python_extended_constructor():
    context = mapnik.Context()
    context.push('foo')
    context.push('foo')
    f = mapnik.Feature(context,1)
    wkt = 'POLYGON ((35 10, 10 20, 15 40, 45 45, 35 10),(20 30, 35 35, 30 20, 20 30))'
    f.add_geometries_from_wkt(wkt)
    f['foo'] = 'bar'
    eq_(f['foo'], 'bar')
    eq_(f.envelope(),mapnik.Box2d(10.0,10.0,45.0,45.0))
    # reset
    f['foo'] = u"avión"
    eq_(f['foo'], u"avión")
    f['foo'] = 1.4
    eq_(f['foo'], 1.4)
    f['foo'] = True
    eq_(f['foo'], True)

def test_add_geom_wkb():
# POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))
    wkb = '010300000001000000050000000000000000003e4000000000000024400000000000002440000000000000344000000000000034400000000000004440000000000000444000000000000044400000000000003e400000000000002440'
    context = mapnik.Context()
    f = mapnik.Feature(context,1)
    eq_(len(f.geometries()), 0)
    f.add_geometries_from_wkb(unhexlify(wkb))
    eq_(len(f.geometries()), 1)
    e = mapnik.Box2d()
    eq_(e.valid(), False)
    for g in f.geometries():
        if not e.valid():
            e = g.envelope()
        else:
            e +=g.envelope()

    eq_(e, f.envelope())

def test_feature_expression_evaluation():
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,1)
    f['name'] = 'a'
    eq_(f['name'],u'a')
    expr = mapnik.Expression("[name]='a'")
    evaluated = expr.evaluate(f)
    eq_(evaluated,True)
    num_attributes = len(f)
    eq_(num_attributes,1)
    eq_(f.id(),1)

# https://github.com/mapnik/mapnik/issues/933
def test_feature_expression_evaluation_missing_attr():
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,1)
    f['name'] = u'a'
    eq_(f['name'],u'a')
    expr = mapnik.Expression("[fielddoesnotexist]='a'")
    eq_(f.has_key('fielddoesnotexist'),False)
    try:
        evaluated = expr.evaluate(f)
    except Exception, e:
        eq_("Key does not exist" in str(e),True)
    num_attributes = len(f)
    eq_(num_attributes,1)
    eq_(f.id(),1)

# https://github.com/mapnik/mapnik/issues/934
def test_feature_expression_evaluation_attr_with_spaces():
    context = mapnik.Context()
    context.push('name with space')
    f = mapnik.Feature(context,1)
    f['name with space'] = u'a'
    eq_(f['name with space'],u'a')
    expr = mapnik.Expression("[name with space]='a'")
    eq_(str(expr),"([name with space]='a')")
    eq_(expr.evaluate(f),True)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
