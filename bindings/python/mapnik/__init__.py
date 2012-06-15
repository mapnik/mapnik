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
    warnings.warn("'Filter' is deprecated and will be removed in Mapnik 3.x, use 'Expression' instead",
    DeprecationWarning, 2)
    return Expression(*args, **kwargs)

class Envelope(Box2d):
    def __init__(self, *args, **kwargs):
        warnings.warn("'Envelope' is deprecated and will be removed in Mapnik 3.x, use 'Box2d' instead",
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

class _Color(Color,_injector):
    def __repr__(self):
        return "Color(R=%d,G=%d,B=%d,A=%d)" % (self.r,self.g,self.b,self.a)

class _ProcessedText(ProcessedText, _injector):
    def append(self, properties, text):
        #More pythonic name
        self.push_back(properties, text)

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

class _TextSymbolizer(TextSymbolizer,_injector):
    @property
    def text_size(self):
        warnings.warn("'text_size' is deprecated, use format.text_size",
        DeprecationWarning, 2)
        return self.format.text_size

    @text_size.setter
    def text_size(self, text_size):
        warnings.warn("'text_size' is deprecated, use format.text_size",
        DeprecationWarning, 2)
        self.format.text_size = text_size

    @property
    def face_name(self):
        warnings.warn("'face_name' is deprecated, use format.face_name",
        DeprecationWarning, 2)
        return self.format.face_name

    @face_name.setter
    def face_name(self, face_name):
        warnings.warn("'face_name' is deprecated, use format.face_name",
        DeprecationWarning, 2)
        self.format.face_name = face_name



    @property
    def fontset(self):
        warnings.warn("'fontset' is deprecated, use format.fontset",
        DeprecationWarning, 2)
        return self.format.fontset

    @fontset.setter
    def fontset(self, fontset):
        warnings.warn("'fontset' is deprecated, use format.fontset",
        DeprecationWarning, 2)
        self.format.fontset = fontset



    @property
    def character_spacing(self):
        warnings.warn("'character_spacing' is deprecated, use format.character_spacing",
        DeprecationWarning, 2)
        return self.format.character_spacing

    @character_spacing.setter
    def character_spacing(self, character_spacing):
        warnings.warn("'character_spacing' is deprecated, use format.character_spacing",
        DeprecationWarning, 2)
        self.format.character_spacing = character_spacing



    @property
    def line_spacing(self):
        warnings.warn("'line_spacing' is deprecated, use format.line_spacing",
        DeprecationWarning, 2)
        return self.format.line_spacing

    @line_spacing.setter
    def line_spacing(self, line_spacing):
        warnings.warn("'line_spacing' is deprecated, use format.line_spacing",
        DeprecationWarning, 2)
        self.format.line_spacing = line_spacing



    @property
    def text_opacity(self):
        warnings.warn("'text_opacity' is deprecated, use format.text_opacity",
        DeprecationWarning, 2)
        return self.format.text_opacity

    @text_opacity.setter
    def text_opacity(self, text_opacity):
        warnings.warn("'text_opacity' is deprecated, use format.text_opacity",
        DeprecationWarning, 2)
        self.format.text_opacity = text_opacity



    @property
    def wrap_char(self):
        warnings.warn("'wrap_char' is deprecated, use format.wrap_char",
        DeprecationWarning, 2)
        return self.format.wrap_char

    @wrap_char.setter
    def wrap_char(self, wrap_char):
        warnings.warn("'wrap_char' is deprecated, use format.wrap_char",
        DeprecationWarning, 2)
        self.format.wrap_char = wrap_char



    @property
    def wrap_before(self):
        warnings.warn("'wrap_before' is deprecated, use format.wrap_before",
        DeprecationWarning, 2)
        return self.format.wrap_before

    @wrap_before.setter
    def wrap_before(self, wrap_before):
        warnings.warn("'wrap_before' is deprecated, use format.wrap_before",
        DeprecationWarning, 2)
        self.format.wrap_before = wrap_before



    @property
    def text_transform(self):
        warnings.warn("'text_transform' is deprecated, use format.text_transform",
        DeprecationWarning, 2)
        return self.format.text_transform

    @text_transform.setter
    def text_transform(self, text_transform):
        warnings.warn("'text_transform' is deprecated, use format.text_transform",
        DeprecationWarning, 2)
        self.format.text_transform = text_transform



    @property
    def fill(self):
        warnings.warn("'fill' is deprecated, use format.fill",
        DeprecationWarning, 2)
        return self.format.fill

    @fill.setter
    def fill(self, fill):
        warnings.warn("'fill' is deprecated, use format.fill",
        DeprecationWarning, 2)
        self.format.fill = fill



    @property
    def halo_fill(self):
        warnings.warn("'halo_fill' is deprecated, use format.halo_fill",
        DeprecationWarning, 2)
        return self.format.halo_fill

    @halo_fill.setter
    def halo_fill(self, halo_fill):
        warnings.warn("'halo_fill' is deprecated, use format.halo_fill",
        DeprecationWarning, 2)
        self.format.halo_fill = halo_fill



    @property
    def halo_radius(self):
        warnings.warn("'halo_radius' is deprecated, use format.halo_radius",
        DeprecationWarning, 2)
        return self.format.halo_radius

    @halo_radius.setter
    def halo_radius(self, halo_radius):
        warnings.warn("'halo_radius' is deprecated, use format.halo_radius",
        DeprecationWarning, 2)
        self.format.halo_radius = halo_radius


    @property
    def label_placement(self):
        warnings.warn("'label_placement' is deprecated, use properties.label_placement",
        DeprecationWarning, 2)
        return self.properties.label_placement

    @label_placement.setter
    def label_placement(self, label_placement):
        warnings.warn("'label_placement' is deprecated, use properties.label_placement",
        DeprecationWarning, 2)
        self.properties.label_placement = label_placement



    @property
    def horizontal_alignment(self):
        warnings.warn("'horizontal_alignment' is deprecated, use properties.horizontal_alignment",
        DeprecationWarning, 2)
        return self.properties.horizontal_alignment

    @horizontal_alignment.setter
    def horizontal_alignment(self, horizontal_alignment):
        warnings.warn("'horizontal_alignment' is deprecated, use properties.horizontal_alignment",
        DeprecationWarning, 2)
        self.properties.horizontal_alignment = horizontal_alignment



    @property
    def justify_alignment(self):
        warnings.warn("'justify_alignment' is deprecated, use properties.justify_alignment",
        DeprecationWarning, 2)
        return self.properties.justify_alignment

    @justify_alignment.setter
    def justify_alignment(self, justify_alignment):
        warnings.warn("'justify_alignment' is deprecated, use properties.justify_alignment",
        DeprecationWarning, 2)
        self.properties.justify_alignment = justify_alignment



    @property
    def vertical_alignment(self):
        warnings.warn("'vertical_alignment' is deprecated, use properties.vertical_alignment",
        DeprecationWarning, 2)
        return self.properties.vertical_alignment

    @vertical_alignment.setter
    def vertical_alignment(self, vertical_alignment):
        warnings.warn("'vertical_alignment' is deprecated, use properties.vertical_alignment",
        DeprecationWarning, 2)
        self.properties.vertical_alignment = vertical_alignment



    @property
    def orientation(self):
        warnings.warn("'orientation' is deprecated, use properties.orientation",
        DeprecationWarning, 2)
        return self.properties.orientation

    @orientation.setter
    def orientation(self, orientation):
        warnings.warn("'orientation' is deprecated, use properties.orientation",
        DeprecationWarning, 2)
        self.properties.orientation = orientation



    @property
    def displacement(self):
        warnings.warn("'displacement' is deprecated, use properties.displacement",
        DeprecationWarning, 2)
        return self.properties.displacement

    @displacement.setter
    def displacement(self, displacement):
        warnings.warn("'displacement' is deprecated, use properties.displacement",
        DeprecationWarning, 2)
        self.properties.displacement = displacement



    @property
    def label_spacing(self):
        warnings.warn("'label_spacing' is deprecated, use properties.label_spacing",
        DeprecationWarning, 2)
        return self.properties.label_spacing

    @label_spacing.setter
    def label_spacing(self, label_spacing):
        warnings.warn("'label_spacing' is deprecated, use properties.label_spacing",
        DeprecationWarning, 2)
        self.properties.label_spacing = label_spacing



    @property
    def label_position_tolerance(self):
        warnings.warn("'label_position_tolerance' is deprecated, use properties.label_position_tolerance",
        DeprecationWarning, 2)
        return self.properties.label_position_tolerance

    @label_position_tolerance.setter
    def label_position_tolerance(self, label_position_tolerance):
        warnings.warn("'label_position_tolerance' is deprecated, use properties.label_position_tolerance",
        DeprecationWarning, 2)
        self.properties.label_position_tolerance = label_position_tolerance



    @property
    def avoid_edges(self):
        warnings.warn("'avoid_edges' is deprecated, use properties.avoid_edges",
        DeprecationWarning, 2)
        return self.properties.avoid_edges

    @avoid_edges.setter
    def avoid_edges(self, avoid_edges):
        warnings.warn("'avoid_edges' is deprecated, use properties.avoid_edges",
        DeprecationWarning, 2)
        self.properties.avoid_edges = avoid_edges



    @property
    def minimum_distance(self):
        warnings.warn("'minimum_distance' is deprecated, use properties.minimum_distance",
        DeprecationWarning, 2)
        return self.properties.minimum_distance

    @minimum_distance.setter
    def minimum_distance(self, minimum_distance):
        warnings.warn("'minimum_distance' is deprecated, use properties.minimum_distance",
        DeprecationWarning, 2)
        self.properties.minimum_distance = minimum_distance



    @property
    def minimum_padding(self):
        warnings.warn("'minimum_padding' is deprecated, use properties.minimum_padding",
        DeprecationWarning, 2)
        return self.properties.minimum_padding

    @minimum_padding.setter
    def minimum_padding(self, minimum_padding):
        warnings.warn("'minimum_padding' is deprecated, use properties.minimum_padding",
        DeprecationWarning, 2)
        self.properties.minimum_padding = minimum_padding



    @property
    def minimum_path_length(self):
        warnings.warn("'minimum_path_length' is deprecated, use properties.minimum_path_length",
        DeprecationWarning, 2)
        return self.properties.minimum_path_length

    @minimum_path_length.setter
    def minimum_path_length(self, minimum_path_length):
        warnings.warn("'minimum_path_length' is deprecated, use properties.minimum_path_length",
        DeprecationWarning, 2)
        self.properties.minimum_path_length = minimum_path_length



    @property
    def maximum_angle_char_delta(self):
        warnings.warn("'maximum_angle_char_delta' is deprecated, use properties.maximum_angle_char_delta",
        DeprecationWarning, 2)
        return self.properties.maximum_angle_char_delta

    @maximum_angle_char_delta.setter
    def maximum_angle_char_delta(self, maximum_angle_char_delta):
        warnings.warn("'maximum_angle_char_delta' is deprecated, use properties.maximum_angle_char_delta",
        DeprecationWarning, 2)
        self.properties.maximum_angle_char_delta = maximum_angle_char_delta



    @property
    def force_odd_labels(self):
        warnings.warn("'force_odd_labels' is deprecated, use properties.force_odd_labels",
        DeprecationWarning, 2)
        return self.properties.force_odd_labels

    @force_odd_labels.setter
    def force_odd_labels(self, force_odd_labels):
        warnings.warn("'force_odd_labels' is deprecated, use properties.force_odd_labels",
        DeprecationWarning, 2)
        self.properties.force_odd_labels = force_odd_labels



    @property
    def allow_overlap(self):
        warnings.warn("'allow_overlap' is deprecated, use properties.allow_overlap",
        DeprecationWarning, 2)
        return self.properties.allow_overlap

    @allow_overlap.setter
    def allow_overlap(self, allow_overlap):
        warnings.warn("'allow_overlap' is deprecated, use properties.allow_overlap",
        DeprecationWarning, 2)
        self.properties.allow_overlap = allow_overlap



    @property
    def text_ratio(self):
        warnings.warn("'text_ratio' is deprecated, use properties.text_ratio",
        DeprecationWarning, 2)
        return self.properties.text_ratio

    @text_ratio.setter
    def text_ratio(self, text_ratio):
        warnings.warn("'text_ratio' is deprecated, use properties.text_ratio",
        DeprecationWarning, 2)
        self.properties.text_ratio = text_ratio



    @property
    def wrap_width(self):
        warnings.warn("'wrap_width' is deprecated, use properties.wrap_width",
        DeprecationWarning, 2)
        return self.properties.wrap_width

    @wrap_width.setter
    def wrap_width(self, wrap_width):
        warnings.warn("'wrap_width' is deprecated, use properties.wrap_width",
        DeprecationWarning, 2)
        self.properties.wrap_width = wrap_width





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
