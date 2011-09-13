#encoding: utf8
import itertools
import unittest

from utilities import Todo

class FeatureTest(unittest.TestCase):
    def makeOne(self, *args, **kw):
        from mapnik2 import Feature
        return Feature(*args, **kw)
    
    def test_default_constructor(self):
        f = self.makeOne(1)
        self.failUnless(f is not None)

    def test_python_extended_constructor(self):
        from mapnik2 import Box2d
        f = self.makeOne(1, 'POLYGON ((35 10, 10 20, 15 40, 45 45, 35 10),(20 30, 35 35, 30 20, 20 30))', foo="bar")
        self.failUnlessEqual(f['foo'], 'bar')
        self.failUnlessEqual(f.envelope(),Box2d(10.0,10.0,45.0,45.0))
        area = 0.0
        for g in f.geometries():
            area += g.area()
        self.failUnlessEqual(area,-450.0)
    
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
        from mapnik2 import Geometry2d
        from mapnik2 import Box2d
        def add_geom(wkt):
            f = self.makeOne(1)
            self.failUnlessEqual(len(f.geometries()), 0)
            f.add_geometries_from_wkt(wkt)
            self.failUnlessEqual(len(f.geometries()), 3)
            e = Box2d()
            self.failUnlessEqual(e.valid(), False)
            for g in f.geometries():
                if not e.valid():
                    e = g.envelope()
                else:
                    e +=g.envelope()
                    
            self.failUnlessEqual(e, f.envelope())
            
        add_geom('GEOMETRYCOLLECTION(POINT(4 6),LINESTRING(4 6,7 10),POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10)))')

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
