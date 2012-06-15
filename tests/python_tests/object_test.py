#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from nose.tools import *
from utilities import execution_path
from utilities import Todo
import tempfile

import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# Tests that exercise the functionality of Mapnik classes.

# LineSymbolizer initialization
def test_line_symbolizer_init():
    s = mapnik.LineSymbolizer()
    eq_(s.rasterizer, mapnik.line_rasterizer.FULL)

# ShieldSymbolizer initialization
def test_shieldsymbolizer_init():
    s = mapnik.ShieldSymbolizer(mapnik.Expression('[Field Name]'), 'DejaVu Sans Bold', 6, mapnik.Color('#000000'), mapnik.PathExpression('../data/images/dummy.png'))
    eq_(s.displacement, (0.0,0.0))
    eq_(s.allow_overlap, False)
    eq_(s.avoid_edges, False)
    eq_(s.character_spacing,0)
    #eq_(str(s.name), str(mapnik2.Expression('[Field Name]'))) name field is no longer supported
    eq_(s.face_name, 'DejaVu Sans Bold')
    eq_(s.allow_overlap, False)
    eq_(s.fill, mapnik.Color('#000000'))
    eq_(s.force_odd_labels, False)
    eq_(s.halo_fill, mapnik.Color('rgb(255,255,255)'))
    eq_(s.halo_radius, 0)
    eq_(s.label_placement, mapnik.label_placement.POINT_PLACEMENT)
    eq_(s.minimum_distance, 0.0)
    eq_(s.text_ratio, 0)
    eq_(s.text_size, 6)
    eq_(s.wrap_width, 0)
    eq_(s.vertical_alignment, mapnik.vertical_alignment.AUTO)
    eq_(s.label_spacing, 0)
    eq_(s.label_position_tolerance, 0)
    # 22.5 * M_PI/180.0 initialized by default
    assert_almost_equal(s.max_char_angle_delta, 0.39269908169872414)

    eq_(s.wrap_character, ' ')
    eq_(s.text_transform, mapnik.text_transform.NONE)
    eq_(s.line_spacing, 0)
    eq_(s.character_spacing, 0)

    # r1341
    eq_(s.wrap_before, False)
    eq_(s.horizontal_alignment, mapnik.horizontal_alignment.AUTO)
    eq_(s.justify_alignment, mapnik.justify_alignment.AUTO)
    eq_(s.opacity, 1.0)

    # r2300
    eq_(s.minimum_padding, 0.0)

    # was mixed with s.opacity
    eq_(s.text_opacity, 1.0)

    eq_(s.shield_displacement, (0.0,0.0))
    # TODO - the pattern in bindings seems to be to get/set
    # strings for PathExpressions... should we pass objects?
    eq_(s.filename, '../data/images/dummy.png')

    # 11c34b1: default transform list is empty, not identity matrix
    eq_(s.transform, '')

    eq_(len(s.fontset.names), 0)

# ShieldSymbolizer missing image file
# images paths are now PathExpressions are evaluated at runtime
# so it does not make sense to throw...
#@raises(RuntimeError)
#def test_shieldsymbolizer_missing_image():
#    s = mapnik.ShieldSymbolizer(mapnik.Expression('[Field Name]'), 'DejaVu Sans Bold', 6, mapnik.Color('#000000'), mapnik.PathExpression('../#data/images/broken.png'))

# ShieldSymbolizer modification
def test_shieldsymbolizer_modify():
    s = mapnik.ShieldSymbolizer(mapnik.Expression('[Field Name]'), 'DejaVu Sans Bold', 6, mapnik.Color('#000000'), mapnik.PathExpression('../data/images/dummy.png'))
    # transform expression
    s.transform = "rotate(30+[a]) scale(2*[sx] [sy])"
    eq_(s.transform, "rotate((30+[a])) scale(2*[sx], [sy])")

def test_polygonsymbolizer_init():
    p = mapnik.PolygonSymbolizer()

    eq_(p.fill, mapnik.Color('gray'))
    eq_(p.fill_opacity, 1)
    eq_(p.placement, mapnik.point_placement.CENTROID)

    p = mapnik.PolygonSymbolizer(mapnik.Color('blue'))
    p.placement = mapnik.point_placement.INTERIOR

    eq_(p.fill, mapnik.Color('blue'))
    eq_(p.fill_opacity, 1)
    eq_(p.placement, mapnik.point_placement.INTERIOR)

