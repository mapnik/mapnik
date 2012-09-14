#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_loading_fontset_from_map():
    m = mapnik.Map(256,256)
    mapnik.load_map(m,'../data/good_maps/fontset.xml',True)
    fs = m.find_fontset('book-fonts')
    eq_(len(fs.names),2)
    eq_(list(fs.names),['DejaVu Sans Book','DejaVu Sans Oblique'])

def test_loading_fontset_from_python():
    m = mapnik.Map(256,256)
    fset = mapnik.FontSet('foo')
    fset.add_face_name('Comic Sans')
    fset.add_face_name('Papyrus')
    eq_(fset.name,'foo')
    fset.name = 'my-set'
    eq_(fset.name,'my-set')
    m.append_fontset('my-set', fset)
    sty = mapnik.Style()
    rule = mapnik.Rule()
    tsym = mapnik.TextSymbolizer()
    eq_(tsym.fontset,None)
    tsym.fontset = fset
    rule.symbols.append(tsym)
    sty.rules.append(rule)
    m.append_style('Style',sty)
    serialized_map = mapnik.save_map_to_string(m)
    eq_('fontset-name="my-set"' in serialized_map,True)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
