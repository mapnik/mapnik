# -*- coding: utf-8 -*-

#import math, operator
import Image
import sys

COMPUTE_THRESHOLD = 16

errors = []
passed = 0

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
def compare(fn1, fn2):
    global errors
    global passed
    im1 = Image.open(fn1)
    try:
        im2 = Image.open(fn2)
    except IOError:
        errors.append((fn1, None, fn2))
        return -1
    diff = 0
    pixels = im1.size[0] * im1.size[1]
    delta_pixels = im2.size[0] * im2.size[1]  - pixels
    if delta_pixels != 0:
        errors.append((fn1, delta_pixels, fn2))
        return delta_pixels
    im1 = im1.getdata()
    im2 = im2.getdata()
    for i in range(3, pixels - 1, 3):
        if(compare_pixels(im1[i], im2[i])):
            diff = diff + 1
    if diff != 0:
        errors.append((fn1, diff, fn2))
    passed += 1
    return diff

def summary():
    global errors
    global passed
    print "-"*80
    print "Visual text rendering summary:",
    if len(errors) != 0:
        for error in errors:
            if (error[1] is None):
                print "Could not verify %s: No reference image found!" % error[0]
            else:
                print "%s failed: %d different pixels \n\t%s (expected)" % error
        sys.exit(1)
    else:
        print 'All %s tests passed: \x1b[1;32m✓ \x1b[0m' % passed
        sys.exit(0)
