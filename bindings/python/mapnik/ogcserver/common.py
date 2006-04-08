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

from exceptions import OGCException, ServerConfigurationError
from mapnik import Color
import re

PIL_TYPE_MAPPING = {'image/jpeg': 'JPEG', 'image/png': 'PNG', 'image/gif': 'GIF'}

class ParameterDefinition:

    def __init__(self, mandatory, default=None, allowedvalues=None, fallback=False, match=None, cast=None):
        """ An OGC request parameter definition.  Used to describe a
            parameter's characteristics.

            @param mandatory: Is this parameter required by the request?
            @type mandatory: Boolean.
            
            @param default: Default value to use if one is not provided
                            and the parameter is optional.
            @type default: None or any valid value.
            
            @param allowedvalues: A list of allowed values for the parameter.
                                  If a value is provided that is not in this
                                  list, an error is raised.
            @type allowedvalues: A python tuple of values.
            
            @param fallback: Whether the value of the parameter should fall
                             back to the default should an illegal value be
                             provided.
            @type fallback: Boolean.
            
            @return: A L{ParameterDefinition} instance.
        """
        if match and cast:
            raise ServerConfigurationError("'cast' and 'match' are mutually exclusive.")
        if not match and not cast:
            raise ServerConfigurationError("One of 'cast' or 'match' MUST be used.")
        if match:
            try:
                self.pattern = re.compile(match)
            except:
                raise ServerConfigurationError("Invalid regular expression.")
        else:
            self.pattern = None
        if cast and not callable(cast):
            raise ServerConfigurationError('Cast parameter definition must be callable.')
        self.cast = cast
        if mandatory not in [True, False]:
            raise ServerConfigurationError("Bad value for 'mandatory' parameter, must be True or False.")
        self.mandatory = mandatory
        self.default = default
        if allowedvalues and type(allowedvalues) != type(()):
            raise ServerConfigurationError("Bad value for 'allowedvalues' parameter, must be a tuple.")
        self.allowedvalues = allowedvalues
        if fallback not in [True, False]:
            raise ServerConfigurationError("Bad value for 'fallback' parameter, must be True or False.")
        self.fallback = fallback

class BaseServiceHandler:

    def processParameters(self, requestname, params):
        finalparams = {}
        for paramname, paramdef in self.SERVICE_PARAMS[requestname].items():
            if paramname not in params.keys() and paramdef.mandatory:
                raise OGCException("Mandatory parameter '%s' missing from request." % paramname)
            elif paramname in params.keys():
                if paramdef.pattern:
                    if not paramdef.pattern.match(params[paramname]):
                        raise OGCException("Parameter '%s' has an illegal value." % paramname)
                elif paramdef.cast:
                    try:
                        params[paramname] = paramdef.cast(params[paramname])
                    except:
                        raise OGCException("Parameter '%s' has an illegal value." % paramname)
                if paramdef.allowedvalues and params[paramname] not in paramdef.allowedvalues:
                    if not paramdef.fallback:
                        raise OGCException("Parameter '%s' has an illegal value." % paramname)
                    else:
                        finalparams[paramname] = paramdef.default
                else:
                    finalparams[paramname] = params[paramname]
            elif not paramdef.mandatory and paramdef.default:
                finalparams[paramname] = paramdef.default
        return finalparams

class Response:

    def __init__(self, content_type, content):
        self.content_type = content_type
        self.content = content

class Version:

    def __init__(self, version):
        version = version.split('.')
        if len(version) != 3:
            raise OGCException('Badly formatted version number.')
        try:
            version = map(int, version)
        except:
            raise OGCException('Badly formatted version number.')
        self.version = version

    def __repr__(self):
        return '%s.%s.%s' % (self.version[0], self.version[1], self.version[2])

    def __cmp__(self, other):
        if isinstance(other, str):
            other = Version(other)
        if not isinstance(other, Version):
            raise TypeError('Version instances can only be compared to each other, or version strings.')
        if self.version[0] < other.version[0]:
            return -1
        elif self.version[0] > other.version[0]:
            return 1
        else:
            if self.version[1] < other.version[1]:
                return -1
            elif self.version[1] > other.version[1]:
                return 1
            else:
                if self.version[2] < other.version[2]:
                    return -1
                elif self.version[2] > other.version[2]:
                    return 1
                else:
                    return 0

class ListFactory:
    
    def __init__(self, cast):
        self.cast = cast
    
    def __call__(self, string):
        seq = string.split(',')
        return map(self.cast, seq)

def ColorFactory(colorstring):
    if re.match('^0x[a-fA-F0-9]{6}$', colorstring):
        return Color(eval('0x' + colorstring[2:4]), eval('0x' + colorstring[4:6]), eval('0x' + colorstring[6:8]))
    else:
        raise OGCException("Invalid color value.")