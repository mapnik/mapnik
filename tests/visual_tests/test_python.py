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


class IfElse(mapnik.FormatingNode):
    def __init__(self, condition, if_node, else_node):
        mapnik.FormatingNode.__init__(self)
        self.condition = mapnik.Expression(condition)
        self.if_node = if_node
        self.else_node = else_node

    def apply(self, properties, feature, output):
        c = self.condition.evaluate(feature)
        if c:
            self.if_node.apply(properties, feature, output)
        else:
            self.else_node.apply(properties, feature, output)

    def add_expressions(self, output):
        output.insert(self.condition)
        self.if_node.add_expressions(output)
        self.else_node.add_expressions(output)

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

formatnode = mapnik.FormatingFormatNode()
formatnode.child = mapnik.FormatingTextNode("[name]")
formatnode.fill = mapnik.Color("green")

format_trees = [
    ('TextNode', mapnik.FormatingTextNode("[name]")),
    ('MyText', MyText()),
    ('IfElse', IfElse("[nr] != '5'",
                mapnik.FormatingTextNode("[name]"),
                mapnik.FormatingTextNode("'SPECIAL!'"))),
    ('Format', formatnode),
    ('List',   mapnik.FormatingListNode([
                mapnik.FormatingTextNode("[name]+'\n'"),
                MyText()
                ]))
]

for format_tree in format_trees:
    text.placements.defaults.format_tree = format_tree[1]
    mapnik.render_to_file(m, 'python-%s.png' % format_tree[0], 'png')
