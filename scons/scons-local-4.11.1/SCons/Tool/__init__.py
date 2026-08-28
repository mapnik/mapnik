#
# MIT License
#
# Copyright The SCons Foundation
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

"""SCons tool subsystem.

Tool specification modules are callable objects that modify construction
environments to dynamically enable specific types of builds. This module
provides the support for handling tool modules:

  - a Tool class to locate and load a tool module.
  - lists of default tools for supported platform types and a mechanism to
    select the available ones for the current platform.
  - various rules for name remapping, special-cased toolchains, etc.
  - utility functions for creating common Builders (eliminating duplication
    when multiple tool modules could potentially create a Builder).
  - create Scanners for common types used by the Builder utilities.

This is not set up like a traditional Python package: this file implements
the part that other subsystems can import and call. The remaining files
are either dynamically loaded tool modules that present entry points
(``exists()`` and ``generate()``) called through the Tool instance,
or support files/common logic that can be imported by those tool
modules. Neither are expected to be directly imported by any other SCons
subsystem (test code may reach in and do so).

Tool modules are simply callable objects that modify a construction
environment. You can define custom tool specifications in any callable
without needing to integrate with this subsystem.
"""

from __future__ import annotations

import sys
import os
import importlib.util

import SCons.Action
import SCons.Builder
import SCons.Errors
import SCons.Node.FS
import SCons.Scanner
import SCons.Scanner.C
import SCons.Scanner.D
import SCons.Scanner.Java
import SCons.Scanner.LaTeX
import SCons.Scanner.Prog
import SCons.Scanner.SWIG
import SCons.Util
from SCons.Tool.linkCommon import LibSymlinksActionFunction, LibSymlinksStrFun

DefaultToolpath = []

CScanner = SCons.Scanner.C.CScanner()
DScanner = SCons.Scanner.D.DScanner()
JavaScanner = SCons.Scanner.Java.JavaScanner()
LaTeXScanner = SCons.Scanner.LaTeX.LaTeXScanner()
PDFLaTeXScanner = SCons.Scanner.LaTeX.PDFLaTeXScanner()
ProgramScanner = SCons.Scanner.Prog.ProgramScanner()
SourceFileScanner = SCons.Scanner.ScannerBase({}, name='SourceFileScanner')
SWIGScanner = SCons.Scanner.SWIG.SWIGScanner()

CSuffixes = [".c", ".C", ".cxx", ".cpp", ".c++", ".cc",
             ".h", ".H", ".hxx", ".hpp", ".hh",
             ".F", ".fpp", ".FPP",
             ".m", ".mm",
             ".S", ".spp", ".SPP", ".sx"]

DSuffixes = ['.d']

IDLSuffixes = [".idl", ".IDL"]

LaTeXSuffixes = [".tex", ".ltx", ".latex"]

SWIGSuffixes = ['.i']

for suffix in CSuffixes:
    SourceFileScanner.add_scanner(suffix, CScanner)

for suffix in DSuffixes:
    SourceFileScanner.add_scanner(suffix, DScanner)

for suffix in SWIGSuffixes:
    SourceFileScanner.add_scanner(suffix, SWIGScanner)

# FIXME: what should be done here? Two scanners scan the same extensions,
# but look for different files, e.g., "picture.eps" vs. "picture.pdf".
# The builders for DVI and PDF explicitly reference their scanners
# I think that means this is not needed???
for suffix in LaTeXSuffixes:
    SourceFileScanner.add_scanner(suffix, LaTeXScanner)
    SourceFileScanner.add_scanner(suffix, PDFLaTeXScanner)

# Tool aliases are needed for those tools whose module names also
# occur in the python standard library (This causes module shadowing and
# can break using python library functions under python3)  or if the current tool/file names
# are not legal module names (violate python's identifier rules or are
# python language keywords).
TOOL_ALIASES = {
    'gettext': 'gettext_tool',
    'clang++': 'clangxx',
    'as': 'asm',
    'ninja' : 'ninja_tool'
}


