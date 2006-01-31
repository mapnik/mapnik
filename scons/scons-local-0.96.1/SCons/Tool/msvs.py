"""SCons.Tool.msvs

Tool-specific initialization for Microsoft Visual Studio project files.

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

__revision__ = "/home/scons/scons/branch.0/branch.96/baseline/src/engine/SCons/Tool/msvs.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import base64
import md5
import os.path
import pickle
import re
import string
import sys
import types

import SCons.Builder
import SCons.Node.FS
import SCons.Platform.win32
import SCons.Script.SConscript
import SCons.Util
import SCons.Warnings

##############################################################################
# Below here are the classes and functions for generation of
# DSP/DSW/SLN/VCPROJ files.
##############################################################################

def _hexdigest(s):
    """Return a string as a string of hex characters.
    """
    # NOTE:  This routine is a method in the Python 2.0 interface
    # of the native md5 module, but we want SCons to operate all
    # the way back to at least Python 1.5.2, which doesn't have it.
    h = string.hexdigits
    r = ''
    for c in s:
        i = ord(c)
        r = r + h[(i >> 4) & 0xF] + h[i & 0xF]
    return r

def _generateGUID(slnfile, name):
    """This generates a dummy GUID for the sln file to use.  It is
    based on the MD5 signatures of the sln filename plus the name of
    the project.  It basically just needs to be unique, and not
    change with each invocation."""
    solution = _hexdigest(md5.new(str(slnfile)+str(name)).digest()).upper()
    # convert most of the signature to GUID form (discard the rest)
    solution = "{" + solution[:8] + "-" + solution[8:12] + "-" + solution[12:16] + "-" + solution[16:20] + "-" + solution[20:32] + "}"
    return solution

# This is how we re-invoke SCons from inside MSVS Project files.
# The problem is that we might have been invoked as either scons.bat
# or scons.py.  If we were invoked directly as scons.py, then we could
# use sys.argv[0] to find the SCons "executable," but that doesn't work
# if we were invoked as scons.bat, which uses "python -c" to execute
# things and ends up with "-c" as sys.argv[0].  Consequently, we have
# the MSVS Project file invoke SCons the same way that scons.bat does,
# which works regardless of how we were invoked.
exec_script_main = "from os.path import join; import sys; sys.path = [ join(sys.prefix, 'Lib', 'site-packages', 'scons-0.96.1'), join(sys.prefix, 'scons-0.96.1'), join(sys.prefix, 'Lib', 'site-packages', 'scons'), join(sys.prefix, 'scons') ] + sys.path; import SCons.Script; SCons.Script.main()"
exec_script_main_xml = string.replace(exec_script_main, "'", "&apos;")

# The string for the Python executable we tell the Project file to use
# is either sys.executable or, if an external PYTHON_ROOT environment
# variable exists, $(PYTHON)ROOT\\python.exe (generalized a little to
# pluck the actual executable name from sys.executable).
try:
    python_root = os.environ['PYTHON_ROOT']
except KeyError:
    python_executable = sys.executable
else:
    python_executable = os.path.join('$(PYTHON_ROOT)',
                                     os.path.split(sys.executable)[1])

class Config:
    pass

class _DSPGenerator:
    """ Base class for DSP generators """
    def __init__(self, dspfile, source, env):
        if type(dspfile) == types.StringType:
            self.dspfile = os.path.abspath(dspfile)
        else:
            self.dspfile = dspfile.get_abspath()

        try:
            self.conspath = source[0].attributes.sconstruct.get_abspath()
        except KeyError:
            raise SCons.Errors.InternalError, \
                  "Unable to determine where the SConstruct is"

        self.config = Config()
        if env.has_key('variant'):
            self.config.variant = env['variant'].capitalize()
        else:
            raise SCons.Errors.InternalError, \
                  "You must specify a 'variant' argument (i.e. 'Debug' or " +\
                  "'Release') to create an MSVSProject."

        if env.has_key('buildtarget'):
            if type(env['buildtarget']) == types.StringType:
                self.config.buildtarget = os.path.abspath(env['buildtarget'])
            elif type(env['buildtarget']) == types.ListType:
                self.config.buildtarget = env['buildtarget'][0].get_abspath()
            else:
                self.config.buildtarget = env['buildtarget'].get_abspath()
        else:
            raise SCons.Errors.InternalError, \
                  "You must specify a target 'buildtarget' file argument (such as the target" +\
                  " executable) to create an MSVSProject."

        self.config.outdir = os.path.dirname(self.config.buildtarget)

        if type(source[0]) == types.StringType:
            self.source = os.path.abspath(source[0])
        else:
            self.source = source[0].get_abspath()
            
        self.env = env

        if self.env.has_key('name'):
            self.name = self.env['name']
        else:
            self.name = os.path.basename(SCons.Util.splitext(self.dspfile)[0])

        print "Adding '" + self.name + ' - ' + self.config.variant + "' to Visual Studio Project '" + str(dspfile) + "'"

        sourcenames = [
            ' Source Files',
            'Header Files',
            'Local Headers',
            'Resource Files',
            'Other Files']

        srcargs = [
            'srcs',
            'incs',
            'localincs',
            'resources',
            'misc']

        self.sources = {}
        for n in sourcenames:
            self.sources[n] = []

        self.configs = {}

        if os.path.exists(self.dspfile):
            self.Parse()

        for t in zip(sourcenames,srcargs):
            if self.env.has_key(t[1]):
                if type(self.env[t[1]]) == types.ListType:
                    for i in self.env[t[1]]:
                        if not i in self.sources[t[0]]:
                            self.sources[t[0]].append(i)
                else:
                    if not self.env[t[1]] in self.sources[t[0]]:
                        self.sources[t[0]].append(self.env[t[1]])

        for n in sourcenames:
            self.sources[n].sort()

        self.configs[self.config.variant] = self.config

    def Build(self):
        pass
        
class _GenerateV6DSP(_DSPGenerator):
    """Generates a Project file for MSVS 6.0"""

    def PrintHeader(self):
        name = self.name
        # pick a default config
        confkeys = self.configs.keys()
        confkeys.sort()

        self.file.write('# Microsoft Developer Studio Project File - Name="%s" - Package Owner=<4>\n'
                        '# Microsoft Developer Studio Generated Build File, Format Version 6.00\n'
                        '# ** DO NOT EDIT **\n\n'
                        '# TARGTYPE "Win32 (x86) External Target" 0x0106\n\n'
                        'CFG=%s - Win32 %s\n'
                        '!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n'
                        '!MESSAGE use the Export Makefile command and run\n'
                        '!MESSAGE \n'
                        '!MESSAGE NMAKE /f "%s.mak".\n'
                        '!MESSAGE \n'
                        '!MESSAGE You can specify a configuration when running NMAKE\n'
                        '!MESSAGE by defining the macro CFG on the command line. For example:\n'
                        '!MESSAGE \n'
                        '!MESSAGE NMAKE /f "%s.mak" CFG="%s - Win32 %s"\n'
                        '!MESSAGE \n'
                        '!MESSAGE Possible choices for configuration are:\n'
                        '!MESSAGE \n' % (name,name,confkeys[0],name,name,name,confkeys[0]))

        for kind in confkeys:
            self.file.write('!MESSAGE "%s - Win32 %s" (based on "Win32 (x86) External Target")\n' % (name, kind))
            
        self.file.write('!MESSAGE \n\n')

    def PrintProject(self):
        name = self.name
        self.file.write('# Begin Project\n'
                        '# PROP AllowPerConfigDependencies 0\n'
                        '# PROP Scc_ProjName ""\n'
                        '# PROP Scc_LocalPath ""\n\n')

        first = 1
        confkeys = self.configs.keys()
        confkeys.sort()
        for kind in confkeys:
            outdir = self.configs[kind].outdir
            buildtarget = self.configs[kind].buildtarget
            if first == 1:
                self.file.write('!IF  "$(CFG)" == "%s - Win32 %s"\n\n' % (name, kind))
                first = 0
            else:
                self.file.write('\n!ELSEIF  "$(CFG)" == "%s - Win32 %s"\n\n' % (name, kind))

            # have to write this twice, once with the BASE settings, and once without
            for base in ("BASE ",""):
                self.file.write('# PROP %sUse_MFC 0\n'
                                '# PROP %sUse_Debug_Libraries ' % (base, base))
                if kind.lower().find('debug') < 0:
                    self.file.write('0\n')
                else:
                    self.file.write('1\n')
                self.file.write('# PROP %sOutput_Dir "%s"\n'
                                '# PROP %sIntermediate_Dir "%s"\n' % (base,outdir,base,outdir))
                (d,c) = os.path.split(str(self.conspath))
                cmd = '"%s" -c "%s" -C %s -f %s %s' % (python_executable,
                                                       exec_script_main,
                                                       d, c, buildtarget)
                self.file.write('# PROP %sCmd_Line "%s"\n' 
                                '# PROP %sRebuild_Opt "-c && %s"\n'
                                '# PROP %sTarget_File "%s"\n'
                                '# PROP %sBsc_Name ""\n'
                                '# PROP %sTarget_Dir ""\n'\
                                %(base,cmd,base,cmd,base,buildtarget,base,base))
            
        self.file.write('\n!ENDIF\n\n'
                        '# Begin Target\n\n')
        for kind in confkeys:
            self.file.write('# Name "%s - Win32 %s"\n' % (name,kind))
        self.file.write('\n')
        first = 0
        for kind in confkeys:
            if first == 0:
                self.file.write('!IF  "$(CFG)" == "%s - Win32 %s"\n\n' % (name,kind))
                first = 1
            else:
                self.file.write('!ELSEIF  "$(CFG)" == "%s - Win32 %s"\n\n' % (name,kind))
        self.file.write('!ENDIF \n\n')
        self.PrintSourceFiles()
        self.file.write('# End Target\n'
                        '# End Project\n')
        
        # now we pickle some data and add it to the file -- MSDEV will ignore it.
        pdata = pickle.dumps(self.configs,1)
        pdata = base64.encodestring(pdata)
        self.file.write(pdata + '\n')
        pdata = pickle.dumps(self.sources,1)
        pdata = base64.encodestring(pdata)
        self.file.write(pdata + '\n')
  
    def PrintSourceFiles(self):
        categories = {' Source Files': 'cpp|c|cxx|l|y|def|odl|idl|hpj|bat',
                      'Header Files': 'h|hpp|hxx|hm|inl',
                      'Local Headers': 'h|hpp|hxx|hm|inl',
                      'Resource Files': 'r|rc|ico|cur|bmp|dlg|rc2|rct|bin|cnt|rtf|gif|jpg|jpeg|jpe',
                      'Other Files': ''}

        cats = categories.keys()
        cats.sort()
        for kind in cats:
            if not self.sources[kind]:
                continue # skip empty groups
            
            self.file.write('# Begin Group "' + kind + '"\n\n')
            typelist = categories[kind].replace('|',';')
            self.file.write('# PROP Default_Filter "' + typelist + '"\n')
            
            for file in self.sources[kind]:
                file = os.path.normpath(file)
                self.file.write('# Begin Source File\n\n'
                                'SOURCE="' + file + '"\n'
                                '# End Source File\n')
            self.file.write('# End Group\n')

        # add the Conscript file outside of the groups
        self.file.write('# Begin Source File\n\n'
                        'SOURCE="' + str(self.source) + '"\n'
                        '# End Source File\n')

    def Parse(self):
        try:
            dspfile = open(self.dspfile,'r')
        except IOError:
            return # doesn't exist yet, so can't add anything to configs.

        line = dspfile.readline()
        while line:
            if line.find("# End Project") > -1:
                break
            line = dspfile.readline()

        line = dspfile.readline()
        datas = line
        while line and line != '\n':
            line = dspfile.readline()
            datas = datas + line

        # OK, we've found our little pickled cache of data.
        try:
            datas = base64.decodestring(datas)
            data = pickle.loads(datas)
        except KeyboardInterrupt:
            raise
        except:
            return # unable to unpickle any data for some reason

        self.configs.update(data)

        data = None
        line = dspfile.readline()
        datas = line
        while line and line != '\n':
            line = dspfile.readline()
            datas = datas + line

        # OK, we've found our little pickled cache of data.
        # it has a "# " in front of it, so we strip that.
        try:
            datas = base64.decodestring(datas)
            data = pickle.loads(datas)
        except KeyboardInterrupt:
            raise
        except:
            return # unable to unpickle any data for some reason

        self.sources.update(data)
    
    def Build(self):
        try:
            self.file = open(self.dspfile,'w')
        except IOError, detail:
            raise SCons.Errors.InternalError, 'Unable to open "' + self.dspfile + '" for writing:' + str(detail)
        else:
            self.PrintHeader()
            self.PrintProject()
            self.file.close()

class _GenerateV7DSP(_DSPGenerator):
    """Generates a Project file for MSVS .NET"""

    def __init__(self, dspfile, source, env):
        _DSPGenerator.__init__(self, dspfile, source, env)
        self.version = float(env['MSVS_VERSION'])
        self.versionstr = '7.00'
        if self.version >= 7.1:
            self.versionstr = '7.10'

    def PrintHeader(self):
        self.file.write('<?xml version="1.0" encoding = "Windows-1252"?>\n'
                        '<VisualStudioProject\n'
                        '	ProjectType="Visual C++"\n'
                        '	Version="%s"\n'
                        '	Name="%s"\n'
                        '	SccProjectName=""\n'
                        '	SccLocalPath=""\n'
                        '	Keyword="MakeFileProj">\n'
                        '	<Platforms>\n'
                        '		<Platform\n'
                        '			Name="Win32"/>\n'
                        '	</Platforms>\n' % (self.versionstr, self.name))

    def PrintProject(self):
        

        self.file.write('	<Configurations>\n')

        confkeys = self.configs.keys()
        confkeys.sort()
        for kind in confkeys:
            outdir = self.configs[kind].outdir
            buildtarget = self.configs[kind].buildtarget

            (d,c) = os.path.split(str(self.conspath))
            cmd = '&quot;%s&quot; -c &quot;%s&quot; -C %s -f %s %s' % (python_executable,
                                                   exec_script_main_xml,
                                                   d, c, buildtarget)

            cleancmd = '&quot;%s&quot; -c &quot;%s&quot; -C %s -f %s -c %s' % (python_executable,
                                                         exec_script_main_xml,
                                                         d, c, buildtarget)

            self.file.write('		<Configuration\n'
                            '			Name="%s|Win32"\n'
                            '			OutputDirectory="%s"\n'
                            '			IntermediateDirectory="%s"\n'
                            '			ConfigurationType="0"\n'
                            '			UseOfMFC="0"\n'
                            '			ATLMinimizesCRunTimeLibraryUsage="FALSE">\n'
                            '			<Tool\n'
                            '				Name="VCNMakeTool"\n'
                            '				BuildCommandLine="%s"\n'
                            '				CleanCommandLine="%s"\n'
                            '				RebuildCommandLine="%s"\n'
                            '				Output="%s"/>\n'
                            '		</Configuration>\n' % (kind.capitalize(),outdir,outdir,\
                                                               cmd,cleancmd,cmd,buildtarget))
            
        self.file.write('	</Configurations>\n')
        if self.version >= 7.1:
            self.file.write('	<References>\n')
            self.file.write('	</References>\n')

        self.PrintSourceFiles()

        self.file.write('</VisualStudioProject>\n')

        # now we pickle some data and add it to the file -- MSDEV will ignore it.
        pdata = pickle.dumps(self.configs,1)
        pdata = base64.encodestring(pdata)
        self.file.write('<!-- SCons Data:\n' + pdata + '\n')
        pdata = pickle.dumps(self.sources,1)
        pdata = base64.encodestring(pdata)
        self.file.write(pdata + '-->\n')
  
    def PrintSourceFiles(self):
        categories = {' Source Files': 'cpp;c;cxx;l;y;def;odl;idl;hpj;bat',
                      'Header Files': 'h;hpp;hxx;hm;inl',
                      'Local Headers': 'h;hpp;hxx;hm;inl',
                      'Resource Files': 'r;rc;ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe',
                      'Other Files': ''}

        self.file.write('	<Files>\n')
        
        cats = categories.keys()
        cats.sort()
        for kind in cats:
            if not self.sources[kind]:
                continue # skip empty groups

            self.file.write('		<Filter\n'
                            '			Name="%s"\n'
                            '			Filter="%s">\n' % (kind, categories[kind]))
            
            for file in self.sources[kind]:
                file = os.path.normpath(file)
                self.file.write('			<File\n'
                                '				RelativePath="%s">\n'
                                '			</File>\n' % file)

            self.file.write('		</Filter>\n')

        # add the Conscript file outside of the groups
        self.file.write('		<File\n'
                        '			RelativePath="%s">\n'
                        '		</File>\n'
                        '	</Files>\n'
                        '	<Globals>\n'
                        '	</Globals>\n' % str(self.source))

    def Parse(self):
        try:
            dspfile = open(self.dspfile,'r')
        except IOError:
            return # doesn't exist yet, so can't add anything to configs.

        line = dspfile.readline()
        while line:
            if line.find('<!-- SCons Data:') > -1:
                break
            line = dspfile.readline()

        line = dspfile.readline()
        datas = line
        while line and line != '\n':
            line = dspfile.readline()
            datas = datas + line

        # OK, we've found our little pickled cache of data.
        try:
            datas = base64.decodestring(datas)
            data = pickle.loads(datas)
        except KeyboardInterrupt:
            raise
        except:
            return # unable to unpickle any data for some reason

        self.configs.update(data)

        data = None
        line = dspfile.readline()
        datas = line
        while line and line != '\n':
            line = dspfile.readline()
            datas = datas + line

        # OK, we've found our little pickled cache of data.
        try:
            datas = base64.decodestring(datas)
            data = pickle.loads(datas)
        except KeyboardInterrupt:
            raise
        except:
            return # unable to unpickle any data for some reason

        self.sources.update(data)
    
    def Build(self):
        try:
            self.file = open(self.dspfile,'w')
        except IOError, detail:
            raise SCons.Errors.InternalError, 'Unable to open "' + self.dspfile + '" for writing:' + str(detail)
        else:
            self.PrintHeader()
            self.PrintProject()
            self.file.close()

class _DSWGenerator:
    """ Base class for DSW generators """
    def __init__(self, dswfile, dspfile, source, env):
        self.dswfile = os.path.normpath(str(dswfile))
        self.dspfile = os.path.abspath(str(dspfile))
        self.env = env

        if self.env.has_key('name'):
            self.name = self.env['name']
        else:
            self.name = os.path.basename(SCons.Util.splitext(self.dspfile)[0])

    def Build(self):
        pass

class _GenerateV7DSW(_DSWGenerator):
    """Generates a Solution file for MSVS .NET"""
    def __init__(self, dswfile, dspfile, source, env):
        _DSWGenerator.__init__(self, dswfile, dspfile, source, env)

        self.version = float(self.env['MSVS_VERSION'])
        self.versionstr = '7.00'
        if self.version >= 7.1:
            self.versionstr = '8.00'

        if env.has_key('slnguid') and env['slnguid']:
            self.slnguid = env['slnguid']
        else:
            self.slnguid = _generateGUID(dswfile, self.name)

        self.config = Config()
        if env.has_key('variant'):
            self.config.variant = env['variant'].capitalize()
        else:
            raise SCons.Errors.InternalError, \
                  "You must specify a 'variant' argument (i.e. 'Debug' or " +\
                  "'Release') to create an MSVS Solution File."

        self.configs = {}

        if os.path.exists(self.dswfile):
            self.Parse()

        self.configs[self.config.variant] = self.config

    def Parse(self):
        try:
            dswfile = open(self.dswfile,'r')
        except IOError:
            return # doesn't exist yet, so can't add anything to configs.

        line = dswfile.readline()
        while line:
            if line[:9] == "EndGlobal":
                break
            line = dswfile.readline()

        line = dswfile.readline()
        datas = line
        while line:
            line = dswfile.readline()
            datas = datas + line

        # OK, we've found our little pickled cache of data.
        try:
            datas = base64.decodestring(datas)
            data = pickle.loads(datas)
        except KeyboardInterrupt:
            raise
        except:
            return # unable to unpickle any data for some reason

        self.configs.update(data)

    def PrintSolution(self):
        """Writes a solution file"""
        self.file.write('Microsoft Visual Studio Solution File, Format Version %s\n'
                        # the next line has the GUID for an external makefile project.
                        'Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "%s", "%s", "%s"\n'
                        % (self.versionstr, self.name, os.path.basename(self.dspfile), self.slnguid))
        if self.version >= 7.1:
            self.file.write('	ProjectSection(ProjectDependencies) = postProject\n'
                            '	EndProjectSection\n')
        self.file.write('EndProject\n'
                        'Global\n'
                        '	GlobalSection(SolutionConfiguration) = preSolution\n')
        confkeys = self.configs.keys()
        confkeys.sort()
        cnt = 0
        for name in confkeys:
            self.file.write('		ConfigName.%d = %s\n' % (cnt, name.capitalize()))
            cnt = cnt + 1
        self.file.write('	EndGlobalSection\n')
        if self.version < 7.1:
            self.file.write('	GlobalSection(ProjectDependencies) = postSolution\n'
                            '	EndGlobalSection\n')
        self.file.write('	GlobalSection(ProjectConfiguration) = postSolution\n')
        for name in confkeys:
            name = name.capitalize()
            self.file.write('		%s.%s.ActiveCfg = %s|Win32\n'
                            '		%s.%s.Build.0 = %s|Win32\n'  %(self.slnguid,name,name,self.slnguid,name,name))
        self.file.write('	EndGlobalSection\n'
                        '	GlobalSection(ExtensibilityGlobals) = postSolution\n'
                        '	EndGlobalSection\n'
                        '	GlobalSection(ExtensibilityAddIns) = postSolution\n'
                        '	EndGlobalSection\n'
                        'EndGlobal\n')
        pdata = pickle.dumps(self.configs,1)
        pdata = base64.encodestring(pdata)
        self.file.write(pdata + '\n')

    def Build(self):
        try:
            self.file = open(self.dswfile,'w')
        except IOError, detail:
            raise SCons.Errors.InternalError, 'Unable to open "' + self.dswfile + '" for writing:' + str(detail)
        else:
            self.PrintSolution()
            self.file.close()

class _GenerateV6DSW(_DSWGenerator):
    """Generates a Workspace file for MSVS 6.0"""

    def PrintWorkspace(self):
        """ writes a DSW file """
        self.file.write('Microsoft Developer Studio Workspace File, Format Version 6.00\n'
                        '# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n'
                        '\n'
                        '###############################################################################\n'
                        '\n'
                        'Project: "%s"="%s" - Package Owner=<4>\n'
                        '\n'
                        'Package=<5>\n'
                        '{{{\n'
                        '}}}\n'
                        '\n'
                        'Package=<4>\n'
                        '{{{\n'
                        '}}}\n'
                        '\n'
                        '###############################################################################\n'
                        '\n'
                        'Global:\n'
                        '\n'
                        'Package=<5>\n'
                        '{{{\n'
                        '}}}\n'
                        '\n'
                        'Package=<3>\n'
                        '{{{\n'
                        '}}}\n'
                        '\n'
                        '###############################################################################\n'\
                         %(self.name,self.dspfile))

    def Build(self):
        try:
            self.file = open(self.dswfile,'w')
        except IOError, detail:
            raise SCons.Errors.InternalError, 'Unable to open "' + self.dswfile + '" for writing:' + str(detail)
        else:
            self.PrintWorkspace()
            self.file.close()


def GenerateDSP(dspfile, source, env):
    """Generates a Project file based on the version of MSVS that is being used"""

    if env.has_key('MSVS_VERSION') and float(env['MSVS_VERSION']) >= 7.0:
        g = _GenerateV7DSP(dspfile, source, env)
        g.Build()
    else:
        g = _GenerateV6DSP(dspfile, source, env)
        g.Build()

def GenerateDSW(dswfile, dspfile, source, env):
    """Generates a Solution/Workspace file based on the version of MSVS that is being used"""
    
    if env.has_key('MSVS_VERSION') and float(env['MSVS_VERSION']) >= 7.0:
        g = _GenerateV7DSW(dswfile, dspfile, source, env)
        g.Build()
    else:
        g = _GenerateV6DSW(dswfile, dspfile, source, env)
        g.Build()


##############################################################################
# Above here are the classes and functions for generation of
# DSP/DSW/SLN/VCPROJ files.
##############################################################################

def get_default_visualstudio_version(env):
    """Returns the version set in the env, or the latest version
    installed, if it can find it, or '6.0' if all else fails.  Also
    updated the environment with what it found."""

    version = '6.0'
    versions = [version]
    if not env.has_key('MSVS') or type(env['MSVS']) != types.DictType:
        env['MSVS'] = {}

    if env.has_key('MSVS_VERSION'):
        version = env['MSVS_VERSION']
        versions = [version]
    else:
        if SCons.Util.can_read_reg:
            versions = get_visualstudio_versions()
            if versions:
                version = versions[0] #use highest version by default

    env['MSVS_VERSION'] = version
    env['MSVS']['VERSIONS'] = versions
    env['MSVS']['VERSION'] = version
    
    return version

def get_visualstudio_versions():
    """
    Get list of visualstudio versions from the Windows registry.  Return a
    list of strings containing version numbers; an exception will be raised
    if we were unable to access the registry (eg. couldn't import
    a registry-access module) or the appropriate registry keys weren't
    found.
    """

    if not SCons.Util.can_read_reg:
        return []

    HLM = SCons.Util.HKEY_LOCAL_MACHINE
    K = r'Software\Microsoft\VisualStudio'
    L = []
    try:
        k = SCons.Util.RegOpenKeyEx(HLM, K)
        i = 0
        while 1:
            try:
                p = SCons.Util.RegEnumKey(k,i)
            except SCons.Util.RegError:
                break
            i = i + 1
            if not p[0] in '123456789' or p in L:
                continue
            # Only add this version number if there is a valid
            # registry structure (includes the "Setup" key),
            # and at least some of the correct directories
            # exist.  Sometimes VS uninstall leaves around
            # some registry/filesystem turds that we don't
            # want to trip over.  Also, some valid registry
            # entries are MSDN entries, not MSVS ('7.1',
            # notably), and we want to skip those too.
            try:
                SCons.Util.RegOpenKeyEx(HLM, K + '\\' + p + '\\Setup')
            except SCons.Util.RegError:
                continue

            id = []
            idk = SCons.Util.RegOpenKeyEx(HLM, K + '\\' + p)
            # This is not always here -- it only exists if the
            # user installed into a non-standard location (at
            # least in VS6 it works that way -- VS7 seems to
            # always write it)
            try:
                id = SCons.Util.RegQueryValueEx(idk, 'InstallDir')
            except SCons.Util.RegError:
                pass

            # If the InstallDir key doesn't exist,
            # then we check the default locations.
            if not id or not id[0]:
                files_dir = SCons.Platform.win32.get_program_files_dir()
                if float(p) < 7.0:
                    vs = r'Microsoft Visual Studio\Common\MSDev98'
                else:
                    vs = r'Microsoft Visual Studio .NET\Common7\IDE'
                id = [ os.path.join(files_dir, vs) ]
            if os.path.exists(id[0]):
                L.append(p)
    except SCons.Util.RegError:
        pass

    if not L:
        return []

    # This is a hack to get around the fact that certain Visual Studio
    # patches place a "6.1" version in the registry, which does not have
    # any of the keys we need to find include paths, install directories,
    # etc.  Therefore we ignore it if it is there, since it throws all
    # other logic off.
    try:
        L.remove("6.1")
    except ValueError:
        pass
    
    L.sort()
    L.reverse()

    return L

def is_msvs_installed():
    """
    Check the registry for an installed visual studio.
    """
    try:
        v = SCons.Tool.msvs.get_visualstudio_versions()
        return v
    except (SCons.Util.RegError, SCons.Errors.InternalError):
        return 0

def get_msvs_install_dirs(version = None):
    """
    Get installed locations for various msvc-related products, like the .NET SDK
    and the Platform SDK.
    """

    if not SCons.Util.can_read_reg:
        return {}

    if not version:
        versions = get_visualstudio_versions()
        if versions:
            version = versions[0] #use highest version by default
        else:
            return {}

    K = 'Software\\Microsoft\\VisualStudio\\' + version

    # vc++ install dir
    rv = {}
    try:
        if (float(version) < 7.0):
            (rv['VCINSTALLDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
                                                             K + r'\Setup\Microsoft Visual C++\ProductDir')
        else:
            (rv['VCINSTALLDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
                                                             K + r'\Setup\VC\ProductDir')
    except SCons.Util.RegError:
        pass

    # visual studio install dir
    if (float(version) < 7.0):
        try:
            (rv['VSINSTALLDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
                                                             K + r'\Setup\Microsoft Visual Studio\ProductDir')
        except SCons.Util.RegError:
            pass

        if not rv.has_key('VSINSTALLDIR') or not rv['VSINSTALLDIR']:
            if rv.has_key('VCINSTALLDIR') and rv['VCINSTALLDIR']:
                rv['VSINSTALLDIR'] = os.path.dirname(rv['VCINSTALLDIR'])
            else:
                rv['VSINSTALLDIR'] = os.path.join(SCons.Platform.win32.get_program_files_dir(),'Microsoft Visual Studio')
    else:
        try:
            (rv['VSINSTALLDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
                                                             K + r'\Setup\VS\ProductDir')
        except SCons.Util.RegError:
            pass

    # .NET framework install dir
    try:
        (rv['FRAMEWORKDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
            r'Software\Microsoft\.NETFramework\InstallRoot')
    except SCons.Util.RegError:
        pass

    if rv.has_key('FRAMEWORKDIR'):
        # try and enumerate the installed versions of the .NET framework.
        contents = os.listdir(rv['FRAMEWORKDIR'])
        l = re.compile('v[0-9]+.*')
        versions = []
        for entry in contents:
            if l.match(entry):
                versions.append(entry)

        def versrt(a,b):
            # since version numbers aren't really floats...
            aa = a[1:]
            bb = b[1:]
            aal = aa.split('.')
            bbl = bb.split('.')
            c = int(bbl[0]) - int(aal[0])
            if c == 0:
                c = int(bbl[1]) - int(aal[1])
                if c == 0:
                    c = int(bbl[2]) - int(aal[2])
            return c
        
        versions.sort(versrt)

        rv['FRAMEWORKVERSIONS'] = versions
        # assume that the highest version is the latest version installed
        rv['FRAMEWORKVERSION'] = versions[0]

    # .NET framework SDK install dir
    try:
        if rv.has_key('FRAMEWORKVERSION') and rv['FRAMEWORKVERSION'][:4] == 'v1.1':
            key = r'Software\Microsoft\.NETFramework\sdkInstallRootv1.1'
        else:
            key = r'Software\Microsoft\.NETFramework\sdkInstallRoot'

        (rv['FRAMEWORKSDKDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,key)

    except SCons.Util.RegError:
        pass

    # MS Platform SDK dir
    try:
        (rv['PLATFORMSDKDIR'], t) = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
            r'Software\Microsoft\MicrosoftSDK\Directories\Install Dir')
    except SCons.Util.RegError:
        pass

    if rv.has_key('PLATFORMSDKDIR'):
        # if we have a platform SDK, try and get some info on it.
        vers = {}
        try:
            loc = r'Software\Microsoft\MicrosoftSDK\InstalledSDKs'
            k = SCons.Util.RegOpenKeyEx(SCons.Util.HKEY_LOCAL_MACHINE,loc)
            i = 0
            while 1:
                try:
                    key = SCons.Util.RegEnumKey(k,i)
                    sdk = SCons.Util.RegOpenKeyEx(k,key)
                    j = 0
                    name = ''
                    date = ''
                    version = ''
                    while 1:
                        try:
                            (vk,vv,t) = SCons.Util.RegEnumValue(sdk,j)
                            if vk.lower() == 'keyword':
                                name = vv
                            if vk.lower() == 'propagation_date':
                                date = vv
                            if vk.lower() == 'version':
                                version = vv
                            j = j + 1
                        except SCons.Util.RegError:
                            break
                    if name:
                        vers[name] = (date, version)
                    i = i + 1
                except SCons.Util.RegError:
                    break
            rv['PLATFORMSDK_MODULES'] = vers
        except SCons.Util.RegError:
            pass

    return rv;

def GetMSVSProjectSuffix(target, source, env, for_signature):
     return env['MSVS']['PROJECTSUFFIX'];

def GetMSVSSolutionSuffix(target, source, env, for_signature):
     return env['MSVS']['SOLUTIONSUFFIX'];

def GenerateProject(target, source, env):
    # generate the dsp file, according to the version of MSVS.
    builddspfile = target[0]
    builddswfile = target[1]
    dswfile = builddswfile.srcnode()
    dspfile = builddspfile.srcnode()

#     print "SConscript    :",str(source[0])
#     print "DSW file      :",dswfile
#     print "DSP file      :",dspfile
#     print "Build DSW file:",builddswfile
#     print "Build DSP file:",builddspfile

    # this detects whether or not we're using a BuildDir
    if os.path.abspath(os.path.normcase(str(dspfile))) != \
           os.path.abspath(os.path.normcase(str(builddspfile))):
        try:
            bdsp = open(str(builddspfile), "w+")
        except IOError, detail:
            print 'Unable to open "' + str(dspfile) + '" for writing:',detail,'\n'
            raise

        bdsp.write("This is just a placeholder file.\nThe real project file is here:\n%s\n" % dspfile.get_abspath())

        try:
            bdsw = open(str(builddswfile), "w+")
        except IOError, detail:
            print 'Unable to open "' + str(dspfile) + '" for writing:',detail,'\n'
            raise

        bdsw.write("This is just a placeholder file.\nThe real workspace file is here:\n%s\n" % dswfile.get_abspath())

    GenerateDSP(dspfile, source, env)
    GenerateDSW(dswfile, dspfile, source, env) 

def projectEmitter(target, source, env):
    """Sets up the DSP and DSW dependencies for an SConscript file."""

    if source[0] == target[0]:
        source = []

    # make sure the suffix is correct for the version of MSVS we're running.
    (base, suff) = SCons.Util.splitext(str(target[0]))
    suff = env.subst('$MSVSPROJECTSUFFIX')
    target[0] = base + suff

    dspfile = SCons.Node.FS.default_fs.File(target[0]).srcnode()
    dswfile = SCons.Node.FS.default_fs.File(SCons.Util.splitext(str(dspfile))[0] + env.subst('$MSVSSOLUTIONSUFFIX'))

    if not source:
        source = [SCons.Script.SConscript.stack[-1].sconscript.srcnode()]

    source[0].attributes.sconstruct = SCons.Script.SConscript.stack[0].sconscript

    bdswpath = SCons.Util.splitext(str(target[0]))[0] + env.subst('$MSVSSOLUTIONSUFFIX')
    bdswfile = SCons.Node.FS.default_fs.File(bdswpath)

    # only make these side effects if they're
    # not the same file.
    if os.path.abspath(os.path.normcase(str(dspfile))) != \
           os.path.abspath(os.path.normcase(str(target[0]))):
        env.SideEffect(dspfile, target[0])
        env.Precious(dspfile)
        # dswfile isn't precious -- it can be blown away and rewritten each time.
        env.SideEffect(dswfile, target[0])
    
    return ([target[0],bdswfile], source)

projectGeneratorAction = SCons.Action.Action(GenerateProject, None)

projectBuilder = SCons.Builder.Builder(action = '$MSVSPROJECTCOM',
                                       suffix = '$MSVSPROJECTSUFFIX',
                                       emitter = projectEmitter)

def generate(env):
    """Add Builders and construction variables for Microsoft Visual
    Studio project files to an Environment."""
    try:
        env['BUILDERS']['MSVSProject']
    except KeyError:
        env['BUILDERS']['MSVSProject'] = projectBuilder

    env['MSVSPROJECTCOM'] = projectGeneratorAction

    try:
        version = get_default_visualstudio_version(env)
        # keep a record of some of the MSVS info so the user can use it.
        dirs = get_msvs_install_dirs(version)
        env['MSVS'].update(dirs)
    except (SCons.Util.RegError, SCons.Errors.InternalError):
        # we don't care if we can't do this -- if we can't, it's
        # because we don't have access to the registry, or because the
        # tools aren't installed.  In either case, the user will have to
        # find them on their own.
        pass

    if (float(env['MSVS_VERSION']) < 7.0):
        env['MSVS']['PROJECTSUFFIX']  = '.dsp'
        env['MSVS']['SOLUTIONSUFFIX'] = '.dsw'
    else:
        env['MSVS']['PROJECTSUFFIX']  = '.vcproj'
        env['MSVS']['SOLUTIONSUFFIX'] = '.sln'

    env['GET_MSVSPROJECTSUFFIX']  = GetMSVSProjectSuffix
    env['GET_MSVSSOLUTIONSUFFIX']  = GetMSVSSolutionSuffix
    env['MSVSPROJECTSUFFIX']  = '${GET_MSVSPROJECTSUFFIX}'
    env['MSVSSOLUTIONSUFFIX']  = '${GET_MSVSSOLUTIONSUFFIX}'

def exists(env):
    try:
        v = SCons.Tool.msvs.get_visualstudio_versions()
    except (SCons.Util.RegError, SCons.Errors.InternalError):
        pass
    
    if not v:
        if env.has_key('MSVS_VERSION') and float(env['MSVS_VERSION']) >= 7.0:
            return env.Detect('devenv')
        else:
            return env.Detect('msdev')
    else:
        # there's at least one version of MSVS installed.
        return 1

