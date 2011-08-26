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

// $Id$

#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <mapnik/datasource.hpp>
#include <mapnik/box2d.hpp>

#include <boost/shared_ptr.hpp>

#include "shape_io.hpp"

using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::coord2d;

class shape_datasource : public datasource
{
public:
    shape_datasource(const parameters &params, bool bind=true);
    virtual ~shape_datasource();
    
    int type() const;
    static std::string name();
    featureset_ptr features(const query& q) const;
    featureset_ptr features_at_point(coord2d const& pt) const;
    box2d<double> envelope() const;
    layer_descriptor get_descriptor() const;
    void bind() const;   
private:
    shape_datasource(const shape_datasource&);
    shape_datasource& operator=(const shape_datasource&);
    void init(shape_io& shape) const;
private:
    int type_;
    std::string shape_name_;
    mutable boost::shared_ptr<shape_io> shape_;
    mutable long file_length_;
    mutable box2d<double> extent_;
    mutable bool indexed_;
    const int row_limit_;
    mutable layer_descriptor desc_;
};

#endif //SHAPE_HPP
