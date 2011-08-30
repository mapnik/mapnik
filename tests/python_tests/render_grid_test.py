#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *

import os, mapnik2
from utilities import Todo
import json


grid_correct = {"keys": ["", "North West", "North East", "South West", "South East"], "data": {"South East": {"Name": "South East"}, "North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         !!!                                 ###                ", "        !!!!!                               #####               ", "        !!!!!                               #####               ", "         !!!                                 ###                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "        $$$$                                %%%%                ", "        $$$$$                               %%%%%               ", "        $$$$$                               %%%%%               ", "         $$$                                 %%%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "]}


grid_correct_new = {"keys": ["", "North West", "North East", "South West", "South East"], "data": {"South East": {"Name": "South East"}, "North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         $$$                                  %%                ", "         $$$                                 %%%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "]}

def resolve(grid,x,y):
    """ Resolve the attributes for a given pixel in a grid.
    
    js version: 
      https://github.com/mapbox/mbtiles-spec/blob/master/1.1/utfgrid.md
    spec:
      https://github.com/mapbox/wax/blob/master/control/lib/gridutil.js
    
    """
    utf_val = grid['grid'][x][y]
    #http://docs.python.org/library/functions.html#ord
    codepoint = ord(utf_val)
    if (codepoint >= 93):
        codepoint-=1
    if (codepoint >= 35):
        codepoint-=1
    codepoint -= 32
    key = grid['keys'][codepoint]
    return grid['data'].get(key)


def create_grid_map(width,height):
    places_ds = mapnik2.PointDatasource()
    places_ds.add_point(143.10,-38.60,'Name','South East')
    places_ds.add_point(142.48,-38.60,'Name','South West')
    places_ds.add_point(142.48,-38.38,'Name','North West')
    places_ds.add_point(143.10,-38.38,'Name','North East')
    s = mapnik2.Style()
    r = mapnik2.Rule()
    #symb = mapnik2.PointSymbolizer()
    symb = mapnik2.MarkersSymbolizer()
    symb.allow_overlap = True
    r.symbols.append(symb)
    label = mapnik2.TextSymbolizer(mapnik2.Expression('[Name]'),
                'DejaVu Sans Book',
                10,
                mapnik2.Color('black')
                )
    label.allow_overlap = True
    label.displacement = (0,-10)
    #r.symbols.append(label)

    s.rules.append(r)
    lyr = mapnik2.Layer('Places')
    lyr.datasource = places_ds
    lyr.styles.append('places_labels')
    m = mapnik2.Map(width,height)
    m.append_style('places_labels',s)
    m.layers.append(lyr)
    return m
    
def test_render_grid():
    """ test old method """
    width,height = 256,256
    m = create_grid_map(width,height)
    ul_lonlat = mapnik2.Coord(142.30,-38.20)
    lr_lonlat = mapnik2.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik2.Box2d(ul_lonlat,lr_lonlat))
    grid = mapnik2.render_grid(m,0,key='Name',resolution=4,fields=['Name'])
    eq_(grid,grid_correct)
    eq_(resolve(grid,0,0),None)
    
    # check every pixel of the nw symbol
    expected = {"Name": "North West"}
    
    # top row
    eq_(resolve(grid,23,9),expected)
    eq_(resolve(grid,23,10),expected)
    eq_(resolve(grid,23,11),expected)

    # core
    eq_(resolve(grid,24,8),expected)
    eq_(resolve(grid,24,9),expected)
    eq_(resolve(grid,24,10),expected)
    eq_(resolve(grid,24,11),expected)
    eq_(resolve(grid,24,12),expected)
    eq_(resolve(grid,25,8),expected)
    eq_(resolve(grid,25,9),expected)
    eq_(resolve(grid,25,10),expected)
    eq_(resolve(grid,25,11),expected)
    eq_(resolve(grid,25,12),expected)
    
    # bottom row
    eq_(resolve(grid,26,9),expected)
    eq_(resolve(grid,26,10),expected)
    eq_(resolve(grid,26,11),expected)

def test_render_grid2():
    """ test old against new"""
    width,height = 256,256
    m = create_grid_map(width,height)
    ul_lonlat = mapnik2.Coord(142.30,-38.20)
    lr_lonlat = mapnik2.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik2.Box2d(ul_lonlat,lr_lonlat))

    # new method
    grid = mapnik2.Grid(m.width,m.height,key='Name')
    mapnik2.render_layer(m,grid,layer=0,fields=['Name'])
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1,grid_correct_new)

    # old method - to be removed
    utf2 = mapnik2.render_grid(m,0,key='Name',resolution=4,fields=['Name'])
    eq_(utf2,grid_correct)

    # for complex polygons these will not be true
    eq_(len(utf2['grid']),len(utf1['grid']))
    eq_(len(utf2['keys']),len(utf1['keys']))
    eq_(len(utf2['data']),len(utf1['data']))
    
    # check a full view is the same as a full image
    grid_view = grid.view(0,0,width,height)
    # for kicks check at full res too
    utf3 = grid.encode('utf',resolution=1)
    utf4 = grid_view.encode('utf',resolution=1)
    eq_(utf3['grid'],utf4['grid'])
    eq_(utf3['keys'],utf4['keys'])
    eq_(utf3['data'],utf4['data'])
    
    eq_(resolve(utf4,0,0),None)
    
    # resolve some center points in the
    # resampled view
    utf5 = grid_view.encode('utf',resolution=4)
    eq_(resolve(utf5,25,10),{"Name": "North West"})
    eq_(resolve(utf5,25,46),{"Name": "North East"})
    eq_(resolve(utf5,38,10),{"Name": "South West"})
    eq_(resolve(utf5,38,46),{"Name": "South East"})
    
if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
