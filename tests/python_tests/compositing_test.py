#encoding: utf8

from nose.tools import *
import os,sys
from utilities import execution_path
from utilities import Todo
import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def validate_pixels_are_demultiplied(image):
    bad_pixels = []
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            r = pixel & 0xff
            g = (pixel >> 8) & 0xff
            b = (pixel >> 16) & 0xff
            a = (pixel >> 24) & 0xff
            is_valid = (r >=0 and r < 256) and \
                    (g >=0 and g < 256) and \
                    (b >=0 and b < 256) and \
                    (a >=0 and a < 256)
            if not is_valid:
                bad_pixels.append("rgba(%s,%s,%s,%s) at %s,%s" % (r,g,b,a,x,y))
    num_bad = len(bad_pixels)
    return (num_bad == 0,num_bad)

def validate_pixels_are_premultiplied(image):
    bad_pixels = []
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            red = pixel & 0xff
            green = (pixel >> 8) & 0xff
            blue = (pixel >> 16) & 0xff
            alpha = (pixel >> 24) & 0xff
            a2 = alpha
            if a2 == 0:
                a2 = 1
            else:
                a2 = a2*256
            is_valid = (red >=0 and red/a2 < 256) and \
                    (green >=0 and red/a2 < 256) and \
                    (blue >=0 and red/a2 < 256) and \
                    (alpha >=0 and alpha < 256)
            if not is_valid:
                import pdb;pdb.set_trace()
                bad_pixels.append("rgba(%s,%s,%s,%s) at %s,%s" % (red,green,blue,alpha,x,y))
    num_bad = len(bad_pixels)
    return (num_bad == 0,bad_pixels)


def test_compare_images():
    b = mapnik.Image.open('./images/support/b.png')
    b.premultiply()
    for name in mapnik.CompositeOp.names:
        a = mapnik.Image.open('./images/support/a.png')
        a.premultiply()
        a.composite(b,getattr(mapnik.CompositeOp,name))
        actual = '/tmp/mapnik-comp-op-test-' + name + '.png'
        expected = 'images/composited/' + name + '.png'
        valid = validate_pixels_are_premultiplied(a)
        if not valid[0]:
            print '%s not validly pre-:\n\t%s pixels (%s)' % (name,len(valid[1]),valid[1][0])
        a.demultiply()
        valid = validate_pixels_are_demultiplied(a)
        if not valid[0]:
            print '%s not validly de-:\n\t%s pixels (%s)' % (name,len(valid[1]),valid[1][0])
        a.save(actual)
        expected_im = mapnik.Image.open(expected)
        # compare them
        eq_(a.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))
    b.demultiply()
    # b will be slightly modified by pre and then de multiplication rounding errors
    # TODO - write test to ensure the image is 99% the same.
    #expected_b = mapnik.Image.open('./images/support/b.png')
    #b.save('/tmp/mapnik-comp-op-test-original-mask.png')
    #eq_(b.tostring(),expected_b.tostring(), '/tmp/mapnik-comp-op-test-original-mask.png is no longer equivalent to original mask: ./images/support/b.png')


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
