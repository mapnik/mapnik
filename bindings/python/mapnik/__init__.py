#
# This file is part of Mapnik (C++/Python mapping toolkit)
# Copyright (C) 2009 Artem Pavlenko, Dane Springmeyer
#
# Mapnik is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

"""Mapnik Python module.

Boost Python bindings to the Mapnik C++ shared library.

Several things happen when you do:
 
    >>> import mapnik

 1) Mapnik C++ objects are imported via the '__init__.py' from the '_mapnik.so' shared object
    which references libmapnik.so (linux), libmapnik.dylib (mac), or libmapnik.dll (win).
    
 2) The paths to the input plugins and font directories are imported from the 'paths.py'
    file which was constructed and installed during SCons installation.

 3) All available input plugins and and TrueType fonts are automatically registered.

 4) Boost Python metaclass injectors are used in the '__init__.py' to wrap/extend several
    objects adding extra convenience when accessed via Python.

Note: besides those objects modified by boost metaclass injection all class objects
are only accessible by various documentation viewers via the 'mapnik._mapnik' module.

"""

from sys import getdlopenflags,setdlopenflags
try:
    from dl import RTLD_NOW, RTLD_GLOBAL
except ImportError:
    RTLD_NOW = 2
    RTLD_GLOBAL = 256

flags = getdlopenflags()
setdlopenflags(RTLD_NOW | RTLD_GLOBAL)

from _mapnik import *
from paths import inputpluginspath, fontscollectionpath

# The base Boost.Python class
BoostPythonMetaclass = Coord.__class__

class _injector(object):
    class __metaclass__(BoostPythonMetaclass):
        def __init__(self, name, bases, dict):
            for b in bases:
                if type(b) not in (self, type):
                    for k,v in dict.items():
                        setattr(b,k,v)
            return type.__init__(self, name, bases, dict)

class _Coord(Coord,_injector):
    def __repr__(self):
        return 'Coord(%s,%s)' % (self.x, self.y)
    def forward(self,obj):
        return forward_(self,obj)
    def inverse(self,obj):
        return inverse_(self,obj)

class _Envelope(Envelope,_injector):
    def __repr__(self):
        return 'Envelope(%s,%s,%s,%s)' % \
            (self.minx,self.miny,self.maxx,self.maxy)
    def forward(self,obj):
        return forward_(self,obj)
    def inverse(self,obj):
        return inverse_(self,obj)

class _Projection(Projection,_injector):
    def __repr__(self):
        return "Projection('%s')" % self.params()
    def forward(self,obj):
        return forward_(obj,self)
    def inverse(self,obj):
        return inverse_(obj,self)
  
class _Datasource(Datasource,_injector):
    def describe(self):
        return Describe(self)

#class _Filter(Filter,_injector):
#    """Mapnik Filter expression.
#    
#    Usage:
#    >>> from mapnik import Filter
#    >>> Filter("[waterway]='canal' and not ([tunnel] = 'yes' or [tunnel] ='true')")
#    
#    """

def Datasource(**keywords):
    """Wrapper around CreateDatasource.

    Create a Mapnik Datasource using a dictionary of parameters.

    Keywords must include:
    
      type='plugin_name' # e.g. type='gdal'
    
    See the convenience factory methods of each input plugin for
    details on additional required keyword arguments.
    
    """

    return CreateDatasource(keywords)

# convenience factory methods

def Shapefile(**keywords):
    """Create a Shapefile Datasource.
  
    Required keyword arguments:
      file -- path to shapefile without extension

    Optional keyword arguments:
      base -- path prefix (default None)
      encoding -- file encoding (default 'utf-8')
    
    >>> from mapnik import Shapefile, Layer
    >>> shp = Shapefile(base='/home/mapnik/data',file='world_borders') 
    >>> lyr = Layer('Shapefile Layer')
    >>> lyr.datasource = shp
    
    """
    keywords['type'] = 'shape'
    return CreateDatasource(keywords)

def PostGIS(**keywords):
    """Create a PostGIS Datasource.
  
    Required keyword arguments:
      dbname -- database name to connect to
      table -- table name or subselect query

    Optional db connection keyword arguments:
      user -- database user to connect as (default: see postgres docs)
      password -- password for database user (default: see postgres docs)
      host -- portgres hostname (default: see postgres docs)
      port -- postgres port (default: see postgres docs)
      initial_size -- integer size of connection pool (default 1)
      max_size -- integer max of connection pool (default 10)
    
    Optional table-level keyword arguments:
      extent -- manually specified data extent (comma delimited string, default None)
      estimate_extent -- boolean, direct PostGIS to use the faster, less accurate estimate_extent() over extent() (default False)
      row_limit -- integer limit of rows to return (default 0)
      cursor_size -- integer size of cursor to fetch (default 0)
      geometry_field -- specify geometry field (default first entry in geometry_columns)
      multiple_geometries -- boolean, direct the Mapnik wkb reader to interpret as multigeometries (default False)
    
    >>> from mapnik import PostGIS, Layer
    >>> params = dict(dbname='mapnik',table='osm',user='postgres',password='gis')
    >>> params['estimate_extent'] = False
    >>> params['extent'] = '-20037508,-19929239,20037508,19929239'
    >>> postgis = PostGIS(**params)
    >>> lyr = Layer('PostGIS Layer')
    >>> lyr.datasource = postgis
    
    """
    keywords['type'] = 'postgis'
    return CreateDatasource(keywords)

