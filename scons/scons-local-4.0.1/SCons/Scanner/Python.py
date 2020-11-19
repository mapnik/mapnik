"""SCons.Scanner.Python

This module implements the dependency scanner for Python code.

One important note about the design is that this does not take any dependencies
upon packages or binaries in the Python installation unless they are listed in
PYTHONPATH. To do otherwise would have required code to determine where the
Python installation is, which is outside of the scope of a scanner like this.
If consumers want to pick up dependencies upon these packages, they must put
those directories in PYTHONPATH.

"""

#
# __COPYRIGHT__
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

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import itertools
import os
import re
import SCons.Scanner

# Capture python "from a import b" and "import a" statements.
from_cre = re.compile(r'^\s*from\s+([^\s]+)\s+import\s+(.*)', re.M)
import_cre = re.compile(r'^\s*import\s+([^\s]+)', re.M)


def path_function(env, dir=None, target=None, source=None, argument=None):
    """Retrieves a tuple with all search paths."""
    paths = env['ENV'].get('PYTHONPATH', '').split(os.pathsep)
    if source:
        paths.append(source[0].dir.abspath)
    return tuple(paths)


def find_include_names(node):
    """
    Scans the node for all imports.

    Returns a list of tuples. Each tuple has two elements:
        1. The main import (e.g. module, module.file, module.module2)
        2. Additional optional imports that could be functions or files
            in the case of a "from X import Y" statement. In the case of a
            normal "import" statement, this is None.
    """
    text = node.get_text_contents()
    all_matches = []
    matches = from_cre.findall(text)
    if matches:
        for match in matches:
            imports = [i.strip() for i in match[1].split(',')]

            # Add some custom logic to strip out "as" because the regex
            # includes it.
            last_import_split = imports[-1].split()
            if len(last_import_split) > 1:
                imports[-1] = last_import_split[0]

            all_matches.append((match[0], imports))

    matches = import_cre.findall(text)
    if matches:
        for match in matches:
            all_matches.append((match, None))

    return all_matches


def scan(node, env, path=()):
    # cache the includes list in node so we only scan it once:
    if node.includes is not None:
        includes = node.includes
    else:
        includes = find_include_names(node)
        # Intern the names of the include files. Saves some memory
        # if the same header is included many times.
        node.includes = list(map(SCons.Util.silent_intern, includes))

    # XXX TODO: Sort?
    nodes = []
    if callable(path):
        path = path()
    for module, imports in includes:
        is_relative = module.startswith('.')
        if is_relative:
            # This is a relative include, so we must ignore PYTHONPATH.
            module_lstripped = module.lstrip('.')
            # One dot is current directory, two is parent, three is
            # grandparent, etc.
            num_parents = len(module) - len(module_lstripped) - 1
            current_dir = node.get_dir()
            for i in itertools.repeat(None, num_parents):
                current_dir = current_dir.up()

            search_paths = [current_dir.abspath]
            search_string = module_lstripped
        else:
            search_paths = path
            search_string = module

        module_components = search_string.split('.')
        for search_path in search_paths:
            candidate_path = os.path.join(search_path, *module_components)
            # The import stored in "module" could refer to a directory or file.
            import_dirs = []
            if os.path.isdir(candidate_path):
                import_dirs = module_components

                # Because this resolved to a directory, there is a chance that
                # additional imports (e.g. from module import A, B) could refer
                # to files to import.
                if imports:
                    for imp in imports:
                        file = os.path.join(candidate_path, imp + '.py')
                        if os.path.isfile(file):
                            nodes.append(file)
            elif os.path.isfile(candidate_path + '.py'):
                nodes.append(candidate_path + '.py')
                import_dirs = module_components[:-1]

                # We can ignore imports because this resolved to a file. Any
                # additional imports (e.g. from module.file import A, B) would
                # only refer to functions in this file.

            # Take a dependency on all __init__.py files from all imported
            # packages unless it's a relative import. If it's a relative
            # import, we don't need to take the dependency because Python
            # requires that all referenced packages have already been imported,
            # which means that the dependency has already been established.
            if import_dirs and not is_relative:
                for i in range(len(import_dirs)):
                    init_components = module_components[:i+1] + ['__init__.py']
                    init_path = os.path.join(search_path, *(init_components))
                    if os.path.isfile(init_path):
                        nodes.append(init_path)
                break

    return sorted(nodes)


PythonSuffixes = ['.py']
PythonScanner = SCons.Scanner.Base(scan, name='PythonScanner',
                                   skeys=PythonSuffixes,
                                   path_function=path_function, recursive=1)

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
