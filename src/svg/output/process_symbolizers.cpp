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
#include <mapnik/svg/output/svg_renderer.hpp>

namespace mapnik {

struct symbol_type_dispatch : public boost::static_visitor<bool>
{
    template <typename Symbolizer>
    bool operator()(Symbolizer const& sym) const
    {
        return false;
    }
    bool operator()(line_symbolizer const& sym) const
    {
        return true;
    }
    bool operator()(polygon_symbolizer const& sym) const
    {
        return true;
    }
};

bool is_path_based(symbolizer const& sym)
{
    return boost::apply_visitor(symbol_type_dispatch(), sym);
}

template <typename OutputIterator>
bool svg_renderer<OutputIterator>::process(rule::symbolizers const& syms,
                                           mapnik::feature_impl & feature,
                                           proj_transform const& prj_trans)
{
    // svg renderer supports processing of multiple symbolizers.
    typedef coord_transform<CoordTransform, geometry_type> path_type;

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
        boost::apply_visitor(symbol_dispatch(*this, feature, prj_trans), sym);
    }

    if (process_path)
    {
        // generate path output for each geometry of the current feature.
        for (auto & geom : feature.paths())
        {
            if(geom.size() > 0)
            {
                path_type path(common_.t_, geom, prj_trans);
                generator_.generate_path(path, path_attributes_);
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

