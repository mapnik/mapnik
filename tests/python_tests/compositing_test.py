#encoding: utf8

from nose.tools import *
import os,sys
from utilities import execution_path, run_tests, Todo
from utilities import get_unique_colors, pixel2channels, side_by_side_image
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
            red,green,blue,alpha = pixel2channels(pixel)
            print "rgba(%s,%s,%s,%s) at %s,%s" % (red,green,blue,alpha,x,y)

def replace_style(m, name, style):
    m.remove_style(name)
    m.append_style(name, style)

# note: it is impossible to know for all pixel colors
# we can only detect likely cases of non premultiplied colors
def validate_pixels_are_not_premultiplied(image):
    over_alpha = False
    transparent = True
    fully_opaque = True
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            red,green,blue,alpha = pixel2channels(pixel)
            if alpha > 0:
                transparent = False
                if alpha < 255:
                    fully_opaque = False
                color_max = max(red,green,blue)
                if color_max > alpha:
                    over_alpha = True
    return over_alpha or transparent or fully_opaque

def validate_pixels_are_not_premultiplied2(image):
    looks_not_multiplied = False
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            red,green,blue,alpha = pixel2channels(pixel)
            #each value of the color channels will never be bigger than that of the alpha channel.
            if alpha > 0:
                if red > 0 and red > alpha:
                    print 'red: %s, a: %s' % (red,alpha)
                    looks_not_multiplied = True
    return looks_not_multiplied

