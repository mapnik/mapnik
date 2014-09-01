#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all
from subprocess import Popen, PIPE, STDOUT
import os

import os, sys, glob, mapnik

def test_mapnik_config_no_args():
    process = Popen('mapnik-config', shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    result = process.communicate()
    eq_('Usage: mapnik-config' in result[0],True)
    eq_(result[1],'')
    eq_(process.returncode,1)

def test_mapnik_config_help():
    process = Popen('mapnik-config --help', shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    result = process.communicate()
    eq_('Usage: mapnik-config' in result[0],True)
    eq_(result[1],'')
    eq_(process.returncode,0)

def test_mapnik_config_help_short():
    process = Popen('mapnik-config -h', shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    result = process.communicate()
    eq_('Usage: mapnik-config' in result[0],True)
    eq_(result[1],'')
    eq_(process.returncode,0)

def test_mapnik_config_valid_opts():
    valid_args = [
      '-h',
      '--help',
      '-v',
      '--version',
      '--version-number',
      '--git-revision',
      '--git-describe',
      '--fonts',
      '--input-plugins',
      '--defines',
      '--prefix',
      '--lib-name',
      '--libs',
      '--dep-libs',
      '--ldflags',
      '--includes',
      '--dep-includes',
      '--cxxflags',
      '--cflags',
      '--all-flags',
      '--cxx'
    ]
    for item in valid_args:
        cmd = 'mapnik-config ' + item
        process = Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        result = process.communicate()
        eq_(process.returncode,0)
        eq_(len(result[0]) > 1,True,cmd)
        eq_(result[1],'')

def test_mapnik_config_invalid_option():
    cmd = 'mapnik-config --invalid-does-not-exist'
    process = Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    result = process.communicate()
    eq_(process.returncode,0)
    eq_(result[0].strip(),'')
    eq_(result[1].strip(),'unknown option --invalid-does-not-exist')

def test_mapnik_config_valid_and_invalid_option():
    cmd = 'mapnik-config --libs --invalid-does-not-exist'
    process = Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    result = process.communicate()
    eq_('mapnik' in result[0],True)
    eq_(result[1].strip(),'unknown option --invalid-does-not-exist')
    eq_(process.returncode,0)

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
