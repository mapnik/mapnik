# #!/usr/bin/env python
# # -*- coding: utf-8 -*-

# import os
# from nose.tools import *
# from utilities import execution_path, run_all
# import tempfile

# import mapnik

# def setup():
#     # All of the paths used are relative, if we run the tests
#     # from another directory we need to chdir()
#     os.chdir(execution_path('.'))

# def test_debug_symbolizer():
#     s = mapnik.DebugSymbolizer()
#     eq_(s.mode,mapnik.debug_symbolizer_mode.collision)

# def test_raster_symbolizer():
#     s = mapnik.RasterSymbolizer()
#     eq_(s.comp_op,mapnik.CompositeOp.src_over) # note: mode is deprecated
#     eq_(s.scaling,mapnik.scaling_method.NEAR)
#     eq_(s.opacity,1.0)
#     eq_(s.colorizer,None)
#     eq_(s.filter_factor,-1)
#     eq_(s.mesh_size,16)
#     eq_(s.premultiplied,None)
#     s.premultiplied = True
#     eq_(s.premultiplied,True)

# def test_line_pattern():
#     s = mapnik.LinePatternSymbolizer(mapnik.PathExpression('../data/images/dummy.png'))
#     eq_(s.filename, '../data/images/dummy.png')
#     eq_(s.smooth,0.0)
#     eq_(s.transform,'')
#     eq_(s.offset,0.0)
#     eq_(s.comp_op,mapnik.CompositeOp.src_over)
#     eq_(s.clip,True)

# def test_line_symbolizer():
#     s = mapnik.LineSymbolizer()
#     eq_(s.rasterizer, mapnik.line_rasterizer.FULL)
#     eq_(s.smooth,0.0)
#     eq_(s.comp_op,mapnik.CompositeOp.src_over)
#     eq_(s.clip,True)
#     eq_(s.stroke.width, 1)
#     eq_(s.stroke.opacity, 1)
#     eq_(s.stroke.color, mapnik.Color('black'))
#     eq_(s.stroke.line_cap, mapnik.line_cap.BUTT_CAP)
#     eq_(s.stroke.line_join, mapnik.line_join.MITER_JOIN)

#     l = mapnik.LineSymbolizer(mapnik.Color('blue'), 5.0)

#     eq_(l.stroke.width, 5)
#     eq_(l.stroke.opacity, 1)
#     eq_(l.stroke.color, mapnik.Color('blue'))
#     eq_(l.stroke.line_cap, mapnik.line_cap.BUTT_CAP)
#     eq_(l.stroke.line_join, mapnik.line_join.MITER_JOIN)

#     s = mapnik.Stroke(mapnik.Color('blue'), 5.0)
#     l = mapnik.LineSymbolizer(s)

#     eq_(l.stroke.width, 5)
#     eq_(l.stroke.opacity, 1)
#     eq_(l.stroke.color, mapnik.Color('blue'))
#     eq_(l.stroke.line_cap, mapnik.line_cap.BUTT_CAP)
#     eq_(l.stroke.line_join, mapnik.line_join.MITER_JOIN)

# def test_line_symbolizer_stroke_reference():
#     l = mapnik.LineSymbolizer(mapnik.Color('green'),0.1)
#     l.stroke.add_dash(.1,.1)
#     l.stroke.add_dash(.1,.1)
#     eq_(l.stroke.get_dashes(), [(.1,.1),(.1,.1)])
#     eq_(l.stroke.color,mapnik.Color('green'))
#     eq_(l.stroke.opacity,1.0)
#     assert_almost_equal(l.stroke.width,0.1)

# # https://github.com/mapnik/mapnik/issues/1427
# def test_stroke_dash_api():
#     stroke = mapnik.Stroke()
#     dashes = [(1.0,1.0)]
#     stroke.dasharray = dashes
#     eq_(stroke.dasharray, dashes)
#     stroke.add_dash(.1,.1)
#     dashes.append((.1,.1))
#     eq_(stroke.dasharray, dashes)


