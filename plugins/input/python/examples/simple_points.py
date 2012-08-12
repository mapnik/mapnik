import mapnik
from shapely.geometry import *

class TestDatasource(mapnik.PythonDatasource):
    def __init__(self):
        super(TestDatasource, self).__init__()

    def features(self, query):
        return mapnik.PythonDatasource.wkb_features(
            keys = ('label',), 
            features = (
                ( Point(5,6).wkb, { 'label': 'foo-bar'} ), 
                ( Point(100,60).wkb, { 'label': 'buzz-quux'} ), 
            )
        )

if __name__ == '__main__':
    m = mapnik.Map(1280,1024)
    m.background = mapnik.Color('white')
    s = mapnik.Style()
    r = mapnik.Rule()
    r.symbols.append(mapnik.PointSymbolizer())
    t = mapnik.TextSymbolizer(mapnik.Expression("[label]"),"DejaVu Sans Book",10,mapnik.Color('black'))
    t.displacement = (5,5)
    r.symbols.append(t)
    s.rules.append(r)
    m.append_style('point_style',s)
    ds = mapnik.Python(factory='TestDatasource')
    layer = mapnik.Layer('python')
    layer.datasource = ds
    layer.styles.append('point_style')
    m.layers.append(layer)
    m.zoom_all()
    mapnik.render_to_file(m,'map.png', 'png')
