"""SCons.Tool.tex

Tool-specific initialization for TeX.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/tex.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os.path
import re
import string

import SCons.Action
import SCons.Defaults
import SCons.Node
import SCons.Node.FS
import SCons.Util

# Define an action to build a generic tex file.  This is sufficient for all 
# tex files.
TeXAction = SCons.Action.CommandAction("$TEXCOM")

# Define an action to build a latex file.  This action might be needed more
# than once if we are dealing with labels and bibtex
LaTeXAction = SCons.Action.CommandAction("$LATEXCOM")

# Define an action to run BibTeX on a file.
BibTeXAction = SCons.Action.CommandAction("$BIBTEXCOM")

def LaTeXAuxAction(target = None, source= None, env=None):
    """A builder for LaTeX files that checks the output in the aux file
    and decides how many times to use LaTeXAction, and BibTeXAction."""
    # Get the base name of the target
    basename, ext = os.path.splitext(str(target[0]))
    # Run LaTeX once to generate a new aux file.
    LaTeXAction(target,source,env)
    # Now if bibtex will need to be run.
    content = open(basename + ".aux","rb").read()
    if string.find(content, "bibdata") != -1:
        bibfile = env.fs.File(basename)
        BibTeXAction(None,bibfile,env)
    # Now check if latex needs to be run yet again.
    for trial in range(3):
        content = open(basename + ".log","rb").read()
        if not re.search("^LaTeX Warning:.*Rerun",content,re.MULTILINE):
            break
        LaTeXAction(target,source,env)
    return 0

def TeXLaTeXAction(target = None, source= None, env=None):
    """A builder for TeX and LaTeX that scans the source file to
    decide the "flavor" of the source and then executes the appropriate
    program."""
    LaTeXFile = None
    for src in source:
	content = src.get_contents()
        if re.search("\\\\document(style|class)",content):
	   LaTeXFile = 1
           break
    if LaTeXFile:
	LaTeXAuxAction(target,source,env)
    else:
	TeXAction(target,source,env)
    return 0

def generate(env):
    """Add Builders and construction variables for TeX to an Environment."""
    try:
        bld = env['BUILDERS']['DVI']
    except KeyError:
        bld = SCons.Defaults.DVI()
        env['BUILDERS']['DVI'] = bld
        
    bld.add_action('.tex', TeXLaTeXAction)

    env['TEX']      = 'tex'
    env['TEXFLAGS'] = SCons.Util.CLVar('')
    env['TEXCOM']   = '$TEX $TEXFLAGS $SOURCES'

    # Duplicate from latex.py.  If latex.py goes away, then this is still OK.
    env['LATEX']      = 'latex'
    env['LATEXFLAGS'] = SCons.Util.CLVar('')
    env['LATEXCOM']   = '$LATEX $LATEXFLAGS $SOURCES'

    env['BIBTEX']      = 'bibtex'
    env['BIBTEXFLAGS'] = SCons.Util.CLVar('')
    env['BIBTEXCOM']   = '$BIBTEX $BIBTEXFLAGS $SOURCES'


def exists(env):
    return env.Detect('tex')