# def test_text_symbolizer():
#     s = mapnik.TextSymbolizer()
#     eq_(s.comp_op,mapnik.CompositeOp.src_over)
#     eq_(s.clip,True)
#     eq_(s.halo_rasterizer,mapnik.halo_rasterizer.FULL)

#     # https://github.com/mapnik/mapnik/issues/1420
#     eq_(s.text_transform, mapnik.text_transform.NONE)

#     # old args required method
#     ts = mapnik.TextSymbolizer(mapnik.Expression('[Field_Name]'), 'Font Name', 8, mapnik.Color('black'))
# #    eq_(str(ts.name), str(mapnik2.Expression('[Field_Name]'))) name field is no longer supported
#     eq_(ts.format.face_name, 'Font Name')
#     eq_(ts.format.text_size, 8)
#     eq_(ts.format.fill, mapnik.Color('black'))
#     eq_(ts.properties.label_placement, mapnik.label_placement.POINT_PLACEMENT)
#     eq_(ts.properties.horizontal_alignment, mapnik.horizontal_alignment.AUTO)

# def test_shield_symbolizer_init():
#     s = mapnik.ShieldSymbolizer(mapnik.Expression('[Field Name]'), 'DejaVu Sans Bold', 6, mapnik.Color('#000000'), mapnik.PathExpression('../data/images/dummy.png'))
#     eq_(s.comp_op,mapnik.CompositeOp.src_over)
#     eq_(s.clip,True)
#     eq_(s.displacement, (0.0,0.0))
#     eq_(s.allow_overlap, False)
#     eq_(s.avoid_edges, False)
#     eq_(s.character_spacing,0)
#     #eq_(str(s.name), str(mapnik2.Expression('[Field Name]'))) name field is no longer supported
#     eq_(s.face_name, 'DejaVu Sans Bold')
#     eq_(s.allow_overlap, False)
#     eq_(s.fill, mapnik.Color('#000000'))
#     eq_(s.halo_fill, mapnik.Color('rgb(255,255,255)'))
#     eq_(s.halo_radius, 0)
#     eq_(s.label_placement, mapnik.label_placement.POINT_PLACEMENT)
#     eq_(s.minimum_distance, 0.0)
#     eq_(s.text_ratio, 0)
#     eq_(s.text_size, 6)
#     eq_(s.wrap_width, 0)
#     eq_(s.vertical_alignment, mapnik.vertical_alignment.AUTO)
#     eq_(s.label_spacing, 0)
#     eq_(s.label_position_tolerance, 0)
#     # 22.5 * M_PI/180.0 initialized by default
#     assert_almost_equal(s.max_char_angle_delta, 0.39269908169872414)

#     eq_(s.text_transform, mapnik.text_transform.NONE)
#     eq_(s.line_spacing, 0)
#     eq_(s.character_spacing, 0)

#     # r1341
#     eq_(s.wrap_before, False)
#     eq_(s.horizontal_alignment, mapnik.horizontal_alignment.AUTO)
#     eq_(s.justify_alignment, mapnik.justify_alignment.AUTO)
#     eq_(s.opacity, 1.0)

#     # r2300
#     eq_(s.minimum_padding, 0.0)

#     # was mixed with s.opacity
#     eq_(s.text_opacity, 1.0)

#     eq_(s.shield_displacement, (0.0,0.0))
#     # TODO - the pattern in bindings seems to be to get/set
#     # strings for PathExpressions... should we pass objects?
#     eq_(s.filename, '../data/images/dummy.png')

#     # 11c34b1: default transform list is empty, not identity matrix
#     eq_(s.transform, '')

#     eq_(s.fontset, None)

