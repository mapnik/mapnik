#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
import cairo
import sys
import os.path
from compare import compare, summary

dirname = os.path.dirname(sys.argv[0])
files = [
    ("list", 800, 600, 400, 300, 250, 200, 150, 100),
    ("simple", 800, 600, 400, 300, 250, 200, 150, 100),
    ("simple-E", 500),
    ("simple-NE", 500),
    ("simple-NW", 500),
    ("simple-N", 500),
    ("simple-SE", 500),
    ("simple-SW", 500),
    ("simple-S", 500),
    ("simple-W", 500),
    ("formating-1", 500),
    ("formating-2", 500),
    ("formating-3", 500),
    ("formating-4", 500),
    ("shieldsymbolizer-1", 490, 495, 497, 498, 499, 500, 501, 502, 505, 510),
    ("expressionformat", 500)]

def render(filename, width):
    width = int(width)
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
    
if len(sys.argv) == 2:
    files = [(sys.argv[1], 500)]
elif len(sys.argv) > 2:
    files = [sys.argv[1:]]
    
for f in files:
    for width in f[1:]:
        m = render(f[0], width)
    mapnik.save_map(m, "%s-out.xml" % f[0])

summary()

