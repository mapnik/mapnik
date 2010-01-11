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

 3) All available input plugins and TrueType fonts are automatically registered.

 4) Boost Python metaclass injectors are used in the '__init__.py' to extend several
    objects adding extra convenience when accessed via Python.

"""

import os

from sys import getdlopenflags, setdlopenflags
try:
    from ctypes import RTLD_NOW, RTLD_GLOBAL
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
    """
    Represents a point with two coordinates (either lon/lat or x/y).

    Following operators are defined for Coord:

    Addition and subtraction of Coord objects:

    >>> Coord(10, 10) + Coord(20, 20)
    Coord(30.0, 30.0)
    >>> Coord(10, 10) - Coord(20, 20)
    Coord(-10.0, -10.0)

    Addition, subtraction, multiplication and division between
    a Coord and a float:

    >>> Coord(10, 10) + 1
    Coord(11.0, 11.0)
    >>> Coord(10, 10) - 1
    Coord(-9.0, -9.0)
    >>> Coord(10, 10) * 2
    Coord(20.0, 20.0)
    >>> Coord(10, 10) / 2
    Coord(5.0, 5.0)

    Equality of coords (as pairwise equality of components):
    >>> Coord(10, 10) is Coord(10, 10)
    False
    >>> Coord(10, 10) == Coord(10, 10)
    True
    """
    def __repr__(self):
        return 'Coord(%s,%s)' % (self.x, self.y)

    def forward(self, projection):
        """
        Projects the point from the geographic coordinate 
        space  into the cartesian space. The x component is 
        considered to be longitude, the y component the 
        latitude.

        Returns the easting (x) and northing (y) as a 
        coordinate pair.
        
        Example: Project the geographic coordinates of the 
                 city center of Stuttgart into the local
                 map projection (GK Zone 3/DHDN, EPSG 31467)  
        >>> p = Projection('+init=epsg:31467') 
        >>> Coord(9.1, 48.7).forward(p)
        Coord(3507360.12813,5395719.2749)
        """
        return forward_(self, projection)

    def inverse(self, projection):
        """
        Projects the point from the cartesian space 
        into the geographic space. The x component is 
        considered to be the easting, the y component 
        to be the northing.
        
        Returns the longitude (x) and latitude (y) as a 
        coordinate pair.

        Example: Project the cartesian coordinates of the 
                 city center of Stuttgart in the local
                 map projection (GK Zone 3/DHDN, EPSG 31467)
                 into geographic coordinates:
        >>> p = Projection('+init=epsg:31467') 
        >>> Coord(3507360.12813,5395719.2749).inverse(p)
        Coord(9.1, 48.7)
        """
        return inverse_(self, projection)

class _Envelope(Envelope,_injector):
    """
    Represents a spatial envelope (i.e. bounding box). 
    
    
    Following operators are defined for Envelope:

    Addition:
    e1 + e2 is equvalent to e1.expand_to_include(e2) but yields 
    a new envelope instead of modifying e1

    Subtraction:
    Currently e1 - e2 returns e1.

    Multiplication and division with floats:
    Multiplication and division change the width and height of the envelope
    by the given factor without modifying its center..

    That is, e1 * x is equivalent to: 
           e1.width(x * e1.width())
           e1.height(x * e1.height()),
    except that a new envelope is created instead of modifying e1.

    e1 / x is equivalent to e1 * (1.0/x).

    Equality: two envelopes are equal if their corner points are equal.
    """
    def __repr__(self):
        return 'Envelope(%s,%s,%s,%s)' % \
            (self.minx,self.miny,self.maxx,self.maxy)
    def forward(self, projection):
        """
        Projects the envelope from the geographic space 
        into the cartesian space by projecting its corner 
        points.

        See also:
           Coord.forward(self, projection)
        """
        return forward_(self, projection)
    def inverse(self, projection):
        """
        Projects the envelope from the cartesian space 
        into the geographic space by projecting its corner 
        points.

        See also:
          Coord.inverse(self, projection).
        """
        return inverse_(self, projection)

class _Projection(Projection,_injector):
    def __repr__(self):
        return "Projection('%s')" % self.params()
    def forward(self,obj):
        """
        Projects the given object (Envelope or Coord) 
        from the geographic space into the cartesian space.

        See also:
          Envelope.forward(self, projection),
          Coord.forward(self, projection).
        """
        return forward_(obj,self)
    def inverse(self,obj):
        """
        Projects the given object (Envelope or Coord) 
        from the cartesian space into the geographic space.

        See also:
          Envelope.inverse(self, projection),
          Coord.inverse(self, projection).
        """
        return inverse_(obj,self)

def get_types(num):
    dispatch = {1: int,
                2: float,
                3: float,
                4: str,
                5: Geometry2d,
                6: object}
    return dispatch.get(num)

class _Datasource(Datasource,_injector):
    def describe(self):
        return Describe(self)
    def field_types(self):
        return map(get_types,self._field_types())
    def all_features(self):
        query = Query(self.envelope(),1.0)
        for fld in self.fields():
            query.add_property_name(fld)
        return self.features(query).features

class _Feature(Feature,_injector):
    @property
    def attributes(self):
        attr = {}
        for prop in self.properties:
            attr[prop[0]] = prop[1]
        return attr

class _Symbolizer(Symbolizer,_injector):
    def symbol(self):
        return getattr(self,self.type())()

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
      
      *Note: if using subselects for the 'table' value consider also 
       passing the 'geometry_field' and 'srid' and 'extent_from_subquery'
       options and/or specifying the 'geometry_table' option.

    Optional db connection keyword arguments:
      user -- database user to connect as (default: see postgres docs)
      password -- password for database user (default: see postgres docs)
      host -- portgres hostname (default: see postgres docs)
      port -- postgres port (default: see postgres docs)
      initial_size -- integer size of connection pool (default: 1)
      max_size -- integer max of connection pool (default: 10)
      persist_connection -- keep connection open (default: True)

    Optional table-level keyword arguments:
      extent -- manually specified data extent (comma delimited string, default: None)
      estimate_extent -- boolean, direct PostGIS to use the faster, less accurate `estimate_extent` over `extent` (default: False)
      extent_from_subquery -- boolean, direct Mapnik to query Postgis for the extent of the raw 'table' value (default: uses 'geometry_table')
      geometry_table -- specify geometry table to use to look up metadata (default: automatically parsed from 'table' value)
      geometry_field -- specify geometry field to use (default: first entry in geometry_columns)
      srid -- specify srid to use (default: auto-detected from geometry_field)
      row_limit -- integer limit of rows to return (default: 0)
      cursor_size -- integer size of binary cursor to use (default: 0, no binary cursor is used)
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
      shared -- boolean, open GdalDataset in shared mode (default: False)

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

def Osm(**keywords):
    """Create a Osm Datasource.

    Required keyword arguments:
      file -- path to OSM file

    Optional keyword arguments:
      encoding -- file encoding (default 'utf-8')
      url -- url to fetch data (default None)
      bbox -- data bounding box for fetching data (default None)

    >>> from mapnik import Osm, Layer
    >>> datasource = Osm(file='test.osm') 
    >>> lyr = Layer('Osm Layer')
    >>> lyr.datasource = datasource

    """
    # note: parser only supports libxml2 so not exposing option
    # parser -- xml parser to use (default libxml2)
    keywords['type'] = 'osm'
    return CreateDatasource(keywords)

def Kismet(**keywords):
    """Create a Kismet Datasource.

    Required keyword arguments:
      host -- kismet hostname
      port -- kismet port

    Optional keyword arguments:
      encoding -- file encoding (default 'utf-8')
      extent -- manually specified data extent (comma delimited string, default None)

    >>> from mapnik import Kismet, Layer
    >>> datasource = Kismet(host='localhost',port=2501,extent='-179,-85,179,85') 
    >>> lyr = Layer('Kismet Server Layer')
    >>> lyr.datasource = datasource

    """
    keywords['type'] = 'kismet'
    return CreateDatasource(keywords)

def mapnik_version_string():
    """Return the Mapnik version as a string."""
    version = mapnik_version()
    patch_level = version % 100
    minor_version = version / 100 % 1000
    major_version = version / 100000
    return '%s.%s.%s' % ( major_version, minor_version,patch_level)

def register_plugins(path=inputpluginspath):
    """Register plugins located by specified path"""
    DatasourceCache.instance().register_datasources(path)

def register_fonts(path=fontscollectionpath):
    """Recursively register fonts using path argument as base directory"""
    for dirpath, _, filenames in os.walk(path):
        for filename in filenames:
            if os.path.splitext(filename)[1] == '.ttf':
                FontEngine.instance().register_font(os.path.join(dirpath, filename))

# auto-register known plugins and fonts
register_plugins()
register_fonts()

#set dlopen flags back to the original
setdlopenflags(flags)

# Explicitly export API members to avoid namespace pollution
# and ensure correct documentation processing
__all__ = [
    # classes
    'Color', 'Coord', 
    'DatasourceCache',
    'Envelope',
    'Feature', 'Featureset', 'FontEngine',
    'Geometry2d',
    'Image', 'ImageView',
    'Layer', 'Layers',
    'LinePatternSymbolizer', 'LineSymbolizer',
    'Map',
    'Names',
    'Parameter', 'Parameters',
    'PointDatasource', 'PointSymbolizer',
    'PolygonPatternSymbolizer', 'PolygonSymbolizer',
    'ProjTransform',
    'Projection',
    'Properties',
    'Query',
    'RasterSymbolizer',
    'Rule', 'Rules',
    'ShieldSymbolizer',
    'Singleton',
    'Stroke', 'Style',
    'Symbolizer', 'Symbolizers',
    'TextSymbolizer',
    'ViewTransform',
    # enums
    'aspect_fix_mode', 'label_placement',
    'line_cap', 'line_join',
    'text_convert', 'vertical_alignment',
    # functions
    # datasources
    'Datasource', 'CreateDatasource',
    'Shapefile', 'PostGIS', 'Raster', 'Gdal',
    'Occi', 'Ogr', 'SQLite',
    'Osm', 'Kismet',
    'Describe',
    #   version and environment
    'mapnik_version_string', 'mapnik_version', 'mapnik_svn_revision',
    'has_cairo', 'has_pycairo',
    #   factory methods
    'Filter',
    #   load/save/render
    'load_map', 'load_map_from_string', 'save_map', 'save_map_to_string',
    'render', 'render_tile_to_file', 'render_to_file',
    #   other
    'register_plugins', 'register_fonts',
    'scale_denominator',
    ]
