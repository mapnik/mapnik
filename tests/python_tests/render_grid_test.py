#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path
import os, mapnik

try:
    import json
except ImportError:
    import simplejson as json

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# first pass impl where resolution is passed as render
# time rather than encoding time, likely will be deprecated soon
grid_correct_old = {"keys": ["", "North West", "North East", "South West", "South East"], "data": {"South East": {"Name": "South East"}, "North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         !!!                                 ###                ", "        !!!!!                               #####               ", "        !!!!!                               #####               ", "         !!!                                 ###                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "        $$$$                                %%%%                ", "        $$$$$                               %%%%%               ", "        $$$$$                               %%%%%               ", "         $$$                                 %%%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "]}

# now using svg rendering
grid_correct_old2 = {"data": {"North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South East": {"Name": "South East"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         !!!                                 ###                ", "         !!!                                 ###                ", "         !!!                                 ###                ", "         !!!                                 ###                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         $$$                                 %%%                ", "         $$$                                 %%%                ", "         $$$                                 %%%                ", "         $$$                                 %%%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "North West", "North East", "South West", "South East"]}

# previous rendering using agg ellipse directly
grid_correct_new = {"data": {"North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South East": {"Name": "South East"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "North West", "North East", "South West", "South East"]}

# newer rendering using svg
grid_correct_new2 = {"data": {"North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South East": {"Name": "South East"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "North West", "North East", "South West", "South East"]}

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


def create_grid_map(width,height,marker=True):
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
    if marker:
        symb = mapnik.MarkersSymbolizer()
        symb.width = mapnik.Expression('10')
        symb.height = mapnik.Expression('10')
    else:
        symb = mapnik.PointSymbolizer(mapnik.PathExpression('../data/images/dummy.png'))
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

def show_grids(name,g1,g2):
    g1_file = '/tmp/mapnik-%s-actual.json' % name
    open(g1_file,'w').write(json.dumps(g1,sort_keys=True))
    g2_file = '/tmp/mapnik-%s-expected.json' % name
    open(g2_file,'w').write(json.dumps(g2,sort_keys=True))
    val = 'JSON does not match  ->\n'
    if g1['grid'] != g2['grid']:
       val += ' X grid does not match\n'
    else:
       val += ' ✓ grid matches\n'
    if g1['data'].keys() != g2['data'].keys():
       val += ' X data does not match\n'
    else:
       val += ' ✓ data matches\n'
    if g1['keys'] != g2['keys']:
       val += ' X keys do not\n'
    else:
       val += ' ✓ keys match\n'
    val += '\n\t%s\n\t%s' % (g1_file,g2_file)
    return val

def test_render_grid_old():
    """ test old method """
    width,height = 256,256
    m = create_grid_map(width,height)
    #print mapnik.save_map_to_string(m)
    ul_lonlat = mapnik.Coord(142.30,-38.20)
    lr_lonlat = mapnik.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
    grid = mapnik.render_grid(m,0,key='Name',resolution=4,fields=['Name'])
    eq_(grid,grid_correct_old2,show_grids('old-markers',grid,grid_correct_old2))
    eq_(resolve(grid,0,0),None)

    # check every pixel of the nw symbol
    expected = {"Name": "North West"}

    # top row
    eq_(resolve(grid,23,9),expected)
    eq_(resolve(grid,23,10),expected)
    eq_(resolve(grid,23,11),expected)

def test_render_grid_new():
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
    eq_(utf1,grid_correct_new2,show_grids('new-markers',utf1,grid_correct_new2))

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

grid_feat_id2 = {"data": {"1": {"Name": "South East"}, "2": {"Name": "South West"}, "3": {"Name": "North West"}, "4": {"Name": "North East"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "3", "4", "2", "1"]}

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
    eq_(utf1,grid_feat_id2,show_grids('id-markers',utf1,grid_feat_id2))
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


def gen_grid_for_id(pixel_key):
    ds = mapnik.MemoryDatasource()
    context = mapnik.Context()
    context.push('Name')
    f = mapnik.Feature(context,pixel_key)
    f['Name'] = str(pixel_key)
    f.add_geometries_from_wkt('POLYGON ((0 0, 0 256, 256 256, 256 0, 0 0))')
    ds.add_feature(f)
    s = mapnik.Style()
    r = mapnik.Rule()
    symb = mapnik.PolygonSymbolizer()
    r.symbols.append(symb)
    s.rules.append(r)
    lyr = mapnik.Layer('Places')
    lyr.datasource = ds
    lyr.styles.append('places_labels')
    width,height = 256,256
    m = mapnik.Map(width,height)
    m.append_style('places_labels',s)
    m.layers.append(lyr)
    m.zoom_all()
    grid = mapnik.Grid(m.width,m.height,key='__id__')
    mapnik.render_layer(m,grid,layer=0,fields=['__id__','Name'])
    return grid

def test_negative_id():
    grid = gen_grid_for_id(-1)
    eq_(grid.get_pixel(128,128),-1)
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1['keys'],['-1'])

def test_32bit_int_id():
    int32 = 2147483647
    grid = gen_grid_for_id(int32)
    eq_(grid.get_pixel(128,128),int32)
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1['keys'],[str(int32)])

    # this will fail because it is used internally to mark alpha
    #max_neg = -(int32+1)
    # so we use max neg-1
    max_neg = -(int32)
    grid = gen_grid_for_id(max_neg)
    eq_(grid.get_pixel(128,128),max_neg)
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1['keys'],[str(max_neg)])

def test_id_zero():
    grid = gen_grid_for_id(0)
    eq_(grid.get_pixel(128,128),0)
    utf1 = grid.encode('utf',resolution=4)
    eq_(utf1['keys'],['0'])

line_expected = {"keys": ["", "1"], "data": {"1": {"Name": "1"}}, "grid": ["                                                               !", "                                                            !!  ", "                                                         !!     ", "                                                      !!        ", "                                                   !!           ", "                                                !!              ", "                                             !!                 ", "                                          !!                    ", "                                       !!                       ", "                                    !!                          ", "                                 !!                             ", "                              !!                                ", "                           !!                                   ", "                        !!                                      ", "                     !!                                         ", "                  !!                                            ", "               !!                                               ", "            !!                                                  ", "         !!                                                     ", "      !!                                                        ", "   !!                                                           ", "!!                                                              ", " !                                                              ", "  !                                                             ", "   !                                                            ", "    !                                                           ", "     !                                                          ", "      !                                                         ", "       !                                                        ", "        !                                                       ", "         !                                                      ", "          !                                                     ", "           !                                                    ", "            !                                                   ", "             !                                                  ", "              !                                                 ", "               !                                                ", "                !                                               ", "                 !                                              ", "                  !                                             ", "                   !                                            ", "                    !                                           ", "                     !                                          ", "                      !                                         ", "                       !                                        ", "                        !                                       ", "                         !                                      ", "                          !                                     ", "                           !                                    ", "                            !                                   ", "                             !                                  ", "                              !                                 ", "                               !                                ", "                                !                               ", "                                 !                              ", "                                  !                             ", "                                   !                            ", "                                    !                           ", "                                     !                          ", "                                      !                         ", "                                       !                        ", "                                        !                       ", "                                         !                      ", "                                          !                     "]}

def test_line_rendering():
    ds = mapnik.MemoryDatasource()
    context = mapnik.Context()
    context.push('Name')
    pixel_key = 1
    f = mapnik.Feature(context,pixel_key)
    f['Name'] = str(pixel_key)
    f.add_geometries_from_wkt('LINESTRING (30 10, 10 30, 40 40)')
    ds.add_feature(f)
    s = mapnik.Style()
    r = mapnik.Rule()
    symb = mapnik.LineSymbolizer()
    r.symbols.append(symb)
    s.rules.append(r)
    lyr = mapnik.Layer('Places')
    lyr.datasource = ds
    lyr.styles.append('places_labels')
    width,height = 256,256
    m = mapnik.Map(width,height)
    m.append_style('places_labels',s)
    m.layers.append(lyr)
    m.zoom_all()
    #mapnik.render_to_file(m,'test.png')
    grid = mapnik.Grid(m.width,m.height,key='__id__')
    mapnik.render_layer(m,grid,layer=0,fields=['__id__','Name'])
    utf1 = grid.encode()
    eq_(utf1,line_expected,show_grids('line',utf1,line_expected))
    #open('test.json','w').write(json.dumps(grid.encode()))

point_expected = {"data": {"1": {"Name": "South East"}, "2": {"Name": "South West"}, "3": {"Name": "North West"}, "4": {"Name": "North East"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         !!!!                                ####               ", "         !!!!                                ####               ", "         !!!!                                ####               ", "         !!!!                                ####               ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         $$$$                                %%%%               ", "         $$$$                                %%%%               ", "         $$$$                                %%%%               ", "         $$$$                                %%%%               ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "3", "4", "2", "1"]}

def test_point_symbolizer_grid():
    width,height = 256,256
    m = create_grid_map(width,height,marker=False)
    ul_lonlat = mapnik.Coord(142.30,-38.20)
    lr_lonlat = mapnik.Coord(143.40,-38.80)
    m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
    #mapnik.render_to_file(m,'test.png')
    #print mapnik.save_map_to_string(m)
    grid = mapnik.Grid(m.width,m.height)
    mapnik.render_layer(m,grid,layer=0,fields=['Name'])
    utf1 = grid.encode()
    #open('test.json','w').write(json.dumps(grid.encode()))
    eq_(utf1,point_expected,show_grids('point-sym',utf1,point_expected))


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
