"""SCons.Debug

Code for debugging SCons internal things.  Not everything here is
guaranteed to work all the way back to Python 1.5.2, and shouldn't be
needed by most users.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Debug.py 0.96.1.D001 2004/08/23 09:55:29 knight"


# Recipe 14.10 from the Python Cookbook.
import string
import sys
try:
    import weakref
except ImportError:
    def logInstanceCreation(instance, name=None):
        pass
else:
    def logInstanceCreation(instance, name=None):
        if name is None:
            name = instance.__class__.__name__
        if not tracked_classes.has_key(name):
            tracked_classes[name] = []
        tracked_classes[name].append(weakref.ref(instance))



tracked_classes = {}

def string_to_classes(s):
    if s == '*':
        c = tracked_classes.keys()
        c.sort()
        return c
    else:
        return string.split(s)

def countLoggedInstances(classes, file=sys.stdout):
    for classname in string_to_classes(classes):
        file.write("%s: %d\n" % (classname, len(tracked_classes[classname])))

def listLoggedInstances(classes, file=sys.stdout):
    for classname in string_to_classes(classes):
        file.write('\n%s:\n' % classname)
        for ref in tracked_classes[classname]:
            obj = ref()
            if obj is not None:
                file.write('    %s\n' % repr(obj))

def dumpLoggedInstances(classes, file=sys.stdout):
    for classname in string_to_classes(classes):
        file.write('\n%s:\n' % classname)
        for ref in tracked_classes[classname]:
            obj = ref()
            if obj is not None:
                file.write('    %s:\n' % obj)
                for key, value in obj.__dict__.items():
                    file.write('        %20s : %s\n' % (key, value))



if sys.platform[:5] == "linux":
    # Linux doesn't actually support memory usage stats from getrusage().
    def memory():
        mstr = open('/proc/self/stat').read()
        mstr = string.split(mstr)[22]
        return int(mstr)
else:
    try:
        import resource
    except ImportError:
        def memory():
            return 0
    else:
        def memory():
            res = resource.getrusage(resource.RUSAGE_SELF)
            return res[4]
