# -*- coding: utf-8 -*-

import os
import sys
import mapnik

try:
    import json
except ImportError:
    import simplejson as json

COMPUTE_THRESHOLD = 16

# testcase images are generated on OS X
# so they should exactly match
if os.uname()[0] == 'Darwin':
    COMPUTE_THRESHOLD = 2

# returns true if pixels are not identical
def compare_pixels(pixel1, pixel2, alpha=True):
    if pixel1 == pixel2:
        return False
    r_diff = abs((pixel1 & 0xff) - (pixel2 & 0xff))
    g_diff = abs(((pixel1 >> 8) & 0xff) - ((pixel2 >> 8) & 0xff))
    b_diff = abs(((pixel1 >> 16) & 0xff)- ((pixel2 >> 16) & 0xff))
    if alpha:
        a_diff = abs(((pixel1 >> 24) & 0xff) - ((pixel2 >> 24) & 0xff))
        if(r_diff > COMPUTE_THRESHOLD or
           g_diff > COMPUTE_THRESHOLD or
           b_diff > COMPUTE_THRESHOLD or
           a_diff > COMPUTE_THRESHOLD):
            return True
    else:
        if(r_diff > COMPUTE_THRESHOLD or
           g_diff > COMPUTE_THRESHOLD or
           b_diff > COMPUTE_THRESHOLD):
            return True
    return False

# compare two images and return number of different pixels
def compare(actual, expected, alpha=True):
    im1 = mapnik.Image.open(actual)
    try:
        im2 = mapnik.Image.open(expected)
    except RuntimeError:
        return 99999990
    diff = 0
    pixels = im1.width() * im1.height()
    delta_pixels = (im2.width() * im2.height()) - pixels
    if delta_pixels != 0:
        return delta_pixels
    for x in range(0,im1.width(),2):
        for y in range(0,im1.height(),2):
            if compare_pixels(im1.get_pixel(x,y),im2.get_pixel(x,y),alpha=alpha):
                diff += 1
    return diff

def compare_grids(actual, expected, threshold=0, alpha=True):
    global errors
    global passed
    im1 = json.loads(open(actual).read())
    try:
        im2 = json.loads(open(expected).read())
    except RuntimeError:
        return 9999990
    equal = (im1 == im2)
    # TODO - real diffing
    if not equal:
        return 99999999
    return 0
