#!/usr/bin/env python

import os
from nose.tools import *
from utilities import execution_path, run_all

import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_introspect_symbolizers():
    # create a symbolizer
    p = mapnik.PointSymbolizer()
    p.file = "../data/images/dummy.png"
    p.allow_overlap = True
    p.opacity = 0.5

    eq_(p.allow_overlap, True)
    eq_(p.opacity, 0.5)
    eq_(p.filename,'../data/images/dummy.png')

    # make sure the defaults
    # are what we think they are
    eq_(p.allow_overlap, True)
    eq_(p.opacity,0.5)
    eq_(p.filename,'../data/images/dummy.png')

    # contruct objects to hold it
    r = mapnik.Rule()
    r.symbols.append(p)
    s = mapnik.Style()
    s.rules.append(r)
    m = mapnik.Map(0,0)
    m.append_style('s',s)

    # try to figure out what is
    # in the map and make sure
    # style is there and the same

    s2 = m.find_style('s')
    rules = s2.rules
    eq_(len(rules),1)
    r2 = rules[0]
    syms = r2.symbols
    eq_(len(syms),1)

    ## TODO here, we can do...
    sym = syms[0]
    p2 = sym.extract()
    assert isinstance(p2,mapnik.PointSymbolizer)

    eq_(p2.allow_overlap, True)
    eq_(p2.opacity, 0.5)
    eq_(p2.filename,'../data/images/dummy.png')

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
