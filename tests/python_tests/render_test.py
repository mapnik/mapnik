#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
import tempfile
import os, mapnik
from nose.tools import *
from utilities import execution_path, run_all

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_simplest_render():
    m = mapnik.Map(256, 256)
    im = mapnik.Image(m.width, m.height)
    eq_(im.painted(),False)
    eq_(im.is_solid(),True)
    mapnik.render(m, im)
    eq_(im.painted(),False)
    eq_(im.is_solid(),True)
    s = im.tostring()
    eq_(s, 256 * 256 * '\x00\x00\x00\x00')

def test_render_image_to_string():
    im = mapnik.Image(256, 256)
    im.background = mapnik.Color('black')
    eq_(im.painted(),False)
    eq_(im.is_solid(),True)
    s = im.tostring()
    eq_(s, 256 * 256 * '\x00\x00\x00\xff')

def test_non_solid_image():
    im = mapnik.Image(256, 256)
    im.background = mapnik.Color('black')
    eq_(im.painted(),False)
    eq_(im.is_solid(),True)
    # set one pixel to a different color
    im.set_pixel(0,0,mapnik.Color('white'))
    eq_(im.painted(),False)
    eq_(im.is_solid(),False)

def test_non_solid_image_view():
    im = mapnik.Image(256, 256)
    im.background = mapnik.Color('black')
    view = im.view(0,0,256,256)
    eq_(view.is_solid(),True)
    # set one pixel to a different color
    im.set_pixel(0,0,mapnik.Color('white'))
    eq_(im.is_solid(),False)
    # view, since it is the exact dimensions of the image
    # should also be non-solid
    eq_(view.is_solid(),False)
    # but not a view that excludes the single diff pixel
    view2 = im.view(1,1,256,256)
    eq_(view2.is_solid(),True)

def test_setting_alpha():
    w,h = 256,256
    im1 = mapnik.Image(w,h)
    # white, half transparent
    c1 = mapnik.Color('rgba(255,255,255,.5)')
    im1.background = c1
    eq_(im1.painted(),False)
    eq_(im1.is_solid(),True)
    # pure white
    im2 = mapnik.Image(w,h)
    c2 = mapnik.Color('rgba(255,255,255,1)')
    im2.background = c2
    im2.set_alpha(c1.a/255.0)
    eq_(im2.painted(),False)
    eq_(im2.is_solid(),True)
    eq_(len(im1.tostring('png32')), len(im2.tostring('png32')))

def test_render_image_to_file():
    im = mapnik.Image(256, 256)
    im.background = mapnik.Color('black')
    if mapnik.has_jpeg():
        im.save('test.jpg')
    im.save('test.png', 'png')
    if os.path.exists('test.jpg'):
        os.remove('test.jpg')
    else:
        return False
    if os.path.exists('test.png'):
        os.remove('test.png')
    else:
        return False

def get_paired_images(w,h,mapfile):
    tmp_map = 'tmp_map.xml'
    m = mapnik.Map(w,h)
    mapnik.load_map(m,mapfile)
    im = mapnik.Image(w,h)
    m.zoom_all()
    mapnik.render(m,im)
    mapnik.save_map(m,tmp_map)
    m2 = mapnik.Map(w,h)
    mapnik.load_map(m2,tmp_map)
    im2 = mapnik.Image(w,h)
    m2.zoom_all()
    mapnik.render(m2,im2)
    os.remove(tmp_map)
    return im,im2

def test_render_from_serialization():
    try:
        im,im2 = get_paired_images(100,100,'../data/good_maps/building_symbolizer.xml')
        eq_(im.tostring('png32'),im2.tostring('png32'))

        im,im2 = get_paired_images(100,100,'../data/good_maps/polygon_symbolizer.xml')
        eq_(im.tostring('png32'),im2.tostring('png32'))
    except RuntimeError, e:
        # only test datasources that we have installed
        if not 'Could not create datasource' in str(e):
            raise RuntimeError(e)

