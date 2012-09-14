""" msgmerget tool 

Tool specific initialization for `msgmerge` tool.
"""

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

__revision__ = "src/engine/SCons/Tool/msgmerge.py issue-2856:2676:d23b7a2f45e8 2012/08/05 15:38:28 garyo"

#############################################################################
def _update_or_init_po_files(target, source, env):
  """ Action function for `POUpdate` builder """
  import SCons.Action
  from SCons.Tool.GettextCommon import _init_po_files
  for tgt in target:
    if tgt.rexists():
      action = SCons.Action.Action('$MSGMERGECOM', '$MSGMERGECOMSTR')
    else:
      action = _init_po_files
    status = action([tgt], source, env)
    if status : return status
  return 0
#############################################################################

#############################################################################
def _POUpdateBuilder(env, **kw):
  """ Create an object of `POUpdate` builder """
  import SCons.Action
  from SCons.Tool.GettextCommon import _POFileBuilder
  action = SCons.Action.Action(_update_or_init_po_files, None)
  return _POFileBuilder(env, action=action, target_alias='$POUPDATE_ALIAS')
#############################################################################

#############################################################################
from SCons.Environment import _null
#############################################################################
def _POUpdateBuilderWrapper(env, target=None, source=_null, **kw):
  """ Wrapper for `POUpdate` builder - make user's life easier """
  if source is _null:
    if 'POTDOMAIN' in kw:
      domain = kw['POTDOMAIN']
    elif env.has_key('POTDOMAIN') and env['POTDOMAIN']:
      domain = env['POTDOMAIN']
    else:
      domain = 'messages'
    source = [ domain ] # NOTE: Suffix shall be appended automatically
  return env._POUpdateBuilder(target, source, **kw)
#############################################################################

#############################################################################
def generate(env,**kw):
  """ Generate the `xgettext` tool """
  from SCons.Tool.GettextCommon import _detect_msgmerge
  env['MSGMERGE'] = _detect_msgmerge(env)
  env.SetDefault(
    POTSUFFIX = ['.pot'],
    POSUFFIX = ['.po'],
    MSGMERGECOM = '$MSGMERGE  $MSGMERGEFLAGS --update $TARGET $SOURCE',
    MSGMERGECOMSTR = '',
    MSGMERGEFLAGS = [ ],
    POUPDATE_ALIAS = 'po-update'
  )
  env.Append(BUILDERS = { '_POUpdateBuilder':_POUpdateBuilder(env) })
  env.AddMethod(_POUpdateBuilderWrapper, 'POUpdate')
  env.AlwaysBuild(env.Alias('$POUPDATE_ALIAS'))
#############################################################################

#############################################################################
def exists(env):
  """ Check if the tool exists """
  from SCons.Tool.GettextCommon import _msgmerge_exists
  return  _msgmerge_exists(env)
#############################################################################

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
