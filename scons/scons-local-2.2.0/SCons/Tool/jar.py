"""SCons.Tool.jar

Tool-specific initialization for jar.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 The SCons Foundation
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

__revision__ = "src/engine/SCons/Tool/jar.py issue-2856:2676:d23b7a2f45e8 2012/08/05 15:38:28 garyo"

import SCons.Subst
import SCons.Util

def jarSources(target, source, env, for_signature):
    """Only include sources that are not a manifest file."""
    try:
        env['JARCHDIR']
    except KeyError:
        jarchdir_set = False
    else:
        jarchdir_set = True
        jarchdir = env.subst('$JARCHDIR', target=target, source=source)
        if jarchdir:
            jarchdir = env.fs.Dir(jarchdir)
    result = []
    for src in source:
        contents = src.get_text_contents()
        if contents[:16] != "Manifest-Version":
            if jarchdir_set:
                _chdir = jarchdir
            else:
                try:
                    _chdir = src.attributes.java_classdir
                except AttributeError:
                    _chdir = None
            if _chdir:
                # If we are changing the dir with -C, then sources should
                # be relative to that directory.
                src = SCons.Subst.Literal(src.get_path(_chdir))
                result.append('-C')
                result.append(_chdir)
            result.append(src)
    return result

def jarManifest(target, source, env, for_signature):
    """Look in sources for a manifest file, if any."""
    for src in source:
        contents = src.get_text_contents()
        if contents[:16] == "Manifest-Version":
            return src
    return ''

def jarFlags(target, source, env, for_signature):
    """If we have a manifest, make sure that the 'm'
    flag is specified."""
    jarflags = env.subst('$JARFLAGS', target=target, source=source)
    for src in source:
        contents = src.get_text_contents()
        if contents[:16] == "Manifest-Version":
            if not 'm' in jarflags:
                return jarflags + 'm'
            break
    return jarflags

def generate(env):
    """Add Builders and construction variables for jar to an Environment."""
    SCons.Tool.CreateJarBuilder(env)

    env['JAR']        = 'jar'
    env['JARFLAGS']   = SCons.Util.CLVar('cf')
    env['_JARFLAGS']  = jarFlags
    env['_JARMANIFEST'] = jarManifest
    env['_JARSOURCES'] = jarSources
    env['_JARCOM']    = '$JAR $_JARFLAGS $TARGET $_JARMANIFEST $_JARSOURCES'
    env['JARCOM']     = "${TEMPFILE('$_JARCOM')}"
    env['JARSUFFIX']  = '.jar'

def exists(env):
    # As reported by Jan Nijtmans in issue #2730, the simple
    #    return env.Detect('jar')
    # doesn't always work during initialization. For now, we
    # stop trying to detect an executable (analogous to the
    # javac Builder).
    # TODO: Come up with a proper detect() routine...and enable it.
    return 1

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
