#
# This file is part of Mapnik (C++/Python mapping toolkit)
# Copyright (C) 2005 Artem Pavlenko
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
  def forward(self,obj):
    return forward_(obj,self)
  def inverse(self,obj):
    return inverse_(obj,self)
  
class _Datasource(Datasource,_injector):
  def describe(self):
    return Describe(self)

def Datasource (**keywords):
  return CreateDatasource(keywords)

# convinience factory methods

def Shapefile(**keywords):
  keywords['type'] = 'shape'
  return CreateDatasource(keywords)

def PostGIS(**keywords):
  keywords['type'] = 'postgis'
  return CreateDatasource(keywords)

def Raster(**keywords):
  keywords['type'] = 'raster'
  return CreateDatasource(keywords)

def Gdal(**keywords):
  keywords['type'] = 'gdal'
  return CreateDatasource(keywords)

def Ogr(**keywords):
  keywords['type'] = 'ogr'
  return CreateDatasource(keywords)

def SQLite(**keywords):
  keywords['type'] = 'sqlite'
  return CreateDatasource(keywords)

def mapnik_version_string():
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

