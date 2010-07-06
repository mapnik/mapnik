/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Hermann Kraus
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


#ifndef METAWRITER_HPP
#define METAWRITER_HPP

// Mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/filter_factory.hpp>

// Boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>


namespace mapnik {

/** Abstract baseclass for all metawriter classes. */
class metawriter
{
    public:
        virtual ~metawriter() {};
        virtual void add_box(box2d<double> box, Feature const &feature, proj_transform const& prj_trans, CoordTransform const &t, expression_ptr expression)=0;
};

typedef boost::shared_ptr<metawriter> metawriter_ptr;

};

#endif
