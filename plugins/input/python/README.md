# Python plugin

This plugin allows you to write data sources in the Python programming language.
This is useful if you want to rapidly prototype a plugin, perform some custom
manipulation on data or if you want to bind mapnik to a datasource which is most
conveniently accessed through Python.

The plugin may be used from the existing mapnik Python bindings or it can embed
the Python interpreter directly allowing it to be used from C++, XML or even
JavaScript.

## Rationale

Mapnik already has excellent Python bindings but they only directly support
calling *into* mapnik *from* Python. This forces mapnik and its input plugins to
be the lowest layer of the stack. The role of this plugin is to allow mapnik to
call *into* Python itself. This allows mapnik to sit as rendering middleware
between a custom Python frontend and a custom Python datasource. This increases
the utility of mapnik as a component in a larger system.

There already exists MemoryDatasource which can be used to dynamically create
geometry in Python. It suffers from the problem that it does not allow
generating only the geometry which is seen by a particular query. Similarly the
entire geometry must exist in memory before rendering can progress. By using a
custom iterator object or by using generator expressions this plugin allows
geometry to be created on demand and to be destroyed after use. This can have a
great impact on memory efficiency. Since geometry is generated on-demand as
rendering progresses there can be arbitrarily complex 'cleverness' optimising
the geometry generated for a particular query. Obvious examples of this would
be generating only geometry within the query bounding box and generating
geometry with an appropriate level of detail for the output resolution.

## Initialization

Only the `factory` parameter is required. This is of the form
`[module:]callable`. If `module` is present then `module` will be imported and
its attribute named `callable` will be used as a factory callable. If `module`
is omitted, then `__main__` is used. Any other parameter aside from `factory` or
`type` will be passed directly to the callable as keyword arguments. Note that
these will always be passed as strings even if the parameter can be parsed as an
integer of floating point value.

The callable should return an object with the following required attributes:

* `envelope` - a 4-tuple giving the (minx, miny, maxx, maxy) extent of the
  datasource;

* `data_type` - a `mapnik.DataType` instance giving the type of data stored in
  this datasource. This will usually be one of `mapnik.DataType.Vector` or
  `mapnik.DataType.Raster`.

The following attributes are optional:

* `geometry_type` - if the dataset is a vector dataset, this is an instance of
  `mapnik.DataGeometryType` giving the type of geometry returned by the
   datasource.

The following methods must be present:

* `features(query)` - takes a single argument which is an instance of
  `mapnik.Query` and returns an iterable of `mapnik.Feature` instances for that
  query.

* `features_at_point(point)` - almost never used. Takes a single argument which
  is an instance of `mapnik.Point` (I think) and returns an iterable of
  features associated with that point.

## Convenience classes

The standard `mapnik` module provides a convenience class called
`mapnik.PythonDatasource` which has default implementations for the required
methods and accepts the geometry type, data type and envelope as constructor
arguments. It also provides some convenience class methods which take care of
constructing features for you:

* `mapnik.PythonDatasource.wkb_features` - constructs features from
  well-known-binary (WKB) format geometry. Takes two keyword arguments: `keys`
  which is a sequence of keys associated with each feature and `features` which
  is a sequence of pairs. The first element in each pair is the WKB
  representation of the feature and the second element is a dictionary mapping
  keys to values.

# Caveats

* If used directly from C++, `Py_Initialize()` must have been called before the
  plugin is loaded to initialise the interpreter correctly.

* When inside the interpreter the global interpreter lock is held each time a
  feature is fetched and so multi-threaded rendering performance may suffer. You
  can mitigate this by making sure that the feature iterator yields its value as
  quickly as possible, potentially from an in-memory buffer filled fom another
  process over IPC.

# Examples

In XML:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Map srs="+init=epsg:4326" background-color="white">
    <Style name="style">
        <Rule>
            <PointSymbolizer />
            <TextSymbolizer name="[label]" face_name="DejaVu Sans Book" size="10" dx="5" dy="5"/>
        </Rule>
    </Style>
    <Layer name="test" srs="+init=epsg:4326">
        <StyleName>style</StyleName>
        <Datasource>
            <Parameter name="type">python</Parameter>
            <Parameter name="factory">test:TestDatasource</Parameter>
        </Datasource>
    </Layer>
</Map>
```

In Python using the shapely geometry library:

```python
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
```

A more complex Python example which makes use of iterators to generate geometry
dynamically:

```python
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
        super(TestDatasource, self).__init__(geometry_type=mapnik.DataGeometryType.Polygon)

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
```