# PointSymbolizer initialization
def test_pointsymbolizer_init():
    p = mapnik.PointSymbolizer() 
    eq_(p.allow_overlap, False)
    eq_(p.opacity,1)
    eq_(p.filename,'')
    eq_(p.ignore_placement,False)
    eq_(p.placement, mapnik.point_placement.CENTROID)

    p = mapnik.PointSymbolizer(mapnik.PathExpression("../data/images/dummy.png"))
    p.allow_overlap = True
    p.opacity = 0.5
    p.ignore_placement = True
    p.placement = mapnik.point_placement.INTERIOR
    eq_(p.allow_overlap, True)
    eq_(p.opacity, 0.5)
    eq_(p.filename,'../data/images/dummy.png')
    eq_(p.ignore_placement,True)
    eq_(p.placement, mapnik.point_placement.INTERIOR)


# MarkersSymbolizer initialization
def test_markersymbolizer_init():
    p = mapnik.MarkersSymbolizer() 
    eq_(p.allow_overlap, False)
    eq_(p.opacity,1)
    eq_(p.filename,'')
    eq_(p.marker_type,mapnik.marker_type.ARROW)
    eq_(p.placement,mapnik.marker_placement.LINE_PLACEMENT)
    eq_(p.fill,mapnik.Color(0,0,255))
    eq_(p.ignore_placement,False)
    eq_(p.spacing,100)
    eq_(p.max_error,0.2)

    stroke = mapnik.Stroke()
    stroke.color = mapnik.Color('black')
    stroke.width = 1.0
    
    p.stroke = stroke
    p.fill = mapnik.Color('white')
    p.allow_overlap = True
    p.opacity = 0.5

    eq_(p.allow_overlap, True)
    eq_(p.opacity, 0.5)


# PointSymbolizer missing image file
# images paths are now PathExpressions are evaluated at runtime
# so it does not make sense to throw...
#@raises(RuntimeError)
#def test_pointsymbolizer_missing_image():
 #   p = mapnik.PointSymbolizer(mapnik.PathExpression("../data/images/broken.png"))

# PolygonSymbolizer initialization
def test_polygonsymbolizer_init():
    p = mapnik.PolygonSymbolizer()

    eq_(p.fill, mapnik.Color('gray'))
    eq_(p.fill_opacity, 1)

    p = mapnik.PolygonSymbolizer(mapnik.Color('blue'))

    eq_(p.fill, mapnik.Color('blue'))
    eq_(p.fill_opacity, 1)

# Stroke initialization
def test_stroke_init():
    s = mapnik.Stroke()

    eq_(s.width, 1)
    eq_(s.opacity, 1)
    eq_(s.color, mapnik.Color('black'))
    eq_(s.line_cap, mapnik.line_cap.BUTT_CAP)
    eq_(s.line_join, mapnik.line_join.MITER_JOIN)
    eq_(s.gamma,1.0)

    s = mapnik.Stroke(mapnik.Color('blue'), 5.0)
    s.gamma = .5

    eq_(s.width, 5)
    eq_(s.opacity, 1)
    eq_(s.color, mapnik.Color('blue'))
    eq_(s.gamma, .5)
    eq_(s.line_cap, mapnik.line_cap.BUTT_CAP)
    eq_(s.line_join, mapnik.line_join.MITER_JOIN)

# Stroke dashes
def test_stroke_dash_arrays():
    s = mapnik.Stroke()
    s.add_dash(1,2)
    s.add_dash(3,4)
    s.add_dash(5,6)

    eq_(s.get_dashes(), [(1,2),(3,4),(5,6)])

# LineSymbolizer initialization
def test_linesymbolizer_init():
    l = mapnik.LineSymbolizer()

    eq_(l.stroke.width, 1)
    eq_(l.stroke.opacity, 1)
    eq_(l.stroke.color, mapnik.Color('black'))
    eq_(l.stroke.line_cap, mapnik.line_cap.BUTT_CAP)
    eq_(l.stroke.line_join, mapnik.line_join.MITER_JOIN)

    l = mapnik.LineSymbolizer(mapnik.Color('blue'), 5.0)

    eq_(l.stroke.width, 5)
    eq_(l.stroke.opacity, 1)
    eq_(l.stroke.color, mapnik.Color('blue'))
    eq_(l.stroke.line_cap, mapnik.line_cap.BUTT_CAP)
    eq_(l.stroke.line_join, mapnik.line_join.MITER_JOIN)

    s = mapnik.Stroke(mapnik.Color('blue'), 5.0)
    l = mapnik.LineSymbolizer(s)

    eq_(l.stroke.width, 5)
    eq_(l.stroke.opacity, 1)
    eq_(l.stroke.color, mapnik.Color('blue'))
    eq_(l.stroke.line_cap, mapnik.line_cap.BUTT_CAP)
    eq_(l.stroke.line_join, mapnik.line_join.MITER_JOIN)