class Tool:
    """A class for loading and applying tool modules.

    *name* is looked up using standard paths plus any specified *toolpath*.
    To avoid duplicate creation of instances, recognize if *name*
    is actually an existing instance, if so, just return ourselves
    without further setup.

    .. versionchanged:: 4.11.0
       Accept an existing instance at creation time and don't duplicate it.
    """

    def __new__(cls, name, toolpath=None, **kwargs) -> "Tool":
        if isinstance(name, Tool):
            return name
        return super().__new__(cls)

    def __init__(self, name, toolpath=None, **kwargs) -> None:
        if isinstance(name, Tool):
            return
        if toolpath is None:
            toolpath = []

        # Rename if there's a TOOL_ALIAS for this tool
        self.name = TOOL_ALIASES.get(name, name)
        self.toolpath = toolpath + DefaultToolpath
        # remember these so we can merge them into the call
        self.init_kw = kwargs

        module = self._tool_module()
        self.generate = module.generate
        self.exists = module.exists
        if hasattr(module, 'options'):
            self.options = module.options

    def _tool_module(self):
        """Try to load a tool module.

        This will hunt in the toolpath for both a Python file (toolname.py)
        and a Python module (toolname directory), then try the regular
        import machinery, then fallback to try a zipfile.
        """
        oldpythonpath = sys.path
        sys.path = self.toolpath + sys.path
        # These could be enabled under "if debug:"
        # sys.stderr.write(f"Tool: {self.name}\n")
        # sys.stderr.write(f"PATH: {sys.path}\n")
        # sys.stderr.write(f"toolpath: {self.toolpath}\n")
        # sys.stderr.write(f"SCONS.TOOL path: {sys.modules['SCons.Tool'].__path__}\n")
        debug = False
        spec = None
        found_name = self.name
        add_to_scons_tools_namespace = False

        # Search for the tool module, but don't import it, yet.
        #
        # First look in the toolpath: these take priority.
        # TODO: any reason to not just use find_spec here?
        for path in self.toolpath:
            sepname = self.name.replace('.', os.path.sep)
            file_path = os.path.join(path, sepname + ".py")
            file_package = os.path.join(path, sepname)

            if debug: sys.stderr.write(f"Trying: {file_path} {file_package}\n")

            if os.path.isfile(file_path):
                spec = importlib.util.spec_from_file_location(self.name, file_path)
                if debug: sys.stderr.write(f"file_Path: {file_path} FOUND\n")
                break
            elif os.path.isdir(file_package):
                file_package = os.path.join(file_package, '__init__.py')
                spec = importlib.util.spec_from_file_location(self.name, file_package)
                if debug: sys.stderr.write(f"PACKAGE: {file_package} Found\n")
                break
            else:
                continue

        # Now look in the builtin tools (SCons.Tool package)
        if spec is None:
            if debug: sys.stderr.write(f"NO SPEC: {self.name}\n")
            spec = importlib.util.find_spec("." + self.name, package='SCons.Tool')
            if spec:
                found_name = 'SCons.Tool.' + self.name
                add_to_scons_tools_namespace = True
            if debug: sys.stderr.write(f"Spec Found? .{self.name}: {spec}\n")

        if spec is None:
            # we are going to bail out here, format up stuff for the msg
            sconstools = os.path.normpath(sys.modules['SCons.Tool'].__path__[0])
            if self.toolpath:
                sconstools = ", ".join(self.toolpath) + ", " + sconstools
            msg = f"No tool module '{self.name}' found in {sconstools}"
            raise SCons.Errors.UserError(msg)

        # We have a module spec, so we're good to go.
        module = importlib.util.module_from_spec(spec)
        if module is None:
            if debug: sys.stderr.write(f"MODULE IS NONE: {self.name}\n")
            msg = f"Tool module '{self.name}' failed import"
            raise SCons.Errors.SConsEnvironmentError(msg)

        # Don't reload a tool we already loaded.
        sys_modules_value = sys.modules.get(found_name, False)

        found_module = None
        if sys_modules_value and sys_modules_value.__file__ == spec.origin:
            found_module = sys.modules[found_name]
        else:
            # Not sure what to do in the case that there already
            # exists sys.modules[self.name] but the source file is
            # different.. ?
            sys.modules[found_name] = module
            spec.loader.exec_module(module)
            if add_to_scons_tools_namespace:
                # If we found it in SCons.Tool, add it to the module
                setattr(SCons.Tool, self.name, module)
            found_module = module

        if found_module is not None:
            sys.path = oldpythonpath
            return found_module

        sys.path = oldpythonpath

        # We try some other things here, but this is essentially dead code,
        # because we bailed out above if we didn't find a module spec.
        full_name = 'SCons.Tool.' + self.name
        try:
            return sys.modules[full_name]
        except KeyError:
            try:
                # This support was added to enable running inside
                # a py2exe bundle a long time ago - unclear if it's
                # still needed. It is *not* intended to load individual
                # tool modules stored in a zipfile.
                import zipimport

                tooldir = sys.modules['SCons.Tool'].__path__[0]
                importer = zipimport.zipimporter(tooldir)
                # TODO: remove this check when Python 3.10 becomes base version
                if hasattr(importer, 'find_spec'):
                    # Note zipimporter got these methods later than importlib
                    spec = importer.find_spec(full_name)
                    mod = importlib.util.module_from_spec(spec)
                    importer.exec_module(mod)
                else:
                    # Older Pythons. load_module dropped entirely from
                    # zimpiporter in 3.15, but a new Python won't take this
                    # branch anyway so it's okay to ignore checker gripes here.
                    mod = importer.load_module(full_name)
                sys.modules[full_name] = module
                setattr(SCons.Tool, self.name, module)
                return module
            except zipimport.ZipImportError as e:
                msg = "No tool named '{self.name}': {e}"
                raise SCons.Errors.SConsEnvironmentError(msg)

    def __call__(self, env, *args, **kw) -> None:
        if self.init_kw is not None:
            # Merge call kws into init kws;
            # but don't bash self.init_kw.
            if kw is not None:
                call_kw = kw
                kw = self.init_kw.copy()
                kw.update(call_kw)
            else:
                kw = self.init_kw
        env.AppendUnique(TOOLS=[self.name])
        if hasattr(self, 'options'):
            import SCons.Variables
            if 'options' not in env:
                from SCons.Script import ARGUMENTS
                env['options'] = SCons.Variables.Variables(args=ARGUMENTS)
            opts = env['options']

            self.options(opts)
            opts.Update(env)

        self.generate(env, *args, **kw)

    def __str__(self) -> str:
        return self.name

    def __eq__(self, other) -> bool:
        """Compare a Tool by name.

        A Tool is equal to another Tool with the same name, and to a
        string matching its name (so a Tool can be used interchangeably
        with its name in membership tests and sets). Comparison against
        any other type is deferred (returns ``NotImplemented``).
        """
        if isinstance(other, Tool):
            return self.name == other.name
        if isinstance(other, str):
            return self.name == other
        return NotImplemented

    def __hash__(self) -> int:
        return hash(self.name)


