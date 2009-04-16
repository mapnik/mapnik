#!/usr/bin/env python

from nose.tools import *
from utilities import Todo

import mapnik, pickle

# Tests that exercise the functionality of Mapnik classes.

# Shapefile initialization
def test_shapefile_init():
    s = mapnik.Shapefile(file='../../demo/data/boundaries', encoding='latin1')

    e = s.envelope()
   
    assert_almost_equal(e.minx, -11121.6896651, places=7)
    assert_almost_equal(e.miny, -724724.216526, places=6)
    assert_almost_equal(e.maxx, 2463000.67866, places=5)
    assert_almost_equal(e.maxy, 1649661.267, places=3)

# TextSymbolizer initialization
def test_textsymbolizer_init():
    ts = mapnik.TextSymbolizer('Name', 'Font Name', 8, mapnik.Color('black'))

    eq_(ts.name, 'Name')
    eq_(ts.face_name, 'Font Name')
    eq_(ts.text_size, 8)
    eq_(ts.fill, mapnik.Color('black'))

# Map initialization
def test_map_init():
    m = mapnik.Map(256, 256)
   
    eq_(m.width, 256)
    eq_(m.height, 256)
    eq_(m.srs, '+proj=latlong +datum=WGS84')

    m = mapnik.Map(256, 256, '+proj=latlong')
    
    eq_(m.width, 256)
    eq_(m.height, 256)
    eq_(m.srs, '+proj=latlong')

# Map initialization from string
# Trac Ticket #99
def test_map_init_from_string():
    map_string = '''<Map bgcolor="steelblue" srs="+proj=latlong +datum=WGS84">
     <Style name="My Style">
      <Rule>
       <PolygonSymbolizer>
        <CssParameter name="fill">#f2eff9</CssParameter>
       </PolygonSymbolizer>
       <LineSymbolizer>
        <CssParameter name="stroke">rgb(50%,50%,50%)</CssParameter>
        <CssParameter name="stroke-width">0.1</CssParameter>
       </LineSymbolizer>
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
    
    # TODO: Test some properties here    
    mapnik.load_map_from_string(m, map_string)
    mapnik.load_map_from_string(m, map_string, True)

# Map pickling
def test_map_pickle():
    # Fails due to scale() not matching, possibly other things
    raise(Todo("Not implemented yet (Tickets #167, #233)."))

    m = mapnik.Map(256, 256)

    eq_(pickle.loads(pickle.dumps(m)), m)

    m = mapnik.Map(256, 256, '+proj=latlong')

    eq_(pickle.loads(pickle.dumps(m)), m)

# Color initialization
def test_color_init():
    c = mapnik.Color('blue')

    eq_(c.a, 255)
    eq_(c.r, 0)
    eq_(c.g, 0)
    eq_(c.b, 255)

    eq_(c.to_hex_string(), '#0000ff')

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

    eq_(c.to_hex_string(), '#004080')

# Color pickling
def test_color_pickle():
    c = mapnik.Color('blue')

    eq_(pickle.loads(pickle.dumps(c)), c)

    c = mapnik.Color(0, 64, 128)

    eq_(pickle.loads(pickle.dumps(c)), c)

    c = mapnik.Color(0, 64, 128, 192)

    eq_(pickle.loads(pickle.dumps(c)), c)

# Rule initialization
def test_rule_init():
    min_scale = 5
    max_scale = 10
    
    r = mapnik.Rule()
   
    eq_(r.name, '')
    eq_(r.title, '')
    eq_(r.min_scale, 0)
    eq_(r.max_scale, float('inf'))
    
    r = mapnik.Rule("Name")
    
    eq_(r.name, 'Name')
    eq_(r.title, '')
    eq_(r.min_scale, 0)
    eq_(r.max_scale, float('inf'))
    
    r = mapnik.Rule("Name", "Title")
    
    eq_(r.name, 'Name')
    eq_(r.title, 'Title')
    eq_(r.min_scale, 0)
    eq_(r.max_scale, float('inf'))
    
    r = mapnik.Rule("Name", "Title", min_scale)
    
    eq_(r.name, 'Name')
    eq_(r.title, 'Title')
    eq_(r.min_scale, min_scale)
    eq_(r.max_scale, float('inf'))
    
    r = mapnik.Rule("Name", "Title", min_scale, max_scale)
    
    eq_(r.name, 'Name')
    eq_(r.title, 'Title')
    eq_(r.min_scale, min_scale)
    eq_(r.max_scale, max_scale)
    
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

# Envelope initialization
def test_envelope_init():
    e = mapnik.Envelope(100, 100, 200, 200)

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

    c = e.center()

    eq_(c.x, 150)
    eq_(c.y, 150)

# Envelope pickling
def test_envelope_pickle():
    e = mapnik.Envelope(100, 100, 200, 200)

    eq_(pickle.loads(pickle.dumps(e)), e)

# Envelope multiplication
def test_envelope_multiplication():
    e = mapnik.Envelope(100, 100, 200, 200)
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
