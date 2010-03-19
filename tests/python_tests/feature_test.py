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
        for v in (1, True, 1.4):
            test_val(v)

        raise Todo("Support setting unicode and string properties")

        for v in ("foo", u"foó"):
            test_val(v)
