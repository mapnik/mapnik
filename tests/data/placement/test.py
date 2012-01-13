#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
import cairo
import sys
import os.path

dirname = os.path.dirname(sys.argv[0])

widths = [ 800, 600, 400, 300, 250, 200, 150, 100]
filenames = ["list", "simple"]
filenames_one_width = ["simple-E", "simple-NE", "simple-NW", "simple-N",
    "simple-SE", "simple-SW", "simple-S", "simple-W"]
    
def render(filename, width):
    print "Rendering style \"%s\" with width %d" % (filename, width)
    m = mapnik.Map(width, 100)
    mapnik.load_map(m, os.path.join(dirname, "%s.xml" % filename), False)
    bbox = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)
    m.zoom_to_box(bbox)
    mapnik.render_to_file(m, '%s-%d-agg.png' % (filename, width))
    return m
    

for filename in filenames:
    for width in widths:
        m = render(filename, width)
    mapnik.save_map(m, "%s-out.xml" % filename)

for filename in filenames_one_width:
    render(filename, 500)