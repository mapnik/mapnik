# $Id$
#
# This file is part of Mapnik (c++ mapping toolkit)
# Copyright (C) 2005 Jean-Francois Doyon
#
# Mapnik is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# Import everything.  In this case this is safe, in more complex systems, you
# will want to be more selective.

try:
    from mapnik import *
except:
    print '\n\nThe mapnik library and python bindings must have been compiled and \
installed successfully before running this script.\n\n'
    raise

# Instanciate a map, giving it a width and height. Remember: the word "map" is
# reserved in Python! :)

m = Map(800,600,"+proj=latlong +ellps=WGS84")

# Set its background colour. More on colours later ...

m.background = Color('white')

# Now we can start adding layers, in stacking order (i.e. bottom layer first)

# Canadian Provinces (Polygons)

# Instanciate a layer.  The parameters depend on the type of data:
# shape:
#     type='shape'
#     file='/path/to/shape'
# raster:
#     type='raster'
#     file='/path/to/raster'
# postgis:
#     type='postgis'
#     host='127.0.0.1'
#     dbname='mydatabase'
#     user='myusername'
#     password='mypassword'
#     table= TODO

provpoly_lyr = Layer('Provinces')
provpoly_lyr.datasource = Shapefile(file='../data/boundaries', encoding='latin1')

# We then define a style for the layer.  A layer can have one or many styles.
# Styles are named, so they can be shared across different layers.
# Multiple styles per layer behaves functionally like multiple layers.  The
# data is completely re-scanned for each style within one layer, and a style
# will be drawn entirely "above" the previous one.  Performance wise using
# multiple styles in one layer is the same has having multiple layers.
# The paradigm is useful mostly as a convenience.

provpoly_style = Style()

# A Style needs one or more rules.  A rule will normally consist of a filter
# for feature selection, and one or more symbolizers.

provpoly_rule_on = Rule()

# A Filter() allows the selection of features to which the symbology will
# be applied.  More on Mapnik expressions can be found in Tutorial #2.
# A given feature can only match one filter per rule per style.

provpoly_rule_on.filter = Filter("[NAME_EN] = 'Ontario'")

# Here a symbolizer is defined.  Available are:
#     - LineSymbolizer(Color(),<width>)
#     - LineSymbolizer(Stroke())
#     - PolygonSymbolizer(Color())
#     - PointSymbolizer(<file>,<type>,<width>,<height>)

# Some of them can accept a Color() instance, which can be created with:
#     - Color(<red>, <green>, <blue>)
#     - Color(<red>, <green>, <blue>, <alpha>)
#     - Color(<string>) where <string> will be something like '#00FF00'
#       or '#0f0' or 'green'

provpoly_rule_on.symbols.append(PolygonSymbolizer(Color(250, 190, 183)))
provpoly_style.rules.append(provpoly_rule_on)

provpoly_rule_qc = Rule()
provpoly_rule_qc.filter = Filter("[NAME_EN] = 'Quebec'")
provpoly_rule_qc.symbols.append(PolygonSymbolizer(Color(217, 235, 203)))
provpoly_style.rules.append(provpoly_rule_qc)

# Add the style to the map, giving it a name.  This is the name that will be
# used to refer to it from here on.  Having named styles allows them to be
# re-used throughout the map.

m.append_style('provinces', provpoly_style)

# Then associate the style to the layer itself.

provpoly_lyr.styles.append('provinces')

# Then add the layer to the map.  In reality, it's the order in which you
# append them to the map that will determine the drawing order, though by
# convention it is recommended to define them in drawing order as well.

m.layers.append(provpoly_lyr)

# Drainage

# A simple example ...

qcdrain_lyr = Layer('Quebec Hydrography')

qcdrain_lyr.datasource = Shapefile(file='../data/qcdrainage')

qcdrain_style = Style()
qcdrain_rule = Rule()
qcdrain_rule.filter = Filter('[HYC] = 8')
qcdrain_rule.symbols.append(PolygonSymbolizer(Color(153, 204, 255)))
qcdrain_style.rules.append(qcdrain_rule)

