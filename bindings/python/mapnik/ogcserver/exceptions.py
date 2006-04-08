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

from copy import deepcopy

class OGCException(Exception):
    pass

class ServerConfigurationError(Exception):
    pass

class BaseExceptionHandler:
    
    def getexcetree(self, exc):
        ogcexcetree = deepcopy(self.xmltemplate)
        e = ogcexcetree.find(self.xpath)
        if len(exc.args) > 0:
            e.text = exc.args[0]
            if len(exc.args) > 1:
                e.set('code', exc.args[1])
        return ogcexcetree