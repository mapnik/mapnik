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
#
#
#

from sys import getdlopenflags,setdlopenflags
from dl import RTLD_NOW,RTLD_GLOBAL
flags = getdlopenflags()
setdlopenflags(RTLD_NOW | RTLD_GLOBAL)

from _mapnik import *
from paths import inputpluginspath

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

class _Envelope(Envelope,_injector):
    def __repr__(self):
        return 'Envelope(%s,%s,%s,%s)' % \
               (self.minx,self.miny,self.maxx,self.maxy)

#register datasources
from mapnik import DatasourceCache
DatasourceCache.instance().register_datasources('%s' % inputpluginspath)
#set dlopen flags back to the original
setdlopenflags(flags)