# TextSymbolizer initialization
def test_textsymbolizer_init():
    ts = mapnik.TextSymbolizer(mapnik.Expression('[Field_Name]'), 'Font Name', 8, mapnik.Color('black'))

#    eq_(str(ts.name), str(mapnik2.Expression('[Field_Name]'))) name field is no longer supported
    eq_(ts.format.face_name, 'Font Name')
    eq_(ts.format.text_size, 8)
    eq_(ts.format.fill, mapnik.Color('black'))
    eq_(ts.properties.label_placement, mapnik.label_placement.POINT_PLACEMENT)
    eq_(ts.properties.horizontal_alignment, mapnik.horizontal_alignment.AUTO)

# Map initialization
def test_layer_init():
    l = mapnik.Layer('test')
    eq_(l.name,'test')
    eq_(l.envelope(),mapnik.Box2d())
    eq_(l.clear_label_cache,False)
    eq_(l.cache_features,False)
    eq_(l.visible(1),True)
    eq_(l.active,True)
    eq_(l.datasource,None)
    eq_(l.queryable,False)
    eq_(l.srs,'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')

# Map initialization
def test_map_init():
    m = mapnik.Map(256, 256)

    eq_(m.width, 256)
    eq_(m.height, 256)
    eq_(m.srs, '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')
    eq_(m.base, '')
    eq_(m.maximum_extent, None)

    m = mapnik.Map(256, 256, '+proj=latlong')
    eq_(m.srs, '+proj=latlong')

def test_map_maximum_extent_modification():
    m = mapnik.Map(256, 256)
    eq_(m.maximum_extent, None)
    m.maximum_extent = mapnik.Box2d()
    eq_(m.maximum_extent, mapnik.Box2d())
    m.maximum_extent = None
    eq_(m.maximum_extent, None)

# Map initialization from string
def test_map_init_from_string():
    map_string = '''<Map background-color="steelblue" base="./" srs="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs">
     <Style name="My Style">
      <Rule>
       <PolygonSymbolizer fill="#f2eff9"/>
       <LineSymbolizer stroke="rgb(50%,50%,50%)" stroke-width="0.1"/>
      </Rule>
     </Style>
     <Layer name="boundaries">
      <StyleName>My Style</StyleName>
       <Datasource>
        <Parameter name="type">shape</Parameter>
        <Parameter name="file">../../demo/data/boundaries</Parameter>
       </Datasource>
      </Layer>
    </Map>'''

    m = mapnik.Map(600, 300)
    eq_(m.base, '')
    try:
        mapnik.load_map_from_string(m, map_string)
        eq_(m.base, './')
        mapnik.load_map_from_string(m, map_string, False, "") # this "" will have no effect
        eq_(m.base, './')

        tmp_dir = tempfile.gettempdir()
        try:
            mapnik.load_map_from_string(m, map_string, False, tmp_dir)
        except RuntimeError:
            pass # runtime error expected because shapefile path should be wrong and datasource will throw
        eq_(m.base, tmp_dir) # tmp_dir will be set despite the exception because load_map mostly worked
        m.base = 'foo'
        mapnik.load_map_from_string(m, map_string, True, ".")
        eq_(m.base, '.')
    except RuntimeError, e:
        # only test datasources that we have installed
        if not 'Could not create datasource' in str(e):
            raise RuntimeError(e)

# Color initialization
@raises(Exception) # Boost.Python.ArgumentError
def test_color_init_errors():
    c = mapnik.Color()

@raises(RuntimeError)
def test_color_init_errors():
    c = mapnik.Color('foo') # mapnik config 

