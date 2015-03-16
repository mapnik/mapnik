from distutils.core import setup, Extension
from distutils.command.build_py import build_py
import subprocess

from glob import glob
import os
import sys

try:
    from distutils.command.build_py import build_py_2to3
except ImportError:
    pass

def run_cmd(*args):
    if sys.version_info[0] == 2:
        return subprocess.check_output(*args)
    else:
        return subprocess.check_output(*args).decode()

flags = run_cmd(['mapnik-config', '--defines', '--cxxflags']).split()
libs = [i[2:] for i in run_cmd(['mapnik-config', '--dep-libs', '--libs']).split() if i[:2] != '-L']
includes = [i[2:] for i in run_cmd(['mapnik-config', '--dep-includes', '--includes']).split()]

if sys.version_info[0] == 2:
    libs.append('boost_python')
else:
    libs.append('boost_python3')

paths_contents = """
from os.path import normpath,join,dirname

inputpluginspath = '%s'
fontscollectionpath = '%s'
__all__ = [inputpluginspath,fontscollectionpath]
""" % (run_cmd(['mapnik-config', '--input-plugins']).strip(), run_cmd(['mapnik-config', '--fonts']).strip())

if sys.version_info[0] == 2:
    baseclass = build_py
else:
    baseclass = build_py_2to3

class build_paths(baseclass):
    def run(self):
        # honor the --dry-run flag
        if not self.dry_run:
            target_dir = os.path.join(self.build_lib, 'mapnik')

            # mkpath is a distutils helper to create directories
            self.mkpath(target_dir)

            with open(os.path.join(target_dir, 'paths.py'), 'w') as fobj:
                fobj.write(paths_contents)

        # distutils uses old-style classes, so no super()
        baseclass.run(self)

setup(
    name='mapnik',
    version='2.3.x',
    ext_modules=[Extension('_mapnik', glob('*.cpp'),
                           libraries=libs,
                           extra_compile_args=flags,
                           include_dirs=includes,
                           extra_link_args=['-Wl,--no-undefined'],
                           language='c++')],
    packages=['mapnik', 'mapnik2'],
    cmdclass={'build_py': build_paths}
    )
