#encoding: utf8
import itertools
import unittest

class MemoryDatasource(unittest.TestCase):
    ids = itertools.count(0)

    def makeOne(self, *args, **kw):
        from mapnik import MemoryDatasource
        return MemoryDatasource(*args, **kw)

    def makeFeature(self, wkt, **properties):
        from mapnik import Feature
        f = Feature(self.ids.next())
        f.add_geometries_from_wkt(wkt)
        for k,v in properties.iteritems():
            f[k] = v
        return f

    def test_default_constructor(self):
        f = self.makeOne()
        self.failUnless(f is not None)

    def test_add_feature(self):
        md = self.makeOne()
        self.failUnlessEqual(md.num_features(), 0)
        md.add_feature(self.makeFeature('Point(2 3)', foo='bar'))
        self.failUnlessEqual(md.num_features(), 1)

        from mapnik import Coord
        retrieved = md.features_at_point(Coord(2,3)).features
        self.failUnlessEqual(len(retrieved), 1)
        f = retrieved[0]
        self.failUnlessEqual(f['foo'], 'bar')

        retrieved = md.features_at_point(Coord(20,30)).features
        self.failUnlessEqual(len(retrieved), 0)

    def test_statistics(self):
        md = self.makeOne()
        self.failUnlessEqual(md.num_features(), 0)
        md.add_feature(self.makeFeature('Point(2 3)', a=1))
        self.failUnlessEqual(md.num_features(), 1)
        stats = md.statistics()
        self.failUnless(stats.has_key('a'))
        self.failUnlessEqual(stats['a']['min'], 1)
        self.failUnlessEqual(stats['a']['max'], 1)
        self.failUnlessEqual(stats['a']['mean'], 1)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
