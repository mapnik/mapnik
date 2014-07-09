#!/usr/bin/env python

from nose.tools import *
import atexit
import time
from utilities import execution_path, run_all
from subprocess import Popen, PIPE
import os, mapnik
from Queue import Queue
import threading


MAPNIK_TEST_DBNAME = 'mapnik-tmp-pgraster-test-db'
POSTGIS_TEMPLATE_DBNAME = 'template_postgis'

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def call(cmd,silent=False):
    proc = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)
    stdout, stderr = proc.communicate()
    if stderr and not silent:
        print stderr.strip()
    if proc.returncode:
        raise RuntimeError(stderr.strip())

def psql_can_connect():
    """Test ability to connect to a postgis template db with no options.

    Basically, to run these tests your user must have full read
    access over unix sockets without supplying a password. This
    keeps these tests simple and focused on postgis not on postgres
    auth issues.
    """
    try:
        call('psql %s -c "select postgis_version()"' % POSTGIS_TEMPLATE_DBNAME)
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests (connection)'
        return False

def psql_run(cmd):
  cmd = 'psql --set ON_ERROR_STOP=1 %s -c "%s"' % (MAPNIK_TEST_DBNAME,cmd)
  call(cmd)

def raster2pgsql_on_path():
    """Test for presence of raster2pgsql on the user path.

    We require this program to load test data into a temporarily database.
    """
    try:
        call('raster2pgsql')
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests (raster2pgsql)'
        return False

def createdb_and_dropdb_on_path():
    """Test for presence of dropdb/createdb on user path.

    We require these programs to setup and teardown the testing db.
    """
    try:
        call('createdb --help')
        call('dropdb --help')
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests (createdb/dropdb)'
        return False

def postgis_setup():
    call('dropdb %s' % MAPNIK_TEST_DBNAME,silent=True)
    call('createdb -T %s %s' % (POSTGIS_TEMPLATE_DBNAME,MAPNIK_TEST_DBNAME),silent=False)

def postgis_takedown():
    pass
    # fails as the db is in use: https://github.com/mapnik/mapnik/issues/960
    #call('dropdb %s' % MAPNIK_TEST_DBNAME)

if 'pgraster' in mapnik.DatasourceCache.plugin_names() \
        and createdb_and_dropdb_on_path() \
        and psql_can_connect() \
        and raster2pgsql_on_path():

    # initialize test database
    postgis_setup()

    def _test_dataraster_16bsi_rendering(overview, rescale, clip):
      ds = mapnik.PgRaster(dbname=MAPNIK_TEST_DBNAME,table='dataraster',
        band=1,use_overviews=1 if overview else 0,
        prescale_rasters=rescale,clip_rasters=clip)
      fs = ds.featureset()
      feature = fs.next()
      eq_(feature['rid'],1)
      lyr = mapnik.Layer('dataraster_16bsi')
      lyr.datasource = ds
      expenv = mapnik.Box2d(-14637, 3903178, 1126863, 4859678)
      env = lyr.envelope()
      # As the input size is a prime number both horizontally
      # and vertically, we expect the extent of the overview
      # tables to be a pixel wider than the original, whereas
      # the pixel size in geographical units depends on the
      # overview factor. So we start with the original pixel size
      # as base scale and multiply by the overview factor.
      # NOTE: the overview table extent only grows north and east
      pixsize = 500 # see gdalinfo dataraster.tif
      tol = pixsize * max(overview.split(',')) if overview else 0
      assert_almost_equal(env.minx, expenv.minx)
      assert_almost_equal(env.miny, expenv.miny, delta=tol) 
      assert_almost_equal(env.maxx, expenv.maxx, delta=tol)
      assert_almost_equal(env.maxy, expenv.maxy)
      mm = mapnik.Map(256, 256)
      style = mapnik.Style()
      col = mapnik.RasterColorizer();
      col.default_mode = mapnik.COLORIZER_DISCRETE;
      col.add_stop(0, mapnik.Color(0x40,0x40,0x40,255));
      col.add_stop(10, mapnik.Color(0x80,0x80,0x80,255));
      col.add_stop(20, mapnik.Color(0xa0,0xa0,0xa0,255));
      sym = mapnik.RasterSymbolizer()
      sym.colorizer = col
      rule = mapnik.Rule()
      rule.symbols.append(sym)
      style.rules.append(rule)
      mm.append_style('foo', style)
      lyr.styles.append('foo')
      mm.layers.append(lyr)
      mm.zoom_to_box(expenv)
      im = mapnik.Image(mm.width, mm.height)
      mapnik.render(mm, im)
      # no data
      eq_(im.view(1,1,1,1).tostring(), '\x00\x00\x00\x00') 
      eq_(im.view(255,255,1,1).tostring(), '\x00\x00\x00\x00') 
      eq_(im.view(195,116,1,1).tostring(), '\x00\x00\x00\x00') 
      # A0A0A0
      eq_(im.view(100,120,1,1).tostring(), '\xa0\xa0\xa0\xff')
      eq_(im.view( 75, 80,1,1).tostring(), '\xa0\xa0\xa0\xff')
      # 808080
      eq_(im.view( 74,170,1,1).tostring(), '\x80\x80\x80\xff')
      eq_(im.view( 30, 50,1,1).tostring(), '\x80\x80\x80\xff')
      # 404040
      eq_(im.view(190, 70,1,1).tostring(), '\x40\x40\x40\xff')
      eq_(im.view(140,170,1,1).tostring(), '\x40\x40\x40\xff')

    def _test_dataraster_16bsi(tilesize, constraint, overview):
      rf = os.path.join(execution_path('.'),'../data/raster/dataraster.tif')
      print 'tile: ' + tilesize + ' constraints: ' + str(constraint) \
          + ' overviews: ' + overview
      cmd = 'raster2pgsql -Y'
      if constraint:
        cmd += ' -C'
      if tilesize:
        cmd += ' -t ' + tilesize
      if overview:
        cmd += ' -l ' + overview
      cmd += ' %s dataraster | psql --set ON_ERROR_STOP=1 -q %s' % (rf,MAPNIK_TEST_DBNAME)
      print 'Import call: ' + cmd
      psql_run('DROP TABLE IF EXISTS dataraster;')
      if overview:
        for of in overview.split(','):
          psql_run('DROP TABLE IF EXISTS o_' + of + '_dataraster;')
      call(cmd)
      for prescale in [0,1]:
        for clip in [0,1]:
          _test_dataraster_16bsi_rendering(overview, prescale, clip)

    def test_dataraster_16bsi():
      for tilesize in ['','256x256']:
        for constraint in [0,1]:
          for overview in ['','4','2,16']:
            _test_dataraster_16bsi(tilesize, constraint, overview)


    atexit.register(postgis_takedown)

if __name__ == "__main__":
    setup()
    run_all(eval(x) for x in dir() if x.startswith("test_"))
