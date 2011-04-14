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
        from mapnik2 import Geometry2d
        f = self.makeOne(1, Geometry2d.from_wkt('Point(3 6)'), foo="bar")
        self.failUnlessEqual(f['foo'], 'bar')
        env = f.get_geometry(0).envelope()
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
        from mapnik2 import Geometry2d

        def add_it(geometry):
            f = self.makeOne(1)
            self.failUnlessEqual(len(f.geometries), 0)
            f.add_geometry(geometry)
            self.failUnlessEqual(len(f.geometries), 1)
            env = f.get_geometry(0).envelope()
            self.failUnlessEqual(env.minx, 3)
            self.failUnlessEqual(env.minx, env.maxx)
            self.failUnlessEqual(env.miny, 6)
            self.failUnlessEqual(env.miny, env.maxy)
        
        add_it(Geometry2d.from_wkt('Point(3 6)'))
