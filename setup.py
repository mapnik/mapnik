import os
import re
import sys
import glob

from setuptools import setup, find_packages, Extension
from subprocess import Popen, PIPE, check_output
from ctypes import CDLL


def rst(filename):
    '''
    Load an rst file and sanitize it for PyPI.
    Remove unsupported github tags:
     - code-block directive
    '''
    content = open(filename).read()
    return re.sub(r'\.\.\s? code-block::\s*\w+', '::', content)


def mapnik_config(key):
    cmd = 'mapnik-config --%s' % key
    try:
        ret = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)
    except OSError as e:
        if e.errno == os.errno.ENOENT:
            raise Exception('mapnik-config not found')  # handle file not found error.
        else:
            raise
    if ret.wait() != 0:
        raise Exception(
            '%s error: %s\n%s' % (
                cmd,
                ret.stdout.read(),
                ret.stderr.read(),
            )
        )
    return ret.stdout.read().strip()


bindings_dir = os.path.join('bindings', 'python')
include_dirs = [
    bindings_dir,
    os.path.join('deps', 'agg', 'include'),
]
libraries = ['jpeg', 'png', 'boost_thread', 'python%s.%s' % sys.version_info[:2]]
extra_compile_args = mapnik_config('cflags').split()
extra_link_args = mapnik_config('libs').split()


if sys.platform.startswith("linux"):
    # Multiarch support
    try:
        arch = check_output(['dpkg-architecture', '-qDEB_HOST_MULTIARCH']).strip()
        extra_compile_args.append('-I/usr/lib/%s/sigc++-2.0/include' % arch)
        extra_link_args.append('-L/usr/lib/%s' % arch)
    except:
        pass


if sys.platform == "win32":
    libraries.extend(
        "boost_python-mgw",
    )
else:
    prefix = 'cyg' if sys.platform == 'cygwin' else 'lib'
    if sys.platform == 'darwin':
        suffix = 'dylib'
    elif os.name == 'nt':
        suffix = 'dll'
    else:
        suffix = 'so'

    for lib in ["boost_python", "boost_python-gcc"]:
        try:
            if ".%s" % suffix in lib:
                lib = re.sub(".%s.*" % suffix, '', lib)
            if lib.startswith(prefix):
                lib = lib[len(prefix):]
            CDLL('%s%s.%s' % (prefix, lib, suffix))
            libraries.append(lib)
            break
        except OSError:
            pass
        except IndexError:
            raise Exception('Cant find boost_python lib!')


long_description = '\n'.join((
    rst(os.path.join(bindings_dir, 'README.rst')),
    rst(os.path.join(bindings_dir, 'CHANGELOG.rst')),
    ''
))

files = glob.glob(os.path.join(bindings_dir, '*.cpp'))

install_requires = ['setuptools', ]
for lib in extra_link_args:
    if 'cairo' in lib:
        dep = 'pycairo'
        if sys.version_info[0] < 3:
            dep = 'py2cairo'
        install_requires.append(dep)
        break

version = mapnik_config('version')

# nosetests configuration
os.environ['NOSE_WHERE'] = 'tests/python_tests'

setup(
    name='mapnik',
    version=version,
    description="Python bindings for mapnik",
    long_description=long_description,
    keywords='',
    author='Mathieu Le Marec - Pasquet & the mapnik community',
    author_email='kiorky@cryptelium.net',
    url='http://pypi.python.org/pypi/mapnik',
    license='LGPL',
    include_package_data=True,
    zip_safe=False,
    packages=find_packages(bindings_dir),
    package_dir={'': bindings_dir},
    ext_modules=[
        Extension(
            "_mapnik", files,
            include_dirs=include_dirs,
            extra_compile_args=extra_compile_args,
            libraries=libraries,
            extra_link_args=extra_link_args
        ),
    ],
    install_requires=install_requires,
    tests_require=['nose>=1.0'],
    test_suite='nose.collector',
    entry_points={},
    classifiers=[
        "Programming Language :: Python",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Scientific/Engineering :: GIS",
        "License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
    ],
)