m.append_style('drainage', qcdrain_style)
qcdrain_lyr.styles.append('drainage')
m.layers.append(qcdrain_lyr)

# In this case, we have 2 data sets with similar schemas (same filtering
# attributes, and same desired style), so we're going to
# re-use the style defined in the above layer for the next one.

ondrain_lyr = Layer('Ontario Hydrography')
ondrain_lyr.datasource = Shapefile(file='../data/ontdrainage')

ondrain_lyr.styles.append('drainage')
m.layers.append(ondrain_lyr)

# Provincial boundaries

provlines_lyr = Layer('Provincial borders')
provlines_lyr.datasource = Shapefile(file='../data/boundaries_l')

# Here we define a "dash dot dot dash" pattern for the provincial boundaries.

provlines_stk = Stroke()
provlines_stk.add_dash(8, 4)
provlines_stk.add_dash(2, 2)
provlines_stk.add_dash(2, 2)
provlines_stk.color = Color('black')
provlines_stk.width = 1.0

provlines_style = Style()
provlines_rule = Rule()
provlines_rule.symbols.append(LineSymbolizer(provlines_stk))
provlines_style.rules.append(provlines_rule)

m.append_style('provlines', provlines_style)
provlines_lyr.styles.append('provlines')
m.layers.append(provlines_lyr)

# Roads 3 and 4 (The "grey" roads)

roads34_lyr = Layer('Roads')
# create roads datasource (we're going to re-use it later) 

roads34_lyr.datasource = Shapefile(file='../data/roads')

roads34_style = Style()
roads34_rule = Rule()
roads34_rule.filter = Filter('[CLASS] = 3 or [CLASS] = 4')

# With lines of a certain width, you can control how the ends
# are closed off using line_cap as below.

roads34_rule_stk = Stroke()
roads34_rule_stk.color = Color(171,158,137)
roads34_rule_stk.line_cap = line_cap.ROUND_CAP

# Available options are:
# line_cap: BUTT_CAP, SQUARE_CAP, ROUND_CAP
# line_join: MITER_JOIN, MITER_REVERT_JOIN, ROUND_JOIN, BEVEL_JOIN

# And one last Stroke() attribute not used here is "opacity", which
# can be set to a numerical value.

roads34_rule_stk.width = 2.0
roads34_rule.symbols.append(LineSymbolizer(roads34_rule_stk))
roads34_style.rules.append(roads34_rule)

m.append_style('smallroads', roads34_style)
roads34_lyr.styles.append('smallroads')
m.layers.append(roads34_lyr)

# Roads 2 (The thin yellow ones)

roads2_lyr = Layer('Roads')

# Just get a copy from roads34_lyr
roads2_lyr.datasource = roads34_lyr.datasource 

roads2_style_1 = Style()
roads2_rule_1 = Rule()
roads2_rule_1.filter = Filter('[CLASS] = 2')
roads2_rule_stk_1 = Stroke()
roads2_rule_stk_1.color = Color(171,158,137)
roads2_rule_stk_1.line_cap = line_cap.ROUND_CAP
roads2_rule_stk_1.width = 4.0
roads2_rule_1.symbols.append(LineSymbolizer(roads2_rule_stk_1))
roads2_style_1.rules.append(roads2_rule_1)

m.append_style('road-border', roads2_style_1)

roads2_style_2 = Style()
roads2_rule_2 = Rule()
roads2_rule_2.filter = Filter('[CLASS] = 2')
roads2_rule_stk_2 = Stroke()
roads2_rule_stk_2.color = Color(255,250,115)
roads2_rule_stk_2.line_cap = line_cap.ROUND_CAP
roads2_rule_stk_2.width = 2.0
roads2_rule_2.symbols.append(LineSymbolizer(roads2_rule_stk_2))
roads2_style_2.rules.append(roads2_rule_2)