def Raster(**keywords):
    """Create a Raster (Tiff) Datasource.
  
    Required keyword arguments:
      file -- path to stripped or tiled tiff
      lox -- lowest (min) x/longitude of tiff extent
      loy -- lowest (min) y/latitude of tiff extent
      hix -- highest (max) x/longitude of tiff extent
      hiy -- highest (max) y/latitude of tiff extent
    
    Hint: lox,loy,hix,hiy make a Mapnik Envelope

    Optional keyword arguments:
      base -- path prefix (default None)
    
    >>> from mapnik import Raster, Layer
    >>> raster = Raster(base='/home/mapnik/data',file='elevation.tif',lox=-122.8,loy=48.5,hix=-122.7,hiy=48.6) 
    >>> lyr = Layer('Tiff Layer')
    >>> lyr.datasource = raster
    
    """
    keywords['type'] = 'raster'
    return CreateDatasource(keywords)

def Gdal(**keywords):
    """Create a GDAL Raster Datasource.
  
    Required keyword arguments:
      file -- path to GDAL supported dataset

    Optional keyword arguments:
      base -- path prefix (default None)
    
    >>> from mapnik import Gdal, Layer
    >>> dataset = Gdal(base='/home/mapnik/data',file='elevation.tif') 
    >>> lyr = Layer('GDAL Layer from TIFF file')
    >>> lyr.datasource = dataset
    
    """
    keywords['type'] = 'gdal'
    return CreateDatasource(keywords)

def Occi(**keywords):
    """Create a Oracle Spatial (10g) Vector Datasource.
  
    Required keyword arguments:
      user -- database user to connect as
      password -- password for database user
      host -- oracle host to connect to (does not refer to SID in tsnames.ora)
      table -- table name or subselect query

    Optional keyword arguments:
      initial_size -- integer size of connection pool (default 1)
      max_size -- integer max of connection pool (default 10)
      extent -- manually specified data extent (comma delimited string, default None)
      estimate_extent -- boolean, direct Oracle to use the faster, less accurate estimate_extent() over extent() (default False)
      encoding -- file encoding (default 'utf-8')
      geometry_field -- specify geometry field (default 'GEOLOC')
      use_spatial_index -- boolean, force the use of the spatial index (default True)
      multiple_geometries -- boolean, direct the Mapnik wkb reader to interpret as multigeometries (default False)
    
    >>> from mapnik import Occi, Layer
    >>> params = dict(host='myoracle',user='scott',password='tiger',table='test')
    >>> params['estimate_extent'] = False
    >>> params['extent'] = '-20037508,-19929239,20037508,19929239'
    >>> oracle = Occi(**params)
    >>> lyr = Layer('Oracle Spatial Layer')
    >>> lyr.datasource = oracle
    """
    keywords['type'] = 'occi'
    return CreateDatasource(keywords)

def Ogr(**keywords):
    """Create a OGR Vector Datasource.
  
    Required keyword arguments:
      file -- path to OGR supported dataset
      layer -- layer to use within datasource

    Optional keyword arguments:
      base -- path prefix (default None)
      encoding -- file encoding (default 'utf-8')
      multiple_geometries -- boolean, direct the Mapnik wkb reader to interpret as multigeometries (default False)
    
    >>> from mapnik import Ogr, Layer
    >>> datasource = Ogr(base='/home/mapnik/data',file='rivers.geojson',layer='OGRGeoJSON') 
    >>> lyr = Layer('OGR Layer from GeoJSON file')
    >>> lyr.datasource = datasource
    
    """
    keywords['type'] = 'ogr'
    return CreateDatasource(keywords)

def SQLite(**keywords):
    """Create a SQLite Datasource.
  
    Required keyword arguments:
      file -- path to SQLite database file
      table -- table name or subselect query

    Optional keyword arguments:
      base -- path prefix (default None)
      encoding -- file encoding (default 'utf-8')
      extent -- manually specified data extent (comma delimited string, default None)
      metadata -- name of auxillary table containing record for table with xmin, ymin, xmax, ymax, and f_table_name
      geometry_field -- name of geometry field (default 'the_geom')
      key_field -- name of primary key field (default 'OGC_FID')
      row_offset -- specify a custom integer row offset (default 0)
      row_limit -- specify a custom integer row limit (default 0)
      wkb_format -- specify a wkb type of 'spatialite' (default None)
      multiple_geometries -- boolean, direct the Mapnik wkb reader to interpret as multigeometries (default False)
      use_spatial_index -- boolean, instruct sqlite plugin to use Rtree spatial index (default True)

    >>> from mapnik import SQLite, Layer
    >>> sqlite = SQLite(base='/home/mapnik/data',file='osm.db',table='osm',extent='-20037508,-19929239,20037508,19929239') 
    >>> lyr = Layer('SQLite Layer')
    >>> lyr.datasource = sqlite
    
    """
    keywords['type'] = 'sqlite'
    return CreateDatasource(keywords)

def mapnik_version_string():
    """Return the Mapnik version as a string."""
    version = mapnik_version()
    patch_level = version % 100
    minor_version = version / 100 % 1000
    major_version = version / 100000
    return '%s.%s.%s' % ( major_version, minor_version,patch_level)

#register datasources
from mapnik import DatasourceCache
DatasourceCache.instance().register_datasources('%s' % inputpluginspath)
#register some fonts
from mapnik import FontEngine
from glob import glob
fonts = glob('%s/*.ttf' % fontscollectionpath)
if len( fonts ) == 0:
    print "### WARNING: No ttf files found in '%s'." % fontscollectionpath
else:
    map(FontEngine.instance().register_font, fonts)

#set dlopen flags back to the original
setdlopenflags(flags)
