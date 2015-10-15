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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/json/feature_grammar.hpp>
#include <mapnik/util/utf_conv_win.hpp>
// stl
#include <string>
#include <vector>

#include "geojson_index_featureset.hpp"

geojson_index_featureset::geojson_index_featureset(std::string const& filename)
:
#ifdef _WINDOWS
    file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose),
#else
    file_(std::fopen(filename.c_str(),"rb"), std::fclose),
#endif
    ctx_(std::make_shared<mapnik::context_type>())
{
    if (!file_) throw std::runtime_error("Can't open " + filename);
}

geojson_index_featureset::~geojson_index_featureset() {}

mapnik::feature_ptr geojson_index_featureset::next()
{
    return mapnik::feature_ptr();
}