# # ShieldSymbolizer missing image file
# # images paths are now PathExpressions are evaluated at runtime
# # so it does not make sense to throw...
# #@raises(RuntimeError)
# #def test_shieldsymbolizer_missing_image():
# #    s = mapnik.ShieldSymbolizer(mapnik.Expression('[Field Name]'), 'DejaVu Sans Bold', 6, mapnik.Color('#000000'), mapnik.PathExpression('../#data/images/broken.png'))

# def test_shield_symbolizer_modify():
#     s = mapnik.ShieldSymbolizer(mapnik.Expression('[Field Name]'), 'DejaVu Sans Bold', 6, mapnik.Color('#000000'), mapnik.PathExpression('../data/images/dummy.png'))
#     # transform expression
#     def check_transform(expr, expect_str=None):
#         s.transform = expr
#         eq_(s.transform, expr if expect_str is None else expect_str)
#     check_transform("matrix(1 2 3 4 5 6)", "matrix(1, 2, 3, 4, 5, 6)")
#     check_transform("matrix(1, 2, 3, 4, 5, 6 +7)", "matrix(1, 2, 3, 4, 5, (6+7))")
#     check_transform("rotate([a])")
#     check_transform("rotate([a] -2)", "rotate(([a]-2))")
#     check_transform("rotate([a] -2 -3)", "rotate([a], -2, -3)")
#     check_transform("rotate([a] -2 -3 -4)", "rotate(((([a]-2)-3)-4))")
#     check_transform("rotate([a] -2, 3, 4)", "rotate(([a]-2), 3, 4)")
#     check_transform("translate([tx]) rotate([a])")
#     check_transform("scale([sx], [sy]/2)")
#     # TODO check expected failures

# def test_point_symbolizer():
#     p = mapnik.PointSymbolizer()
#     eq_(p.filename,'')
#     eq_(p.transform,'')
#     eq_(p.opacity,1.0)
#     eq_(p.allow_overlap,False)
#     eq_(p.ignore_placement,False)
#     eq_(p.comp_op,mapnik.CompositeOp.src_over)
#     eq_(p.placement, mapnik.point_placement.CENTROID)

#     p = mapnik.PointSymbolizer(mapnik.PathExpression("../data/images/dummy.png"))
#     p.allow_overlap = True
#     p.opacity = 0.5
#     p.ignore_placement = True
#     p.placement = mapnik.point_placement.INTERIOR
#     eq_(p.allow_overlap, True)
#     eq_(p.opacity, 0.5)
#     eq_(p.filename,'../data/images/dummy.png')
#     eq_(p.ignore_placement,True)
#     eq_(p.placement, mapnik.point_placement.INTERIOR)

# def test_markers_symbolizer():
#     p = mapnik.MarkersSymbolizer()
#     eq_(p.allow_overlap, False)
#     eq_(p.opacity,1.0)
#     eq_(p.fill_opacity,None)
#     eq_(p.filename,'shape://ellipse')
#     eq_(p.placement,mapnik.marker_placement.POINT_PLACEMENT)
#     eq_(p.multi_policy,mapnik.marker_multi_policy.EACH)
#     eq_(p.fill,None)
#     eq_(p.ignore_placement,False)
#     eq_(p.spacing,100)
#     eq_(p.max_error,0.2)
#     eq_(p.width,None)
#     eq_(p.height,None)
#     eq_(p.transform,'')
#     eq_(p.clip,True)
#     eq_(p.comp_op,mapnik.CompositeOp.src_over)


#     p.width = mapnik.Expression('12')
#     p.height = mapnik.Expression('12')
#     eq_(str(p.width),'12')
#     eq_(str(p.height),'12')

#     p.width = mapnik.Expression('[field] + 2')
#     p.height = mapnik.Expression('[field] + 2')
#     eq_(str(p.width),'([field]+2)')
#     eq_(str(p.height),'([field]+2)')

#     stroke = mapnik.Stroke()
#     stroke.color = mapnik.Color('black')
#     stroke.width = 1.0

