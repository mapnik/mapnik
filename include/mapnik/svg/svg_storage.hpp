/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_SVG_STORAGE_HPP
#define MAPNIK_SVG_STORAGE_HPP

// mapnik
#include <mapnik/box2d.hpp>

// boost
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

    void set_bounding_box(box2d<double> const& b)
    {
        bounding_box_ = b;
    }

    void set_bounding_box(double x0, double y0, double x1, double y1)
    {
        bounding_box_.init(x0,y0,x1,y1);
    }

    box2d<double> const& bounding_box() const
    {
        return bounding_box_;
    }

private:

    VertexSource source_;
    AttributeSource attributes_;
    box2d<double> bounding_box_;

};

}}


#endif // MAPNIK_SVG_STORAGE_HPP
