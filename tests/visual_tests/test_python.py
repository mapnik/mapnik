#!/usr/bin/env python
import mapnik
import sys

class MyText(mapnik.FormatingNode):
    def __init__(self):
        mapnik.FormatingNode.__init__(self)
        self.expr = mapnik.Expression("[name]")
        self.expr_nr = mapnik.Expression("[nr]")

    def apply(self, properties, feature, output):
        colors = [mapnik.Color('red'), 
                  mapnik.Color('green'), 
                  mapnik.Color('blue')]
        text = self.expr.evaluate(feature)
        if int(feature['nr']) > 5:
            i = 0
            my_properties = mapnik.CharProperties(properties)
            for char in text:
                my_properties.fill = colors[i % len(colors)]
                output.append(my_properties, char)
                i += 1
        else:
            output.append(properties, text)
            
    def add_expressions(self, output):
        output.insert(self.expr)
        output.insert(self.expr_nr)

m = mapnik.Map(600, 100)
m.background = mapnik.Color('white')

text = mapnik.TextSymbolizer()
text.placements.defaults.displacement = (0, 5)
text.placements.defaults.default_format.face_name = 'DejaVu Sans Book'

point = mapnik.PointSymbolizer()

rule = mapnik.Rule() 
rule.symbols.append(text)
rule.symbols.append(point)

style = mapnik.Style() 
style.rules.append(rule)

m.append_style('Style', style)


layer = mapnik.Layer('Layer')
layer.datasource = mapnik.Shapefile(file="points.shp")
layer.styles.append('Style')
m.layers.append(layer)

bbox = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)
m.zoom_to_box(bbox)


format_trees = [
    ('TextNode', mapnik.FormatingTextNode(mapnik.Expression("[name]"))),
    ('MyText', MyText())
]

for format_tree in format_trees:
    text.placements.defaults.format_tree = format_tree[1]
    mapnik.render_to_file(m, 'python-%s.png' % format_tree[0], 'png')