#     p.stroke = stroke
#     p.fill = mapnik.Color('white')
#     p.allow_overlap = True
#     p.opacity = 0.5
#     p.fill_opacity = 0.5
#     p.placement = mapnik.marker_placement.LINE_PLACEMENT
#     p.multi_policy = mapnik.marker_multi_policy.WHOLE

#     eq_(p.allow_overlap, True)
#     eq_(p.opacity, 0.5)
#     eq_(p.fill_opacity, 0.5)
#     eq_(p.multi_policy,mapnik.marker_multi_policy.WHOLE)
#     eq_(p.placement,mapnik.marker_placement.LINE_PLACEMENT)

#     #https://github.com/mapnik/mapnik/issues/1285
#     #https://github.com/mapnik/mapnik/issues/1427
#     p.marker_type = 'arrow'
#     eq_(p.marker_type,'shape://arrow')
#     eq_(p.filename,'shape://arrow')


# # PointSymbolizer missing image file
# # images paths are now PathExpressions are evaluated at runtime
# # so it does not make sense to throw...
# #@raises(RuntimeError)
# #def test_pointsymbolizer_missing_image():
#  #   p = mapnik.PointSymbolizer(mapnik.PathExpression("../data/images/broken.png"))

# def test_polygon_symbolizer():
#     p = mapnik.PolygonSymbolizer()
#     eq_(p.smooth,0.0)
#     eq_(p.comp_op,mapnik.CompositeOp.src_over)
#     eq_(p.clip,True)
#     eq_(p.fill, mapnik.Color('gray'))
#     eq_(p.fill_opacity, 1)

#     p = mapnik.PolygonSymbolizer(mapnik.Color('blue'))

#     eq_(p.fill, mapnik.Color('blue'))
#     eq_(p.fill_opacity, 1)

# def test_building_symbolizer_init():
#     p = mapnik.BuildingSymbolizer()

#     eq_(p.fill, mapnik.Color('gray'))
#     eq_(p.fill_opacity, 1)
#     eq_(p.height,None)

# def test_group_symbolizer_init():
#     s = mapnik.GroupSymbolizer()

#     p = mapnik.GroupSymbolizerProperties()

#     l = mapnik.PairLayout()
#     l.item_margin = 5.0
#     p.set_layout(l)

#     r = mapnik.GroupRule(mapnik.Expression("[name%1]"))
#     r.append(mapnik.PointSymbolizer())
#     p.add_rule(r)
#     s.symbolizer_properties = p

#     eq_(s.comp_op,mapnik.CompositeOp.src_over)

# def test_stroke_init():
#     s = mapnik.Stroke()

#     eq_(s.width, 1)
#     eq_(s.opacity, 1)
#     eq_(s.color, mapnik.Color('black'))
#     eq_(s.line_cap, mapnik.line_cap.BUTT_CAP)
#     eq_(s.line_join, mapnik.line_join.MITER_JOIN)
#     eq_(s.gamma,1.0)

#     s = mapnik.Stroke(mapnik.Color('blue'), 5.0)
#     s.gamma = .5

#     eq_(s.width, 5)
#     eq_(s.opacity, 1)
#     eq_(s.color, mapnik.Color('blue'))
#     eq_(s.gamma, .5)
#     eq_(s.line_cap, mapnik.line_cap.BUTT_CAP)
#     eq_(s.line_join, mapnik.line_join.MITER_JOIN)

# def test_stroke_dash_arrays():
#     s = mapnik.Stroke()
#     s.add_dash(1,2)
#     s.add_dash(3,4)
#     s.add_dash(5,6)

#     eq_(s.get_dashes(), [(1,2),(3,4),(5,6)])

# def test_map_init():
#     m = mapnik.Map(256, 256)

#     eq_(m.width, 256)
#     eq_(m.height, 256)
#     eq_(m.srs, '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')
#     eq_(m.base, '')
#     eq_(m.maximum_extent, None)
#     eq_(m.background_image, None)
#     eq_(m.background_image_comp_op, mapnik.CompositeOp.src_over)
#     eq_(m.background_image_opacity, 1.0)

