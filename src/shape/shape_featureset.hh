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

#ifndef SHAPE_FS_HH
#define SHAPE_FS_HH

#include "shape.hh"

using namespace mapnik;

template <typename filterT>
class ShapeFeatureset : public Featureset
{
    int srid_;
    filterT filter_;
    int shape_type_;
    shape_io shape_;
    Envelope<double> query_ext_;
    long file_length_;
    mutable Envelope<double> feature_ext_;
    mutable int total_geom_size;
    mutable int count_;
public:
    ShapeFeatureset(const filterT& filter, const std::string& shape_file,long file_length,int srid);
    virtual ~ShapeFeatureset();
    Feature* next();
private:
    ShapeFeatureset(const ShapeFeatureset&);
    const ShapeFeatureset& operator=(const ShapeFeatureset&);
};

#endif                                            //SHAPE_FS_HH
