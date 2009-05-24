/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

#ifndef PROJ_TRANSFORM_HPP
#define PROJ_TRANSFORM_HPP

// mapnik
#include <mapnik/projection.hpp>
// boost
#include <boost/utility.hpp>

namespace mapnik {
    
    class MAPNIK_DECL proj_transform : private boost::noncopyable
    {
    public:
        proj_transform(projection const& source, 
                       projection const& dest);
        
        bool forward (double& x, double& y , double& z) const;
        bool backward (double& x, double& y , double& z) const;
        mapnik::projection const& source() const;
        mapnik::projection const& dest() const;
        
    private:
        projection const& source_;
        projection const& dest_;
        bool is_source_latlong_;
        bool is_dest_latlong_;
        bool is_source_equal_dest;
    };
}

#endif // PROJ_TRANSFORM_HPP
