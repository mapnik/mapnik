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

    def test_add_wkb_geometry_(self):
        try:
            from shapely.geometry import Point
        except ImportError:
            raise Todo("Make this test not dependant on shapely")

        f = self.makeOne(1)
        self.failUnlessEqual(f.num_geometries(), 0)
        f.add_geometry(Point(3,6).wkb)
        self.failUnlessEqual(f.num_geometries(), 1)
        geom = f.get_geometry(0)
        env = geom.envelope()
        self.failUnlessEqual(env.minx, 3)
        self.failUnlessEqual(env.minx, env.maxx)
        self.failUnlessEqual(env.miny, 6)
        self.failUnlessEqual(env.miny, env.maxy)
