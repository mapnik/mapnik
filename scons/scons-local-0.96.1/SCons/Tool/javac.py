"""SCons.Tool.javac

Tool-specific initialization for javac.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/javac.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os
import os.path
import string

import SCons.Builder
from SCons.Node.FS import _my_normcase
from SCons.Tool.JavaCommon import parse_java_file
import SCons.Util

def classname(path):
    """Turn a string (path name) into a Java class name."""
    return string.replace(os.path.normpath(path), os.sep, '.')

def emit_java_classes(target, source, env):
    """Create and return lists of source java files
    and their corresponding target class files.
    """
    java_suffix = env.get('JAVASUFFIX', '.java')
    class_suffix = env.get('JAVACLASSSUFFIX', '.class')

    slist = []
    js = _my_normcase(java_suffix)
    for sdir in source:
        def visit(arg, dirname, names, js=js, dirnode=sdir.rdir()):
            java_files = filter(lambda n, js=js:
                                _my_normcase(n[-len(js):]) == js,
                                names)
            mydir = dirnode.Dir(dirname)
            java_paths = map(lambda f, d=mydir: d.File(f), java_files)
            arg.extend(java_paths)
        os.path.walk(sdir.rdir().get_abspath(), visit, slist)

    tlist = []
    for f in slist:
        pkg_dir, classes = parse_java_file(f.get_abspath())
        if pkg_dir:
            for c in classes:
                t = target[0].Dir(pkg_dir).File(c+class_suffix)
                t.attributes.java_classdir = target[0]
                t.attributes.java_classname = classname(pkg_dir + os.sep + c)
                tlist.append(t)
        elif classes:
            for c in classes:
                t = target[0].File(c+class_suffix)
                t.attributes.java_classdir = target[0]
                t.attributes.java_classname = classname(c)
                tlist.append(t)
        else:
            # This is an odd end case:  no package and no classes.
            # Just do our best based on the source file name.
            base = str(f)[:-len(java_suffix)]
            t = target[0].File(base + class_suffix)
            t.attributes.java_classdir = target[0]
            t.attributes.java_classname = classname(base)
            tlist.append(t)

    return tlist, slist

JavaBuilder = SCons.Builder.Builder(action = '$JAVACCOM',
                    emitter = emit_java_classes,
                    target_factory = SCons.Node.FS.default_fs.Dir,
                    source_factory = SCons.Node.FS.default_fs.Dir)

def generate(env):
    """Add Builders and construction variables for javac to an Environment."""
    env['BUILDERS']['Java'] = JavaBuilder

    env['JAVAC']            = 'javac'
    env['JAVACFLAGS']       = SCons.Util.CLVar('')
    env['JAVACCOM']         = '$JAVAC $JAVACFLAGS -d ${TARGET.attributes.java_classdir} -sourcepath ${SOURCE.dir.rdir()} $SOURCES'
    env['JAVACLASSSUFFIX']  = '.class'
    env['JAVASUFFIX']       = '.java'

def exists(env):
    return env.Detect('javac')
