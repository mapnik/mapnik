#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik

try:
    import json
except ImportError:
    import simplejson as json

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if mapnik.has_grid_renderer():
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
    
    def show_grids2(name,g1,g2):
        g2_expected = '../data/grids/mapnik-%s-actual.json' % name
        if not os.path.exists(g2_expected):
            # create test fixture based on actual results
            open(g2_expected,'a+').write(json.dumps(g1,sort_keys=True))
            return
        g1_file = '/tmp/mapnik-%s-actual.json' % name
        open(g1_file,'w').write(json.dumps(g1,sort_keys=True))
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
        val += '\n\t%s\n\t%s' % (g1_file,g2_expected)
        return val
    
    # previous rendering using agg ellipse directly
    grid_correct_new = {"data": {"North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South East": {"Name": "South East"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "North West", "North East", "South West", "South East"]}
    
    # newer rendering using svg
    grid_correct_new2 = {"data": {"North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South East": {"Name": "South East"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $$                                  %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "North West", "North East", "South West", "South East"]}
    
    grid_correct_new3 = {"data": {"North East": {"Name": "North East"}, "North West": {"Name": "North West"}, "South East": {"Name": "South East"}, "South West": {"Name": "South West"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $                                   %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "North West", "North East", "South West", "South East"]}
    
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
    
    
    def create_grid_map(width,height,sym):
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
        sym.allow_overlap = True
        r.symbols.append(sym)
        s.rules.append(r)
        lyr = mapnik.Layer('Places')
        lyr.datasource = ds
        lyr.styles.append('places_labels')
        m = mapnik.Map(width,height)
        m.append_style('places_labels',s)
        m.layers.append(lyr)
        return m
    
    
    def test_render_grid():
        """ test render_grid method"""
        width,height = 256,256
        sym = mapnik.MarkersSymbolizer()
        sym.width = mapnik.Expression('10')
        sym.height = mapnik.Expression('10')
        m = create_grid_map(width,height,sym)
        ul_lonlat = mapnik.Coord(142.30,-38.20)
        lr_lonlat = mapnik.Coord(143.40,-38.80)
        m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
    
        # new method
        grid = mapnik.Grid(m.width,m.height,key='Name')
        mapnik.render_layer(m,grid,layer=0,fields=['Name'])
        utf1 = grid.encode('utf',resolution=4)
        eq_(utf1,grid_correct_new3,show_grids('new-markers',utf1,grid_correct_new3))
    
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
    
    grid_feat_id3 = {"data": {"1": {"Name": "South East", "__id__": 1}, "2": {"Name": "South West", "__id__": 2}, "3": {"Name": "North West", "__id__": 3}, "4": {"Name": "North East", "__id__": 4}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          !!                                  ##                ", "         !!!                                 ###                ", "          !!                                  ##                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "          $$                                  %%                ", "         $$$                                  %%                ", "          $                                   %%                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "3", "4", "2", "1"]}
    
    def test_render_grid3():
        """ test using feature id"""
        width,height = 256,256
        sym = mapnik.MarkersSymbolizer()
        sym.width = mapnik.Expression('10')
        sym.height = mapnik.Expression('10')
        m = create_grid_map(width,height,sym)
        ul_lonlat = mapnik.Coord(142.30,-38.20)
        lr_lonlat = mapnik.Coord(143.40,-38.80)
        m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
    
        grid = mapnik.Grid(m.width,m.height,key='__id__')
        mapnik.render_layer(m,grid,layer=0,fields=['__id__','Name'])
        utf1 = grid.encode('utf',resolution=4)
        eq_(utf1,grid_feat_id3,show_grids('id-markers',utf1,grid_feat_id3))
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
        eq_(resolve(utf5,25,10),{"Name": "North West","__id__": 3})
        eq_(resolve(utf5,25,46),{"Name": "North East","__id__": 4})
        eq_(resolve(utf5,38,10),{"Name": "South West","__id__": 2})
        eq_(resolve(utf5,38,46),{"Name": "South East","__id__": 1})
    
    
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
        max_neg = -(int32)
        grid = gen_grid_for_id(max_neg)
        eq_(grid.get_pixel(128,128),max_neg)
        utf1 = grid.encode('utf',resolution=4)
        eq_(utf1['keys'],[str(max_neg)])
    
    def test_64bit_int_id():
        int64 = 0x7FFFFFFFFFFFFFFF
        grid = gen_grid_for_id(int64)
        eq_(grid.get_pixel(128,128),int64)
        utf1 = grid.encode('utf',resolution=4)
        eq_(utf1['keys'],[str(int64)])
        max_neg = -(int64)
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
        mapnik.render_layer(m,grid,layer=0,fields=['Name'])
        utf1 = grid.encode()
        eq_(utf1,line_expected,show_grids('line',utf1,line_expected))
    
    point_expected = {"data": {"1": {"Name": "South East"}, "2": {"Name": "South West"}, "3": {"Name": "North West"}, "4": {"Name": "North East"}}, "grid": ["                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         !!!!                                ####               ", "         !!!!                                ####               ", "         !!!!                                ####               ", "         !!!!                                ####               ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "         $$$$                                %%%%               ", "         $$$$                                %%%%               ", "         $$$$                                %%%%               ", "         $$$$                                %%%%               ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                ", "                                                                "], "keys": ["", "3", "4", "2", "1"]}
    
    def test_point_symbolizer_grid():
        width,height = 256,256
        sym = mapnik.PointSymbolizer()
        sym.file = '../data/images/dummy.png'
        m = create_grid_map(width,height,sym)
        ul_lonlat = mapnik.Coord(142.30,-38.20)
        lr_lonlat = mapnik.Coord(143.40,-38.80)
        m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
        grid = mapnik.Grid(m.width,m.height)
        mapnik.render_layer(m,grid,layer=0,fields=['Name'])
        utf1 = grid.encode()
        eq_(utf1,point_expected,show_grids('point-sym',utf1,point_expected))
    
    
    # should throw because this is a mis-usage
    # https://github.com/mapnik/mapnik/issues/1325
    @raises(RuntimeError)
    def test_render_to_grid_multiple_times():
        # create map with two layers
        m = mapnik.Map(256,256)
        s = mapnik.Style()
        r = mapnik.Rule()
        sym = mapnik.MarkersSymbolizer()
        sym.allow_overlap = True
        r.symbols.append(sym)
        s.rules.append(r)
        m.append_style('points',s)
    
        # NOTE: we use a csv datasource here
        # because the memorydatasource fails silently for
        # queries requesting fields that do not exist in the datasource
        ds1 = mapnik.Datasource(**{"type":"csv","inline":'''
          wkt,Name
          "POINT (143.10 -38.60)",South East'''})
        lyr1 = mapnik.Layer('One')
        lyr1.datasource = ds1
        lyr1.styles.append('points')
        m.layers.append(lyr1)
    
        ds2 = mapnik.Datasource(**{"type":"csv","inline":'''
          wkt,Value
          "POINT (142.48 -38.60)",South West'''})
        lyr2 = mapnik.Layer('Two')
        lyr2.datasource = ds2
        lyr2.styles.append('points')
        m.layers.append(lyr2)
    
        ul_lonlat = mapnik.Coord(142.30,-38.20)
        lr_lonlat = mapnik.Coord(143.40,-38.80)
        m.zoom_to_box(mapnik.Box2d(ul_lonlat,lr_lonlat))
        grid = mapnik.Grid(m.width,m.height)
        mapnik.render_layer(m,grid,layer=0,fields=['Name'])
        # should throw right here since Name will be a property now on the `grid` object
        # and it is not found on the second layer
        mapnik.render_layer(m,grid,layer=1,fields=['Value'])
        utf1 = grid.encode()

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
