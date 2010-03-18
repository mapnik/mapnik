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

def contains_word(word, bytestring_):
    """
    Checks that a bytestring contains a given word. len(bytestring) should be
    a multiple of len(word).

    >>> contains_word("abcd", "abcd"*5)
    True

    >>> contains_word("ab", "ba"*5)
    False

    >>> contains_word("ab", "ab"*5+"a")
    Traceback (most recent call last):
    ...
    AssertionError: len(bytestring_) not multiple of len(word)
    """
    n = len(word)
    assert len(bytestring_)%n == 0, "len(bytestring_) not multiple of len(word)"
    chunks = [bytestring_[i:i+n] for i in xrange(0, len(bytestring_), n)]
    return word in chunks
