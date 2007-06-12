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
#include <mapnik/envelope.hpp>

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
      shape_datasource(const parameters &params);
      virtual ~shape_datasource();
    
      int type() const;
      static std::string name();
      featureset_ptr features(const query& q) const;
      featureset_ptr features_at_point(coord2d const& pt) const;
      Envelope<double> envelope() const;
      layer_descriptor get_descriptor() const;   
   private:
      shape_datasource(const shape_datasource&);
      shape_datasource& operator=(const shape_datasource&);
      void init(shape_io& shape);
   private:
      int type_;
      std::string shape_name_;
      long file_length_;
      Envelope<double> extent_;
      bool indexed_;
      layer_descriptor desc_;
      static const std::string name_;
};

#endif //SHAPE_HPP
