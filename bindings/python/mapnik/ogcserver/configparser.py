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

""" Change SafeConfigParser behavior to treat options without values as
    non-existent.
"""

from ConfigParser import SafeConfigParser as OrigSafeConfigParser

class SafeConfigParser(OrigSafeConfigParser):
    
    def items_with_value(self, section):
        finallist = []
        items = self.items(section)
        for item in items:
            if item[1] != '':
                finallist.append(item)
        return finallist
    
    def has_option_with_value(self, section, option):
        if self.has_option(section, option):
            if self.get(section, option) == '':
                return False
        else:
            return False
        return True