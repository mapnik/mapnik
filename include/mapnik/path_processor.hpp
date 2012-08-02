/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#ifndef MAPNIK_PATH_PROCESSOR_HPP
#define MAPNIK_PATH_PROCESSOR_HPP

//mapnik
#include "pixel_position.hpp"

// agg
#include "agg_path_length.h"
namespace mapnik
{
template <typename T>
class path_processor
{
public:
    path_processor(T &path)
        : path_(path), length_(agg::path_length(path_)), valid_(true)
    {
        rewind();
    }

    double length() const { return length_ ;}

    bool next()
    {
        if (!valid_) return false;
        valid_ = path_.vertex(&current_point_.x, &current_point_.y);
        return valid_;
    }

    pixel_position const& current_point() const
    {
        return current_point_;
    }

    void rewind()
    {
        path_.rewind(0);
        valid_ = true;
    }

    bool valid() const { return valid_; }

    /** Skip a certain amount of space.
     *
     * This function automatically calculates new points if the position is not exactly
     * on a point on the path.
     */
    bool skip(double length)
    {
        //TODO
        return valid_;
    }

private:
    T &path_;
    double length_;
    bool valid_;
    pixel_position current_point_;
};
}
#endif // PATH_PROCESSOR_HPP
