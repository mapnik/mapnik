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
//$Id$

//mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/map.hpp>

namespace mapnik {

void symbolizer_base::add_metawriter(std::string const& name, metawriter_properties const& properties)
{
    writer_name_ = name;
    properties_ = properties;
}

void symbolizer_base::add_metawriter(metawriter_ptr writer_ptr, metawriter_properties const& properties,
                    std::string const& name)
{
    writer_ptr_ = writer_ptr;
    properties_ = properties;
    writer_name_ = name;
    if (writer_ptr) {
        properties_complete_ = writer_ptr->get_default_properties();
        properties_complete_.insert(properties_.begin(), properties_.end());
    } else {
        properties_complete_.clear();
    }
}

void symbolizer_base::cache_metawriters(Map const &m)
{
    if (writer_name_.empty()) {
        properties_complete_.clear();
        writer_ptr_ = metawriter_ptr();
        return; // No metawriter
    }

    writer_ptr_ = m.find_metawriter(writer_name_);
    if (writer_ptr_) {
        properties_complete_ = writer_ptr_->get_default_properties();
        properties_complete_.insert(properties_.begin(), properties_.end());
    } else {
        properties_complete_.clear();
        std::cerr << "WARNING: Metawriter '" << writer_name_ << "' used but not defined.\n";
    }
}

metawriter_with_properties symbolizer_base::get_metawriter() const
{
    return metawriter_with_properties(writer_ptr_, properties_complete_);
}

symbolizer_with_image::symbolizer_with_image(path_expression_ptr file)
    : image_filename_( file ),
      opacity_(1.0f)

{
    matrix_[0] = 1.0;
    matrix_[1] = 0.0;
    matrix_[2] = 0.0;
    matrix_[3] = 1.0;
    matrix_[4] = 0.0;
    matrix_[5] = 0.0;
}

symbolizer_with_image::symbolizer_with_image( symbolizer_with_image const& rhs)
    : image_filename_(rhs.image_filename_),
      opacity_(rhs.opacity_),
      matrix_(rhs.matrix_) {}
   
path_expression_ptr symbolizer_with_image::get_filename() const
{
    return image_filename_;
}

void symbolizer_with_image::set_filename(path_expression_ptr image_filename) 
{
    image_filename_ = image_filename;
}
      
void symbolizer_with_image::set_transform(transform_type const& matrix)
{
    matrix_ = matrix;
}

transform_type const& symbolizer_with_image::get_transform() const
{
    return matrix_;
}

std::string const symbolizer_with_image::get_transform_string() const
{
    std::stringstream ss;
    ss << "matrix(" << matrix_[0] << ", " << matrix_[1] << ", "
       << matrix_[2] << ", " << matrix_[3] << ", "
       << matrix_[4] << ", " << matrix_[5] << ")";
    return ss.str();
}


void symbolizer_with_image::set_opacity(float opacity)
{
    opacity_ = opacity;
}

float symbolizer_with_image::get_opacity() const
{
    return opacity_;
}

} // end of namespace mapnik



