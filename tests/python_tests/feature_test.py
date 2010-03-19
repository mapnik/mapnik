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
        try:
            from shapely.geometry import Point
        except ImportError:
            raise Todo("Make this test not dependant on shapely")

        f = self.makeOne(1, Point(3,6), foo="bar")
        self.failUnlessEqual(f['foo'], 'bar')
        env = f.geometry.envelope()
        self.failUnlessEqual(env.minx, 3)
        self.failUnlessEqual(env.miny, 6)

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


    def test_add_wkb_geometry(self):
        try:
            from shapely.geometry import Point
        except ImportError:
            raise Todo("Make this test not dependant on shapely")

        def add_it(geometry):
            f = self.makeOne(1)
            self.failUnlessEqual(len(f.geometries), 0)
            f.add_geometry(geometry)
            self.failUnlessEqual(len(f.geometries), 1)
            env = f.geometry.envelope()
            self.failUnlessEqual(env.minx, 3)
            self.failUnlessEqual(env.minx, env.maxx)
            self.failUnlessEqual(env.miny, 6)
            self.failUnlessEqual(env.miny, env.maxy)

        geometries = (Point(3,6), 'POINT(3 6)', Point(3,6).wkb)
        for geom in geometries:
            add_it(geom)
