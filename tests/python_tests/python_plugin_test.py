#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import math
import mapnik
import sys
from utilities import execution_path
from nose.tools import *

try:
    from shapely.geometry import Point
    have_shapely = True
except ImportError:
    print('Shapely is required for python data source test.')
    have_shapely = False

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

class PointDatasource(mapnik.PythonDatasource):
    def __init__(self):
        super(PointDatasource, self).__init__(
                envelope = mapnik.Box2d(0,-10,100,110)
        )

    def features(self, query):
        return mapnik.PythonDatasource.wkb_features(
            keys = ('label',), 
            features = (
                ( Point(5,6).wkb, { 'label': 'foo-bar'} ), 
                ( Point(60,50).wkb, { 'label': 'buzz-quux'} ), 
            )
        )

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
                self.radius = step
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

class CirclesDatasource(mapnik.PythonDatasource):
    def __init__(self, centre_x=-20, centre_y=0, step=10):
        super(CirclesDatasource, self).__init__(
                geometry_type=mapnik.DataGeometryType.Polygon
        )

        # note that the plugin loader will set all arguments to strings and will not try to parse them
        centre_x = int(centre_x)
        centre_y = int(centre_y)
        step = int(step)

        self.centre_x = centre_x
        self.centre_y = centre_y
        self.step = step

    def features(self, query):
        # Get the query bounding-box as a shapely bounding box
        bounding_box = box2d_to_shapely(query.bbox)
        centre = Point(self.centre_x, self.centre_y)

        return mapnik.PythonDatasource.wkb_features(
            keys = (),
            features = ConcentricCircles(centre, bounding_box, self.step)
        )

if 'python' in mapnik.DatasourceCache.instance().plugin_names() and have_shapely:
    # make sure we can load from ourself as a module
    sys.path.append(execution_path('.'))

    def test_python_point_init():
        ds = mapnik.Python(factory='python_plugin_test:PointDatasource')
        e = ds.envelope()

        assert_almost_equal(e.minx, 0, places=7)
        assert_almost_equal(e.miny, -10, places=7)
        assert_almost_equal(e.maxx, 100, places=7)
        assert_almost_equal(e.maxy, 110, places=7)

    def test_python_circle_init():
        ds = mapnik.Python(factory='python_plugin_test:CirclesDatasource')
        e = ds.envelope()

        assert_almost_equal(e.minx, -180, places=7)
        assert_almost_equal(e.miny, -90, places=7)
        assert_almost_equal(e.maxx, 180, places=7)
        assert_almost_equal(e.maxy, 90, places=7)

    def test_python_circle_init_with_args():
        ds = mapnik.Python(factory='python_plugin_test:CirclesDatasource', centre_x=40, centre_y=7)
        e = ds.envelope()

        assert_almost_equal(e.minx, -180, places=7)
        assert_almost_equal(e.miny, -90, places=7)
        assert_almost_equal(e.maxx, 180, places=7)
        assert_almost_equal(e.maxy, 90, places=7)

    def test_python_point_rendering():
        m = mapnik.Map(512,512)
        mapnik.load_map(m,'../data/good_maps/python_point_datasource.xml')
        m.zoom_all()
        im = mapnik.Image(512,512)
        mapnik.render(m,im)
        actual = '/tmp/mapnik-python-point-render1.png'
        expected = 'images/support/mapnik-python-point-render1.png'
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(),
                'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

    def test_python_circle_rendering():
        m = mapnik.Map(512,512)
        mapnik.load_map(m,'../data/good_maps/python_circle_datasource.xml')
        m.zoom_all()
        im = mapnik.Image(512,512)
        mapnik.render(m,im)
        actual = '/tmp/mapnik-python-circle-render1.png'
        expected = 'images/support/mapnik-python-circle-render1.png'
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(),
                'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
