/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#if defined(SVG_RENDERER)

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/svg/geometry_svg_generator_impl.hpp>
#include <mapnik/svg/output/svg_output_grammars.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>
#include <mapnik/symbolizer_dispatch.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

namespace mapnik {

struct symbol_type_dispatch : public util::static_visitor<bool>
{
    template <typename Symbolizer>
    bool operator()(Symbolizer const&) const
    {
        return false;
    }
    bool operator()(line_symbolizer const&) const
    {
        return true;
    }
    bool operator()(polygon_symbolizer const&) const
    {
        return true;
    }
};

bool is_path_based(symbolizer const& sym)
{
    return util::apply_visitor(symbol_type_dispatch(), sym);
}

template <typename OutputIterator, typename PathType>
void generate_path(OutputIterator & output_iterator, PathType const& path, svg::path_output_attributes const& path_attributes)
{
    using path_dash_array_grammar = svg::svg_path_dash_array_grammar<OutputIterator>;
    using path_attributes_grammar = svg::svg_path_attributes_grammar<OutputIterator>;
    static const path_attributes_grammar attributes_grammar;
    static const path_dash_array_grammar dash_array_grammar;
    static const svg::svg_path_generator<OutputIterator,PathType> svg_path_grammer;
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::generate(output_iterator, lit("<path ") << svg_path_grammer, path);
    boost::spirit::karma::generate(output_iterator, lit(" ") << dash_array_grammar, path_attributes.stroke_dasharray());
    boost::spirit::karma::generate(output_iterator, lit(" ") << attributes_grammar << lit("/>\n"), path_attributes);
}

template <typename OutputIterator>
bool svg_renderer<OutputIterator>::process(rule::symbolizers const& syms,
                                           mapnik::feature_impl & feature,
                                           proj_transform const& prj_trans)
{
    // svg renderer supports processing of multiple symbolizers.
    using path_type = transform_path_adapter<view_transform, geometry_type>;

    bool process_path = false;
    // process each symbolizer to collect its (path) information.
    // path information (attributes from line_ and polygon_ symbolizers)
    // is collected with the path_attributes_ data member.
    for (auto const& sym : syms)
    {
        if (is_path_based(sym))
        {
            process_path = true;
        }
        util::apply_visitor(symbolizer_dispatch<svg_renderer<OutputIterator>>(*this, feature, prj_trans), sym);
    }

    if (process_path)
    {
        // generate path output for each geometry of the current feature.
        for (auto & geom : feature.paths())
        {
            if(geom.size() > 0)
            {
                path_type path(common_.t_, geom, prj_trans);
                generate_path(generator_.output_iterator_, path, path_attributes_);
            }
        }
        // set the previously collected values back to their defaults
        // for the feature that will be processed next.
        path_attributes_.reset();
    }
    return true;
}

template bool svg_renderer<std::ostream_iterator<char> >::process(rule::symbolizers const& syms,
                                                                  mapnik::feature_impl & feature,
                                                                  proj_transform const& prj_trans);

}

#endif