LibSymlinksAction = SCons.Action.Action(LibSymlinksActionFunction, LibSymlinksStrFun)


##########################################################################
#  Create common executable program / library / object builders

def createProgBuilder(env):
    """This is a utility function that creates the Program
    Builder in an Environment if it is not there already.

    If it is already there, we return the existing one.
    """

    try:
        program = env['BUILDERS']['Program']
    except KeyError:
        import SCons.Defaults
        program = SCons.Builder.Builder(action=SCons.Defaults.LinkAction,
                                        emitter='$PROGEMITTER',
                                        prefix='$PROGPREFIX',
                                        suffix='$PROGSUFFIX',
                                        src_suffix='$OBJSUFFIX',
                                        src_builder='Object',
                                        target_scanner=ProgramScanner)
        env['BUILDERS']['Program'] = program

    return program


def createStaticLibBuilder(env):
    """This is a utility function that creates the StaticLibrary
    Builder in an Environment if it is not there already.

    If it is already there, we return the existing one.
    """

    try:
        static_lib = env['BUILDERS']['StaticLibrary']
    except KeyError:
        action_list = [SCons.Action.Action("$ARCOM", "$ARCOMSTR")]
        if env.get('RANLIB', False) or env.Detect('ranlib'):
            ranlib_action = SCons.Action.Action("$RANLIBCOM", "$RANLIBCOMSTR")
            action_list.append(ranlib_action)

        static_lib = SCons.Builder.Builder(action=action_list,
                                           emitter='$LIBEMITTER',
                                           prefix='$LIBPREFIX',
                                           suffix='$LIBSUFFIX',
                                           src_suffix='$OBJSUFFIX',
                                           src_builder='StaticObject')
        env['BUILDERS']['StaticLibrary'] = static_lib
        env['BUILDERS']['Library'] = static_lib

    return static_lib


