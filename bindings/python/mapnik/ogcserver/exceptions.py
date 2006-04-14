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
from lxml import etree as ElementTree
from StringIO import StringIO
from traceback import print_tb
from sys import exc_info

class OGCException(Exception):
    pass

class ServerConfigurationError(Exception):
    pass

class BaseExceptionHandler:
    
    def __init__(self, debug):
        self.debug = debug
    
    def getcontent(self):
        excinfo = exc_info()
        ogcexcetree = deepcopy(self.xmltemplate)
        e = ogcexcetree.find(self.xpath)
        if self.debug:
            fh = StringIO()
            print_tb(excinfo[2], None, fh)
            fh.seek(0)
            e.text = '\n' + fh.read() + '\n' + str(excinfo[0]) + ': ' + ', '.join(excinfo[1].args) + '\n'
        elif len(excinfo[1].args) > 0:
            e.text = excinfo[1].args[0]
        if isinstance(excinfo[1], OGCException) and len(excinfo[1].args) > 1:
            e.set('code', excinfo[1].args[1])
        return ElementTree.tostring(ogcexcetree)