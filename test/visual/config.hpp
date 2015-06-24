/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef CONFIG_HPP
#define CONFIG_HPP

// stl
#include <vector>
#include <string>
#include <exception>

// boost
#include <boost/filesystem.hpp>

namespace visual_tests
{

class early_exit_error : public std::exception
{
public:
    early_exit_error() :
        what_() {}

    early_exit_error( std::string const& what ) :
        what_( what )
    {
    }

    virtual ~early_exit_error() throw() {}

    virtual const char * what() const throw()
    {
        return what_.c_str();
    }

protected:
    mutable std::string what_;
};

struct map_size
{
    map_size(int _width, int _height) : width(_width), height(_height) { }
    map_size() { }
    unsigned width = 0;
    unsigned height = 0;
};

struct config
{
    config() : status(true),
               scales({ 1.0, 2.0 }),
               sizes({ { 500, 100 } }) { }

    bool status;
    std::vector<double> scales;
    std::vector<map_size> sizes;
};

enum result_state : std::uint8_t
{
    STATE_OK,
    STATE_FAIL,
    STATE_ERROR,
    STATE_OVERWRITE
};

struct result
{
    std::string name;
    result_state state;
    std::string renderer_name;
    map_size size;
    double scale_factor;
    boost::filesystem::path actual_image_path;
    boost::filesystem::path reference_image_path;
    std::string error_message;
    unsigned diff;
};

using result_list = std::vector<result>;

}

#endif