#     m = mapnik.Map(256, 256, '+proj=latlong')
#     eq_(m.srs, '+proj=latlong')

# def test_map_style_access():
#     m = mapnik.Map(256, 256)
#     sty = mapnik.Style()
#     m.append_style("style",sty)
#     styles = list(m.styles)
#     eq_(len(styles),1)
#     eq_(styles[0][0],'style')
#     # returns a copy so let's just check it is the right instance
#     eq_(isinstance(styles[0][1],mapnik.Style),True)

# def test_map_maximum_extent_modification():
#     m = mapnik.Map(256, 256)
#     eq_(m.maximum_extent, None)
#     m.maximum_extent = mapnik.Box2d()
#     eq_(m.maximum_extent, mapnik.Box2d())
#     m.maximum_extent = None
#     eq_(m.maximum_extent, None)

# # Map initialization from string
# def test_map_init_from_string():
#     map_string = '''<Map background-color="steelblue" base="./" srs="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs">
#      <Style name="My Style">
#       <Rule>
#        <PolygonSymbolizer fill="#f2eff9"/>
#        <LineSymbolizer stroke="rgb(50%,50%,50%)" stroke-width="0.1"/>
#       </Rule>
#      </Style>
#      <Layer name="boundaries">
#       <StyleName>My Style</StyleName>
#        <Datasource>
#         <Parameter name="type">shape</Parameter>
#         <Parameter name="file">../../demo/data/boundaries</Parameter>
#        </Datasource>
#       </Layer>
#     </Map>'''

#     m = mapnik.Map(600, 300)
#     eq_(m.base, '')
#     try:
#         mapnik.load_map_from_string(m, map_string)
#         eq_(m.base, './')
#         mapnik.load_map_from_string(m, map_string, False, "") # this "" will have no effect
#         eq_(m.base, './')

#         tmp_dir = tempfile.gettempdir()
#         try:
#             mapnik.load_map_from_string(m, map_string, False, tmp_dir)
#         except RuntimeError:
#             pass # runtime error expected because shapefile path should be wrong and datasource will throw
#         eq_(m.base, tmp_dir) # tmp_dir will be set despite the exception because load_map mostly worked
#         m.base = 'foo'
#         mapnik.load_map_from_string(m, map_string, True, ".")
#         eq_(m.base, '.')
#     except RuntimeError, e:
#         # only test datasources that we have installed
#         if not 'Could not create datasource' in str(e):
#             raise RuntimeError(e)

# # Color initialization
# @raises(Exception) # Boost.Python.ArgumentError
# def test_color_init_errors():
#     c = mapnik.Color()

# @raises(RuntimeError)
# def test_color_init_errors():
#     c = mapnik.Color('foo') # mapnik config

# def test_color_init():
#     c = mapnik.Color('blue')

#     eq_(c.a, 255)
#     eq_(c.r, 0)
#     eq_(c.g, 0)
#     eq_(c.b, 255)

#     eq_(c.to_hex_string(), '#0000ff')

#     c = mapnik.Color('#f2eff9')

#     eq_(c.a, 255)
#     eq_(c.r, 242)
#     eq_(c.g, 239)
#     eq_(c.b, 249)

#     eq_(c.to_hex_string(), '#f2eff9')

#     c = mapnik.Color('rgb(50%,50%,50%)')

#     eq_(c.a, 255)
#     eq_(c.r, 128)
#     eq_(c.g, 128)
#     eq_(c.b, 128)

#     eq_(c.to_hex_string(), '#808080')

#     c = mapnik.Color(0, 64, 128)

#     eq_(c.a, 255)
#     eq_(c.r, 0)
#     eq_(c.g, 64)
#     eq_(c.b, 128)

