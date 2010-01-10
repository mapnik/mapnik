#!/usr/bin/env python

from nose.tools import *

import os, mapnik
from utilities import Todo

def test_simplest_render():
    m = mapnik.Map(256, 256)
    i = mapnik.Image(m.width, m.height)

    mapnik.render(m, i)

    s = i.tostring()

    eq_(s, 256 * 256 * '\x00\x00\x00\x00')

def test_render_image_to_string():
    i = mapnik.Image(256, 256)
    
    i.background = mapnik.Color('black')
    
    s = i.tostring()

    eq_(s, 256 * 256 * '\x00\x00\x00\xff')

    s = i.tostring('png')

def test_render_image_to_file():
    i = mapnik.Image(256, 256)
    
    i.background = mapnik.Color('black')

    i.save('test.jpg')
    i.save('test.png', 'png')

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
    i = mapnik.Image(w,h)
    m.zoom_all()
    mapnik.render(m,i)
    mapnik.save_map(m,tmp_map)
    m2 = mapnik.Map(w,h)
    mapnik.load_map(m2,tmp_map)
    i2 = mapnik.Image(w,h)
    m2.zoom_all()
    mapnik.render(m2,i2)
    os.remove(tmp_map)
    return i,i2    

def test_render_from_serialization():
    i,i2 = get_paired_images(100,100,'../data/good_maps/building_symbolizer.xml')
    eq_(i.tostring(),i2.tostring())

    i,i2 = get_paired_images(100,100,'../data/good_maps/polygon_symbolizer.xml')
    eq_(i.tostring(),i2.tostring())

def test_render_points():
	# Test for effectivenes of ticket #402 (borderline points get lost on reprojection)
	raise Todo("See: http://trac.mapnik.org/ticket/402")
	
	if not mapnik.has_pycairo(): return

	# create and populate point datasource (WGS84 lat-lon coordinates)
	places_ds = mapnik.PointDatasource()
	places_ds.add_point(142.48,-38.38,'Name','Westernmost Point') # westernmost
	places_ds.add_point(143.10,-38.60,'Name','Southernmost Point') # southernmost
	# create layer/rule/style
	s = mapnik.Style()
	r = mapnik.Rule()
	symb = mapnik.PointSymbolizer()
	symb.allow_overlap = True
	r.symbols.append(symb)
	s.rules.append(r)
	lyr = mapnik.Layer('Places','+proj=latlon +datum=WGS84')
	lyr.datasource = places_ds
	lyr.styles.append('places_labels')
	# latlon bounding box corners
	ul_lonlat = mapnik.Coord(142.30,-38.20)
	lr_lonlat = mapnik.Coord(143.40,-38.80)
	# render for different projections 
	projs = { 
		'latlon': '+proj=latlon +datum=WGS84',
		'merc': '+proj=merc +datum=WGS84 +k=1.0 +units=m +over +no_defs',
		'google': '+proj=merc +ellps=sphere +R=6378137 +a=6378137 +units=m',
		'utm': '+proj=utm +zone=54 +datum=WGS84'
		}
	from cairo import SVGSurface
	for projdescr in projs.iterkeys():
		m = mapnik.Map(1000, 500, projs[projdescr])
		m.append_style('places_labels',s)
		m.layers.append(lyr)
		p = mapnik.Projection(projs[projdescr])
		m.zoom_to_box(p.forward(mapnik.Envelope(ul_lonlat,lr_lonlat)))
		# Render to SVG so that it can be checked how many points are there with string comparison
		import StringIO
		svg_memory_file = StringIO.StringIO()
		surface = SVGSurface(svg_memory_file, m.width, m.height)
		mapnik.render(m, surface)
		surface.flush()
		surface.finish()
		svg = svg_memory_file.getvalue()
		svg_memory_file.close()
		num_points_present = len(places_ds.all_features())
		num_points_rendered = svg.count('<image ')
		eq_(num_points_present, num_points_rendered, "Not all points were rendered (%d instead of %d) at projection %s" % (num_points_rendered, num_points_present, projdescr)) 