def createSharedLibBuilder(env, shlib_suffix: str='$_SHLIBSUFFIX'):
    """This is a utility function that creates the SharedLibrary
    Builder in an Environment if it is not there already.

    If it is already there, we return the existing one.

    Args:
        shlib_suffix: The suffix specified for the shared library builder

    """

    try:
        shared_lib = env['BUILDERS']['SharedLibrary']
    except KeyError:
        import SCons.Defaults
        action_list = [SCons.Defaults.SharedCheck,
                       SCons.Defaults.ShLinkAction,
                       LibSymlinksAction]
        shared_lib = SCons.Builder.Builder(action=action_list,
                                           emitter="$SHLIBEMITTER",
                                           prefix="$SHLIBPREFIX",
                                           suffix=shlib_suffix,
                                           target_scanner=ProgramScanner,
                                           src_suffix='$SHOBJSUFFIX',
                                           src_builder='SharedObject')
        env['BUILDERS']['SharedLibrary'] = shared_lib

    return shared_lib


def createLoadableModuleBuilder(env, loadable_module_suffix: str='$_LDMODULESUFFIX'):
    """This is a utility function that creates the LoadableModule
    Builder in an Environment if it is not there already.

    If it is already there, we return the existing one.

    Args:
        loadable_module_suffix: The suffix specified for the loadable module builder

    """

    try:
        ld_module = env['BUILDERS']['LoadableModule']
    except KeyError:
        import SCons.Defaults
        action_list = [SCons.Defaults.SharedCheck,
                       SCons.Defaults.LdModuleLinkAction,
                       LibSymlinksAction]
        ld_module = SCons.Builder.Builder(action=action_list,
                                          emitter="$LDMODULEEMITTER",
                                          prefix="$LDMODULEPREFIX",
                                          suffix=loadable_module_suffix,
                                          target_scanner=ProgramScanner,
                                          src_suffix='$SHOBJSUFFIX',
                                          src_builder='SharedObject')
        env['BUILDERS']['LoadableModule'] = ld_module

    return ld_module


def createObjBuilders(env):
    """This is a utility function that creates the StaticObject
    and SharedObject Builders in an Environment if they
    are not there already.

    If they are there already, we return the existing ones.

    This is a separate function because soooo many Tools
    use this functionality.

    The return is a 2-tuple of (StaticObject, SharedObject)
    """

    try:
        static_obj = env['BUILDERS']['StaticObject']
    except KeyError:
        static_obj = SCons.Builder.Builder(action={},
                                           emitter={},
                                           prefix='$OBJPREFIX',
                                           suffix='$OBJSUFFIX',
                                           src_builder=['CFile', 'CXXFile'],
                                           source_scanner=SourceFileScanner,
                                           single_source=True)
        env['BUILDERS']['StaticObject'] = static_obj
        env['BUILDERS']['Object'] = static_obj

    try:
        shared_obj = env['BUILDERS']['SharedObject']
    except KeyError:
        shared_obj = SCons.Builder.Builder(action={},
                                           emitter={},
                                           prefix='$SHOBJPREFIX',
                                           suffix='$SHOBJSUFFIX',
                                           src_builder=['CFile', 'CXXFile'],
                                           source_scanner=SourceFileScanner,
                                           single_source=True)
        env['BUILDERS']['SharedObject'] = shared_obj

    return (static_obj, shared_obj)


