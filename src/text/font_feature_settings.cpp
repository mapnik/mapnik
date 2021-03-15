/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/text/font_feature_settings.hpp>
#include <mapnik/config_error.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/version.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>
#include <cctype>
#include <sstream>
#include <functional>

namespace mapnik
{

font_feature_settings::font_feature_settings(std::string const& features)
    : features_()
{
    from_string(features);
}

font_feature_settings::font_feature_settings()
    : features_() {}

void font_feature_settings::from_string(std::string const& features)
{
    features_.clear();
    if (std::all_of(features.begin(), features.end(), isspace)) return;

    namespace x3 = boost::spirit::x3;
    auto appender = [&](auto const& ctx)
        {
            this->append(_attr(ctx));
        };
    if (!x3::parse(features.begin(), features.end(), (+(x3::char_ - ','))[appender] % ','))
    {
        throw config_error("failed to parse font-feature-settings: '" + features + "'");
    }
}

std::string font_feature_settings::to_string() const
{
    std::ostringstream output;
    constexpr size_t buffsize = 128;
    char buff[buffsize];
    bool first = true;

    for (auto feature : features_)
    {
        if (!first)
        {
            output << ", ";
            first = false;
        }
        hb_feature_to_string(&feature, buff, buffsize);
        output << buff;
    }

    return output.str();
}

void font_feature_settings::append(std::string const& feature)
{
    features_.emplace_back();
    feature_iterator current_feature = features_.end() - 1;

    if (!hb_feature_from_string(feature.c_str(), feature.length(), &*current_feature))
    {
        features_.erase(current_feature);
        throw config_error("failed to parse font-feature-settings: '" + feature + "'");
    }
}

}
