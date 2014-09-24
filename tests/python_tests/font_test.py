#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all
import mapnik

# Tests that exercise fonts. 

# Trac Ticket #31
# Todo: Add logic to use this TextSymbolizer in a rendering
#@raises(UserWarning)
#def test_invalid_font():
#    ts = mapnik.TextSymbolizer('Name', 'Invalid Font Name', int(8), mapnik.Color('black'))

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
