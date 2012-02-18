#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
import cairo
import sys
import os.path
from compare import compare, summary

dirname = os.path.dirname(sys.argv[0])

widths = [ 800, 600, 400, 300, 250, 200, 150, 100]
filenames = ["list", "simple"]
filenames_one_width = ["simple-E", "simple-NE", "simple-NW", "simple-N",
    "simple-SE", "simple-SW", "simple-S", "simple-W",
    "formating-1", "formating-2", "formating-3", "formating-4",
    "shieldsymbolizer-1", "expressionformat"]

def render(filename, width):
    m = mapnik.Map(width, 100)
    mapnik.load_map(m, os.path.join(dirname, "%s.xml" % filename), False)
    bbox = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)
    m.zoom_to_box(bbox)
    basefn = '%s-%d' % (filename, width)
    mapnik.render_to_file(m, basefn+'-agg.png')
    diff = compare(basefn + '-agg.png', basefn + '-reference.png')
    if diff == 0:
        rms = 'ok'
    else:
        rms = 'error: %u different pixels' % diff

    print "Rendering style \"%s\" with width %d ... %s" % (filename, width, rms)
    return m
    
if len(sys.argv) > 1:
    filenames = []
    filenames_one_width = sys.argv[1:]
    
for filename in filenames:
    for width in widths:
        m = render(filename, width)
    mapnik.save_map(m, "%s-out.xml" % filename)

for filename in filenames_one_width:
    m = render(filename, 500)
    mapnik.save_map(m, "%s-out.xml" % filename)

summary()

