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

#include "shape_featureset.hpp"
#include <set>
#include <vector>

template <typename filterT>
class shape_index_featureset : public Featureset
{
    filterT filter_;
    int shape_type_;
    shape_io shape_;
    std::set<int> ids_;
    std::set<int>::iterator itr_;
    std::vector<int> attr_ids_;
    mutable Envelope<double> feature_ext_;
    mutable int total_geom_size;
    mutable int count_;

public:
    shape_index_featureset(const filterT& filter,const std::string& shape_file,
			   const std::set<std::string>& attribute_names);
    virtual ~shape_index_featureset();
    feature_ptr next();
private:
    //no copying
    shape_index_featureset(const shape_index_featureset&);
    shape_index_featureset& operator=(const shape_index_featureset&);
};
#endif //SHAPE_SQT_FS_HH
