## hello world plugin

This is a very simple sample plugin. It is designed to help developers
see the skeletal basics needed to achieve a functional datasource plugin.

It is not a model plugin of best practices as much as a model of the bare
minimum you need to have a working plugin that returns a single feature.

Code comments attempt to highlight which code is mandatory, which is
simply recommended, and which is purely fluff used to get the plugin to
actually show some data.

When added to a map it provides a single point geometry representing
the center of any query. This means that it should place a point in
the middle of any map tile and display a "hello world!" label if used like:

<?xml version="1.0" encoding="utf-8"?>
<Map srs="+init=epsg:4326" background-color="white">
    <Style name="style">
        <Rule>
            <PointSymbolizer />
            <TextSymbolizer name="[key]" face_name="DejaVu Sans Book" size="10" dx="5" dy="5"/>
        </Rule>
    </Style>
    <Layer name="test" srs="+init=epsg:4326">
        <StyleName>style</StyleName>
        <Datasource>
            <Parameter name="type">hello</Parameter>
        </Datasource>
    </Layer>
</Map>


Or used in python like:

from mapnik import *
m = Map(600,400)
m.background = Color('white')
s = Style()
r = Rule()
r.symbols.append(PointSymbolizer())
t = TextSymbolizer(Expression("[key]"),"DejaVu Sans Book",10,Color('black'))
t.displacement = (15,15)
r.symbols.append(t)
s.rules.append(r)
m.append_style('style',s)
ds = Datasource(type="hello")
l = Layer('test')
l.styles.append('style')
l.datasource = ds
m.layers.append(l)
m.zoom_all()
render_to_file(m,'test.png')
