#encoding: utf8
import itertools
import unittest
from nose.tools import *

import mapnik2
from binascii import unhexlify
from utilities import Todo

class FeatureTest(unittest.TestCase):
    def makeOne(self, *args, **kw):
        return mapnik2.Feature(*args, **kw)
    
    def test_default_constructor(self):
        f = self.makeOne(1)
        self.failUnless(f is not None)

    def test_python_extended_constructor(self):
        f = self.makeOne(1, 'POLYGON ((35 10, 10 20, 15 40, 45 45, 35 10),(20 30, 35 35, 30 20, 20 30))', foo="bar")
        self.failUnlessEqual(f['foo'], 'bar')
        self.failUnlessEqual(f.envelope(),mapnik2.Box2d(10.0,10.0,45.0,45.0))
    
    def test_set_get_properties(self):
        f = self.makeOne(1)
        counter = itertools.count(0)
        def test_val(expected):
            key = 'prop%d'%counter.next()
            try:
                f[key] = expected
            except TypeError:
                self.fail("%r (%s)"%(expected, type(expected)))
            self.failUnlessEqual(f[key], expected)
        for v in (1, True, 1.4, "foo", u"avión"):
            test_val(v)

    def test_add_wkt_geometry(self):
        def add_geom_wkt(wkt):
            f = self.makeOne(1)
            self.failUnlessEqual(len(f.geometries()), 0)
            f.add_geometries_from_wkt(wkt)
            self.failUnlessEqual(len(f.geometries()), 3)
            e = mapnik2.Box2d()
            self.failUnlessEqual(e.valid(), False)
            for g in f.geometries():
                if not e.valid():
                    e = g.envelope()
                else:
                    e +=g.envelope()
                    
            self.failUnlessEqual(e, f.envelope())

        def add_geom_wkb(wkb):
            f = self.makeOne(1)
            self.failUnlessEqual(len(f.geometries()), 0)
            f.add_geometries_from_wkb(unhexlify(wkb))
            self.failUnlessEqual(len(f.geometries()), 1)
            e = mapnik2.Box2d()
            self.failUnlessEqual(e.valid(), False)
            for g in f.geometries():
                if not e.valid():
                    e = g.envelope()
                else:
                    e +=g.envelope()
                    
            self.failUnlessEqual(e, f.envelope())

        def run() :
            add_geom_wkt('GEOMETRYCOLLECTION(POINT(4 6),LINESTRING(4 6,7 10),POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10)))')
            add_geom_wkb('010300000001000000050000000000000000003e4000000000000024400000000000002440000000000000344000000000000034400000000000004440000000000000444000000000000044400000000000003e400000000000002440') # POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))
        run()


def test_feature_expression_evaluation():
    f = mapnik2.Feature(1)
    f['name'] = 'a'
    expr = mapnik2.Expression("[name]='a'")
    evaluated = expr.evaluate(f)
    eq_(evaluated,True)
    num_attributes = len(f)
    eq_(num_attributes,1)
    eq_(f.id(),1)

# https://github.com/mapnik/mapnik/issues/933
def test_feature_expression_evaluation_missing_attr():
    f = mapnik2.Feature(1)
    f['name'] = 'a'
    expr = mapnik2.Expression("[fielddoesnotexist]='a'")
    evaluated = expr.evaluate(f)
    eq_(evaluated,True)
    num_attributes = len(f)
    eq_(num_attributes,1)
    eq_(f.id(),1)

# https://github.com/mapnik/mapnik/issues/934
def test_feature_expression_evaluation_attr_with_spaces():
    f = mapnik2.Feature(1)
    f['name with space'] = 'a'
    expr = mapnik2.Expression("[name with space]='a'")
    eq_(expr.evaluate(f),True)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
