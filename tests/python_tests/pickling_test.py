#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from nose.tools import *
from utilities import execution_path
from utilities import Todo
import tempfile

import mapnik, pickle

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# PointSymbolizer pickling
def test_pointsymbolizer_pickle():
    raise Todo("point_symbolizer pickling currently disabled")
    p = mapnik.PointSymbolizer(mapnik.PathExpression("../data/images/dummy.png"))
    p2 = pickle.loads(pickle.dumps(p,pickle.HIGHEST_PROTOCOL))
    # image type, width, and height only used in contructor...
    eq_(p.filename, p2.filename)
    eq_(p.allow_overlap, p2.allow_overlap)
    eq_(p.opacity, p2.opacity)
    eq_(p.ignore_placement, p2.ignore_placement)
    eq_(p.placement, p2.placement)


# PolygonSymbolizer pickling
def test_polygonsymbolizer_pickle():
    p = mapnik.PolygonSymbolizer(mapnik.Color('black'))
    p.fill_opacity = .5
    # does not work for some reason...
    #eq_(pickle.loads(pickle.dumps(p)), p)
    p2 = pickle.loads(pickle.dumps(p,pickle.HIGHEST_PROTOCOL))
    eq_(p.fill, p2.fill)
    eq_(p.fill_opacity, p2.fill_opacity)


# Stroke pickling
def test_stroke_pickle():
    s = mapnik.Stroke(mapnik.Color('black'),4.5)

    eq_(s.width, 4.5)
    eq_(s.color, mapnik.Color('black'))

    s.add_dash(1,2)
    s.add_dash(3,4)
    s.add_dash(5,6)

    s2 = pickle.loads(pickle.dumps(s,pickle.HIGHEST_PROTOCOL))
    eq_(s.color, s2.color)
    eq_(s.width, s2.width)
    eq_(s.opacity, s2.opacity)
    eq_(s.get_dashes(), s2.get_dashes())
    eq_(s.line_cap, s2.line_cap)
    eq_(s.line_join, s2.line_join)

# LineSymbolizer pickling
def test_linesymbolizer_pickle():
    p = mapnik.LineSymbolizer()
    p2 = pickle.loads(pickle.dumps(p,pickle.HIGHEST_PROTOCOL))
    # line and stroke eq fails, so we compare attributes for now..
    s,s2 = p.stroke, p2.stroke
    eq_(s.color, s2.color)
    eq_(s.opacity, s2.opacity)
    eq_(s.width, s2.width)
    eq_(s.get_dashes(), s2.get_dashes())
    eq_(s.line_cap, s2.line_cap)
    eq_(s.line_join, s2.line_join)



# TextSymbolizer pickling
def test_textsymbolizer_pickle():
    ts = mapnik.TextSymbolizer(mapnik.Expression('[Field_Name]'), 'Font Name', 8, mapnik.Color('black'))

    eq_(str(ts.name), str(mapnik.Expression('[Field_Name]')))
    eq_(ts.face_name, 'Font Name')
    eq_(ts.text_size, 8)
    eq_(ts.fill, mapnik.Color('black'))
    
    raise Todo("text_symbolizer pickling currently disabled")

    ts2 = pickle.loads(pickle.dumps(ts,pickle.HIGHEST_PROTOCOL))
    eq_(ts.name, ts2.name)
    eq_(ts.face_name, ts2.face_name)
    eq_(ts.allow_overlap, ts2.allow_overlap)
    eq_(ts.displacement, ts2.displacement)
    eq_(ts.anchor, ts2.anchor)
    eq_(ts.fill, ts2.fill)
    eq_(ts.force_odd_labels, ts2.force_odd_labels)
    eq_(ts.halo_fill, ts2.halo_fill)
    eq_(ts.halo_radius, ts2.halo_radius)
    eq_(ts.label_placement, ts2.label_placement)
    eq_(ts.minimum_distance, ts2.minimum_distance)
    eq_(ts.text_ratio, ts2.text_ratio)
    eq_(ts.text_size, ts2.text_size)
    eq_(ts.wrap_width, ts2.wrap_width)
    eq_(ts.vertical_alignment, ts2.vertical_alignment)
    eq_(ts.label_spacing, ts2.label_spacing)
    eq_(ts.label_position_tolerance, ts2.label_position_tolerance)
    # 22.5 * M_PI/180.0 initialized by default
    assert_almost_equal(s.max_char_angle_delta, 0.39269908169872414)
    
    eq_(ts.wrap_character, ts2.wrap_character)
    eq_(ts.text_transform, ts2.text_transform)
    eq_(ts.line_spacing, ts2.line_spacing)
    eq_(ts.character_spacing, ts2.character_spacing)
    
    # r1341
    eq_(ts.wrap_before, ts2.wrap_before)
    eq_(ts.horizontal_alignment, ts2.horizontal_alignment)
    eq_(ts.justify_alignment, ts2.justify_alignment)
    eq_(ts.opacity, ts2.opacity)

    # r2300
    eq_(s.minimum_padding, 0.0)
        
    raise Todo("FontSet pickling support needed: http://trac.mapnik.org/ticket/348")
    eq_(ts.fontset, ts2.fontset)


def test_map_pickle():
    # Fails due to scale() not matching, possibly other things
    raise(Todo("Map does not support pickling yet (Tickets #345)."))

    m = mapnik.Map(256, 256)

    eq_(pickle.loads(pickle.dumps(m)), m)

    m = mapnik.Map(256, 256, '+proj=latlong')

    eq_(pickle.loads(pickle.dumps(m)), m)

def test_color_pickle():
    c = mapnik.Color('blue')

    eq_(pickle.loads(pickle.dumps(c)), c)

    c = mapnik.Color(0, 64, 128)

    eq_(pickle.loads(pickle.dumps(c)), c)

    c = mapnik.Color(0, 64, 128, 192)

    eq_(pickle.loads(pickle.dumps(c)), c)


def test_envelope_pickle():
    e = mapnik.Box2d(100, 100, 200, 200)

    eq_(pickle.loads(pickle.dumps(e)), e)


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
