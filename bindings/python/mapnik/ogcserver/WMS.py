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
from mapnik import Style, Layer
import re

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
        self.aggregatestyles = {}

    def register_layer(self, layer, defaultstyle, extrastyles=()):
        layername = layer.name
        if not layername:
            raise ServerConfigurationError('Attempted to register an unnamed layer.')
        if not re.match('^\+init=epsg:\d+$', layer.srs):
            raise ServerConfigurationError('Attempted to register a layer without an epsg projection defined.')
        if defaultstyle not in self.styles.keys() + self.aggregatestyles.keys():
            raise ServerConfigurationError('Attempted to register a layer with an non-existent default style.')
        layer.wmsdefaultstyle = defaultstyle
        if isinstance(extrastyles, tuple):
            for stylename in extrastyles:
                if type(stylename) == type(''):
                    if stylename not in self.styles.keys() + self.aggregatestyles.keys():
                        raise ServerConfigurationError('Attempted to register a layer with an non-existent extra style.')
                else:
                    ServerConfigurationError('Attempted to register a layer with an invalid extra style name.')
            layer.wmsextrastyles = extrastyles
        else:
            raise ServerConfigurationError('Layer "%s" was passed an invalid list of extra styles.  List must be a tuple of strings.' % layername)
        self.layers[layername] = layer

    def register_style(self, name, style):
        if not name:
            raise ServerConfigurationError('Attempted to register a style without providing a name.')
        if name in self.aggregatestyles.keys() or name in self.styles.keys():
            raise ServerConfigurationError('Attempted to register an aggregate style with a name already in use.')
        if not isinstance(style, Style):
            raise ServerConfigurationError('Bad style object passed to register_style() for style "%s".' % name)
        self.styles[name] = style

    def register_aggregate_style(self, name, stylenames):
        if not name:
            raise ServerConfigurationError('Attempted to register an aggregate style without providing a name.')
        if name in self.aggregatestyles.keys() or name in self.styles.keys():
            raise ServerConfigurationError('Attempted to register an aggregate style with a name already in use.')
        self.aggregatestyles[name] = []
        for stylename in stylenames:
            if stylename not in self.styles.keys():
                raise ServerConfigurationError('Attempted to register an aggregate style containing a style that does not exist.')
            self.aggregatestyles[name].append(stylename)

    def finalize(self):
        if len(self.layers) == 0:
            raise ServerConfigurationError('No layers defined!')
        if len(self.styles) == 0:
            raise ServerConfigurationError('No styles defined!')
        for layer in self.layers.values():
            for style in list(layer.styles) + list(layer.wmsextrastyles):
                if style not in self.styles.keys() + self.aggregatestyles.keys():
                    raise ServerConfigurationError('Layer "%s" refers to undefined style "%s".' % (layer.name(), style))