def createCFileBuilders(env):
    """This is a utility function that creates the CFile/CXXFile
    Builders in an Environment if they
    are not there already.

    If they are there already, we return the existing ones.

    This is a separate function because soooo many Tools
    use this functionality.

    The return is a 2-tuple of (CFile, CXXFile)
    """

    try:
        c_file = env['BUILDERS']['CFile']
    except KeyError:
        c_file = SCons.Builder.Builder(action={},
                                       emitter={},
                                       suffix={None: '$CFILESUFFIX'})
        env['BUILDERS']['CFile'] = c_file

        env.SetDefault(CFILESUFFIX='.c')

    try:
        cxx_file = env['BUILDERS']['CXXFile']
    except KeyError:
        cxx_file = SCons.Builder.Builder(action={},
                                         emitter={},
                                         suffix={None: '$CXXFILESUFFIX'})
        env['BUILDERS']['CXXFile'] = cxx_file
        env.SetDefault(CXXFILESUFFIX='.cc')

    return (c_file, cxx_file)


##########################################################################
#  Create common Java builders

def CreateJarBuilder(env):
    """The Jar builder expects a list of class files
    which it can package into a jar file.

    The jar tool provides an interface for passing other types
    of java files such as .java, directories or swig interfaces
    and will build them to class files in which it can package
    into the jar.
    """
    try:
        java_jar = env['BUILDERS']['JarFile']
    except KeyError:
        fs = SCons.Node.FS.get_default_fs()
        jar_com = SCons.Action.Action('$JARCOM', '$JARCOMSTR')
        java_jar = SCons.Builder.Builder(action=jar_com,
                                         suffix='$JARSUFFIX',
                                         src_suffix='$JAVACLASSSUFFIX',
                                         src_builder='JavaClassFile',
                                         source_factory=fs.Entry)
        env['BUILDERS']['JarFile'] = java_jar
    return java_jar


def CreateJavaHBuilder(env):
    try:
        java_javah = env['BUILDERS']['JavaH']
    except KeyError:
        fs = SCons.Node.FS.get_default_fs()
        java_javah_com = SCons.Action.Action('$JAVAHCOM', '$JAVAHCOMSTR')
        java_javah = SCons.Builder.Builder(action=java_javah_com,
                                           src_suffix='$JAVACLASSSUFFIX',
                                           target_factory=fs.Entry,
                                           source_factory=fs.File,
                                           src_builder='JavaClassFile')
        env['BUILDERS']['JavaH'] = java_javah
    return java_javah


def CreateJavaClassFileBuilder(env):
    try:
        java_class_file = env['BUILDERS']['JavaClassFile']
    except KeyError:
        fs = SCons.Node.FS.get_default_fs()
        javac_com = SCons.Action.Action('$JAVACCOM', '$JAVACCOMSTR')
        java_class_file = SCons.Builder.Builder(action=javac_com,
                                                emitter={},
                                                # suffix = '$JAVACLASSSUFFIX',
                                                src_suffix='$JAVASUFFIX',
                                                src_builder=['JavaFile'],
                                                target_factory=fs.Entry,
                                                source_factory=fs.File,
                                                target_scanner=JavaScanner)
        env['BUILDERS']['JavaClassFile'] = java_class_file
    return java_class_file


def CreateJavaClassDirBuilder(env):
    try:
        java_class_dir = env['BUILDERS']['JavaClassDir']
    except KeyError:
        fs = SCons.Node.FS.get_default_fs()
        javac_com = SCons.Action.Action('$JAVACCOM', '$JAVACCOMSTR')
        java_class_dir = SCons.Builder.Builder(action=javac_com,
                                               emitter={},
                                               target_factory=fs.Dir,
                                               source_factory=fs.Dir,
                                               target_scanner=JavaScanner)
        env['BUILDERS']['JavaClassDir'] = java_class_dir
    return java_class_dir


def CreateJavaFileBuilder(env):
    try:
        java_file = env['BUILDERS']['JavaFile']
    except KeyError:
        java_file = SCons.Builder.Builder(action={},
                                          emitter={},
                                          suffix={None: '$JAVASUFFIX'})
        env['BUILDERS']['JavaFile'] = java_file
        env['JAVASUFFIX'] = '.java'
    return java_file


