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

def fail(actual,expected,message):
    global errors
    errors.append((message, actual, expected))

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

def summary(generate=False):
    global errors
    global passed
    
    if len(errors) != 0:
        msg = "Visual text rendering: %s failures" % len(errors)
        print "-"*len(msg)
        print msg
        print "-"*len(msg)
        for idx,error in enumerate(errors):
            if error[0] is None:
                if generate:
                    actual = open(error[1],'r').read()
                    open(error[2],'wb').write(actual)
                    print "Generating reference image: '%s'" % error[2]
                    continue
                else:
                    print "Could not verify %s: No reference image found!" % error[1]
            elif isinstance(error[0],int):
                print str(idx+1) + ") \x1b[34m%s different pixels\x1b[0m:\n\t%s (\x1b[31mactual\x1b[0m)\n\t%s (\x1b[32mexpected\x1b[0m)" % error
            elif isinstance(error[0],str):
                print str(idx+1) + ") \x1b[31mfailure to run test:\x1b[0m %s" % error[0]
        sys.exit(1)
    else:
        msg = 'All %s visual tests passed: \x1b[1;32m✓ \x1b[0m' % passed
        print "-"*len(msg)
        print msg
        print "-"*len(msg)
        sys.exit(0)
