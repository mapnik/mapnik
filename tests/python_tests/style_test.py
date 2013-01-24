#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from nose.tools import *
import mapnik

def test_style_init():
   s = mapnik.Style()
   eq_(s.filter_mode,mapnik.filter_mode.ALL)
   eq_(len(s.rules),0)
   eq_(s.opacity,1)
   eq_(s.comp_op,None)
   eq_(s.image_filters,"")

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
