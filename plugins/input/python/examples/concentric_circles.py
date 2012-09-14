"""A more complex example which renders an infinite series of concentric
circles centred on a point.

The circles are represented by a Python iterator which will yield only the
circles which intersect the query's bounding box. The advantage of this
approach over a MemoryDatasource is that a) only those circles which intersect
the viewport are actually generated and b) only the memory for the largest
circle need be available since each circle is created on demand and destroyed
when finished with.
"""
import math
import mapnik
from shapely.geometry import *

def box2d_to_shapely(box):
    import shapely.geometry
    return shapely.geometry.box(box.minx, box.miny, box.maxx, box.maxy)

class ConcentricCircles(object):
    def __init__(self, centre, bounds, step=1):
        self.centre = centre
        self.bounds = bounds
        self.step = step

    class Iterator(object):
        def __init__(self, container):
            self.container = container

            centre = self.container.centre
            bounds = self.container.bounds
            step = self.container.step

            if centre.within(bounds):
                self.radius = 0
            else:
                self.radius = math.ceil(centre.distance(bounds) / float(step)) * step

        def next(self):
            circle = self.container.centre.buffer(self.radius)
            self.radius += self.container.step

            # has the circle grown so large that the boundary is entirely within it?
            if circle.contains(self.container.bounds):
                raise StopIteration()

            return ( circle.wkb, { } )

    def __iter__(self):
        return ConcentricCircles.Iterator(self)

class TestDatasource(mapnik.PythonDatasource):
    def __init__(self):
        super(TestDatasource, self).__init__(
                geometry_type=mapnik.DataGeometryType.Polygon
        )

    def features(self, query):
        # Get the query bounding-box as a shapely bounding box
        bounding_box = box2d_to_shapely(query.bbox)
        centre = Point(-20, 0)

        return mapnik.PythonDatasource.wkb_features(
            keys = (),
            features = ConcentricCircles(centre, bounding_box, 0.5)
        )

if __name__ == '__main__':
    m = mapnik.Map(640, 320)

    m.background = mapnik.Color('white')
    s = mapnik.Style()
    r = mapnik.Rule()
    r.symbols.append(mapnik.LineSymbolizer())
    s.rules.append(r)
    m.append_style('point_style',s)
    ds = mapnik.Python(factory='TestDatasource')
    layer = mapnik.Layer('python')
    layer.datasource = ds
    layer.styles.append('point_style')
    m.layers.append(layer)
    box = mapnik.Box2d(-60, -60, 0, -30)
    m.zoom_to_box(box)
    mapnik.render_to_file(m,'map.png', 'png')
