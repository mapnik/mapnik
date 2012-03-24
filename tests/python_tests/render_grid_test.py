#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *

import os, mapnik

try:
    import json
except ImportError:
    import simplejson as json

grid_correct = {"keys": ["", "North West", "North East", "South West", "South East"], "data": {"South East": {"Name": "South East"}, "North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         !!!                                 ###                ", "        !!!!!                               #####               ", "        !!!!!                               #####               ", "         !!!                                 ###                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "        $$$$                                %%%%                ", "        $$$$$                               %%%%%               ", "        $$$$$                               %%%%%               ", "         $$$                                 %%%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "]}


grid_correct_new = {"keys": ["", "North West", "North East", "South West", "South East"], "data": {"South East": {"Name": "South East"}, "North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         $$$                                  %%                ", "         $$$                                 %%%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "]}

def resolve(grid,row,col):
    """ Resolve the attributes for a given pixel in a grid.
    """
    row = grid['grid'][row]
    utf_val = row[col]
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
    ds = mapnik.MemoryDatasource()
    context = mapnik.Context()
    context.push('Name')
    f = mapnik.Feature(context,1)
    f['Name'] = 'South East'
    f.add_geometries_from_wkt('POINT (143.10 -38.60)')
    ds.add_feature(f)

    f = mapnik.Feature(context,2)
    f['Name'] = 'South West'
    f.add_geometries_from_wkt('POINT (142.48 -38.60)')
    ds.add_feature(f)

    f = mapnik.Feature(context,3)
    f['Name'] = 'North West'
    f.add_geometries_from_wkt('POINT (142.48 -38.38)')
    ds.add_feature(f)

    f = mapnik.Feature(context,4)
    f['Name'] = 'North East'
    f.add_geometries_from_wkt('POINT (143.10 -38.38)')
    ds.add_feature(f)
    s = mapnik.Style()
    r = mapnik.Rule()
    symb = mapnik.MarkersSymbolizer()
    symb.width = 10
    symb.height = 10
    symb.allow_overlap = True
    r.symbols.append(symb)

    s.rules.append(r)
    lyr = mapnik.Layer('Places')
    lyr.datasource = ds
    lyr.styles.append('places_labels')
    m = mapnik.Map(width,height)
    m.append_style('places_labels',s)
    m.layers.append(lyr)
    return m

def test_render_grid():
    """ test old method """
    width,height = 256,256
    m = create_grid_map(width,height)
    ul_lonlat = mapnik.Coord(142.30,-38.20)
    lr_lonlat = mapnik.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
    grid = mapnik.render_grid(m,0,key='Name',resolution=4,fields=['Name'])
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
    ul_lonlat = mapnik.Coord(142.30,-38.20)
    lr_lonlat = mapnik.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))

    # new method
    grid = mapnik.Grid(m.width,m.height,key='Name')
    mapnik.render_layer(m,grid,layer=0,fields=['Name'])
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1,grid_correct_new)

    # old method - to be removed
    utf2 = mapnik.render_grid(m,0,key='Name',resolution=4,fields=['Name'])
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


grid_feat_id = {'keys': ['', '3', '4', '2', '1'], 'data': {'1': {'Name': 'South East'}, '3': {'Name': u'North West'}, '2': {'Name': 'South West'}, '4': {'Name': 'North East'}}, 'grid': ['                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '          !!                                  ##                ', '         !!!                                 ###                ', '          !!                                  ##                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '         $$$                                  %%                ', '         $$$                                 %%%                ', '          $$                                  %%                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ', '                                                                ']}

def test_render_grid3():
    """ test using feature id"""
    width,height = 256,256
    m = create_grid_map(width,height)
    ul_lonlat = mapnik.Coord(142.30,-38.20)
    lr_lonlat = mapnik.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))

    grid = mapnik.Grid(m.width,m.height,key='__id__')
    mapnik.render_layer(m,grid,layer=0,fields=['__id__','Name'])
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1['keys'],grid_feat_id['keys'])
    eq_(utf1['grid'],grid_feat_id['grid'])
    eq_(utf1['data'],grid_feat_id['data'])
    eq_(utf1,grid_feat_id)
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
