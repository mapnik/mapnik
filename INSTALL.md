# Mapnik Installation


## Quick Start

To configure and build mapnik do:

    ./configure
    make
    sudo make install

If you need to uninstall do:

    sudo make uninstall

For more details see the 'Building' Section below.

Platform specific install guides at https://github.com/mapnik/mapnik/wiki/Mapnik-Installation

For troubleshooting help see https://github.com/mapnik/mapnik/wiki/InstallationTroubleshooting


## Depends

Mapnik is cross platform and runs on Linux, Mac OSX, Solaris, *BSD, and Windows.

The build system should work for all posix/unix systems but for windows see:

    https://github.com/mapnik/mapnik/wiki/BuildingOnWindows

Build dependencies are:

 * C++ compiler (like g++ or clang++)
 * Python >= 2.4
 * >= 2 GB RAM

Mapnik Core depends on:

 * Boost
    - >= 1.47 is required.
    - These libraries are required:
      - filesystem
      - system
      - thread (if mapnik threadsafe support is required, default on)
      - regex (optionally built with icu regex support)
      - program_options (optionally for mapnik command line programs)

 * libicuuc >= 4.0 (ideally >= 4.2) - International Components for Unicode
 * libpng >= 1.2.x - PNG Graphics
 * libjpeg - JPEG Graphics
 * libtiff - TIFF Graphics 
 * libz - Zlib Compression
 * libfreetype - Freetype2 for Font support (Install requires freetype-config)
 * libxml2 - XML parsing (Install requires xml2-config)
 * libproj - PROJ.4 Projection library

Mapnik Python binding depend on:

 * Python >= 2.4
 * Boost python

Optional dependencies:

 * Cairo - Graphics library for PDF, PS, and SVG formats
    - pkg-config - Required for building with cairo support
    - libsigc++ - C++ support for cairomm
    - cairomm - C++ bindings for cairo
    - pycairo - Python bindings for cairo
 * libpq - PostgreSQL libraries (For PostGIS plugin support)
 * libgdal - GDAL/OGR input (For gdal and ogr plugin support)
 * libsqlite3 - SQLite input (needs RTree support) (sqlite plugin support)
 * libocci - Oracle input plugin support
 * libcurl - OSM input plugin support

Instructions for installing many of these dependencies on
various platforms can be found at the Mapnik Community Wiki
(https://github.com/mapnik/mapnik/wiki/Mapnik-Installation).



## Building

The build system uses SCons, a pure python equivalent to autotools or cmake.

We provide a simple Makefile wrapper that can be used like:

    ./configure && make && make install

To interact with the local copy of scons directly you can do:

    python scons/scons.py configure

You can also use a globally installed scons:

    scons configure

If you want to clean your build do:

    make clean

If you experience odd configure errors, try resetting the SCons caches:

    make reset

To install in a custom location do:

    ./configure PREFIX=/opt/mapnik

To pass custom CXXFLAGS or LDFLAGS do:

    ./configure CUSTOM_CXXFLAGS="-g -I/usr/include" CUSTOM_LDFLAGS="-L/usr/lib"

To pass custom paths to a dependency, like boost, do:

    ./configure BOOST_INCLUDES=/opt/boost/include BOOST_LIBS=/opt/boost/lib

To pass custom paths to a dependency, like icu, do:

    ./configure ICU_INCLUDES=/usr/local/include ICU_LIBS=/usr/local/include

If you want to see configure options do:

    ./configure --help

For more details on all the options see:

    https://github.com/mapnik/mapnik/wiki/UsingScons


## Testing Installation

First, try importing the Mapnik python module in a python interpreter,
and make sure it does so without errors:

    $ python
    Python 2.5.1 (r251:54863, Jan 17 2008, 19:35:17) 
    [GCC 4.0.1 (Apple Inc. build 5465)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import mapnik
    >>> 

Then, try rendering the demo map, included in the Mapnik source code::

    cd demo/python
    python rundemo.py 

If the resulting maps look good, this indicates the core components of
Mapnik are installed properly, as well as the Shapefile plugin, Unicode
text support (ICU), and re-projection support using Proj.

For further tests see the `tests` folder within the Mapnik source code.


## Learning Mapnik

### Users

Visit https://github.com/mapnik/mapnik/wiki/LearningMapnik for basic tutorials on making maps with Mapnik using the Python bindings.

### Developers

Read docs/contributing.markdown for resources for getting involved with Mapnik development.


## Mapnik Community


Mapnik has an active community of talented users and developers making
amazing maps.

Please feel free to subscribe to the community list and post on both
usage and development topics: http://mapnik.org/contact/

You can also get involved by joining the #mapnik channel on irc://irc.freenode.net/mapnik
