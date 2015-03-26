# -*- coding: utf-8 -*-

import mapnik
import os
from unittest import TestCase

try:
    import json
except ImportError:
    import simplejson as json

# compare two images and return number of different pixels
def compare(actual, expected, alpha=True):
    im1 = mapnik.Image.open(actual)
    im2 = mapnik.Image.open(expected)
    pixels = im1.width() * im1.height()
    delta_pixels = (im2.width() * im2.height()) - pixels
    if delta_pixels != 0:
        return delta_pixels
    if os.name == 'nt':
        return im1.compare(im2, 3, alpha)
    else:
        return im1.compare(im2, 0, alpha)

def compare_grids(actual, expected, threshold=0, alpha=True):
    global errors
    global passed
    im1 = json.loads(open(actual).read())
    im2 = json.loads(open(expected).read())
    # TODO - real diffing
    if not im1['data'] == im2['data']:
        return 99999999
    if not im1['keys'] == im2['keys']:
        return 99999999
    grid1 = im1['grid']
    grid2 = im2['grid']
    try:
        assertSequenceEqual(grid1, grid2)
        return 0
    except:
        # dimensions must be exact
        width1 = len(grid1[0])
        width2 = len(grid2[0])
        if not width1 == width2:
            return 99999999
        height1 = len(grid1)
        height2 = len(grid2)
        if not height1 == height2:
            return 99999999
        diff = 0;
        for y in range(0,height1-1):
            row1 = grid1[y]
            row2 = grid2[y]
            if row1 == row2:
                continue;
            #width = min(len(row1),len(row2))
            for w in range(0,width1):
                if row1[w] != row2[w]:
                    diff += 1
        return diff