def test_color_init():
    c = mapnik.Color('blue')

    eq_(c.a, 255)
    eq_(c.r, 0)
    eq_(c.g, 0)
    eq_(c.b, 255)

    eq_(c.to_hex_string(), '#0000ff')

    c = mapnik.Color('#f2eff9')

    eq_(c.a, 255)
    eq_(c.r, 242)
    eq_(c.g, 239)
    eq_(c.b, 249)

    eq_(c.to_hex_string(), '#f2eff9')

    c = mapnik.Color('rgb(50%,50%,50%)')

    eq_(c.a, 255)
    eq_(c.r, 128)
    eq_(c.g, 128)
    eq_(c.b, 128)

    eq_(c.to_hex_string(), '#808080')

    c = mapnik.Color(0, 64, 128)

    eq_(c.a, 255)
    eq_(c.r, 0)
    eq_(c.g, 64)
    eq_(c.b, 128)

    eq_(c.to_hex_string(), '#004080')

    c = mapnik.Color(0, 64, 128, 192)

    eq_(c.a, 192)
    eq_(c.r, 0)
    eq_(c.g, 64)
    eq_(c.b, 128)

    eq_(c.to_hex_string(), '#004080c0')

# Color equality
def test_color_equality():

    c1 = mapnik.Color('blue')
    c2 = mapnik.Color(0,0,255)
    c3 = mapnik.Color('black')

    c3.r = 0
    c3.g = 0
    c3.b = 255
    c3.a = 255

    eq_(c1, c2)
    eq_(c1, c3)

    c1 = mapnik.Color(0, 64, 128)
    c2 = mapnik.Color(0, 64, 128)
    c3 = mapnik.Color(0, 0, 0)

    c3.r = 0
    c3.g = 64
    c3.b = 128

    eq_(c1, c2)
    eq_(c1, c3)

    c1 = mapnik.Color(0, 64, 128, 192)
    c2 = mapnik.Color(0, 64, 128, 192)
    c3 = mapnik.Color(0, 0, 0, 255)

    c3.r = 0
    c3.g = 64
    c3.b = 128
    c3.a = 192

    eq_(c1, c2)
    eq_(c1, c3)

    c1 = mapnik.Color('rgb(50%,50%,50%)')
    c2 = mapnik.Color(128, 128, 128, 255)
    c3 = mapnik.Color('#808080')
    c4 = mapnik.Color('gray')

    eq_(c1, c2)
    eq_(c1, c3)
    eq_(c1, c4)

    c1 = mapnik.Color('hsl(0, 100%, 50%)')   # red
    c2 = mapnik.Color('hsl(120, 100%, 50%)') # lime
    c3 = mapnik.Color('hsla(240, 100%, 50%, 0.5)') # semi-transparent solid blue

    eq_(c1, mapnik.Color('red'))
    eq_(c2, mapnik.Color('lime'))
    eq_(c3, mapnik.Color(0,0,255,128))

# Rule initialization
def test_rule_init():
    min_scale = 5
    max_scale = 10

    r = mapnik.Rule()

    eq_(r.name, '')
    eq_(r.min_scale, 0)
    eq_(r.max_scale, float('inf'))
    eq_(r.has_else(), False)
    eq_(r.has_also(), False)

    r = mapnik.Rule()

    r.set_else(True)
    eq_(r.has_else(), True)
    eq_(r.has_also(), False)

    r = mapnik.Rule()

    r.set_also(True)
    eq_(r.has_else(), False)
    eq_(r.has_also(), True)

    r = mapnik.Rule("Name")

    eq_(r.name, 'Name')
    eq_(r.min_scale, 0)
    eq_(r.max_scale, float('inf'))
    eq_(r.has_else(), False)
    eq_(r.has_also(), False)

    r = mapnik.Rule("Name")

    eq_(r.name, 'Name')
    eq_(r.min_scale, 0)
    eq_(r.max_scale, float('inf'))
    eq_(r.has_else(), False)
    eq_(r.has_also(), False)

    r = mapnik.Rule("Name", min_scale)

    eq_(r.name, 'Name')
    eq_(r.min_scale, min_scale)
    eq_(r.max_scale, float('inf'))
    eq_(r.has_else(), False)
    eq_(r.has_also(), False)

    r = mapnik.Rule("Name", min_scale, max_scale)

    eq_(r.name, 'Name')
    eq_(r.min_scale, min_scale)
    eq_(r.max_scale, max_scale)
    eq_(r.has_else(), False)
    eq_(r.has_also(), False)

