#
# This file is part of Mapnik (C++/Python mapping toolkit)
# Copyright (C) 2009 Artem Pavlenko
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
    (_mapnik.pyd on win) which references libmapnik.so (linux), libmapnik.dylib (mac), or
    mapnik.dll (win32).

 2) The paths to the input plugins and font directories are imported from the 'paths.py'
    file which was constructed and installed during SCons installation.

 3) All available input plugins and TrueType fonts are automatically registered.

 4) Boost Python metaclass injectors are used in the '__init__.py' to extend several
    objects adding extra convenience when accessed via Python.

"""

import os
import sys
import warnings

from _mapnik import *
from paths import inputpluginspath, fontscollectionpath

import printing
printing.renderer = render

# The base Boost.Python class
BoostPythonMetaclass = Coord.__class__

class _MapnikMetaclass(BoostPythonMetaclass):
    def __init__(self, name, bases, dict):
        for b in bases:
            if type(b) not in (self, type):
                for k,v in list(dict.items()):
                    if hasattr(b, k):
                        setattr(b, '_c_'+k, getattr(b, k))
                    setattr(b,k,v)
        return type.__init__(self, name, bases, dict)

# metaclass injector compatible with both python 2 and 3
# http://mikewatkins.ca/2008/11/29/python-2-and-3-metaclasses/
_injector = _MapnikMetaclass('_injector', (object, ), {})

def Filter(*args,**kwargs):
    warnings.warn("'Filter' is deprecated and will be removed in Mapnik 2.0.1, use 'Expression' instead",
    DeprecationWarning, 2)
    return Expression(*args, **kwargs)

class Envelope(Box2d):
    def __init__(self, *args, **kwargs):
        warnings.warn("'Envelope' is deprecated and will be removed in Mapnik 2.0.1, use 'Box2d' instead",
        DeprecationWarning, 2)
        Box2d.__init__(self, *args, **kwargs)

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

class _Box2d(Box2d,_injector):
    """
    Represents a spatial envelope (i.e. bounding box). 
    
    
    Following operators are defined for Box2d:

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
        return 'Box2d(%s,%s,%s,%s)' % \
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
        Projects the given object (Box2d or Coord) 
        from the geographic space into the cartesian space.

        See also:
          Box2d.forward(self, projection),
          Coord.forward(self, projection).
        """
        return forward_(obj,self)

    def inverse(self,obj):
        """
        Projects the given object (Box2d or Coord) 
        from the cartesian space into the geographic space.

        See also:
          Box2d.inverse(self, projection),
          Coord.inverse(self, projection).
        """
        return inverse_(obj,self)

class _Datasource(Datasource,_injector):

    def all_features(self,fields=None):
        query = Query(self.envelope())
        attributes = fields or self.fields()
        for fld in attributes:
            query.add_property_name(fld)
        return self.features(query).features

    def featureset(self,fields=None):
        query = Query(self.envelope())
        attributes = fields or self.fields()
        for fld in attributes:
            query.add_property_name(fld)
        return self.features(query)

class _DeprecatedFeatureProperties(object):

    def __init__(self, feature):
        self._feature = feature

    def __getitem__(self, name):
        warnings.warn("indexing feature.properties is deprecated, index the "
             "feature object itself for the same effect", DeprecationWarning, 2)
        return self._feature[name]

    def __iter__(self):
        warnings.warn("iterating feature.properties is deprecated, iterate the "
             "feature object itself for the same effect", DeprecationWarning, 2)
        return iter(self._feature)

class _Feature(Feature, _injector):
    """
    A Feature.

    TODO: docs
    """
    @property
    def properties(self):
        return _DeprecatedFeatureProperties(self)

    @property
    def attributes(self):
        #XXX Returns a copy! changes to it won't affect feat.'s attrs.
        #    maybe deprecate?
        return dict(self)
    
    def __init__(self, id, wkt=None, **properties):
        Feature._c___init__(self, id)
        if wkt is not None:
            self.add_geometries_from_wkt(wkt)
        for k, v in properties.iteritems():
            self[k] = v
    
class _Color(Color,_injector):
    def __repr__(self):
        return "Color(R=%d,G=%d,B=%d,A=%d)" % (self.r,self.g,self.b,self.a)

class _Symbolizers(Symbolizers,_injector):

    def __getitem__(self, idx):
        sym = Symbolizers._c___getitem__(self, idx)
        return sym.symbol()

def _add_symbol_method_to_symbolizers(vars=globals()):

    def symbol_for_subcls(self):
        return self

    def symbol_for_cls(self):
        return getattr(self,self.type())()

    for name, obj in vars.items():
        if name.endswith('Symbolizer') and not name.startswith('_'):
            if name == 'Symbolizer':
                symbol = symbol_for_cls
            else:
                symbol = symbol_for_subcls
            type('dummy', (obj,_injector), {'symbol': symbol})
_add_symbol_method_to_symbolizers()

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

    Hint: lox,loy,hix,hiy make a Mapnik Box2d

    Optional keyword arguments:
      base -- path prefix (default None)
      multi -- whether the image is in tiles on disk (default False)

    Multi-tiled keyword arguments:
      x_width -- virtual image number of tiles in X direction (required)
      y_width -- virtual image number of tiles in Y direction (required)
      tile_size -- if an image is in tiles, how large are the tiles (default 256)
      tile_stride -- if an image is in tiles, what's the increment between rows/cols (default 1)

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
      bbox -- tuple (minx, miny, maxx, maxy). If specified, overrides the bbox detected by GDAL.

    >>> from mapnik import Gdal, Layer
    >>> dataset = Gdal(base='/home/mapnik/data',file='elevation.tif')
    >>> lyr = Layer('GDAL Layer from TIFF file')
    >>> lyr.datasource = dataset

    """
    keywords['type'] = 'gdal'
    if 'bbox' in keywords:
        if isinstance(keywords['bbox'], (tuple, list)):
            keywords['bbox'] = ','.join([str(item) for item in keywords['bbox']])
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
      layer -- name of layer to use within datasource (optional if layer_by_index or layer_by_sql is used)

    Optional keyword arguments:
      layer_by_index -- choose layer by index number instead of by layer name or sql.
      layer_by_sql -- choose layer by sql query number instead of by layer name or index.
      base -- path prefix (default None)
      encoding -- file encoding (default 'utf-8')

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
      use_spatial_index -- boolean, instruct sqlite plugin to use Rtree spatial index (default True)

    >>> from mapnik import SQLite, Layer
    >>> sqlite = SQLite(base='/home/mapnik/data',file='osm.db',table='osm',extent='-20037508,-19929239,20037508,19929239') 
    >>> lyr = Layer('SQLite Layer')
    >>> lyr.datasource = sqlite

    """
    keywords['type'] = 'sqlite'
    return CreateDatasource(keywords)

def Rasterlite(**keywords):
    """Create a Rasterlite Datasource.

    Required keyword arguments:
      file -- path to Rasterlite database file
      table -- table name or subselect query

    Optional keyword arguments:
      base -- path prefix (default None)
      extent -- manually specified data extent (comma delimited string, default None)

    >>> from mapnik import Rasterlite, Layer
    >>> rasterlite = Rasterlite(base='/home/mapnik/data',file='osm.db',table='osm',extent='-20037508,-19929239,20037508,19929239') 
    >>> lyr = Layer('Rasterlite Layer')
    >>> lyr.datasource = rasterlite

    """
    keywords['type'] = 'rasterlite'
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

def Geos(**keywords):
    """Create a GEOS Vector Datasource.

    Required keyword arguments:
      wkt -- inline WKT text of the geometry

    Optional keyword arguments:
      extent -- manually specified data extent (comma delimited string, default None)

    >>> from mapnik import Geos, Layer
    >>> datasource = Geos(wkt='MULTIPOINT(100 100, 50 50, 0 0)') 
    >>> lyr = Layer('GEOS Layer from WKT string')
    >>> lyr.datasource = datasource

    """
    keywords['type'] = 'geos'
    return CreateDatasource(keywords)

def mapnik_version_string(version=mapnik_version()):
    """Return the Mapnik version as a string."""
    patch_level = version % 100
    minor_version = version / 100 % 1000
    major_version = version / 100000
    return '%s.%s.%s' % ( major_version, minor_version,patch_level)

def mapnik_version_from_string(version_string):
    """Return the Mapnik version from a string."""
    n = version_string.split('.')
    return (int(n[0]) * 100000) + (int(n[1]) * 100) + (int(n[2]));

def register_plugins(path=inputpluginspath):
    """Register plugins located by specified path"""
    DatasourceCache.instance().register_datasources(path)

def register_fonts(path=fontscollectionpath,valid_extensions=['.ttf','.otf','.ttc','.pfa','.pfb','.ttc','.dfont']):
    """Recursively register fonts using path argument as base directory"""
    for dirpath, _, filenames in os.walk(path):
        for filename in filenames:
            if os.path.splitext(filename.lower())[1] in valid_extensions:
                FontEngine.instance().register_font(os.path.join(dirpath, filename))

# auto-register known plugins and fonts
register_plugins()
register_fonts()

# Explicitly export API members to avoid namespace pollution
# and ensure correct documentation processing
__all__ = [
    # classes
    'Color',
    'Coord',
    'Palette',
    #'ColorBand',
    'CompositeOp',
    'DatasourceCache',
    'MemoryDatasource',
    'Box2d',
    'Feature',
    'Featureset',
    'FontEngine',
    'FontSet',
    'Geometry2d',
    'GlyphSymbolizer',
    'Image',
    'ImageView',
    'Grid',
    'GridView',
    'Layer',
    'Layers',
    'LinePatternSymbolizer',
    'LineSymbolizer',
    'Map',
    'MarkersSymbolizer',
    'Names',
    'Path',
    'Parameter',
    'Parameters',
    'PointDatasource',
    'PointSymbolizer',
    'PolygonPatternSymbolizer',
    'PolygonSymbolizer',
    'ProjTransform',
    'Projection',
    'Query',
    'RasterSymbolizer',
    'RasterColorizer',
    'Rule', 'Rules',
    'ShieldSymbolizer',
    'Singleton',
    'Stroke',
    'Style',
    'Symbolizer',
    'Symbolizers',
    'TextSymbolizer',
    'ViewTransform',
    # enums
    'aspect_fix_mode',
    'point_placement',
    'label_placement',
    'line_cap',
    'line_join',
    'text_transform',
    'vertical_alignment',
    'horizontal_alignment',
    'justify_alignment',
    'pattern_alignment',
    'filter_mode',
    # functions
    # datasources
    'Datasource',
    'CreateDatasource',
    'Shapefile',
    'PostGIS',
    'Raster',
    'Gdal',
    'Occi',
    'Ogr',
    'SQLite',
    'Osm',
    'Kismet',
    #   version and environment
    'mapnik_version_string',
    'mapnik_version',
    'has_cairo',
    'has_pycairo',
    #   factory methods
    'Expression',
    'PathExpression',
    #   load/save/render
    'load_map',
    'load_map_from_string',
    'save_map',
    'save_map_to_string',
    'render',
    'render_grid',
    'render_tile_to_file',
    'render_to_file',
    #   other
    'register_plugins',
    'register_fonts',
    'scale_denominator',
    # deprecated
    'Filter',
    'Envelope',
    ]
