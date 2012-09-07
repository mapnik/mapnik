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

def is_pre(color,alpha):
    return (color*255.0/alpha) <= 255

def debug_image(image,step=2):
    for x in range(0,image.width(),step):
        for y in range(0,image.height(),step):
            pixel = image.get_pixel(x,y)
            alpha = (pixel >> 24) & 0xff
            red = pixel & 0xff
            green = (pixel >> 8) & 0xff
            blue = (pixel >> 16) & 0xff
            print "rgba(%s,%s,%s,%s) at %s,%s" % (red,green,blue,alpha,x,y)

# note: it is impossible to know for all pixel colors
# we can only detect likely cases of non premultiplied colors
def validate_pixels_are_not_premultiplied(image):
    over_alpha = False
    transparent = True
    fully_opaque = True
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            alpha = (pixel >> 24) & 0xff
            if alpha > 0:
                transparent = False
                if alpha < 255:
                    fully_opaque = False
                red = pixel & 0xff
                green = (pixel >> 8) & 0xff
                blue = (pixel >> 16) & 0xff
                color_max = max(red,green,blue)
                if color_max > alpha:
                    over_alpha = True
    return over_alpha or transparent or fully_opaque

def validate_pixels_are_not_premultiplied2(image):
    looks_not_multiplied = False
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            alpha = (pixel >> 24) & 0xff
            #each value of the color channels will never be bigger than that of the alpha channel.
            if alpha > 0:
                red = pixel & 0xff
                green = (pixel >> 8) & 0xff
                blue = (pixel >> 16) & 0xff
                if red > 0 and red > alpha:
                    print 'red: %s, a: %s' % (red,alpha)
                    looks_not_multiplied = True
    return looks_not_multiplied

def validate_pixels_are_premultiplied(image):
    bad_pixels = []
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            alpha = (pixel >> 24) & 0xff
            if alpha > 0:
                pixel = image.get_pixel(x,y)
                red = pixel & 0xff
                green = (pixel >> 8) & 0xff
                blue = (pixel >> 16) & 0xff
                is_valid = ((0 <= red <= alpha) and is_pre(red,alpha)) \
                        and ((0 <= green <= alpha) and is_pre(green,alpha)) \
                        and ((0 <= blue <= alpha) and is_pre(blue,alpha)) \
                        and (alpha >= 0 and alpha <= 255)
                if not is_valid:
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
        if not validate_pixels_are_not_premultiplied(a):
            print '%s not validly demultiplied' % (name)
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

def test_pre_multiply_status():
    b = mapnik.Image.open('./images/support/b.png')
    # not premultiplied yet, should appear that way
    result = validate_pixels_are_not_premultiplied(b)
    eq_(result,True)
    # not yet premultiplied therefore should return false
    result = validate_pixels_are_premultiplied(b)
    eq_(result[0],False)
    # now actually premultiply the pixels
    b.premultiply()
    # now checking if premultiplied should succeed
    result = validate_pixels_are_premultiplied(b)
    eq_(result[0],True)
    # should now not appear to look not premultiplied
    result = validate_pixels_are_not_premultiplied(b)
    eq_(result,False)
    # now actually demultiply the pixels
    b.demultiply()
    # should now appear demultiplied
    result = validate_pixels_are_not_premultiplied(b)
    eq_(result,True)

def test_pre_multiply_status_of_map1():
    m = mapnik.Map(256,256)
    im = mapnik.Image(m.width,m.height)
    eq_(validate_pixels_are_not_premultiplied(im),True)
    mapnik.render(m,im)
    eq_(validate_pixels_are_not_premultiplied(im),True)

def test_pre_multiply_status_of_map2():
    m = mapnik.Map(256,256)
    m.background = mapnik.Color(1,1,1,255)
    im = mapnik.Image(m.width,m.height)
    eq_(validate_pixels_are_not_premultiplied(im),True)
    mapnik.render(m,im)
    eq_(validate_pixels_are_not_premultiplied(im),True)

def test_style_level_opacity():
    m = mapnik.Map(512,512)
    mapnik.load_map(m,'../data/good_maps/style_level_opacity_and_blur.xml')
    m.zoom_all()
    im = mapnik.Image(512,512)
    mapnik.render(m,im)
    actual = '/tmp/mapnik-style-level-opacity.png'
    expected = 'images/support/mapnik-style-level-opacity.png'
    im.save(actual)
    expected_im = mapnik.Image.open(expected)
    eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