# Coordinate initialization
def test_coord_init():
    c = mapnik.Coord(100, 100)

    eq_(c.x, 100)
    eq_(c.y, 100)

# Coordinate multiplication
def test_coord_multiplication():
    c = mapnik.Coord(100, 100)
    c *= 2

    eq_(c.x, 200)
    eq_(c.y, 200)

# Box2d initialization
def test_envelope_init():
    e = mapnik.Box2d(100, 100, 200, 200)

    assert_true(e.contains(100, 100))
    assert_true(e.contains(100, 200))
    assert_true(e.contains(200, 200))
    assert_true(e.contains(200, 100))

    assert_true(e.contains(e.center()))

    assert_false(e.contains(99.9, 99.9))
    assert_false(e.contains(99.9, 200.1))
    assert_false(e.contains(200.1, 200.1))
    assert_false(e.contains(200.1, 99.9))

    eq_(e.width(), 100)
    eq_(e.height(), 100)

    eq_(e.minx, 100)
    eq_(e.miny, 100)

    eq_(e.maxx, 200)
    eq_(e.maxy, 200)

    eq_(e[0],100)
    eq_(e[1],100)
    eq_(e[2],200)
    eq_(e[3],200)
    eq_(e[0],e[-4])
    eq_(e[1],e[-3])
    eq_(e[2],e[-2])
    eq_(e[3],e[-1])

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

# Box2d static initialization
def test_envelope_static_init():
    e = mapnik.Box2d.from_string('100 100 200 200')
    e2 = mapnik.Box2d.from_string('100,100,200,200')
    e3 = mapnik.Box2d.from_string('100 , 100 , 200 , 200')
    eq_(e,e2)
    eq_(e,e3)

    assert_true(e.contains(100, 100))
    assert_true(e.contains(100, 200))
    assert_true(e.contains(200, 200))
    assert_true(e.contains(200, 100))

    assert_true(e.contains(e.center()))

    assert_false(e.contains(99.9, 99.9))
    assert_false(e.contains(99.9, 200.1))
    assert_false(e.contains(200.1, 200.1))
    assert_false(e.contains(200.1, 99.9))

    eq_(e.width(), 100)
    eq_(e.height(), 100)

    eq_(e.minx, 100)
    eq_(e.miny, 100)

    eq_(e.maxx, 200)
    eq_(e.maxy, 200)

    eq_(e[0],100)
    eq_(e[1],100)
    eq_(e[2],200)
    eq_(e[3],200)
    eq_(e[0],e[-4])
    eq_(e[1],e[-3])
    eq_(e[2],e[-2])
    eq_(e[3],e[-1])

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

# Box2d multiplication
def test_envelope_multiplication():
    e = mapnik.Box2d(100, 100, 200, 200)
    e *= 2

    assert_true(e.contains(50, 50))
    assert_true(e.contains(50, 250))
    assert_true(e.contains(250, 250))
    assert_true(e.contains(250, 50))

    assert_false(e.contains(49.9, 49.9))
    assert_false(e.contains(49.9, 250.1))
    assert_false(e.contains(250.1, 250.1))
    assert_false(e.contains(250.1, 49.9))

    assert_true(e.contains(e.center()))

    eq_(e.width(), 200)
    eq_(e.height(), 200)

    eq_(e.minx, 50)
    eq_(e.miny, 50)

    eq_(e.maxx, 250)
    eq_(e.maxy, 250)

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

# Box2d clipping
def test_envelope_clipping():
    e1 = mapnik.Box2d(-180,-90,180,90)
    e2 = mapnik.Box2d(-120,40,-110,48)
    e1.clip(e2)
    eq_(e1,e2)

    # madagascar in merc
    e1 = mapnik.Box2d(4772116.5490, -2744395.0631, 5765186.4203, -1609458.0673)
    e2 = mapnik.Box2d(5124338.3753, -2240522.1727, 5207501.8621, -2130452.8520)
    e1.clip(e2)
    eq_(e1,e2)

    # nz in lon/lat
    e1 = mapnik.Box2d(163.8062, -47.1897, 179.3628, -33.9069)
    e2 = mapnik.Box2d(173.7378, -39.6395, 174.4849, -38.9252)
    e1.clip(e2)
    eq_(e1,e2)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
