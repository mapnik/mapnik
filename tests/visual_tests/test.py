#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
import cairo
import sys
import os.path
import math, operator
import Image

dirname = os.path.dirname(sys.argv[0])

widths = [ 800, 600, 400, 300, 250, 200, 150, 100]
filenames = ["list", "simple"]
filenames_one_width = ["simple-E", "simple-NE", "simple-NW", "simple-N",
    "simple-SE", "simple-SW", "simple-S", "simple-W",
    "formating-1", "formating-2", "formating-3", "formating-4",
    "shieldsymbolizer-1", "expressionformat"]

COMPUTE_THRESHOLD = 0

# returns true if pixels are not identical
def compare_pixels(pixel1, pixel2):
    r_diff = abs(pixel1[0] - pixel2[0])
    g_diff = abs(pixel1[1] - pixel2[1])
    b_diff = abs(pixel1[2] - pixel2[2])
    if(r_diff > COMPUTE_THRESHOLD or g_diff > COMPUTE_THRESHOLD or b_diff > COMPUTE_THRESHOLD):
        return True
    else:
        return False

# compare tow images and return number of different pixels
def compare(im1, im2):
    diff = 0
    pixels = im1.size[0] * im1.size[1]
    im1 = im1.getdata()
    im2 = im2.getdata()
    for i in range(3, pixels - 1, 3):
        if(compare_pixels(im1[i], im2[i])):
            diff = diff + 1
    return diff

def render(filename, width):
    m = mapnik.Map(width, 100)
    mapnik.load_map(m, os.path.join(dirname, "%s.xml" % filename), False)
    bbox = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)
    m.zoom_to_box(bbox)
    mapnik.render_to_file(m, '%s-%d-agg.png' % (filename, width))
    im1 = Image.open('%s-%d-reference.png' % (filename, width))
    im2 = Image.open('%s-%d-agg.png' % (filename, width))
    diff = compare(im1, im2)
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

