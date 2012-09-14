"""SCons.Tool.dvips

Tool-specific initialization for dvips.

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

__revision__ = "src/engine/SCons/Tool/dvips.py issue-2856:2676:d23b7a2f45e8 2012/08/05 15:38:28 garyo"

import SCons.Action
import SCons.Builder
import SCons.Tool.dvipdf
import SCons.Util

def DviPsFunction(target = None, source= None, env=None):
    result = SCons.Tool.dvipdf.DviPdfPsFunction(PSAction,target,source,env)
    return result

def DviPsStrFunction(target = None, source= None, env=None):
    """A strfunction for dvipdf that returns the appropriate
    command string for the no_exec options."""
    if env.GetOption("no_exec"):
        result = env.subst('$PSCOM',0,target,source)
    else:
        result = ''
    return result

PSAction = None
DVIPSAction = None
PSBuilder = None

def generate(env):
    """Add Builders and construction variables for dvips to an Environment."""
    global PSAction
    if PSAction is None:
        PSAction = SCons.Action.Action('$PSCOM', '$PSCOMSTR')

    global DVIPSAction
    if DVIPSAction is None:
        DVIPSAction = SCons.Action.Action(DviPsFunction, strfunction = DviPsStrFunction)

    global PSBuilder
    if PSBuilder is None:
        PSBuilder = SCons.Builder.Builder(action = PSAction,
                                          prefix = '$PSPREFIX',
                                          suffix = '$PSSUFFIX',
                                          src_suffix = '.dvi',
                                          src_builder = 'DVI',
                                          single_source=True)

    env['BUILDERS']['PostScript'] = PSBuilder
    
    env['DVIPS']      = 'dvips'
    env['DVIPSFLAGS'] = SCons.Util.CLVar('')
    # I'm not quite sure I got the directories and filenames right for variant_dir
    # We need to be in the correct directory for the sake of latex \includegraphics eps included files.
    env['PSCOM']      = 'cd ${TARGET.dir} && $DVIPS $DVIPSFLAGS -o ${TARGET.file} ${SOURCE.file}'
    env['PSPREFIX'] = ''
    env['PSSUFFIX'] = '.ps'

def exists(env):
    SCons.Tool.tex.generate_darwin(env)
    return env.Detect('dvips')

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
