#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path, run_all, Todo
from mapnik import *

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))


def test_charplacement():
    m = Map(690,690,"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs")
    m.background = Color(255,100,100,255)
    road_style = Style()
    road_rule = Rule()
    road_stroke = Stroke(Color('white'), 1)
    road_stroke.opacity = 0.7
    road_rule.symbols.append(LineSymbolizer(road_stroke))
    road_style.rules.append(road_rule);
    text_symbolizer = TextSymbolizer(Expression('[NAME]'), 'DejaVu Sans Book', 20, Color('black'))
    text_symbolizer.label_placement=label_placement.LINE_PLACEMENT
    text_symbolizer.minimum_distance = 0
    text_symbolizer.label_spacing = 20
    text_symbolizer.label_position_tolerance = 50
    text_symbolizer.minimum_distance = 5
    text_symbolizer.avoid_edges = 0
    text_symbolizer.halo_fill = Color('yellow')
    text_symbolizer.halo_radius = 1
    road_rule = Rule()
    road_rule.symbols.append(text_symbolizer)
    road_style.rules.append(road_rule)
    road_layer = Layer('road')
    road_layer.datasource = Shapefile(file='../data/shp/charplacement')
    m.append_style('road', road_style)
    road_layer.styles.append('road')
    m.layers.append(road_layer)
    m.zoom_to_box(Box2d(0,0,14,-14))
    im = Image(m.width,m.height)
    render(m, im)
    actual = '/tmp/mapnik-char_placement.png'
    expected = 'images/support/char_placement.png'
    im.save(actual)
    if not os.path.exists(expected):
        print 'generating expected test image: %s' % expected
        im.save(expected)
    expected_im = Image.open(expected)
    eq_(Image.open(actual).tostring('png32'),expected_im.tostring('png32'),'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

def test_overlap():
    m = Map(690,690,"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs")
    m.background = Color(255,100,100,255)
    road_style = Style()
    road_rule = Rule()
    road_stroke = Stroke(Color('white'), 12)
    road_stroke.opacity = 0.7
    road_rule.symbols.append(LineSymbolizer(road_stroke))
    road_style.rules.append(road_rule);
    text_symbolizer = TextSymbolizer(Expression('[NAME]'), 'DejaVu Sans Book', 10, Color('black'))
    text_symbolizer.label_placement=label_placement.LINE_PLACEMENT
    text_symbolizer.minimum_distance = 0
    text_symbolizer.label_spacing = 60
    text_symbolizer.label_position_tolerance = 50
    text_symbolizer.minimum_distance = 5
    text_symbolizer.avoid_edges = 0
    text_symbolizer.halo_fill = Color('yellow')
    text_symbolizer.halo_radius = 1
    road_rule = Rule()
    road_rule.symbols.append(text_symbolizer)
    road_style.rules.append(road_rule)
    road_layer = Layer('road')
    road_layer.datasource = Shapefile(file='../data/shp/overlap')
    m.append_style('road', road_style)
    road_layer.styles.append('road')
    m.layers.append(road_layer)
    m.zoom_to_box(Box2d(0,0,14,-14))
    im = Image(m.width,m.height)
    render(m, im)
    actual = '/tmp/mapnik-overlap.png'
    expected = 'images/support/overlap.png'
    im.save(actual)
    if not os.path.exists(expected):
        print 'generating expected test image: %s' % expected
        im.save(expected)
    expected_im = Image.open(expected)
    eq_(Image.open(actual).tostring('png32'),expected_im.tostring('png32'),'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

def test_displacement():
    m = Map(690,690,"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs")
    m.background = Color(255,100,100,255)
    road_rule = Rule()
    road_style = Style()
    road_rule.symbols.append(LineSymbolizer(Stroke(Color("white"),.5)))
    road_style.rules.append(road_rule);
    text_symbolizer = TextSymbolizer(Expression('[NAME]'), 'DejaVu Sans Book', 10, Color('black'))
    text_symbolizer.label_placement=label_placement.LINE_PLACEMENT
    text_symbolizer.minimum_distance = 0
    text_symbolizer.label_spacing = 60
    text_symbolizer.label_position_tolerance = 5
    text_symbolizer.avoid_edges = 0
    text_symbolizer.halo_fill = Color('yellow')
    text_symbolizer.halo_radius = 1
    text_symbolizer.displacement = (0,5)
    road_rule.symbols.append(text_symbolizer)
    road_style.rules.append(road_rule)
    road_layer = Layer('road')
    road_layer.datasource = Shapefile(file='../data/shp/displacement')
    m.append_style('road', road_style)
    road_layer.styles.append('road')
    m.layers.append(road_layer)
    m.zoom_to_box(Box2d(0,0,14,-14))
    im = Image(m.width,m.height)
    render(m, im)
    actual = '/tmp/mapnik-displacement.png'
    expected = 'images/support/displacement.png'
    im.save(actual)
    if not os.path.exists(expected):
        print 'generating expected test image: %s' % expected
        im.save(expected)
    expected_im = Image.open(expected)
    eq_(Image.open(actual).tostring('png32'),expected_im.tostring('png32'),'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

def test_textspacing():
    m = Map(690,690,"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs")
    m.background = Color(255,100,100,255)
    road_style = Style()
    road_rule = Rule()
    road_stroke = Stroke(Color('white'), 12)
    road_stroke.line_cap = line_cap.ROUND_CAP
    road_stroke.line_join = line_join.ROUND_JOIN
    road_rule.symbols.append(LineSymbolizer(road_stroke))
    road_style.rules.append(road_rule);
    text_symbolizer = TextSymbolizer(Expression('[NAME]'), 'DejaVu Sans Book', 10, Color('black'))
    text_symbolizer.label_placement=label_placement.LINE_PLACEMENT
    text_symbolizer.minimum_distance = 0
    text_symbolizer.label_spacing = 80
    text_symbolizer.label_position_tolerance = 5
    text_symbolizer.avoid_edges = 0
    text_symbolizer.halo_fill = Color('yellow')
    text_symbolizer.halo_radius = 1
    road_rule = Rule()
    road_rule.symbols.append(text_symbolizer)
    road_style.rules.append(road_rule)
    road_layer = Layer('road')
    road_layer.datasource = Shapefile(file='../data/shp/textspacing')
    m.append_style('road', road_style)
    road_layer.styles.append('road')
    m.layers.append(road_layer)
    m.zoom_to_box(Box2d(0,0,14,-14))
    im = Image(m.width,m.height)
    render(m, im)
    actual = '/tmp/mapnik-textspacing.png'
    expected = 'images/support/textspacing.png'
    im.save(actual)
    if not os.path.exists(expected):
        print 'generating expected test image: %s' % expected
        im.save(expected)
    expected_im = Image.open(expected)
    eq_(Image.open(actual).tostring('png32'),expected_im.tostring('png32'),'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

if __name__ == "__main__":
    setup()
    run_all(eval(x) for x in dir() if x.startswith("test_"))
