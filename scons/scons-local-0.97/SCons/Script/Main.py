"""SCons.Script

This file implements the main() function used by the scons script.

Architecturally, this *is* the scons script, and will likely only be
called from the external "scons" wrapper.  Consequently, anything here
should not be, or be considered, part of the build engine.  If it's
something that we expect other software to want to use, it should go in
some other module.  If it's specific to the "scons" script invocation,
it goes here.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The SCons Foundation
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Script/Main.py 0.97.D001 2007/05/17 11:35:19 knight"

import SCons.compat

import os
import os.path
import random
import string
import sys
import time
import traceback

# Strip the script directory from sys.path() so on case-insensitive
# (Windows) systems Python doesn't think that the "scons" script is the
# "SCons" package.  Replace it with our own version directory so, if
# if they're there, we pick up the right version of the build engine
# modules.
#sys.path = [os.path.join(sys.prefix,
#                         'lib',
#                         'scons-%d' % SCons.__version__)] + sys.path[1:]

import SCons.Debug
import SCons.Defaults
import SCons.Environment
import SCons.Errors
import SCons.Job
import SCons.Node
import SCons.Node.FS
from SCons.Optik import OptionParser, SUPPRESS_HELP, OptionValueError
import SCons.SConf
import SCons.Script
import SCons.Sig
import SCons.Taskmaster
import SCons.Util
import SCons.Warnings

#
display = SCons.Util.display
progress_display = SCons.Util.DisplayEngine()

first_command_start = None
last_command_end = None

# Task control.
#
class BuildTask(SCons.Taskmaster.Task):
    """An SCons build task."""
    def display(self, message):
        display('scons: ' + message)

    def execute(self):
        for target in self.targets:
            if target.get_state() == SCons.Node.up_to_date: 
                continue
            if target.has_builder() and not hasattr(target.builder, 'status'):
                if print_time:
                    start_time = time.time()
                    global first_command_start
                    if first_command_start is None:
                        first_command_start = start_time
                SCons.Taskmaster.Task.execute(self)
                if print_time:
                    global cumulative_command_time
                    global last_command_end
                    finish_time = time.time()
                    last_command_end = finish_time
                    cumulative_command_time = cumulative_command_time+finish_time-start_time
                    sys.stdout.write("Command execution time: %f seconds\n"%(finish_time-start_time))
                break
        else:
            if self.top and target.has_builder():
                display("scons: `%s' is up to date." % str(self.node))

    def do_failed(self, status=2):
        global exit_status
        if ignore_errors:
            SCons.Taskmaster.Task.executed(self)
        elif keep_going_on_error:
            SCons.Taskmaster.Task.fail_continue(self)
            exit_status = status
        else:
            SCons.Taskmaster.Task.fail_stop(self)
            exit_status = status
            
    def executed(self):
        t = self.targets[0]
        if self.top and not t.has_builder() and not t.side_effect:
            if not t.exists():
                sys.stderr.write("scons: *** Do not know how to make target `%s'." % t)
                if not keep_going_on_error:
                    sys.stderr.write("  Stop.")
                sys.stderr.write("\n")
                self.do_failed()
            else:
                print "scons: Nothing to be done for `%s'." % t
                SCons.Taskmaster.Task.executed(self)
        else:
            SCons.Taskmaster.Task.executed(self)

    def failed(self):
        # Handle the failure of a build task.  The primary purpose here
        # is to display the various types of Errors and Exceptions
        # appropriately.
        status = 2
        exc_info = self.exc_info()
        try:
            t, e, tb = exc_info
        except ValueError:
            t, e = exc_info
            tb = None
        if t is None:
            # The Taskmaster didn't record an exception for this Task;
            # see if the sys module has one.
            t, e = sys.exc_info()[:2]

        def nodestring(n):
            if not SCons.Util.is_List(n):
                n = [ n ]
            return string.join(map(str, n), ', ')

        errfmt = "scons: *** [%s] %s\n"

        if t == SCons.Errors.BuildError:
            tname = nodestring(e.node)
            errstr = e.errstr
            if e.filename:
                errstr = e.filename + ': ' + errstr
            sys.stderr.write(errfmt % (tname, errstr))
        elif t == SCons.Errors.TaskmasterException:
            tname = nodestring(e.node)
            sys.stderr.write(errfmt % (tname, e.errstr))
            type, value, trace = e.exc_info
            traceback.print_exception(type, value, trace)
        elif t == SCons.Errors.ExplicitExit:
            status = e.status
            tname = nodestring(e.node)
            errstr = 'Explicit exit, status %s' % status
            sys.stderr.write(errfmt % (tname, errstr))
        else:
            if e is None:
                e = t
            s = str(e)
            if t == SCons.Errors.StopError and not keep_going_on_error:
                s = s + '  Stop.'
            sys.stderr.write("scons: *** %s\n" % s)

            if tb and print_stacktrace:
                sys.stderr.write("scons: internal stack trace:\n")
                traceback.print_tb(tb, file=sys.stderr)

        self.do_failed(status)

        self.exc_clear()

    def postprocess(self):
        if self.top:
            t = self.targets[0]
            for tp in tree_printers:
                tp.display(t)
            if print_includes:
                tree = t.render_include_tree()
                if tree:
                    print
                    print tree
        SCons.Taskmaster.Task.postprocess(self)

    def make_ready(self):
        """Make a task ready for execution"""
        SCons.Taskmaster.Task.make_ready(self)
        if self.out_of_date and print_explanations:
            explanation = self.out_of_date[0].explain()
            if explanation:
                sys.stdout.write("scons: " + explanation)

class CleanTask(SCons.Taskmaster.Task):
    """An SCons clean task."""
    def dir_index(self, directory):
        dirname = lambda f, d=directory: os.path.join(d, f)
        files = map(dirname, os.listdir(directory))

        # os.listdir() isn't guaranteed to return files in any specific order,
        # but some of the test code expects sorted output.
        files.sort()
        return files

    def fs_delete(self, path, remove=1):
        try:
            if os.path.exists(path):
                if os.path.isfile(path):
                    if remove: os.unlink(path)
                    display("Removed " + path)
                elif os.path.isdir(path) and not os.path.islink(path):
                    # delete everything in the dir
                    for p in self.dir_index(path):
                        if os.path.isfile(p):
                            if remove: os.unlink(p)
                            display("Removed " + p)
                        else:
                            self.fs_delete(p, remove)
                    # then delete dir itself
                    if remove: os.rmdir(path)
                    display("Removed directory " + path)
        except (IOError, OSError), e:
            print "scons: Could not remove '%s':" % str(path), e.strerror

    def show(self):
        target = self.targets[0]
        if (target.has_builder() or target.side_effect) and not target.noclean:
            for t in self.targets:
                if not t.isdir():
                    display("Removed " + str(t))
        if SCons.Environment.CleanTargets.has_key(target):
            files = SCons.Environment.CleanTargets[target]
            for f in files:
                self.fs_delete(str(f), 0)

    def remove(self):
        target = self.targets[0]
        if (target.has_builder() or target.side_effect) and not target.noclean:
            for t in self.targets:
                try:
                    removed = t.remove()
                except OSError, e:
                    # An OSError may indicate something like a permissions
                    # issue, an IOError would indicate something like
                    # the file not existing.  In either case, print a
                    # message and keep going to try to remove as many
                    # targets aa possible.
                    print "scons: Could not remove '%s':" % str(t), e.strerror
                else:
                    if removed:
                        display("Removed " + str(t))
        if SCons.Environment.CleanTargets.has_key(target):
            files = SCons.Environment.CleanTargets[target]
            for f in files:
                self.fs_delete(str(f))

    execute = remove

    # Have the taskmaster arrange to "execute" all of the targets, because
    # we'll figure out ourselves (in remove() or show() above) whether
    # anything really needs to be done.
    make_ready = SCons.Taskmaster.Task.make_ready_all

    def prepare(self):
        pass

class QuestionTask(SCons.Taskmaster.Task):
    """An SCons task for the -q (question) option."""
    def prepare(self):
        pass
    
    def execute(self):
        if self.targets[0].get_state() != SCons.Node.up_to_date:
            global exit_status
            exit_status = 1
            self.tm.stop()

    def executed(self):
        pass


class TreePrinter:
    def __init__(self, derived=False, prune=False, status=False):
        self.derived = derived
        self.prune = prune
        self.status = status
    def get_all_children(self, node):
        return node.all_children()
    def get_derived_children(self, node):
        children = node.all_children(None)
        return filter(lambda x: x.has_builder(), children)
    def display(self, t):
        if self.derived:
            func = self.get_derived_children
        else:
            func = self.get_all_children
        s = self.status and 2 or 0
        SCons.Util.print_tree(t, func, prune=self.prune, showtags=s)


# Global variables

tree_printers = []

keep_going_on_error = 0
print_explanations = 0
print_includes = 0
print_objects = 0
print_memoizer = 0
print_stacktrace = 0
print_time = 0
ignore_errors = 0
sconscript_time = 0
cumulative_command_time = 0
exit_status = 0 # exit status, assume success by default
repositories = []
num_jobs = None
delayed_warnings = []

diskcheck_all = SCons.Node.FS.diskcheck_types()
diskcheck_option_set = None

def diskcheck_convert(value):
    if value is None:
        return []
    if not SCons.Util.is_List(value):
        value = string.split(value, ',')
    result = []
    for v in map(string.lower, value):
        if v == 'all':
            result = diskcheck_all
        elif v == 'none':
            result = []
        elif v in diskcheck_all:
            result.append(v)
        else:
            raise ValueError, v
    return result

#
class Stats:
    def __init__(self):
        self.stats = []
        self.labels = []
        self.append = self.do_nothing
        self.print_stats = self.do_nothing
    def enable(self, outfp):
        self.outfp = outfp
        self.append = self.do_append
        self.print_stats = self.do_print
    def do_nothing(self, *args, **kw):
        pass

class CountStats(Stats):
    def do_append(self, label):
        self.labels.append(label)
        self.stats.append(SCons.Debug.fetchLoggedInstances())
    def do_print(self):
        stats_table = {}
        for s in self.stats:
            for n in map(lambda t: t[0], s):
                stats_table[n] = [0, 0, 0, 0]
        i = 0
        for s in self.stats:
            for n, c in s:
                stats_table[n][i] = c
            i = i + 1
        keys = stats_table.keys()
        keys.sort()
        self.outfp.write("Object counts:\n")
        pre = ["   "]
        post = ["   %s\n"]
        l = len(self.stats)
        fmt1 = string.join(pre + [' %7s']*l + post, '')
        fmt2 = string.join(pre + [' %7d']*l + post, '')
        labels = self.labels[:l]
        labels.append(("", "Class"))
        self.outfp.write(fmt1 % tuple(map(lambda x: x[0], labels)))
        self.outfp.write(fmt1 % tuple(map(lambda x: x[1], labels)))
        for k in keys:
            r = stats_table[k][:l] + [k]
            self.outfp.write(fmt2 % tuple(r))

count_stats = CountStats()

class MemStats(Stats):
    def do_append(self, label):
        self.labels.append(label)
        self.stats.append(SCons.Debug.memory())
    def do_print(self):
        fmt = 'Memory %-32s %12d\n'
        for label, stats in map(None, self.labels, self.stats):
            self.outfp.write(fmt % (label, stats))

memory_stats = MemStats()

# utility functions

def _scons_syntax_error(e):
    """Handle syntax errors. Print out a message and show where the error
    occurred.
    """
    etype, value, tb = sys.exc_info()
    lines = traceback.format_exception_only(etype, value)
    for line in lines:
        sys.stderr.write(line+'\n')
    sys.exit(2)

def find_deepest_user_frame(tb):
    """
    Find the deepest stack frame that is not part of SCons.

    Input is a "pre-processed" stack trace in the form
    returned by traceback.extract_tb() or traceback.extract_stack()
    """
    
    tb.reverse()

    # find the deepest traceback frame that is not part
    # of SCons:
    for frame in tb:
        filename = frame[0]
        if string.find(filename, os.sep+'SCons'+os.sep) == -1:
            return frame
    return tb[0]

def _scons_user_error(e):
    """Handle user errors. Print out a message and a description of the
    error, along with the line number and routine where it occured. 
    The file and line number will be the deepest stack frame that is
    not part of SCons itself.
    """
    global print_stacktrace
    etype, value, tb = sys.exc_info()
    if print_stacktrace:
        traceback.print_exception(etype, value, tb)
    filename, lineno, routine, dummy = find_deepest_user_frame(traceback.extract_tb(tb))
    sys.stderr.write("\nscons: *** %s\n" % value)
    sys.stderr.write('File "%s", line %d, in %s\n' % (filename, lineno, routine))
    sys.exit(2)

def _scons_user_warning(e):
    """Handle user warnings. Print out a message and a description of
    the warning, along with the line number and routine where it occured.
    The file and line number will be the deepest stack frame that is
    not part of SCons itself.
    """
    etype, value, tb = sys.exc_info()
    filename, lineno, routine, dummy = find_deepest_user_frame(traceback.extract_tb(tb))
    sys.stderr.write("\nscons: warning: %s\n" % e)
    sys.stderr.write('File "%s", line %d, in %s\n' % (filename, lineno, routine))

def _scons_internal_warning(e):
    """Slightly different from _scons_user_warning in that we use the
    *current call stack* rather than sys.exc_info() to get our stack trace.
    This is used by the warnings framework to print warnings."""
    filename, lineno, routine, dummy = find_deepest_user_frame(traceback.extract_stack())
    sys.stderr.write("\nscons: warning: %s\n" % e[0])
    sys.stderr.write('File "%s", line %d, in %s\n' % (filename, lineno, routine))

def _scons_internal_error():
    """Handle all errors but user errors. Print out a message telling
    the user what to do in this case and print a normal trace.
    """
    print 'internal error'
    traceback.print_exc()
    sys.exit(2)

def _varargs(option, parser):
    value = None
    if parser.rargs:
        arg = parser.rargs[0]
        if arg[0] != "-":
            value = arg
            del parser.rargs[0]
    return value

def _setup_warn(arg):
    """The --warn option.  An argument to this option
    should be of the form <warning-class> or no-<warning-class>.
    The warning class is munged in order to get an actual class
    name from the SCons.Warnings module to enable or disable.
    The supplied <warning-class> is split on hyphens, each element
    is captialized, then smushed back together.  Then the string
    "SCons.Warnings." is added to the front and "Warning" is added
    to the back to get the fully qualified class name.

    For example, --warn=deprecated will enable the
    SCons.Warnings.DeprecatedWarning class.

    --warn=no-dependency will disable the
    SCons.Warnings.DependencyWarning class.

    As a special case, --warn=all and --warn=no-all
    will enable or disable (respectively) the base
    class of all warnings, which is SCons.Warning.Warning."""

    elems = string.split(string.lower(arg), '-')
    enable = 1
    if elems[0] == 'no':
        enable = 0
        del elems[0]

    if len(elems) == 1 and elems[0] == 'all':
        class_name = "Warning"
    else:
        def _capitalize(s):
            if s[:5] == "scons":
                return "SCons" + s[5:]
            else:
                return string.capitalize(s)
        class_name = string.join(map(_capitalize, elems), '') + "Warning"
    try:
        clazz = getattr(SCons.Warnings, class_name)
    except AttributeError:
        sys.stderr.write("No warning type: '%s'\n" % arg)
    else:
        if enable:
            SCons.Warnings.enableWarningClass(clazz)
        else:
            SCons.Warnings.suppressWarningClass(clazz)

def _SConstruct_exists(dirname=''):
    """This function checks that an SConstruct file exists in a directory.
    If so, it returns the path of the file. By default, it checks the
    current directory.
    """
    global repositories
    for file in ['SConstruct', 'Sconstruct', 'sconstruct']:
        sfile = os.path.join(dirname, file)
        if os.path.isfile(sfile):
            return sfile
        if not os.path.isabs(sfile):
            for rep in repositories:
                if os.path.isfile(os.path.join(rep, sfile)):
                    return sfile
    return None

def _set_globals(options):
    global keep_going_on_error, ignore_errors
    global count_stats
    global print_explanations, print_includes, print_memoizer
    global print_objects, print_stacktrace, print_time
    global tree_printers
    global memory_stats

    keep_going_on_error = options.keep_going
    try:
        debug_values = options.debug
        if debug_values is None:
            debug_values = []
    except AttributeError:
        pass
    else:
        if "count" in debug_values:
            count_stats.enable(sys.stdout)
        if "dtree" in debug_values:
            tree_printers.append(TreePrinter(derived=True))
        if "explain" in debug_values:
            print_explanations = 1
        if "findlibs" in debug_values:
            SCons.Scanner.Prog.print_find_libs = "findlibs"
        if "includes" in debug_values:
            print_includes = 1
        if "memoizer" in debug_values:
            print_memoizer = 1
        if "memory" in debug_values:
            memory_stats.enable(sys.stdout)
        if "objects" in debug_values:
            print_objects = 1
        if "presub" in debug_values:
            SCons.Action.print_actions_presub = 1
        if "stacktrace" in debug_values:
            print_stacktrace = 1
        if "stree" in debug_values:
            tree_printers.append(TreePrinter(status=True))
        if "time" in debug_values:
            print_time = 1
        if "tree" in debug_values:
            tree_printers.append(TreePrinter())
    ignore_errors = options.ignore_errors

def _create_path(plist):
    path = '.'
    for d in plist:
        if os.path.isabs(d):
            path = d
        else:
            path = path + '/' + d
    return path

def _load_site_scons_dir(topdir, site_dir_name=None):
    """Load the site_scons dir under topdir.
    Adds site_scons to sys.path, imports site_scons/site_init.py,
    and adds site_scons/site_tools to default toolpath."""
    if site_dir_name:
        err_if_not_found = True       # user specified: err if missing
    else:
        site_dir_name = "site_scons"
        err_if_not_found = False
        
    site_dir = os.path.join(topdir.path, site_dir_name)
    if not os.path.exists(site_dir):
        if err_if_not_found:
            raise SCons.Errors.UserError, "site dir %s not found."%site_dir
        return

    site_init_filename = "site_init.py"
    site_init_modname = "site_init"
    site_tools_dirname = "site_tools"
    sys.path = [os.path.abspath(site_dir)] + sys.path
    site_init_file = os.path.join(site_dir, site_init_filename)
    site_tools_dir = os.path.join(site_dir, site_tools_dirname)
    if os.path.exists(site_init_file):
        import imp
        try:
            fp, pathname, description = imp.find_module(site_init_modname,
                                                        [site_dir])
            try:
                imp.load_module(site_init_modname, fp, pathname, description)
            finally:
                if fp:
                    fp.close()
        except ImportError, e:
            sys.stderr.write("Can't import site init file '%s': %s\n"%(site_init_file, e))
            raise
        except Exception, e:
            sys.stderr.write("Site init file '%s' raised exception: %s\n"%(site_init_file, e))
            raise
    if os.path.exists(site_tools_dir):
        SCons.Tool.DefaultToolpath.append(os.path.abspath(site_tools_dir))

def version_string(label, module):
    fmt = "\t%s: v%s.%s, %s, by %s on %s\n"
    return fmt % (label,
                  module.__version__,
                  module.__build__,
                  module.__date__,
                  module.__developer__,
                  module.__buildsys__)

class OptParser(OptionParser):
    def __init__(self):
        import __main__

        parts = ["SCons by Steven Knight et al.:\n"]
        try:
            parts.append(version_string("script", __main__))
        except KeyboardInterrupt:
            raise
        except:
            # On Windows there is no scons.py, so there is no
            # __main__.__version__, hence there is no script version.
            pass 
        parts.append(version_string("engine", SCons))
        parts.append("Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The SCons Foundation")
        OptionParser.__init__(self, version=string.join(parts, ''),
                              usage="usage: scons [OPTION] [TARGET] ...")

        # options ignored for compatibility
        def opt_ignore(option, opt, value, parser):
            sys.stderr.write("Warning:  ignoring %s option\n" % opt)
        self.add_option("-b", "-m", "-S", "-t", "--no-keep-going", "--stop",
                        "--touch", action="callback", callback=opt_ignore,
                        help="Ignored for compatibility.")

        self.add_option('-c', '--clean', '--remove', action="store_true",
                        dest="clean",
                        help="Remove specified targets and dependencies.")

        self.add_option('-C', '--directory', type="string", action = "append",
                        metavar="DIR",
                        help="Change to DIR before doing anything.")

        self.add_option('--cache-debug', action="store",
                        dest="cache_debug", metavar="FILE",
                        help="Print CacheDir debug info to FILE.")

        self.add_option('--cache-disable', '--no-cache',
                        action="store_true", dest='cache_disable', default=0,
                        help="Do not retrieve built targets from CacheDir.")

        self.add_option('--cache-force', '--cache-populate',
                        action="store_true", dest='cache_force', default=0,
                        help="Copy already-built targets into the CacheDir.")

        self.add_option('--cache-show',
                        action="store_true", dest='cache_show', default=0,
                        help="Print build actions for files from CacheDir.")

        config_options = ["auto", "force" ,"cache"]

        def opt_config(option, opt, value, parser, c_options=config_options):
            if value in c_options:
                parser.values.config = value
            else:
                raise OptionValueError("Warning:  %s is not a valid config type" % value)
        self.add_option('--config', action="callback", type="string",
                        callback=opt_config, nargs=1, dest="config",
                        metavar="MODE", default="auto",
                        help="Controls Configure subsystem: "
                             "%s." % string.join(config_options, ", "))

        def opt_not_yet(option, opt, value, parser):
            sys.stderr.write("Warning:  the %s option is not yet implemented\n" % opt)
            sys.exit(0)
        self.add_option('-d', action="callback",
                        callback=opt_not_yet,
                        help = "Print file dependency information.")
        
        self.add_option('-D', action="store_const", const=2, dest="climb_up",
                        help="Search up directory tree for SConstruct,       "
                             "build all Default() targets.")

        debug_options = ["count", "dtree", "explain", "findlibs",
                         "includes", "memoizer", "memory", "objects",
                         "pdb", "presub", "stacktrace", "stree",
                         "time", "tree"]

        deprecated_debug_options = {
            "nomemoizer" : ' and has no effect',
        }

        def opt_debug(option, opt, value, parser, debug_options=debug_options, deprecated_debug_options=deprecated_debug_options):
            if value in debug_options:
                try:
                    if parser.values.debug is None:
                        parser.values.debug = []
                except AttributeError:
                    parser.values.debug = []
                parser.values.debug.append(value)
            elif value in deprecated_debug_options.keys():
                msg = deprecated_debug_options[value]
                w = "The --debug=%s option is deprecated%s." % (value, msg)
                delayed_warnings.append((SCons.Warnings.DeprecatedWarning, w))
            else:
                raise OptionValueError("Warning:  %s is not a valid debug type" % value)
        self.add_option('--debug', action="callback", type="string",
                        callback=opt_debug, nargs=1, dest="debug",
                        metavar="TYPE",
                        help="Print various types of debugging information: "
                             "%s." % string.join(debug_options, ", "))

        def opt_diskcheck(option, opt, value, parser):
            try:
                global diskcheck_option_set
                diskcheck_option_set = diskcheck_convert(value)
                SCons.Node.FS.set_diskcheck(diskcheck_option_set)
            except ValueError, e:
                raise OptionValueError("Warning: `%s' is not a valid diskcheck type" % e)

            
        self.add_option('--diskcheck', action="callback", type="string",
                        callback=opt_diskcheck, dest='diskcheck',
                        metavar="TYPE",
                        help="Enable specific on-disk checks.")

        def opt_duplicate(option, opt, value, parser):
            if not value in SCons.Node.FS.Valid_Duplicates:
                raise OptionValueError("`%s' is not a valid duplication style." % value)
            parser.values.duplicate = value
            # Set the duplicate style right away so it can affect linking
            # of SConscript files.
            SCons.Node.FS.set_duplicate(value)
        self.add_option('--duplicate', action="callback", type="string",
                        callback=opt_duplicate, nargs=1, dest="duplicate",
                        help="Set the preferred duplication methods. Must be one of "
                        + string.join(SCons.Node.FS.Valid_Duplicates, ", "))

        self.add_option('-f', '--file', '--makefile', '--sconstruct',
                        action="append", nargs=1,
                        help="Read FILE as the top-level SConstruct file.")

        self.add_option('-h', '--help', action="store_true", default=0,
                        dest="help_msg",
                        help="Print defined help message, or this one.")

        self.add_option("-H", "--help-options",
                        action="help",
                        help="Print this message and exit.")

        self.add_option('-i', '--ignore-errors', action="store_true",
                        default=0, dest='ignore_errors',
                        help="Ignore errors from build actions.")

        self.add_option('-I', '--include-dir', action="append",
                        dest='include_dir', metavar="DIR",
                        help="Search DIR for imported Python modules.")

        self.add_option('--implicit-cache', action="store_true",
                        dest='implicit_cache',
                        help="Cache implicit dependencies")

        self.add_option('--implicit-deps-changed', action="store_true",
                        default=0, dest='implicit_deps_changed',
                        help="Ignore cached implicit dependencies.")
        self.add_option('--implicit-deps-unchanged', action="store_true",
                        default=0, dest='implicit_deps_unchanged',
                        help="Ignore changes in implicit dependencies.")

        def opt_j(option, opt, value, parser):
            value = int(value)
            parser.values.num_jobs = value
        self.add_option('-j', '--jobs', action="callback", type="int",
                        callback=opt_j, metavar="N",
                        help="Allow N jobs at once.")

        self.add_option('-k', '--keep-going', action="store_true", default=0,
                        dest='keep_going',
                        help="Keep going when a target can't be made.")

        self.add_option('--max-drift', type="int", action="store",
                        dest='max_drift', metavar="N",
                        help="Set maximum system clock drift to N seconds.")

        self.add_option('-n', '--no-exec', '--just-print', '--dry-run',
                        '--recon', action="store_true", dest='noexec',
                        default=0, help="Don't build; just print commands.")

        self.add_option('--no-site-dir', action="store_true",
                        dest='no_site_dir', default=0,
                        help="Don't search or use the usual site_scons dir.")

        self.add_option('--profile', action="store",
                        dest="profile_file", metavar="FILE",
                        help="Profile SCons and put results in FILE.")

        self.add_option('-q', '--question', action="store_true", default=0,
                        help="Don't build; exit status says if up to date.")

        self.add_option('-Q', dest='no_progress', action="store_true",
                        default=0,
                        help="Suppress \"Reading/Building\" progress messages.")

        self.add_option('--random', dest="random", action="store_true",
                        default=0, help="Build dependencies in random order.")

        self.add_option('-s', '--silent', '--quiet', action="store_true",
                        default=0, help="Don't print commands.")

        self.add_option('--site-dir', action="store",
                        dest='site_dir', metavar="DIR",
                        help="Use DIR instead of the usual site_scons dir.")

        self.add_option('--taskmastertrace', action="store",
                        dest="taskmastertrace_file", metavar="FILE",
                        help="Trace Node evaluation to FILE.")

        tree_options = ["all", "derived", "prune", "status"]

        def opt_tree(option, opt, value, parser, tree_options=tree_options):
            tp = TreePrinter()
            for o in string.split(value, ','):
                if o == 'all':
                    tp.derived = False
                elif o == 'derived':
                    tp.derived = True
                elif o == 'prune':
                    tp.prune = True
                elif o == 'status':
                    tp.status = True
                else:
                    raise OptionValueError("Warning:  %s is not a valid --tree option" % o)
            tree_printers.append(tp)

        self.add_option('--tree', action="callback", type="string",
                        callback=opt_tree, nargs=1, metavar="OPTIONS",
                        help="Print a dependency tree in various formats: "
                             "%s." % string.join(tree_options, ", "))

        self.add_option('-u', '--up', '--search-up', action="store_const",
                        dest="climb_up", default=0, const=1,
                        help="Search up directory tree for SConstruct,       "
                             "build targets at or below current directory.")
        self.add_option('-U', action="store_const", dest="climb_up",
                        default=0, const=3,
                        help="Search up directory tree for SConstruct,       "
                             "build Default() targets from local SConscript.")

        self.add_option("-v", "--version",
                        action="version",
                        help="Print the SCons version number and exit.")

        self.add_option('--warn', '--warning', nargs=1, action="store",
                        metavar="WARNING-SPEC",
                        help="Enable or disable warnings.")

        self.add_option('-Y', '--repository', '--srcdir',
                        nargs=1, action="append",
                        help="Search REPOSITORY for source and target files.")

        self.add_option('-e', '--environment-overrides', action="callback",
                        callback=opt_not_yet,
                        # help="Environment variables override makefiles."
                        help=SUPPRESS_HELP)
        self.add_option('-l', '--load-average', '--max-load', action="callback",
                        callback=opt_not_yet, type="int", dest="load_average",
                        # action="store",
                        # help="Don't start multiple jobs unless load is below "
                        #      "LOAD-AVERAGE."
                        # type="int",
                        help=SUPPRESS_HELP)
        self.add_option('--list-derived', action="callback",
                        callback=opt_not_yet,
                        # help="Don't build; list files that would be built."
                        help=SUPPRESS_HELP)
        self.add_option('--list-actions', action="callback",
                        callback=opt_not_yet,
                        # help="Don't build; list files and build actions."
                        help=SUPPRESS_HELP)
        self.add_option('--list-where', action="callback",
                        callback=opt_not_yet,
                        # help="Don't build; list files and where defined."
                        help=SUPPRESS_HELP)
        self.add_option('-o', '--old-file', '--assume-old', action="callback",
                        callback=opt_not_yet, type="string", dest="old_file",
                        # help = "Consider FILE to be old; don't rebuild it."
                        help=SUPPRESS_HELP)
        self.add_option('--override', action="callback", dest="override",
                        callback=opt_not_yet, type="string",
                        # help="Override variables as specified in FILE."
                        help=SUPPRESS_HELP)
        self.add_option('-p', action="callback",
                        callback=opt_not_yet,
                        # help="Print internal environments/objects."
                        help=SUPPRESS_HELP)
        self.add_option('-r', '-R', '--no-builtin-rules',
                        '--no-builtin-variables', action="callback",
                        callback=opt_not_yet,
                        # help="Clear default environments and variables."
                        help=SUPPRESS_HELP)
        self.add_option('-w', '--print-directory', action="callback",
                        callback=opt_not_yet,
                        # help="Print the current directory."
                        help=SUPPRESS_HELP)
        self.add_option('--no-print-directory', action="callback",
                        callback=opt_not_yet,
                        # help="Turn off -w, even if it was turned on implicitly."
                        help=SUPPRESS_HELP)
        self.add_option('--write-filenames', action="callback",
                        callback=opt_not_yet, type="string", dest="write_filenames",
                        # help="Write all filenames examined into FILE."
                        help=SUPPRESS_HELP)
        self.add_option('-W', '--what-if', '--new-file', '--assume-new',
                        dest="new_file",
                        action="callback", callback=opt_not_yet, type="string",
                        # help="Consider FILE to be changed."
                        help=SUPPRESS_HELP)
        self.add_option('--warn-undefined-variables', action="callback",
                        callback=opt_not_yet,
                        # help="Warn when an undefined variable is referenced."
                        help=SUPPRESS_HELP)

    def parse_args(self, args=None, values=None):
        opt, arglist = OptionParser.parse_args(self, args, values)
        if opt.implicit_deps_changed or opt.implicit_deps_unchanged:
            opt.implicit_cache = 1
        return opt, arglist

class SConscriptSettableOptions:
    """This class wraps an OptParser instance and provides
    uniform access to options that can be either set on the command
    line or from a SConscript file. A value specified on the command
    line always overrides a value set in a SConscript file.
    Not all command line options are SConscript settable, and the ones
    that are must be explicitly added to settable dictionary and optionally
    validated and coerced in the set() method."""
    
    def __init__(self, options):
        self.options = options

        # This dictionary stores the defaults for all the SConscript
        # settable options, as well as indicating which options
        # are SConscript settable. 
        self.settable = {'num_jobs':1,
                         'max_drift':SCons.Node.FS.default_max_drift,
                         'implicit_cache':0,
                         'clean':0,
                         'duplicate':'hard-soft-copy',
                         'diskcheck':diskcheck_all}

    def get(self, name):
        if not self.settable.has_key(name):
            raise SCons.Errors.UserError, "This option is not settable from a SConscript file: %s"%name
        if hasattr(self.options, name) and getattr(self.options, name) is not None:
            return getattr(self.options, name)
        else:
            return self.settable[name]

    def set(self, name, value):
        if not self.settable.has_key(name):
            raise SCons.Errors.UserError, "This option is not settable from a SConscript file: %s"%name

        if name == 'num_jobs':
            try:
                value = int(value)
                if value < 1:
                    raise ValueError
            except ValueError:
                raise SCons.Errors.UserError, "A positive integer is required: %s"%repr(value)
        elif name == 'max_drift':
            try:
                value = int(value)
            except ValueError:
                raise SCons.Errors.UserError, "An integer is required: %s"%repr(value)
        elif name == 'duplicate':
            try:
                value = str(value)
            except ValueError:
                raise SCons.Errors.UserError, "A string is required: %s"%repr(value)
            if not value in SCons.Node.FS.Valid_Duplicates:
                raise SCons.Errors.UserError, "Not a valid duplication style: %s" % value
            # Set the duplicate stye right away so it can affect linking
            # of SConscript files.
            SCons.Node.FS.set_duplicate(value)
        elif name == 'diskcheck':
            try:
                value = diskcheck_convert(value)
            except ValueError, v:
                raise SCons.Errors.UserError, "Not a valid diskcheck value: %s"%v
            if not diskcheck_option_set:
                SCons.Node.FS.set_diskcheck(value)

        self.settable[name] = value
    

def _main(args, parser):
    global exit_status

    # Here's where everything really happens.

    # First order of business:  set up default warnings and and then
    # handle the user's warning options, so we can warn about anything
    # that happens appropriately.
    default_warnings = [ SCons.Warnings.CorruptSConsignWarning,
                         SCons.Warnings.DeprecatedWarning,
                         SCons.Warnings.DuplicateEnvironmentWarning,
                         SCons.Warnings.MissingSConscriptWarning,
                         SCons.Warnings.NoMD5ModuleWarning,
                         SCons.Warnings.NoMetaclassSupportWarning,
                         SCons.Warnings.NoParallelSupportWarning,
                         SCons.Warnings.MisleadingKeywordsWarning, ]
    for warning in default_warnings:
        SCons.Warnings.enableWarningClass(warning)
    SCons.Warnings._warningOut = _scons_internal_warning
    if options.warn:
        _setup_warn(options.warn)

    for warning_type, message in delayed_warnings:
        SCons.Warnings.warn(warning_type, message)

    # Next, we want to create the FS object that represents the outside
    # world's file system, as that's central to a lot of initialization.
    # To do this, however, we need to be in the directory from which we
    # want to start everything, which means first handling any relevant
    # options that might cause us to chdir somewhere (-C, -D, -U, -u).
    if options.directory:
        cdir = _create_path(options.directory)
        try:
            os.chdir(cdir)
        except OSError:
            sys.stderr.write("Could not change directory to %s\n" % cdir)

    # The SConstruct file may be in a repository, so initialize those
    # before we start the search up our path for one.
    global repositories
    if options.repository:
        repositories.extend(options.repository)

    target_top = None
    if options.climb_up:
        target_top = '.'  # directory to prepend to targets
        script_dir = os.getcwd()  # location of script
        while script_dir and not _SConstruct_exists(script_dir):
            script_dir, last_part = os.path.split(script_dir)
            if last_part:
                target_top = os.path.join(last_part, target_top)
            else:
                script_dir = ''
        if script_dir:
            display("scons: Entering directory `%s'" % script_dir)
            os.chdir(script_dir)

    # Now that we're in the top-level SConstruct directory, go ahead
    # and initialize the FS object that represents the file system,
    # and make it the build engine default.
    fs = SCons.Node.FS.default_fs = SCons.Node.FS.FS()

    for rep in repositories:
        fs.Repository(rep)

    # Now that we have the FS object, the next order of business is to
    # check for an SConstruct file (or other specified config file).
    # If there isn't one, we can bail before doing any more work.
    scripts = []
    if options.file:
        scripts.extend(options.file)
    if not scripts:
        sfile = _SConstruct_exists()
        if sfile:
            scripts.append(sfile)

    if not scripts:
        if options.help_msg:
            # There's no SConstruct, but they specified -h.
            # Give them the options usage now, before we fail
            # trying to read a non-existent SConstruct file.
            parser.print_help()
            exit_status = 0
            return
        raise SCons.Errors.UserError, "No SConstruct file found."

    if scripts[0] == "-":
        d = fs.getcwd()
    else:
        d = fs.File(scripts[0]).dir
    fs.set_SConstruct_dir(d)

    # Now that we have the FS object and it's intialized, set up (most
    # of) the rest of the options.
    global ssoptions
    ssoptions = SConscriptSettableOptions(options)

    _set_globals(options)
    SCons.Node.implicit_cache = options.implicit_cache
    SCons.Node.implicit_deps_changed = options.implicit_deps_changed
    SCons.Node.implicit_deps_unchanged = options.implicit_deps_unchanged
    if options.noexec:
        SCons.SConf.dryrun = 1
        SCons.Action.execute_actions = None
        CleanTask.execute = CleanTask.show
    if options.question:
        SCons.SConf.dryrun = 1
    SCons.SConf.SetCacheMode(options.config)
    SCons.SConf.SetProgressDisplay(progress_display)

    if options.no_progress or options.silent:
        progress_display.set_mode(0)
    if options.silent:
        display.set_mode(0)
    if options.silent:
        SCons.Action.print_actions = None

    if options.cache_debug:
        fs.CacheDebugEnable(options.cache_debug)
    if options.cache_disable:
        def disable(self): pass
        fs.CacheDir = disable
    if options.cache_force:
        fs.cache_force = 1
    if options.cache_show:
        fs.cache_show = 1

    if options.site_dir:
        _load_site_scons_dir(d, options.site_dir)
    elif not options.no_site_dir:
        _load_site_scons_dir(d)
        
    if options.include_dir:
        sys.path = options.include_dir + sys.path

    # That should cover (most of) the options.  Next, set up the variables
    # that hold command-line arguments, so the SConscript files that we
    # read and execute have access to them.
    targets = []
    xmit_args = []
    for a in args:
        if '=' in a:
            xmit_args.append(a)
        else:
            targets.append(a)
    SCons.Script._Add_Targets(targets)
    SCons.Script._Add_Arguments(xmit_args)

    sys.stdout = SCons.Util.Unbuffered(sys.stdout)

    memory_stats.append('before reading SConscript files:')
    count_stats.append(('pre-', 'read'))

    progress_display("scons: Reading SConscript files ...")

    start_time = time.time()
    try:
        for script in scripts:
            SCons.Script._SConscript._SConscript(fs, script)
    except SCons.Errors.StopError, e:
        # We had problems reading an SConscript file, such as it
        # couldn't be copied in to the BuildDir.  Since we're just
        # reading SConscript files and haven't started building
        # things yet, stop regardless of whether they used -i or -k
        # or anything else.
        sys.stderr.write("scons: *** %s  Stop.\n" % e)
        exit_status = 2
        sys.exit(exit_status)
    global sconscript_time
    sconscript_time = time.time() - start_time
    SCons.SConf.CreateConfigHBuilder(SCons.Defaults.DefaultEnvironment())
    progress_display("scons: done reading SConscript files.")

    # Tell the Node.FS subsystem that we're all done reading the
    # SConscript files and calling Repository() and BuildDir() and the
    # like, so it can go ahead and start memoizing the string values of
    # file system nodes.
    SCons.Node.FS.save_strings(1)

    memory_stats.append('after reading SConscript files:')
    count_stats.append(('post-', 'read'))

    fs.chdir(fs.Top)

    if options.help_msg:
        help_text = SCons.Script.help_text
        if help_text is None:
            # They specified -h, but there was no Help() inside the
            # SConscript files.  Give them the options usage.
            parser.print_help(sys.stdout)
        else:
            print help_text
            print "Use scons -H for help about command-line options."
        exit_status = 0
        return

    # Now that we've read the SConscripts we can set the options
    # that are SConscript settable:
    SCons.Node.implicit_cache = ssoptions.get('implicit_cache')
    SCons.Node.FS.set_duplicate(ssoptions.get('duplicate'))
    fs.set_max_drift(ssoptions.get('max_drift'))

    lookup_top = None
    if targets or SCons.Script.BUILD_TARGETS != SCons.Script._build_plus_default:
        # They specified targets on the command line or modified
        # BUILD_TARGETS in the SConscript file(s), so if they used -u,
        # -U or -D, we have to look up targets relative to the top,
        # but we build whatever they specified.
        if target_top:
            lookup_top = fs.Dir(target_top)
            target_top = None

        targets = SCons.Script.BUILD_TARGETS
    else:
        # There are no targets specified on the command line,
        # so if they used -u, -U or -D, we may have to restrict
        # what actually gets built.
        d = None
        if target_top:
            if options.climb_up == 1:
                # -u, local directory and below
                target_top = fs.Dir(target_top)
                lookup_top = target_top
            elif options.climb_up == 2:
                # -D, all Default() targets
                target_top = None
                lookup_top = None
            elif options.climb_up == 3:
                # -U, local SConscript Default() targets
                target_top = fs.Dir(target_top)
                def check_dir(x, target_top=target_top):
                    if hasattr(x, 'cwd') and not x.cwd is None:
                        cwd = x.cwd.srcnode()
                        return cwd == target_top
                    else:
                        # x doesn't have a cwd, so it's either not a target,
                        # or not a file, so go ahead and keep it as a default
                        # target and let the engine sort it out:
                        return 1                
                d = filter(check_dir, SCons.Script.DEFAULT_TARGETS)
                SCons.Script.DEFAULT_TARGETS[:] = d
                target_top = None
                lookup_top = None

        targets = SCons.Script._Get_Default_Targets(d, fs)

    if not targets:
        sys.stderr.write("scons: *** No targets specified and no Default() targets found.  Stop.\n")
        sys.exit(2)

    def Entry(x, ltop=lookup_top, ttop=target_top, fs=fs):
        if isinstance(x, SCons.Node.Node):
            node = x
        else:
            node = None
            # Why would ltop be None? Unfortunately this happens.
            if ltop == None: ltop = ''
            # Curdir becomes important when SCons is called with -u, -C,
            # or similar option that changes directory, and so the paths
            # of targets given on the command line need to be adjusted.
            curdir = os.path.join(os.getcwd(), str(ltop))
            for lookup in SCons.Node.arg2nodes_lookups:
                node = lookup(x, curdir=curdir)
                if node != None:
                    break
            if node is None:
                node = fs.Entry(x, directory=ltop, create=1)
        if ttop and not node.is_under(ttop):
            if isinstance(node, SCons.Node.FS.Dir) and ttop.is_under(node):
                node = ttop
            else:
                node = None
        return node

    nodes = filter(None, map(Entry, targets))

    task_class = BuildTask      # default action is to build targets
    opening_message = "Building targets ..."
    closing_message = "done building targets."
    if keep_going_on_error:
        failure_message = "done building targets (errors occurred during build)."
    else:
        failure_message = "building terminated because of errors."
    if options.question:
        task_class = QuestionTask
    try:
        if ssoptions.get('clean'):
            task_class = CleanTask
            opening_message = "Cleaning targets ..."
            closing_message = "done cleaning targets."
            if keep_going_on_error:
                closing_message = "done cleaning targets (errors occurred during clean)."
            else:
                failure_message = "cleaning terminated because of errors."
    except AttributeError:
        pass

    if options.random:
        def order(dependencies):
            """Randomize the dependencies."""
            # This is cribbed from the implementation of
            # random.shuffle() in Python 2.X.
            d = dependencies
            for i in xrange(len(d)-1, 0, -1):
                j = int(random.random() * (i+1))
                d[i], d[j] = d[j], d[i]
            return d
    else:
        def order(dependencies):
            """Leave the order of dependencies alone."""
            return dependencies

    progress_display("scons: " + opening_message)
    if options.taskmastertrace_file == '-':
        tmtrace = sys.stdout
    elif options.taskmastertrace_file:
        tmtrace = open(options.taskmastertrace_file, 'wb')
    else:
        tmtrace = None
    taskmaster = SCons.Taskmaster.Taskmaster(nodes, task_class, order, tmtrace)

    global num_jobs
    num_jobs = ssoptions.get('num_jobs')
    jobs = SCons.Job.Jobs(num_jobs, taskmaster)
    if num_jobs > 1 and jobs.num_jobs == 1:
        msg = "parallel builds are unsupported by this version of Python;\n" + \
              "\tignoring -j or num_jobs option.\n"
        SCons.Warnings.warn(SCons.Warnings.NoParallelSupportWarning, msg)

    memory_stats.append('before building targets:')
    count_stats.append(('pre-', 'build'))

    try:
        jobs.run()
    finally:
        if exit_status:
            progress_display("scons: " + failure_message)
        else:
            progress_display("scons: " + closing_message)
        if not options.noexec:
            SCons.SConsign.write()

    memory_stats.append('after building targets:')
    count_stats.append(('post-', 'build'))

def _exec_main():
    sconsflags = os.environ.get('SCONSFLAGS', '')
    all_args = string.split(sconsflags) + sys.argv[1:]

    parser = OptParser()
    global options
    options, args = parser.parse_args(all_args)
    if type(options.debug) == type([]) and "pdb" in options.debug:
        import pdb
        pdb.Pdb().runcall(_main, args, parser)
    elif options.profile_file:
        from profile import Profile

        # Some versions of Python 2.4 shipped a profiler that had the
        # wrong 'c_exception' entry in its dispatch table.  Make sure
        # we have the right one.  (This may put an unnecessary entry
        # in the table in earlier versions of Python, but its presence
        # shouldn't hurt anything).
        try:
            dispatch = Profile.dispatch
        except AttributeError:
            pass
        else:
            dispatch['c_exception'] = Profile.trace_dispatch_return

        prof = Profile()
        try:
            prof.runcall(_main, args, parser)
        except SystemExit:
            pass
        prof.dump_stats(options.profile_file)
    else:
        _main(args, parser)

def main():
    global exit_status
    global first_command_start
    
    try:
        _exec_main()
    except SystemExit, s:
        if s:
            exit_status = s
    except KeyboardInterrupt:
        print "Build interrupted."
        sys.exit(2)
    except SyntaxError, e:
        _scons_syntax_error(e)
    except SCons.Errors.InternalError:
        _scons_internal_error()
    except SCons.Errors.UserError, e:
        _scons_user_error(e)
    except:
        # An exception here is likely a builtin Python exception Python
        # code in an SConscript file.  Show them precisely what the
        # problem was and where it happened.
        SCons.Script._SConscript.SConscript_exception()
        sys.exit(2)

    memory_stats.print_stats()
    count_stats.print_stats()

    if print_objects:
        SCons.Debug.listLoggedInstances('*')
        #SCons.Debug.dumpLoggedInstances('*')

    if print_memoizer:
        SCons.Memoize.Dump("Memoizer (memory cache) hits and misses:")

    # Dump any development debug info that may have been enabled.
    # These are purely for internal debugging during development, so
    # there's no need to control them with --debug= options; they're
    # controlled by changing the source code.
    SCons.Debug.dump_caller_counts()
    SCons.Taskmaster.dump_stats()

    if print_time:
        total_time = time.time() - SCons.Script.start_time
        if num_jobs == 1:
            ct = cumulative_command_time
        else:
            ct = last_command_end - first_command_start
        scons_time = total_time - sconscript_time - ct
        print "Total build time: %f seconds"%total_time
        print "Total SConscript file execution time: %f seconds"%sconscript_time
        print "Total SCons execution time: %f seconds"%scons_time
        print "Total command execution time: %f seconds"%ct

    sys.exit(exit_status)
