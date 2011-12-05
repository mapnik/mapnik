#!/usr/bin/env python

from nose.tools import *

import mapnik

# Tests that exercise fonts. 

# Trac Ticket #31
# Todo: Add logic to use this TextSymbolizer in a rendering
#@raises(UserWarning)
#def test_invalid_font():
#    ts = mapnik.TextSymbolizer('Name', 'Invalid Font Name', int(8), mapnik.Color('black'))

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
