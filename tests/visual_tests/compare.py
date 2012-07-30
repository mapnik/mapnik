# -*- coding: utf-8 -*-

import sys
import mapnik

COMPUTE_THRESHOLD = 16

errors = []
passed = 0

# returns true if pixels are not identical
def compare_pixels(pixel1, pixel2):
    if pixel1 == pixel2:
        return False
    r_diff = abs((pixel1 & 0xff) - (pixel2 & 0xff))
    g_diff = abs(((pixel1 >> 8) & 0xff) - ((pixel2 >> 8) & 0xff))
    b_diff = abs(((pixel1 >> 16) & 0xff)- ((pixel2 >> 16) & 0xff))
    a_diff = abs(((pixel1 >> 24) & 0xff) - ((pixel2 >> 24) & 0xff))
    if(r_diff > COMPUTE_THRESHOLD or
       g_diff > COMPUTE_THRESHOLD or
       b_diff > COMPUTE_THRESHOLD or
       a_diff > COMPUTE_THRESHOLD):
        return True
    else:
        return False

# compare two images and return number of different pixels
def compare(actual, expected):
    global errors
    global passed
    im1 = mapnik.Image.open(actual)
    try:
        im2 = mapnik.Image.open(expected)
    except RuntimeError:
        errors.append((None, actual, expected))
        return -1
    diff = 0
    pixels = im1.width() * im1.height()
    delta_pixels = (im2.width() * im2.height()) - pixels
    if delta_pixels != 0:
        errors.append((delta_pixels, actual, expected))
        return delta_pixels
    for x in range(0,im1.width(),2):
        for y in range(0,im1.height(),2):
            if compare_pixels(im1.get_pixel(x,y),im2.get_pixel(x,y)):
                diff += 1
    if diff != 0:
        errors.append((diff, actual, expected))
    passed += 1
    return diff

def summary():
    global errors
    global passed
    print "-"*80
    print "Visual text rendering summary:",
    if len(errors) != 0:
        for error in errors:
            if (error[0] is None):
                print "Could not verify %s: No reference image found!" % error[1]
            else:
                print "Failed: %d different pixels:\n\t%s (actual)\n\t%s (expected)" % error
        sys.exit(1)
    else:
        print 'All %s tests passed: \x1b[1;32m✓ \x1b[0m' % passed
        sys.exit(0)
