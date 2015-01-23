/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_FEATURE_LAYER_DESC_HPP
#define MAPNIK_FEATURE_LAYER_DESC_HPP

// mapnik
#include <mapnik/attribute_descriptor.hpp>
#include <mapnik/params.hpp>

// stl
#include <iosfwd>
#include <vector>

namespace mapnik
{

class layer_descriptor
{
public:
    layer_descriptor(std::string const& name, std::string const& encoding)
        : name_(name),
          encoding_(encoding),
          desc_ar_(),
          extra_params_() {}

    layer_descriptor(layer_descriptor const& other)
        : name_(other.name_),
          encoding_(other.encoding_),
          desc_ar_(other.desc_ar_),
          extra_params_(other.extra_params_) {}

    void set_name(std::string const& name)
    {
        name_ = name;
    }

    std::string const& get_name() const
    {
        return name_;
    }

    void set_encoding(std::string const& encoding)
    {
        encoding_ = encoding;
    }

    std::string const& get_encoding() const
    {
        return encoding_;
    }

    void add_descriptor(attribute_descriptor const& desc)
    {
        desc_ar_.push_back(desc);
    }

    std::vector<attribute_descriptor> const& get_descriptors() const
    {
        return desc_ar_;
    }

    std::vector<attribute_descriptor>& get_descriptors()
    {
        return desc_ar_;
    }

    parameters const& get_extra_parameters() const
    {
        return extra_params_;
    }

    parameters& get_extra_parameters()
    {
        return extra_params_;
    }

private:
    std::string name_;
    std::string encoding_;
    std::vector<attribute_descriptor> desc_ar_;
    parameters extra_params_;
};

}

#endif // MAPNIK_FEATURE_LAYER_DESC_HPP
