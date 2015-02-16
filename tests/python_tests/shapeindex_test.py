#!/usr/bin/env python

from nose.tools import eq_
from utilities import execution_path, run_all
from subprocess import Popen, PIPE
import shutil
import os
import fnmatch
import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_shapeindex():
    # first copy shapefiles to tmp directory
    source_dir = '../data/shp/'
    working_dir = '/tmp/mapnik-shp-tmp/'
    if os.path.exists(working_dir):
      shutil.rmtree(working_dir)
    shutil.copytree(source_dir,working_dir)
    matches = []
    for root, dirnames, filenames in os.walk('%s' % source_dir):
      for filename in fnmatch.filter(filenames, '*.shp'):
          matches.append(os.path.join(root, filename))
    for shp in matches:
      source_file = os.path.join(source_dir,os.path.relpath(shp,source_dir))
      dest_file = os.path.join(working_dir,os.path.relpath(shp,source_dir))
      ds = mapnik.Shapefile(file=source_file)
      count = 0;
      fs = ds.featureset()
      try:
        while (fs.next()):
          count = count+1
      except StopIteration:
        pass
      stdin, stderr = Popen('shapeindex %s' % dest_file, shell=True, stdout=PIPE, stderr=PIPE).communicate()
      ds2 = mapnik.Shapefile(file=dest_file)
      count2 = 0;
      fs = ds.featureset()
      try:
        while (fs.next()):
          count2 = count2+1
      except StopIteration:
        pass
      eq_(count,count2)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