#     eq_(c.to_hex_string(), '#004080')

#     c = mapnik.Color(0, 64, 128, 192)

#     eq_(c.a, 192)
#     eq_(c.r, 0)
#     eq_(c.g, 64)
#     eq_(c.b, 128)

#     eq_(c.to_hex_string(), '#004080c0')

# def test_color_equality():

#     c1 = mapnik.Color('blue')
#     c2 = mapnik.Color(0,0,255)
#     c3 = mapnik.Color('black')

#     c3.r = 0
#     c3.g = 0
#     c3.b = 255
#     c3.a = 255

#     eq_(c1, c2)
#     eq_(c1, c3)

#     c1 = mapnik.Color(0, 64, 128)
#     c2 = mapnik.Color(0, 64, 128)
#     c3 = mapnik.Color(0, 0, 0)

#     c3.r = 0
#     c3.g = 64
#     c3.b = 128

#     eq_(c1, c2)
#     eq_(c1, c3)

#     c1 = mapnik.Color(0, 64, 128, 192)
#     c2 = mapnik.Color(0, 64, 128, 192)
#     c3 = mapnik.Color(0, 0, 0, 255)

#     c3.r = 0
#     c3.g = 64
#     c3.b = 128
#     c3.a = 192

#     eq_(c1, c2)
#     eq_(c1, c3)

#     c1 = mapnik.Color('rgb(50%,50%,50%)')
#     c2 = mapnik.Color(128, 128, 128, 255)
#     c3 = mapnik.Color('#808080')
#     c4 = mapnik.Color('gray')

#     eq_(c1, c2)
#     eq_(c1, c3)
#     eq_(c1, c4)

#     c1 = mapnik.Color('hsl(0, 100%, 50%)')   # red
#     c2 = mapnik.Color('hsl(120, 100%, 50%)') # lime
#     c3 = mapnik.Color('hsla(240, 100%, 50%, 0.5)') # semi-transparent solid blue

#     eq_(c1, mapnik.Color('red'))
#     eq_(c2, mapnik.Color('lime'))
#     eq_(c3, mapnik.Color(0,0,255,128))

# def test_rule_init():
#     min_scale = 5
#     max_scale = 10

#     r = mapnik.Rule()

#     eq_(r.name, '')
#     eq_(r.min_scale, 0)
#     eq_(r.max_scale, float('inf'))
#     eq_(r.has_else(), False)
#     eq_(r.has_also(), False)

#     r = mapnik.Rule()

#     r.set_else(True)
#     eq_(r.has_else(), True)
#     eq_(r.has_also(), False)

#     r = mapnik.Rule()

#     r.set_also(True)
#     eq_(r.has_else(), False)
#     eq_(r.has_also(), True)

#     r = mapnik.Rule("Name")

#     eq_(r.name, 'Name')
#     eq_(r.min_scale, 0)
#     eq_(r.max_scale, float('inf'))
#     eq_(r.has_else(), False)
#     eq_(r.has_also(), False)

#     r = mapnik.Rule("Name")

#     eq_(r.name, 'Name')
#     eq_(r.min_scale, 0)
#     eq_(r.max_scale, float('inf'))
#     eq_(r.has_else(), False)
#     eq_(r.has_also(), False)

#     r = mapnik.Rule("Name", min_scale)

#     eq_(r.name, 'Name')
#     eq_(r.min_scale, min_scale)
#     eq_(r.max_scale, float('inf'))
#     eq_(r.has_else(), False)
#     eq_(r.has_also(), False)

#     r = mapnik.Rule("Name", min_scale, max_scale)

#     eq_(r.name, 'Name')
#     eq_(r.min_scale, min_scale)
#     eq_(r.max_scale, max_scale)
#     eq_(r.has_else(), False)
#     eq_(r.has_also(), False)

# if __name__ == "__main__":
#     setup()
#     run_all(eval(x) for x in dir() if x.startswith("test_"))
