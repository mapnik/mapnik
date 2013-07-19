/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_MAP_QUERY
#define MAPNIK_UTIL_MAP_QUERY

#include <mapnik/map.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/filter_featureset.hpp>
#include <mapnik/hit_test_filter.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

#include <sstream>
#include <stdexcept>

namespace mapnik { namespace util {


static mapnik::box2d<double> get_extent(mapnik::Map const& map)
{
    std::vector<mapnik::layer> const& layers = map.layers();
    if (layers.empty())
    {
        throw std::runtime_error("map has no layers so unable to get extent");
    }
    box2d<double> ext;
    bool success = false;
    projection proj0(map.srs());
    bool first = true;
    BOOST_FOREACH ( layer const& lay, map.layers() )
    {
        if (lay.active())
        {
            projection proj1(lay.srs());
            proj_transform prj_trans(proj0,proj1);
            box2d<double> layer_ext = lay.envelope();
            if (prj_trans.backward(layer_ext, PROJ_ENVELOPE_POINTS))
            {
                success = true;
                if (first)
                {
                    ext = layer_ext;
                    first = false;
                }
                else
                {
                    ext.expand_to_include(layer_ext);
                }
            }
        }
    }
    if (!success)
    {
        throw std::runtime_error("could not zoom to combined layer extents");
    }
    return ext;
}

static featureset_ptr query_point(mapnik::Map const& map, unsigned index, double x, double y)
{
    if (!map.get_current_extent().valid())
    {
        throw std::runtime_error("query_point: map extent is not intialized, you need to set a valid extent before querying");
    }
    if (!map.get_current_extent().intersects(x,y))
    {
        throw std::runtime_error("query_point: x,y coords do not intersect map extent");
    }
    std::vector<mapnik::layer> const& layers = map.layers();
    if (index < layers.size())
    {
        mapnik::layer const& layer = layers[index];
        mapnik::datasource_ptr ds = layer.datasource();
        if (ds)
        {
            mapnik::projection dest(map.srs());
            mapnik::projection source(layer.srs());
            proj_transform prj_trans(source,dest);
            double z = 0;
            if (!prj_trans.equal() && !prj_trans.backward(x,y,z))
            {
                throw std::runtime_error("query_point: could not project x,y into layer srs");
            }
            // calculate default tolerance
            mapnik::box2d<double> map_ex = map.get_current_extent();
            if (!prj_trans.backward(map_ex,PROJ_ENVELOPE_POINTS))
            {
                std::ostringstream s;
                s << "query_point: could not project map extent '" << map_ex
                  << "' into layer srs for tolerance calculation";
                throw std::runtime_error(s.str());
            }
            double tol = (map_ex.maxx() - map_ex.minx()) / map.width() * 3;
            featureset_ptr fs = ds->features_at_point(mapnik::coord2d(x,y), tol);
            if (fs)
            {
                return boost::make_shared<filter_featureset<hit_test_filter> >(fs,
                                                                               hit_test_filter(x,y,tol));
            }
        }
    }
    else
    {
        std::ostringstream s;
        s << "Invalid layer index passed to query_point: '" << index << "'";
        if (!layers.empty()) s << " for map with " << layers.size() << " layers(s)";
        else s << " (map has no layers)";
        throw std::out_of_range(s.str());
    }
    return featureset_ptr();
}

static featureset_ptr query_map_point(mapnik::Map const& map, unsigned index, double x, double y)
{
    CoordTransform tr(map.width(),map.height(),map.get_current_extent());
    tr.backward(&x,&y);
    return query_point(map,index,x,y);
}

}}

#endif // MAPNIK_UTIL_MAP_QUERY