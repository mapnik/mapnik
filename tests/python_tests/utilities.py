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
