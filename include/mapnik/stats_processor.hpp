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

#ifndef MAPNIK_STATS_PROCESSOR_HPP
#define MAPNIK_STATS_PROCESSOR_HPP

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/memory_datasource.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


// stl
#include <set>
#include <string>
#include <vector>

namespace mapnik
{

class Map;
class layer;
class projection;

struct stats
{
    stats()
     : elapsed(0),
       count(0),
       reprojected(false)
     {}
    unsigned int elapsed;
    unsigned int count;
    bool reprojected;
};

class stats_processor
{
public:
    explicit stats_processor(Map const& m);
    void apply();
    std::string to_json()
    {
        std::ostringstream ss;
        write_json(ss,tree);
        return ss.str();
    }
private:
    void apply_to_layer(layer const& lay,
                        projection const& proj0,
                        double scale_denom,
                        std::set<std::string>& names);
    void render_style(layer const& lay,
                      feature_type_style* style,
                      std::string const& style_name,
                      featureset_ptr features,
                      proj_transform const& prj_trans,
                      double scale_denom);

    Map const& m_;
    boost::property_tree::ptree tree;
    boost::scoped_ptr<stats> s;
};
}

#endif // MAPNIK_STATS_PROCESSOR_HPP
