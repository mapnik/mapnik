#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from nose.tools import *
from utilities import execution_path, run_all
import mapnik

def test_style_init():
   s = mapnik.Style()
   eq_(s.filter_mode,mapnik.filter_mode.ALL)
   eq_(len(s.rules),0)
   eq_(s.opacity,1)
   eq_(s.comp_op,None)
   eq_(s.image_filters,"")
   eq_(s.image_filters_inflate,False)

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