def test_render_points():
    if not mapnik.has_cairo(): return
    # create and populate point datasource (WGS84 lat-lon coordinates)
    ds = mapnik.MemoryDatasource()
    context = mapnik.Context()
    context.push('Name')
    f = mapnik.Feature(context,1)
    f['Name'] = 'Westernmost Point'
    f.add_geometries_from_wkt('POINT (142.48 -38.38)')
    ds.add_feature(f)

    f = mapnik.Feature(context,2)
    f['Name'] = 'Southernmost Point'
    f.add_geometries_from_wkt('POINT (143.10 -38.60)')
    ds.add_feature(f)

    # create layer/rule/style
    s = mapnik.Style()
    r = mapnik.Rule()
    symb = mapnik.PointSymbolizer()
    symb.allow_overlap = True
    r.symbols.append(symb)
    s.rules.append(r)
    lyr = mapnik.Layer('Places','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')
    lyr.datasource = ds
    lyr.styles.append('places_labels')
    # latlon bounding box corners
    ul_lonlat = mapnik.Coord(142.30,-38.20)
    lr_lonlat = mapnik.Coord(143.40,-38.80)
    # render for different projections 
    projs = { 
        'google': '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over',
        'latlon': '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs',
        'merc': '+proj=merc +datum=WGS84 +k=1.0 +units=m +over +no_defs',
        'utm': '+proj=utm +zone=54 +datum=WGS84'
        }
    for projdescr in projs.iterkeys():
        m = mapnik.Map(1000, 500, projs[projdescr])
        m.append_style('places_labels',s)
        m.layers.append(lyr)
        dest_proj = mapnik.Projection(projs[projdescr])
        src_proj = mapnik.Projection('+init=epsg:4326')
        tr = mapnik.ProjTransform(src_proj,dest_proj)
        m.zoom_to_box(tr.forward(mapnik.Box2d(ul_lonlat,lr_lonlat)))
        # Render to SVG so that it can be checked how many points are there with string comparison
        svg_file = os.path.join(tempfile.gettempdir(), 'mapnik-render-points-%s.svg' % projdescr)
        mapnik.render_to_file(m, svg_file)
        num_points_present = len(ds.all_features())
        svg = open(svg_file,'r').read()
        num_points_rendered = svg.count('<image ')
        eq_(num_points_present, num_points_rendered, "Not all points were rendered (%d instead of %d) at projection %s" % (num_points_rendered, num_points_present, projdescr)) 

@raises(RuntimeError)
def test_render_with_scale_factor_zero_throws():
    m = mapnik.Map(256,256)
    im = mapnik.Image(256, 256)
    mapnik.render(m,im,0.0)

def test_render_with_detector():
    ds = mapnik.MemoryDatasource()
    context = mapnik.Context()
    geojson  = '{ "type": "Feature", "geometry": { "type": "Point", "coordinates": [ 0, 0 ] } }'
    ds.add_feature(mapnik.Feature.from_geojson(geojson,context))
    s = mapnik.Style()
    r = mapnik.Rule()
    lyr = mapnik.Layer('point')
    lyr.datasource = ds
    lyr.styles.append('point')
    symb = mapnik.MarkersSymbolizer()
    symb.allow_overlap = False
    r.symbols.append(symb)
    s.rules.append(r)
    m = mapnik.Map(256,256)
    m.append_style('point',s)
    m.layers.append(lyr)
    m.zoom_to_box(mapnik.Box2d(-180,-85,180,85))
    im = mapnik.Image(256, 256)
    mapnik.render(m,im)
    expected_file = './images/support/marker-in-center.png'
    actual_file = '/tmp/' + os.path.basename(expected_file)
    #im.save(expected_file,'png8')
    im.save(actual_file,'png8')
    actual = mapnik.Image.open(expected_file)
    expected = mapnik.Image.open(expected_file)
    eq_(actual.tostring('png32'),expected.tostring('png32'), 'failed comparing actual (%s) and expected (%s)' % (actual_file,expected_file))
    # now render will a collision detector that should
    # block out the placement of this point
    detector = mapnik.LabelCollisionDetector(m)
    eq_(detector.extent(),mapnik.Box2d(-0.0,-0.0,m.width,m.height))
    eq_(detector.extent(),mapnik.Box2d(-0.0,-0.0,256.0,256.0))
    eq_(detector.boxes(),[])
    detector.insert(detector.extent())
    eq_(detector.boxes(),[detector.extent()])
    im2 = mapnik.Image(256, 256)
    mapnik.render_with_detector(m, im2, detector)
    expected_file_collision = './images/support/marker-in-center-not-placed.png'
    #im2.save(expected_file_collision,'png8')
    actual_file = '/tmp/' + os.path.basename(expected_file_collision)
    im2.save(actual_file,'png8')


if 'shape' in mapnik.DatasourceCache.plugin_names():

    def test_render_with_scale_factor():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/marker-text-line.xml')
        m.zoom_all()
        sizes = [.00001,.005,.1,.899,1,1.5,2,5,10,100]
        for size in sizes:
            im = mapnik.Image(256, 256)
            mapnik.render(m,im,size)
            expected_file = './images/support/marker-text-line-scale-factor-%s.png' % size
            actual_file = '/tmp/' + os.path.basename(expected_file)
            im.save(actual_file,'png32')
            #im.save(expected_file,'png32')
            # we save and re-open here so both png8 images are ready as full color png
            actual = mapnik.Image.open(actual_file)
            expected = mapnik.Image.open(expected_file)
            eq_(actual.tostring('png32'),expected.tostring('png32'), 'failed comparing actual (%s) and expected (%s)' % (actual_file,expected_file))

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
