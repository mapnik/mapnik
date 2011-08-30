#encoding: utf8
import itertools
import unittest

from utilities import Todo

class MemoryDatasource(unittest.TestCase):
    ids = itertools.count(0)

    def makeOne(self, *args, **kw):
        from mapnik2 import MemoryDatasource
        return MemoryDatasource(*args, **kw)

    def makeFeature(self, geom, **properties):
        from mapnik2 import Feature
        f = Feature(self.ids.next())
        f.add_geometry(geom)
        for k,v in properties.iteritems():
            f[k] = v
        return f

    def test_default_constructor(self):
        f = self.makeOne()
        self.failUnless(f is not None)

    def test_add_feature(self):
        md = self.makeOne()
        self.failUnlessEqual(md.num_features(), 0)
        from mapnik2 import Geometry2d
        md.add_feature(self.makeFeature(Geometry2d.from_wkt('Point(2 3)'), foo='bar'))
        self.failUnlessEqual(md.num_features(), 1)

        from mapnik2 import Coord
        retrieved = md.features_at_point(Coord(2,3)).features
        self.failUnlessEqual(len(retrieved), 1)
        f = retrieved[0]
        self.failUnlessEqual(f['foo'], 'bar')

        retrieved = md.features_at_point(Coord(20,30)).features
        self.failUnlessEqual(len(retrieved), 0)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
