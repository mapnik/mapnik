#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import eq_
from utilities import run_all
import mapnik
import json

# geojson box of the world
geojson  = { "type": "Feature", "properties": { }, "geometry": { "type": "Polygon", "coordinates": [ [ [ -17963313.143242701888084, -6300857.11560364998877 ], [ -17963313.143242701888084, 13071343.332991421222687 ], [ 7396658.353099936619401, 13071343.332991421222687 ], [ 7396658.353099936619401, -6300857.11560364998877 ], [ -17963313.143242701888084, -6300857.11560364998877 ] ] ] } }

def test_that_coordinates_do_not_overflow_and_polygon_is_rendered_memory():
  expected_color = mapnik.Color('white')
  projection = '+init=epsg:4326'
  ds = mapnik.MemoryDatasource()
  context = mapnik.Context()
  feat = mapnik.Feature.from_geojson(json.dumps(geojson),context)
  ds.add_feature(feat)
  s = mapnik.Style()
  r = mapnik.Rule()
  sym = mapnik.PolygonSymbolizer()
  sym.fill = expected_color
  r.symbols.append(sym)
  s.rules.append(r)
  lyr = mapnik.Layer('Layer',projection)
  lyr.datasource = ds
  lyr.styles.append('style')
  m = mapnik.Map(256,256,projection)
  m.background_color = mapnik.Color('green')
  m.append_style('style',s)
  m.layers.append(lyr)
  # 17/20864/45265.png
  m.zoom_to_box(mapnik.Box2d(-13658379.710221574,6197514.253362091,-13657768.213995293,6198125.749588372))
  # works 15/5216/11316.png
  #m.zoom_to_box(mapnik.Box2d(-13658379.710221574,6195679.764683247,-13655933.72531645,6198125.749588372))
  im = mapnik.Image(256,256)
  mapnik.render(m,im)
  eq_(im.get_pixel(128,128),expected_color.packed())

def test_that_coordinates_do_not_overflow_and_polygon_is_rendered_csv():
  expected_color = mapnik.Color('white')
  projection = '+init=epsg:4326'
  ds = mapnik.MemoryDatasource()
  context = mapnik.Context()
  feat = mapnik.Feature.from_geojson(json.dumps(geojson),context)
  ds.add_feature(feat)
  geojson_string = "geojson\n'%s'" % json.dumps(geojson['geometry'])
  ds = mapnik.Datasource(**{'type':'csv','inline':geojson_string})
  s = mapnik.Style()
  r = mapnik.Rule()
  sym = mapnik.PolygonSymbolizer()
  sym.fill = expected_color
  r.symbols.append(sym)
  s.rules.append(r)
  lyr = mapnik.Layer('Layer',projection)
  lyr.datasource = ds
  lyr.styles.append('style')
  m = mapnik.Map(256,256,projection)
  m.background_color = mapnik.Color('green')
  m.append_style('style',s)
  m.layers.append(lyr)
  # 17/20864/45265.png
  m.zoom_to_box(mapnik.Box2d(-13658379.710221574,6197514.253362091,-13657768.213995293,6198125.749588372))
  # works 15/5216/11316.png
  #m.zoom_to_box(mapnik.Box2d(-13658379.710221574,6195679.764683247,-13655933.72531645,6198125.749588372))
  im = mapnik.Image(256,256)
  mapnik.render(m,im)
  eq_(im.get_pixel(128,128),expected_color.packed())

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
