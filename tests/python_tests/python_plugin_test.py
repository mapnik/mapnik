# #!/usr/bin/env python
# # -*- coding: utf-8 -*-

# import os
# import math
# import mapnik
# import sys
# from utilities import execution_path, run_all
# from nose.tools import *

# def setup():
#     # All of the paths used are relative, if we run the tests
#     # from another directory we need to chdir()
#     os.chdir(execution_path('.'))

# class PointDatasource(mapnik.PythonDatasource):
#     def __init__(self):
#         super(PointDatasource, self).__init__(
#                 geometry_type = mapnik.DataGeometryType.Point,
#                 envelope = mapnik.Box2d(0,-10,100,110),
#                 data_type = mapnik.DataType.Vector
#         )

#     def features(self, query):
#         return mapnik.PythonDatasource.wkt_features(
#             keys = ('label',),
#             features = (
#                 ( 'POINT (5 6)', { 'label': 'foo-bar'} ),
#                 ( 'POINT (60 50)', { 'label': 'buzz-quux'} ),
#             )
#         )

# class ConcentricCircles(object):
#     def __init__(self, centre, bounds, step=1):
#         self.centre = centre
#         self.bounds = bounds
#         self.step = step

#     class Iterator(object):
#         def __init__(self, container):
#             self.container = container

#             centre = self.container.centre
#             bounds = self.container.bounds
#             step = self.container.step

#             self.radius = step

#         def next(self):
#             points = []
#             for alpha in xrange(0, 361, 5):
#                 x = math.sin(math.radians(alpha)) * self.radius + self.container.centre[0]
#                 y = math.cos(math.radians(alpha)) * self.radius + self.container.centre[1]
#                 points.append('%s %s' % (x,y))
#             circle = 'POLYGON ((' + ','.join(points) + '))'

#             # has the circle grown so large that the boundary is entirely within it?
#             tl = (self.container.bounds.maxx, self.container.bounds.maxy)
#             tr = (self.container.bounds.maxx, self.container.bounds.maxy)
#             bl = (self.container.bounds.minx, self.container.bounds.miny)
#             br = (self.container.bounds.minx, self.container.bounds.miny)
#             def within_circle(p):
#                 delta_x = p[0] - self.container.centre[0]
#                 delta_y = p[0] - self.container.centre[0]
#                 return delta_x*delta_x + delta_y*delta_y < self.radius*self.radius

#             if all(within_circle(p) for p in (tl,tr,bl,br)):
#                 raise StopIteration()

#             self.radius += self.container.step
#             return ( circle, { } )

#     def __iter__(self):
#         return ConcentricCircles.Iterator(self)

# class CirclesDatasource(mapnik.PythonDatasource):
#     def __init__(self, centre_x=-20, centre_y=0, step=10):
#         super(CirclesDatasource, self).__init__(
#                 geometry_type = mapnik.DataGeometryType.Polygon,
#                 envelope = mapnik.Box2d(-180, -90, 180, 90),
#                 data_type = mapnik.DataType.Vector
#         )

#         # note that the plugin loader will set all arguments to strings and will not try to parse them
#         centre_x = int(centre_x)
#         centre_y = int(centre_y)
#         step = int(step)

#         self.centre_x = centre_x
#         self.centre_y = centre_y
#         self.step = step

#     def features(self, query):
#         centre = (self.centre_x, self.centre_y)

#         return mapnik.PythonDatasource.wkt_features(
#             keys = (),
#             features = ConcentricCircles(centre, query.bbox, self.step)
#         )

# if 'python' in mapnik.DatasourceCache.plugin_names():
#     # make sure we can load from ourself as a module
#     sys.path.append(execution_path('.'))

#     def test_python_point_init():
#         ds = mapnik.Python(factory='python_plugin_test:PointDatasource')
#         e = ds.envelope()

#         assert_almost_equal(e.minx, 0, places=7)
#         assert_almost_equal(e.miny, -10, places=7)
#         assert_almost_equal(e.maxx, 100, places=7)
#         assert_almost_equal(e.maxy, 110, places=7)

#     def test_python_circle_init():
#         ds = mapnik.Python(factory='python_plugin_test:CirclesDatasource')
#         e = ds.envelope()

#         assert_almost_equal(e.minx, -180, places=7)
#         assert_almost_equal(e.miny, -90, places=7)
#         assert_almost_equal(e.maxx, 180, places=7)
#         assert_almost_equal(e.maxy, 90, places=7)

#     def test_python_circle_init_with_args():
#         ds = mapnik.Python(factory='python_plugin_test:CirclesDatasource', centre_x=40, centre_y=7)
#         e = ds.envelope()

#         assert_almost_equal(e.minx, -180, places=7)
#         assert_almost_equal(e.miny, -90, places=7)
#         assert_almost_equal(e.maxx, 180, places=7)
#         assert_almost_equal(e.maxy, 90, places=7)

#     def test_python_point_rendering():
#         m = mapnik.Map(512,512)
#         mapnik.load_map(m,'../data/python_plugin/python_point_datasource.xml')
#         m.zoom_all()
#         im = mapnik.Image(512,512)
#         mapnik.render(m,im)
#         actual = '/tmp/mapnik-python-point-render1.png'
#         expected = 'images/support/mapnik-python-point-render1.png'
#         im.save(actual)
#         expected_im = mapnik.Image.open(expected)
#         eq_(im.tostring('png32'),expected_im.tostring('png32'),
#                 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

#     def test_python_circle_rendering():
#         m = mapnik.Map(512,512)
#         mapnik.load_map(m,'../data/python_plugin/python_circle_datasource.xml')
#         m.zoom_all()
#         im = mapnik.Image(512,512)
#         mapnik.render(m,im)
#         actual = '/tmp/mapnik-python-circle-render1.png'
#         expected = 'images/support/mapnik-python-circle-render1.png'
#         im.save(actual)
#         expected_im = mapnik.Image.open(expected)
#         eq_(im.tostring('png32'),expected_im.tostring('png32'),
#                 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

# if __name__ == "__main__":
#     setup()
#     run_all(eval(x) for x in dir() if x.startswith("test_"))
