/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: attribute_container.cc 58 2004-10-31 16:21:26Z artem $

#include "attribute_container.hh"

namespace mapnik
{

    attribute_container::attribute_container() {}

    attribute_container::~attribute_container()
    {
        std::map<std::string,attribute_base*>::iterator itr=attr_.begin();
        for (;itr!=attr_.end();++itr)
        {
            delete itr->second;
        }
    }

    void attribute_container::add(const std::string& name,const attribute_base& attr)
    {
        attr_.insert(make_pair(name,attr.clone()));
    }

    const attribute_base& attribute_container::get(const std::string& name) const
    {
        std::map<std::string,attribute_base*>::const_iterator itr=attr_.find(name);
        if (itr!=attr_.end())
            return *(itr->second);
        return invalid_;
    }
    const invalid_attribute attribute_container::invalid_;
}
