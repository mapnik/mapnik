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

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/version.hpp>
#pragma GCC diagnostic pop


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

    namespace qi = boost::spirit::qi;
    qi::char_type char_;
    qi::as_string_type as_string;

#if BOOST_VERSION <= 104800
    // Call correct overload.
    using std::placeholders::_1;
    void (font_feature_settings::*append)(std::string const&) = &font_feature_settings::append;
    if (!qi::parse(features.begin(), features.end(), as_string[+(char_ - ',')][std::bind(append, this, _1)] % ','))
#else
    auto app = [&](std::string const& s) { append(s); };
    if (!qi::parse(features.begin(), features.end(), as_string[+(char_ - ',')][app] % ','))
#endif

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
