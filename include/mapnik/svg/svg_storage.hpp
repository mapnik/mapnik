/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

// $Id$

#ifndef MAPNIK_SVG_STORAGE_HPP
#define MAPNIK_SVG_STORAGE_HPP

#include <boost/utility.hpp>

namespace mapnik {
namespace svg {

template <typename VertexSource ,typename AttributeSource>
class svg_storage :  boost::noncopyable
{
public:
    svg_storage() {}
    
    VertexSource & source() // FIXME!! make const
    {
        return source_;
    }
    
    AttributeSource & attributes() // FIXME!! make const
    {
        return attributes_;
    }
private:
    VertexSource source_;
    AttributeSource attributes_;

};

}}


#endif // MAPNIK_SVG_STORAGE_HPP
