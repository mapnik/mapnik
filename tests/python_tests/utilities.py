#!/usr/bin/env python

from nose.plugins.errorclass import ErrorClass, ErrorClassPlugin

import os, sys, inspect

def execution_path(filename):
    return os.path.join(os.path.dirname(sys._getframe(1).f_code.co_filename), filename)

class Todo(Exception):
    pass

class TodoPlugin(ErrorClassPlugin):
    name = "todo"

    todo = ErrorClass(Todo, label='TODO', isfailure=False)

def save_data(filename, data, key='MAPNIK_TEST_DATA_DIR'):
    """Saves bytestring 'data' into os.environ[key]/filename if
    key in os.environ"""
    if key in os.environ:
        dir = os.environ[key]
        if not os.path.exists(dir):
            os.makedirs(dir)
        fname = os.path.join(dir, filename)
        f = open(fname, 'w')
        try:
            f.write(data)
        finally:
            f.close()