class ToolInitializerMethod:
    """
    This is added to a construction environment in place of a
    method(s) normally called for a Builder (env.Object, env.StaticObject,
    etc.).  When called, it has its associated ToolInitializer
    object search the specified list of tools and apply the first
    one that exists to the construction environment.  It then calls
    whatever builder was (presumably) added to the construction
    environment in place of this particular instance.
    """

    def __init__(self, name, initializer) -> None:
        """
        Note:  we store the tool name as __name__ so it can be used by
        the class that attaches this to a construction environment.
        """
        self.__name__ = name
        self.initializer = initializer

    def get_builder(self, env):
        """
        Returns the appropriate real Builder for this method name
        after having the associated ToolInitializer object apply
        the appropriate Tool module.
        """
        builder = getattr(env, self.__name__)

        self.initializer.apply_tools(env)

        builder = getattr(env, self.__name__)
        if builder is self:
            # There was no Builder added, which means no valid Tool
            # for this name was found (or possibly there's a mismatch
            # between the name we were called by and the Builder name
            # added by the Tool module).
            return None

        self.initializer.remove_methods(env)

        return builder

    def __call__(self, env, *args, **kw):
        """
        """
        builder = self.get_builder(env)
        if builder is None:
            return [], []
        return builder(*args, **kw)


class ToolInitializer:
    """
    A class for delayed initialization of Tools modules.

    Instances of this class associate a list of Tool modules with
    a list of Builder method names that will be added by those Tool
    modules.  As part of instantiating this object for a particular
    construction environment, we also add the appropriate
    ToolInitializerMethod objects for the various Builder methods
    that we want to use to delay Tool searches until necessary.
    """

    def __init__(self, env, tools, names) -> None:
        if not SCons.Util.is_List(tools):
            tools = [tools]
        if not SCons.Util.is_List(names):
            names = [names]
        self.env = env
        self.tools = tools
        self.names = names
        self.methods = {}
        for name in names:
            method = ToolInitializerMethod(name, self)
            self.methods[name] = method
            env.AddMethod(method)

    def remove_methods(self, env) -> None:
        """
        Removes the methods that were added by the tool initialization
        so we no longer copy and re-bind them when the construction
        environment gets cloned.
        """
        for method in self.methods.values():
            env.RemoveMethod(method)

    def apply_tools(self, env) -> None:
        """
        Searches the list of associated Tool modules for one that
        exists, and applies that to the construction environment.
        """
        for t in self.tools:
            tool = SCons.Tool.Tool(t)
            if tool.exists(env):
                env.Tool(tool)
                return

        # If we fall through here, there was no tool module found.
        # This is where we can put an informative error message
        # about the inability to find the tool.   We'll start doing
        # this as we cut over more pre-defined Builder+Tools to use
        # the ToolInitializer class.


def Initializers(env) -> None:
    ToolInitializer(env, ['install'], ['_InternalInstall', '_InternalInstallAs', '_InternalInstallVersionedLib'])

    def Install(self, *args, **kw):
        return self._InternalInstall(*args, **kw)

    def InstallAs(self, *args, **kw):
        return self._InternalInstallAs(*args, **kw)

    def InstallVersionedLib(self, *args, **kw):
        return self._InternalInstallVersionedLib(*args, **kw)

    env.AddMethod(Install)
    env.AddMethod(InstallAs)
    env.AddMethod(InstallVersionedLib)


def FindTool(tools, env):
    for tool in tools:
        if not SCons.Util.is_String(tool):
            # Already a Tool instance
            if tool.exists(env):
                return tool
            continue
        t = Tool(tool)
        if t.exists(env):
            return t
    return None


def FindAllTools(tools, env):
    def ToolExists(tool, env=env):
        if not SCons.Util.is_String(tool):
            return tool.exists(env)
        t = Tool(tool)
        return t.exists(env)

    results = []
    for tool in tools:
        if not SCons.Util.is_String(tool):
            if tool.exists(env):
                results.append(tool)
            continue
        t = Tool(tool)
        if t.exists(env):
            results.append(t)
    return results

