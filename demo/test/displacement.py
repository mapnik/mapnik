# $Id: rundemo.py 577 2008-01-03 11:39:10Z artem $
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

m = Map(690,690,"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs")

m.background = Color(255,100,100,255)

road_style = Style()

#Road
road_rule = Rule()
road_stroke = Stroke(Color('white'), 1)
road_stroke.line_cap = line_cap.ROUND_CAP
road_stroke.line_join = line_join.ROUND_JOIN
#road_rule.filter = Filter("[CLASS] = 'CROSS'")
road_rule.symbols.append(LineSymbolizer(road_stroke))
road_style.rules.append(road_rule);

#Road text
text_symbolizer = TextSymbolizer(Expression('[NAME]'), 'DejaVu Sans Book', 10, Color('black'))
text_symbolizer.label_placement=label_placement.LINE_PLACEMENT
text_symbolizer.minimum_distance = 0
#text_symbolizer.max_char_angle_delta = 40
#text_symbolizer.force_odd_labels = 1
#FIXME: Displacement cannot be set from python so we can't set it here, lol!
text_symbolizer.label_spacing = 60
text_symbolizer.label_position_tolerance = 5
text_symbolizer.avoid_edges = 0
text_symbolizer.halo_fill = Color('yellow')
text_symbolizer.halo_radius = 1
road_rule = Rule()
#road_rule.filter = Filter("[CLASS] = 'CROSS'")
road_rule.symbols.append(text_symbolizer)
road_style.rules.append(road_rule)


road_layer = Layer('road')
road_layer.datasource = Shapefile(file='../data/test/displacement')

m.append_style('road', road_style)
road_layer.styles.append('road')
m.layers.append(road_layer)

# Draw map

# Set the initial extent of the map.
m.zoom_to_box(Box2d(0,0,14,-14))


# Render
im = Image(m.width,m.height)
render(m, im)

# Save image to file
im.save('output.png') # true-colour RGBA

print "Done\n"
