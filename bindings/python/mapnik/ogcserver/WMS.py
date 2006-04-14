#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2006 Jean-Francois Doyon
#
# Mapnik is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# $Id$

from common import Version
from exceptions import OGCException, ServerConfigurationError
from wms111 import ServiceHandler as ServiceHandler111
from wms130 import ServiceHandler as ServiceHandler130

def ServiceHandlerFactory(conf, mapfactory, onlineresource, version):

    if not version:
        version = Version('1.3.0')
    else:
        version = Version(version)
    if version >= '1.3.0':
        return ServiceHandler130(conf, mapfactory, onlineresource)
    else:
        return ServiceHandler111(conf, mapfactory, onlineresource)

class BaseWMSFactory:
    
    def __init__(self):
        self.layers = {}
        self.styles = {}
    
    def register_layer(self, layer):
        layername = layer.name()
        if not layername:
            raise ServerConfigurationError('There is an un-named layer.')
        self.layers[layername] = layer
    
    def register_style(self, name, style):
        if not name:
            raise ServerConfigurationError('There is an un-named style.')
        self.styles[name] = style