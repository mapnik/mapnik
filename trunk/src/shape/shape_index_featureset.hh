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

#ifndef SHAPE_SQT_FS_HH
#define SHAPE_SQT_FS_HH

#include "shape_featureset.hh"
#include <set>

template <typename filterT>
class ShapeIndexFeatureset : public Featureset
{
    int srid_;
    filterT filter_;
    int shape_type_;
    shape_io shape_;
    Envelope<double> query_ext_;
    std::set<int> ids_;
    std::set<int>::iterator itr_;

    mutable Envelope<double> feature_ext_;
    mutable int total_geom_size;
    mutable int count_;

public:
    ShapeIndexFeatureset(const filterT& filter,const std::string& shape_file,int srid);
    virtual ~ShapeIndexFeatureset();
    Feature* next();
private:
    //no copying
    ShapeIndexFeatureset(const ShapeIndexFeatureset&);
    ShapeIndexFeatureset& operator=(const ShapeIndexFeatureset&);
};
#endif //SHAPE_SQT_FS_HH
