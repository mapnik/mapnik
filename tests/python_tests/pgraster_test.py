#!/usr/bin/env python

from nose.tools import *
import atexit
import cProfile, pstats, io
import time
from utilities import execution_path, run_all
from subprocess import Popen, PIPE
import os, mapnik
from Queue import Queue
import threading
import sys
import re
from binascii import hexlify, unhexlify


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

def import_raster(filename, tabname, tilesize, constraint, overview):
  print 'tile: ' + tilesize + ' constraints: ' + str(constraint) \
      + ' overviews: ' + overview
  cmd = 'raster2pgsql -Y -I'
  if constraint:
    cmd += ' -C'
  if tilesize:
    cmd += ' -t ' + tilesize
  if overview:
    cmd += ' -l ' + overview
  cmd += ' %s %s | psql --set ON_ERROR_STOP=1 -q %s' % (filename,tabname,MAPNIK_TEST_DBNAME)
  print 'Import call: ' + cmd
  call(cmd)

def drop_imported(tabname, overview):
  psql_run('DROP TABLE IF EXISTS "' + tabname + '";')
  if overview:
    for of in overview.split(','):
      psql_run('DROP TABLE IF EXISTS "o_' + of + '_' + tabname + '";')

if 'pgraster' in mapnik.DatasourceCache.plugin_names() \
        and createdb_and_dropdb_on_path() \
        and psql_can_connect() \
        and raster2pgsql_on_path():

    # initialize test database
    postgis_setup()

    # dataraster.tif, 2283x1913 int16 single-band
    def _test_dataraster_16bsi_rendering(lbl, overview, rescale, clip):
      if rescale:
        lbl += ' Sc'
      if clip:
        lbl += ' Cl'
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
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:full'
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

      # Now zoom over a portion of the env (1/10)
      newenv = mapnik.Box2d(273663,4024478,330738,4072303)
      mm.zoom_to_box(newenv)
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:1/10'
      # nodata
      eq_(hexlify(im.view(255,255,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(200,254,1,1).tostring()), '00000000')
      # A0A0A0
      eq_(hexlify(im.view(90,232,1,1).tostring()), 'a0a0a0ff')
      eq_(hexlify(im.view(96,245,1,1).tostring()), 'a0a0a0ff')
      # 808080
      eq_(hexlify(im.view(1,1,1,1).tostring()), '808080ff') 
      eq_(hexlify(im.view(128,128,1,1).tostring()), '808080ff') 
      # 404040
      eq_(hexlify(im.view(255, 0,1,1).tostring()), '404040ff')

    def _test_dataraster_16bsi(lbl, tilesize, constraint, overview):
      rf = os.path.join(execution_path('.'),'../data/raster/dataraster.tif')
      import_raster(rf, 'dataraster', tilesize, constraint, overview)
      if constraint:
        lbl += ' C'
      if tilesize:
        lbl += ' T:' + tilesize
      if overview:
        lbl += ' O:' + overview
      for prescale in [0,1]:
        for clip in [0,1]:
          _test_dataraster_16bsi_rendering(lbl, overview, prescale, clip)
      drop_imported('dataraster', overview)

    def test_dataraster_16bsi():
      for tilesize in ['','256x256']:
        for constraint in [0,1]:
          for overview in ['','4','2,16']:
            _test_dataraster_16bsi('data_16bsi', tilesize, constraint, overview)

    # river.tiff, RGBA 8BUI
    def _test_rgba_8bui_rendering(lbl, overview, rescale, clip):
      if rescale:
        lbl += ' Sc'
      if clip:
        lbl += ' Cl'
      ds = mapnik.PgRaster(dbname=MAPNIK_TEST_DBNAME,table='river',
        use_overviews=1 if overview else 0,
        prescale_rasters=rescale,clip_rasters=clip)
      fs = ds.featureset()
      feature = fs.next()
      eq_(feature['rid'],1)
      lyr = mapnik.Layer('rgba_8bui')
      lyr.datasource = ds
      expenv = mapnik.Box2d(0, -210, 256, 0)
      env = lyr.envelope()
      # As the input size is a prime number both horizontally
      # and vertically, we expect the extent of the overview
      # tables to be a pixel wider than the original, whereas
      # the pixel size in geographical units depends on the
      # overview factor. So we start with the original pixel size
      # as base scale and multiply by the overview factor.
      # NOTE: the overview table extent only grows north and east
      pixsize = 1 # see gdalinfo river.tif
      tol = pixsize * max(overview.split(',')) if overview else 0
      assert_almost_equal(env.minx, expenv.minx)
      assert_almost_equal(env.miny, expenv.miny, delta=tol) 
      assert_almost_equal(env.maxx, expenv.maxx, delta=tol)
      assert_almost_equal(env.maxy, expenv.maxy)
      mm = mapnik.Map(256, 256)
      style = mapnik.Style()
      sym = mapnik.RasterSymbolizer()
      rule = mapnik.Rule()
      rule.symbols.append(sym)
      style.rules.append(rule)
      mm.append_style('foo', style)
      lyr.styles.append('foo')
      mm.layers.append(lyr)
      mm.zoom_to_box(expenv)
      im = mapnik.Image(mm.width, mm.height)
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:full'
      #im.save('/tmp/xfull.png') # for debugging
      # no data
      eq_(hexlify(im.view(3,3,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(250,250,1,1).tostring()), '00000000') 
      # full opaque river color
      eq_(hexlify(im.view(175,118,1,1).tostring()), 'b9d8f8ff') 
      # half-transparent pixel
      pxstr = hexlify(im.view(122,138,1,1).tostring())
      apat = ".*(..)$"
      match = re.match(apat, pxstr)
      assert match, 'pixel ' + pxstr + ' does not match pattern ' + apat
      alpha = match.group(1)
      assert alpha != 'ff' and alpha != '00', \
        'unexpected full transparent/opaque pixel: ' + alpha

      # Now zoom over a portion of the env (1/10)
      newenv = mapnik.Box2d(166,-105,191,-77)
      mm.zoom_to_box(newenv)
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:1/10'
      #im.save('/tmp/xtenth.png') # for debugging
      # no data
      eq_(hexlify(im.view(255,255,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(200,40,1,1).tostring()), '00000000')
      # full opaque river color
      eq_(hexlify(im.view(100,168,1,1).tostring()), 'b9d8f8ff')
      # half-transparent pixel
      pxstr = hexlify(im.view(122,138,1,1).tostring())
      apat = ".*(..)$"
      match = re.match(apat, pxstr)
      assert match, 'pixel ' + pxstr + ' does not match pattern ' + apat
      alpha = match.group(1)
      assert alpha != 'ff' and alpha != '00', \
        'unexpected full transparent/opaque pixel: ' + alpha

    def _test_rgba_8bui(lbl, tilesize, constraint, overview):
      rf = os.path.join(execution_path('.'),'../data/raster/river.tiff')
      import_raster(rf, 'river', tilesize, constraint, overview)
      if constraint:
        lbl += ' C'
      if tilesize:
        lbl += ' T:' + tilesize
      if overview:
        lbl += ' O:' + overview
      for prescale in [0,1]:
        for clip in [0,1]:
          _test_rgba_8bui_rendering(lbl, overview, prescale, clip)
      drop_imported('river', overview)

    def test_rgba_8bui():
      for tilesize in ['','16x16']:
        for constraint in [0,1]:
          for overview in ['2']:
            _test_rgba_8bui('rgba_8bui', tilesize, constraint, overview)

    # nodata-edge.tif, RGB 8BUI
    def _test_rgb_8bui_rendering(lbl, tnam, overview, rescale, clip):
      if rescale:
        lbl += ' Sc'
      if clip:
        lbl += ' Cl'
      ds = mapnik.PgRaster(dbname=MAPNIK_TEST_DBNAME,table=tnam,
        use_overviews=1 if overview else 0,
        prescale_rasters=rescale,clip_rasters=clip)
      fs = ds.featureset()
      feature = fs.next()
      eq_(feature['rid'],1)
      lyr = mapnik.Layer('rgba_8bui')
      lyr.datasource = ds
      expenv = mapnik.Box2d(-12329035.7652168,4508650.39854396, \
                            -12328653.0279471,4508957.34625536)
      env = lyr.envelope()
      # As the input size is a prime number both horizontally
      # and vertically, we expect the extent of the overview
      # tables to be a pixel wider than the original, whereas
      # the pixel size in geographical units depends on the
      # overview factor. So we start with the original pixel size
      # as base scale and multiply by the overview factor.
      # NOTE: the overview table extent only grows north and east
      pixsize = 2 # see gdalinfo nodata-edge.tif
      tol = pixsize * max(overview.split(',')) if overview else 0
      assert_almost_equal(env.minx, expenv.minx, places=0)
      assert_almost_equal(env.miny, expenv.miny, delta=tol)
      assert_almost_equal(env.maxx, expenv.maxx, delta=tol)
      assert_almost_equal(env.maxy, expenv.maxy, places=0)
      mm = mapnik.Map(256, 256)
      style = mapnik.Style()
      sym = mapnik.RasterSymbolizer()
      rule = mapnik.Rule()
      rule.symbols.append(sym)
      style.rules.append(rule)
      mm.append_style('foo', style)
      lyr.styles.append('foo')
      mm.layers.append(lyr)
      mm.zoom_to_box(expenv)
      im = mapnik.Image(mm.width, mm.height)
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:full'
      #im.save('/tmp/xfull.png') # for debugging
      # no data
      eq_(hexlify(im.view(3,16,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(128,16,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(250,16,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(3,240,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(128,240,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(250,240,1,1).tostring()), '00000000')
      # dark brown
      eq_(hexlify(im.view(174,39,1,1).tostring()), 'c3a698ff') 
      # dark gray
      eq_(hexlify(im.view(195,132,1,1).tostring()), '575f62ff') 
      # Now zoom over a portion of the env (1/10)
      newenv = mapnik.Box2d(-12329035.7652168, 4508926.651484220, \
                            -12328997.49148983,4508957.34625536)
      mm.zoom_to_box(newenv)
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:1/10'
      #im.save('/tmp/xtenth.png') # for debugging
      # no data
      eq_(hexlify(im.view(3,16,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(128,16,1,1).tostring()), '00000000')
      eq_(hexlify(im.view(250,16,1,1).tostring()), '00000000')
      # black
      eq_(hexlify(im.view(3,42,1,1).tostring()), '000000ff')
      eq_(hexlify(im.view(3,134,1,1).tostring()), '000000ff')
      eq_(hexlify(im.view(3,244,1,1).tostring()), '000000ff')
      # gray
      eq_(hexlify(im.view(135,157,1,1).tostring()), '4e555bff')
      # brown
      eq_(hexlify(im.view(195,223,1,1).tostring()), 'f2cdbaff')

    def _test_rgb_8bui(lbl, tilesize, constraint, overview):
      rf = os.path.join(execution_path('.'),'../data/raster/nodata-edge.tif')
      tnam = 'nodataedge'
      import_raster(rf, tnam, tilesize, constraint, overview)
      if constraint:
        lbl += ' C'
      if tilesize:
        lbl += ' T:' + tilesize
      if overview:
        lbl += ' O:' + overview
      for prescale in [0,1]:
        for clip in [0,1]:
          _test_rgb_8bui_rendering(lbl, tnam, overview, prescale, clip)
      #drop_imported(tnam, overview)

    def test_rgb_8bui():
      for tilesize in ['64x64']:
        for constraint in [1]:
          for overview in ['']:
            _test_rgb_8bui('rgb_8bui', tilesize, constraint, overview)

    def _test_grayscale_subquery(lbl,pixtype,value):
      sql = "(select 3 as i, ST_AsRaster(ST_MakeEnvelope(0,0,10,10), " \
            "1.0, 1.0, '%s', %s) as r) as foo" % (pixtype,value)
      overview = ''
      rescale = 0
      clip = 0
      if rescale:
        lbl += ' Sc'
      if clip:
        lbl += ' Cl'
      ds = mapnik.PgRaster(dbname=MAPNIK_TEST_DBNAME, table=sql,
        raster_field='r', use_overviews=0 if overview else 0,
        prescale_rasters=rescale,clip_rasters=clip)
      fs = ds.featureset()
      feature = fs.next()
      eq_(feature['i'],3)
      lyr = mapnik.Layer('grayscale_8bui_subquery')
      lyr.datasource = ds
      expenv = mapnik.Box2d(0,0,10,10)
      env = lyr.envelope()
      assert_almost_equal(env.minx, expenv.minx, places=0)
      assert_almost_equal(env.miny, expenv.miny, places=0)
      assert_almost_equal(env.maxx, expenv.maxx, places=0)
      assert_almost_equal(env.maxy, expenv.maxy, places=0)
      mm = mapnik.Map(16, 16)
      style = mapnik.Style()
      sym = mapnik.RasterSymbolizer()
      rule = mapnik.Rule()
      rule.symbols.append(sym)
      style.rules.append(rule)
      mm.append_style('foo', style)
      lyr.styles.append('foo')
      mm.layers.append(lyr)
      mm.zoom_to_box(expenv)
      im = mapnik.Image(mm.width, mm.height)
      t0 = time.time() # we want wall time to include IO waits
      mapnik.render(mm, im)
      lap = time.time() - t0
      print 'T ' + str(lap) + ' -- ' + lbl + ' E:full'
      im.save('/tmp/xfull.png') # for debugging
      h = hex(value)[2:]
      exphex = h+h+h+'ff'
      eq_(hexlify(im.view(1,1,1,1).tostring()), exphex);
      eq_(hexlify(im.view(8,1,1,1).tostring()), exphex);
      eq_(hexlify(im.view(15,1,1,1).tostring()), exphex);
      eq_(hexlify(im.view(1,8,1,1).tostring()), exphex);
      eq_(hexlify(im.view(8,8,1,1).tostring()), exphex);
      eq_(hexlify(im.view(15,8,1,1).tostring()), exphex);
      eq_(hexlify(im.view(1,15,1,1).tostring()), exphex);
      eq_(hexlify(im.view(8,15,1,1).tostring()), exphex);
      eq_(hexlify(im.view(15,15,1,1).tostring()), exphex);

    def test_grayscale_8bui_subquery():
      _test_grayscale_subquery('grayscale_8bui_subquery', '8BUI', 88)

    def test_grayscale_16bui_subquery():
      _test_grayscale_subquery('grayscale_16bui_subquery', '16BUI', 74)

    def test_grayscale_32bui_subquery():
      _test_grayscale_subquery('grayscale_16bui_subquery', '32BUI', 132)

    atexit.register(postgis_takedown)

def enabled(tname):
  enabled = len(sys.argv) < 2 or tname in sys.argv
  if not enabled:
    print "Skipping " + tname + " as not explicitly enabled"
  return enabled

if __name__ == "__main__":
    setup()
    run_all(eval(x) for x in dir() if x.startswith("test_") and enabled(x))
