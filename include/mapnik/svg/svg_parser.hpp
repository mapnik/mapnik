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

#ifndef MAPNIK_SVG_PARSER_HPP
#define MAPNIK_SVG_PARSER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/css/css_grammar_x3.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/gradient.hpp>
#include <mapnik/util/noncopyable.hpp>
// stl
#include <map>
#include <algorithm>
#include <deque>
// boost
#include <boost/optional.hpp>

namespace boost { namespace property_tree { namespace detail { namespace rapidxml {
template <typename T> class xml_node;
}}}}

namespace  mapnik { namespace svg {

struct viewbox
{
    double x0;
    double y0;
    double width;
    double height;
};

class svg_parser_error_handler
{
    using error_message_container = std::vector<std::string> ;
public:
    explicit svg_parser_error_handler(bool strict = false)
        : strict_(strict) {}

    void on_error(std::string const& msg)
    {
        if (strict_) throw std::runtime_error(msg);
        else
        {
            // avoid duplicate messages
            if (std::find(std::begin(error_messages_),std::end(error_messages_), msg) == std::end(error_messages_))
            {
                error_messages_.push_back(msg);
            }
        }
    }
    error_message_container const& error_messages() const
    {
        return error_messages_;
    }
    bool strict() const { return strict_; }
private:

    error_message_container error_messages_;
    bool strict_;
};

class MAPNIK_DECL svg_parser : private util::noncopyable
{
    using error_handler = svg_parser_error_handler;
public:
    explicit svg_parser(svg_converter_type & path, bool strict = false);
    ~svg_parser();
    error_handler & err_handler();
    void parse(std::string const& filename);
    void parse_from_string(std::string const& svg);
    svg_converter_type & path_;
    bool is_defs_;
    bool strict_;
    bool ignore_;
    bool css_style_;
    std::map<std::string, gradient> gradient_map_;
    std::map<std::string, boost::property_tree::detail::rapidxml::xml_node<char> const*> node_cache_;
    mapnik::css_data css_data_;
    boost::optional<viewbox> vbox_{};
    double normalized_diagonal_ = 0.0;
    agg::trans_affine viewbox_tr_{};
    std::deque<double> font_sizes_{};
    error_handler err_handler_;
};

}}


#endif // MAPNIK_SVG_PARSER_HPP
