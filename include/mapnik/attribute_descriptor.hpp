/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_ATTRIBUTE_DESCRIPTOR_HPP
#define MAPNIK_ATTRIBUTE_DESCRIPTOR_HPP

#include <string>

namespace mapnik {

enum eAttributeType { Integer = 1, Float = 2, Double = 3, String = 4, Boolean = 5, Geometry = 6, Object = 7 };

class attribute_descriptor
{
  public:
    attribute_descriptor(std::string const& name,
                         unsigned type,
                         bool primary_key = false,
                         int size = -1,
                         int precision = -1)
        : name_(name),
          type_(type),
          size_(size),
          precision_(precision),
          primary_key_(primary_key)
    {}

    attribute_descriptor(attribute_descriptor const& other)
        : name_(other.name_),
          type_(other.type_),
          size_(other.size_),
          precision_(other.precision_),
          primary_key_(other.primary_key_)
    {}

    attribute_descriptor& operator=(attribute_descriptor rhs)
    {
        using std::swap;
        std::swap(name_, rhs.name_);
        std::swap(type_, rhs.type_);
        std::swap(size_, rhs.size_);
        std::swap(precision_, rhs.precision_);
        std::swap(primary_key_, rhs.primary_key_);
        return *this;
    }

    std::string const& get_name() const { return name_; }

    unsigned int get_type() const { return type_; }

    bool is_primary_key() const { return primary_key_; }

    int get_size() const { return size_; }

    int get_precision() const { return precision_; }

  private:
    std::string name_;
    unsigned int type_;
    int size_;
    int precision_;
    bool primary_key_;
};

} // namespace mapnik

#endif // MAPNIK_ATTRIBUTE_DESCRIPTOR_HPP