def validate_pixels_are_premultiplied(image):
    bad_pixels = []
    for x in range(0,image.width(),2):
        for y in range(0,image.height(),2):
            pixel = image.get_pixel(x,y)
            red,green,blue,alpha = pixel2channels(pixel)
            if alpha > 0:
                pixel = image.get_pixel(x,y)
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
    num_ops = len(mapnik.CompositeOp.names)
    successes = []
    fails = []
    for name in mapnik.CompositeOp.names:
        a = mapnik.Image.open('./images/support/a.png')
        a.premultiply()
        a.composite(b,getattr(mapnik.CompositeOp,name))
        actual = '/tmp/mapnik-comp-op-test-' + name + '.png'
        expected = 'images/composited/' + name + '.png'
        valid = validate_pixels_are_premultiplied(a)
        if not valid[0]:
            fails.append('%s not validly premultiplied!:\n\t %s pixels (%s)' % (name,len(valid[1]),valid[1][0]))
        a.demultiply()
        if not validate_pixels_are_not_premultiplied(a):
            fails.append('%s not validly demultiplied' % (name))
        a.save(actual)
        if not os.path.exists(expected):
            print 'generating expected test image: %s' % expected
            a.save(expected)
        expected_im = mapnik.Image.open(expected)
        # compare them
        if a.tostring() == expected_im.tostring():
            successes.append(name)
        else:
            fails.append('failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))
            fail_im = side_by_side_image(expected_im, a)
            fail_im.save('/tmp/mapnik-comp-op-test-' + name + '.fail.png')
    eq_(len(successes),num_ops,'\n'+'\n'.join(fails))
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

if 'shape' in mapnik.DatasourceCache.plugin_names():
    def test_style_level_comp_op():
        m = mapnik.Map(256, 256)
        mapnik.load_map(m, '../data/good_maps/style_level_comp_op.xml')
        m.zoom_all()
        successes = []
        fails = []
        for name in mapnik.CompositeOp.names:
            # find_style returns a copy of the style object
            style_markers = m.find_style("markers")
            style_markers.comp_op = getattr(mapnik.CompositeOp, name)
            # replace the original style with the modified one
            replace_style(m, "markers", style_markers)
            im = mapnik.Image(m.width, m.height)
            mapnik.render(m, im)
            actual = '/tmp/mapnik-style-comp-op-' + name + '.png'
            expected = 'images/style-comp-op/' + name + '.png'
            im.save(actual)
            if not os.path.exists(expected):
                print 'generating expected test image: %s' % expected
                im.save(expected)
            expected_im = mapnik.Image.open(expected)
            # compare them
            if im.tostring() == expected_im.tostring():
                successes.append(name)
            else:
                fails.append('failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))
                fail_im = side_by_side_image(expected_im, im)
                fail_im.save('/tmp/mapnik-style-comp-op-' + name + '.fail.png')
        eq_(len(fails), 0, '\n'+'\n'.join(fails))

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

def test_rounding_and_color_expectations():
    m = mapnik.Map(1,1)
    m.background = mapnik.Color('rgba(255,255,255,.4999999)')
    im = mapnik.Image(m.width,m.height)
    mapnik.render(m,im)
    # ugh 252, see: https://github.com/mapnik/mapnik/issues/1519
    eq_(get_unique_colors(im),['rgba(252,252,252,127)'])
    m = mapnik.Map(1,1)
    m.background = mapnik.Color('rgba(255,255,255,.5)')
    im = mapnik.Image(m.width,m.height)
    mapnik.render(m,im)
    eq_(get_unique_colors(im),['rgba(253,253,253,128)'])
    im_file = mapnik.Image.open('../data/images/stripes_pattern.png')
    eq_(get_unique_colors(im_file),['rgba(0,0,0,0)', 'rgba(74,74,74,255)'])
    # should have no effect
    im_file.premultiply()
    eq_(get_unique_colors(im_file),['rgba(0,0,0,0)', 'rgba(74,74,74,255)'])
    im_file.set_alpha(.5)
    # should have effect now that image has transparency
    im_file.premultiply()
    eq_(get_unique_colors(im_file),['rgba(0,0,0,0)', 'rgba(37,37,37,127)'])
    # should restore to original nonpremultiplied colors
    im_file.demultiply()
    eq_(get_unique_colors(im_file),['rgba(0,0,0,0)', 'rgba(74,74,74,127)'])


def test_background_image_and_background_color():
    m = mapnik.Map(8,8)
    m.background = mapnik.Color('rgba(255,255,255,.5)')
    m.background_image = '../data/images/stripes_pattern.png'
    im = mapnik.Image(m.width,m.height)
    mapnik.render(m,im)
    # note: data loss due to rounding as per https://github.com/mapnik/mapnik/issues/1519
    # means that background will roundtrip to 253 not 255
    #eq_(get_unique_colors(im),['rgba(255,255,255,128)', 'rgba(74,74,74,255)'])
    eq_(get_unique_colors(im),['rgba(253,253,253,128)', 'rgba(74,74,74,255)'])

def test_background_image_with_alpha_and_background_color():
    m = mapnik.Map(10,10)
    m.background = mapnik.Color('rgba(255,255,255,.5)')
    m.background_image = '../data/images/yellow_half_trans.png'
    im = mapnik.Image(m.width,m.height)
    mapnik.render(m,im)
    eq_(get_unique_colors(im),['rgba(255,255,85,191)'])

def test_background_image_with_alpha_and_background_color_against_composited_control():
    m = mapnik.Map(10,10)
    m.background = mapnik.Color('rgba(255,255,255,.5)')
    m.background_image = '../data/images/yellow_half_trans.png'
    im = mapnik.Image(m.width,m.height)
    mapnik.render(m,im)
    # create and composite the expected result
    im1 = mapnik.Image(10,10)
    im1.background = mapnik.Color('rgba(255,255,255,.5)')
    im1.premultiply()
    im2 = mapnik.Image(10,10)
    im2.background = mapnik.Color('rgba(255,255,0,.5)')
    im2.premultiply()
    im1.composite(im2)
    im1.demultiply()
    # compare image rendered (compositing in `agg_renderer<T>::setup`)
    # vs image composited via python bindings
    raise Todo("looks like we need to investigate PNG rounding when saving")
    eq_(get_unique_colors(im),get_unique_colors(im1))

if __name__ == "__main__":
    setup()
    run_tests(eval(x) for x in dir() if x.startswith("test_"))