m.append_style('road-fill', roads2_style_2)

roads2_lyr.styles.append('road-border')
roads2_lyr.styles.append('road-fill')

m.layers.append(roads2_lyr)

# Roads 1 (The big orange ones, the highways)

roads1_lyr = Layer('Roads')
roads1_lyr.datasource = roads34_lyr.datasource

roads1_style_1 = Style()
roads1_rule_1 = Rule()
roads1_rule_1.filter = Filter('[CLASS] = 1')
roads1_rule_stk_1 = Stroke()
roads1_rule_stk_1.color = Color(188,149,28)
roads1_rule_stk_1.line_cap = line_cap.ROUND_CAP
roads1_rule_stk_1.width = 7.0
roads1_rule_1.symbols.append(LineSymbolizer(roads1_rule_stk_1))
roads1_style_1.rules.append(roads1_rule_1)
m.append_style('highway-border', roads1_style_1)

roads1_style_2 = Style()
roads1_rule_2 = Rule()
roads1_rule_2.filter = Filter('[CLASS] = 1')
roads1_rule_stk_2 = Stroke()
roads1_rule_stk_2.color = Color(242,191,36)
roads1_rule_stk_2.line_cap = line_cap.ROUND_CAP
roads1_rule_stk_2.width = 5.0
roads1_rule_2.symbols.append(LineSymbolizer(roads1_rule_stk_2))
roads1_style_2.rules.append(roads1_rule_2)

m.append_style('highway-fill', roads1_style_2)

roads1_lyr.styles.append('highway-border')
roads1_lyr.styles.append('highway-fill')

m.layers.append(roads1_lyr)

# Populated Places

popplaces_lyr = Layer('Populated Places')
popplaces_lyr.datasource = Shapefile(file='../data/popplaces',encoding='latin1')

popplaces_style = Style()
popplaces_rule = Rule()

# And here we have a TextSymbolizer, used for labeling.
# The first parameter is the name of the attribute to use as the source of the
# text to label with.  Then there is font size in points (I think?), and colour.

popplaces_text_symbolizer = TextSymbolizer('GEONAME',
                                           'DejaVu Sans Book',
                                           10, Color('black'))

# We set a "halo" around the text, which looks like an outline if thin enough,
# or an outright background if large enough.
popplaces_text_symbolizer.set_label_placement=label_placement.POINT_PLACEMENT
popplaces_text_symbolizer.halo_fill = Color('white')
popplaces_text_symbolizer.halo_radius = 1
popplaces_text_symbolizer.avoid_edges = True
popplaces_rule.symbols.append(popplaces_text_symbolizer)

popplaces_style.rules.append(popplaces_rule)

m.append_style('popplaces', popplaces_style)
popplaces_lyr.styles.append('popplaces')
m.layers.append(popplaces_lyr)

# Draw map

# Set the initial extent of the map.

m.zoom_to_box(Envelope(1405120.04127408,-247003.813399447,1706357.31328276,-25098.593149577))

# Render two maps, two PNGs, one JPEG.
im = Image(m.width,m.height)
render(m, im)

# Save image to files
images = []
im.save('demo.png', 'png') # true-colour RGBA
images.append('demo.png')
im.save('demo256.png', 'png256') # save to palette based (max 256 colours) png 
images.append('demo256.png')
im.save('demo.jpg', 'jpeg')
images.append('demo.jpg')

# Render cairo examples
try:
    import cairo
    surface = cairo.SVGSurface('demo.svg', m.width,m.height)
    render(m, surface)
    images.append('demo.svg')
    surface = cairo.PDFSurface('demo.pdf', m.width,m.height)
    render(m, surface)
    images.append('demo.pdf')
except:
    print '\n\nSkipping cairo examples as Pycairo not available'

print "\n\n", len(images), "maps have been rendered in the current directory:"
for image in images:
    print "-", image
print "\n\nHave a look!\n\n"