def tool_list(platform, env):
    other_plat_tools = []
    # XXX this logic about what tool to prefer on which platform
    #     should be moved into either the platform files or
    #     the tool files themselves.
    # The search orders here are described in the man page.  If you
    # change these search orders, update the man page as well.
    if str(platform) == 'win32':
        "prefer Microsoft tools on Windows"
        linkers = ['mslink', 'gnulink', 'ilink', 'linkloc', 'ilink32']
        c_compilers = ['msvc', 'mingw', 'gcc', 'clang', 'intelc', 'icl', 'icc', 'cc', 'bcc32']
        cxx_compilers = ['msvc', 'intelc', 'icc', 'g++', 'clang++', 'cxx', 'bcc32']
        assemblers = ['masm', 'nasm', 'gas', '386asm']
        fortran_compilers = ['gfortran', 'g77', 'ifl', 'cvf', 'f95', 'f90', 'fortran']
        ars = ['mslib', 'ar', 'tlib']
        other_plat_tools = ['msvs', 'midl', 'wix']
    elif str(platform) == 'os2':
        "prefer IBM tools on OS/2"
        linkers = ['ilink', 'gnulink', ]  # 'mslink']
        c_compilers = ['icc', 'gcc', ]  # 'msvc', 'cc']
        cxx_compilers = ['icc', 'g++', ]  # 'msvc', 'cxx']
        assemblers = ['nasm', ]  # 'masm', 'gas']
        fortran_compilers = ['ifl', 'g77']
        ars = ['ar', ]  # 'mslib']
    elif str(platform) == 'irix':
        "prefer MIPSPro on IRIX"
        linkers = ['sgilink', 'gnulink']
        c_compilers = ['sgicc', 'gcc', 'cc']
        cxx_compilers = ['sgicxx', 'g++', 'cxx']
        assemblers = ['as', 'gas']
        fortran_compilers = ['f95', 'f90', 'f77', 'g77', 'fortran']
        ars = ['sgiar']
    elif str(platform) == 'sunos':
        "prefer Forte tools on SunOS"
        linkers = ['sunlink', 'gnulink']
        c_compilers = ['suncc', 'gcc', 'cc']
        cxx_compilers = ['suncxx', 'g++', 'cxx']
        assemblers = ['as', 'gas']
        fortran_compilers = ['sunf95', 'sunf90', 'sunf77', 'f95', 'f90', 'f77',
                             'gfortran', 'g77', 'fortran']
        ars = ['sunar']
    elif str(platform) == 'hpux':
        "prefer aCC tools on HP-UX"
        linkers = ['hplink', 'gnulink']
        c_compilers = ['hpcc', 'gcc', 'cc']
        cxx_compilers = ['hpcxx', 'g++', 'cxx']
        assemblers = ['as', 'gas']
        fortran_compilers = ['f95', 'f90', 'f77', 'g77', 'fortran']
        ars = ['ar']
    elif str(platform) == 'aix':
        "prefer AIX Visual Age tools on AIX"
        linkers = ['aixlink', 'gnulink']
        c_compilers = ['aixcc', 'gcc', 'cc']
        cxx_compilers = ['aixcxx', 'g++', 'cxx']
        assemblers = ['as', 'gas']
        fortran_compilers = ['f95', 'f90', 'aixf77', 'g77', 'fortran']
        ars = ['ar']
    elif str(platform) == 'darwin':
        "prefer GNU tools on Mac OS X, except for some linkers and IBM tools"
        linkers = ['applelink', 'gnulink']
        c_compilers = ['gcc', 'cc']
        cxx_compilers = ['g++', 'cxx']
        assemblers = ['as']
        fortran_compilers = ['gfortran', 'f95', 'f90', 'g77']
        ars = ['ar']
    elif str(platform) == 'cygwin':
        "prefer GNU tools on Cygwin, except for a platform-specific linker"
        linkers = ['cyglink', 'mslink', 'ilink']
        c_compilers = ['gcc', 'msvc', 'intelc', 'icc', 'cc']
        cxx_compilers = ['g++', 'msvc', 'intelc', 'icc', 'cxx']
        assemblers = ['gas', 'nasm', 'masm']
        fortran_compilers = ['gfortran', 'g77', 'ifort', 'ifl', 'f95', 'f90', 'f77']
        ars = ['ar', 'mslib']
    elif str(platform) == 'bsd':
        "prefer Clang tools on the BSDs whose base toolchain is LLVM"
        linkers = ['gnulink', 'ilink']
        c_compilers = ['clang', 'gcc', 'intelc', 'icc', 'cc']
        cxx_compilers = ['clang++', 'g++', 'intelc', 'icc', 'cxx']
        assemblers = ['gas', 'nasm', 'masm']
        fortran_compilers = ['gfortran', 'g77', 'ifort', 'ifl', 'f95', 'f90', 'f77']
        ars = ['ar', ]
    else:
        "prefer GNU tools on all other platforms"
        linkers = ['gnulink', 'ilink']
        c_compilers = ['gcc', 'clang', 'intelc', 'icc', 'cc']
        cxx_compilers = ['g++', 'clang++', 'intelc', 'icc', 'cxx']
        assemblers = ['gas', 'nasm', 'masm']
        fortran_compilers = ['gfortran', 'g77', 'ifort', 'ifl', 'f95', 'f90', 'f77']
        ars = ['ar', ]

    if not str(platform) == 'win32':
        other_plat_tools += ['m4', 'rpm']

    c_compiler = FindTool(c_compilers, env) or c_compilers[0]

    # XXX this logic about what tool provides what should somehow be
    #     moved into the tool files themselves.
    if c_compiler and str(c_compiler) == 'mingw':
        # MinGW contains a linker, C compiler, C++ compiler,
        # Fortran compiler, archiver and assembler:
        cxx_compiler = None
        linker = None
        assembler = None
        fortran_compiler = None
        ar = None
    else:
        # Don't use g++ if the C compiler has built-in C++ support:
        if str(c_compiler) in ('msvc', 'intelc', 'icc'):
            cxx_compiler = None
        else:
            cxx_compiler = FindTool(cxx_compilers, env) or cxx_compilers[0]
        linker = FindTool(linkers, env) or linkers[0]
        assembler = FindTool(assemblers, env) or assemblers[0]
        fortran_compiler = FindTool(fortran_compilers, env) or fortran_compilers[0]
        ar = FindTool(ars, env) or ars[0]

    d_compilers = ['dmd', 'ldc', 'gdc']
    d_compiler = FindTool(d_compilers, env) or d_compilers[0]

    other_tools = FindAllTools(other_plat_tools + [
        # TODO: merge 'install' into 'filesystem' and
        # make 'filesystem' the default
        'filesystem',
        # Parser generators
        'lex', 'yacc',
        # Foreign function interface
        'rpcgen', 'swig',
        # Java
        'jar', 'javac', 'javah', 'rmic',
        # TeX
        'dvipdf', 'dvips', 'gs',
        'tex', 'latex', 'pdflatex', 'pdftex',
        # Archivers
        'tar', 'zip',
        # File builders (text)
        'textfile',
    ], env)

    tools = [
        linker,
        c_compiler,
        cxx_compiler,
        fortran_compiler,
        assembler,
        ar,
        d_compiler,
    ] + other_tools

    return [x for x in tools if x]


def find_program_path(env, key_program, default_paths=None, add_path: bool=False) -> str | None:
    """
    Find the location of a tool using various means.

    Mainly for windows where tools aren't all installed in /usr/bin, etc.

    Args:
        env: Current Construction Environment.
        key_program: Tool to locate.
        default_paths: List of additional paths this tool might be found in.
        add_path: If true, add path found if it was from *default_paths*.
    """
    # First search in the SCons path
    path = env.WhereIs(key_program)
    if path:
        return path

    # Then in the OS path
    path = SCons.Util.WhereIs(key_program)
    if path:
        if add_path:
            env.AppendENVPath('PATH', os.path.dirname(path))
        return path

    # Finally, add the defaults and check again.
    if default_paths is None:
        return path

    save_path = env['ENV']['PATH']
    for p in default_paths:
        env.AppendENVPath('PATH', p)
    path = env.WhereIs(key_program)

    # By default, do not change ['ENV']['PATH'] permananetly
    # leave that to the caller, unless add_path is true.
    env['ENV']['PATH'] = save_path
    if path and add_path:
        env.AppendENVPath('PATH', os.path.dirname(path))

    return path
