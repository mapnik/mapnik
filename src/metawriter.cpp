/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Hermann Kraus
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
 

// Mapnik
#include <mapnik/metawriter.hpp>
#include <mapnik/metawriter_json.hpp>

// Boost
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

// STL
#include <iomanip>
#include <cstdio>

namespace mapnik {

metawriter_json::metawriter_json(metawriter_properties dflt_properties, std::string fn)
    : metawriter(dflt_properties), fn_(fn), count(0)
{

}

metawriter_json::~metawriter_json()
{
    stop();
}

void metawriter_json::start()
{
    if (f.is_open())
    {
        std::cerr << "ERROR: GeoJSON metawriter is already active!\n";
    }
    f.open(fn_.c_str(), std::fstream::out | std::fstream::trunc);
    if (f.fail()) perror((std::string("Failed to open file ") + fn_).c_str());
    f << "{ \"type\": \"FeatureCollection\", \"features\": [\n";
}

void metawriter_json::stop()
{
    if (f.is_open())
    {
        f << " ] }\n";
        f.close();
    }
}

void metawriter_json::add_box(box2d<double> box, Feature const &feature,
                              proj_transform const& prj_trans, CoordTransform const &t,
                              const metawriter_properties& properties)
{
    metawriter_properties props;
    if (properties.empty())
    {
        props = dflt_properties_;
    }
    else
    {
        props = properties;
    }
    if (props.empty())
    {
        std::cerr << "WARNING: No expression available for GeoJSON metawriter.\n";
        return;
    }

    /* Coordinate transform in renderer:
       input: layer srs
       prj_trans.backwards()   [prj_trans: map -> layer]
       intermediate: map srs
       t_.forward()
       output: pixels

       Our transform:
       input: pixels
       t_.backward()
       intermediate: map srs (available via prj_trans.source())
       trans.forward()
       output: WGS84
    */

    proj_transform trans(prj_trans.source(), projection("+proj=latlong +datum=WGS84"));
    //t_ coord transform has transform for box2d combined with prj_trans
    box = t.backward(box, trans);

    double minx = box.minx();
    double miny = box.miny();
    double maxx = box.maxx();
    double maxy = box.maxy();

    if (count++) f << ",\n";
    f << std::fixed << std::setprecision(8) << "{ \"type\": \"Feature\",\n  \"geometry\": { \"type\": \"Polygon\",\n    \"coordinates\": [ [ [" <<
            minx << ", " << miny << "], [" <<
            maxx << ", " << miny << "], [" <<
            maxx << ", " << maxy << "], [" <<
            minx << ", " << maxy << "] ] ] }," <<
            "\n  \"properties\": {";
    std::map<std::string, value> fprops = feature.props();
    int i = 0;
    BOOST_FOREACH(std::string p, props)
    {
        std::map<std::string, value>::const_iterator itr = fprops.find(p);
        std::string text;
        if (itr != fprops.end())
        {
            //Property found
            text = boost::replace_all_copy(boost::replace_all_copy(itr->second.to_string(), "\\", "\\\\"), "\"", "\\\"");
        }
        if (i++) f << ",";
        f << "\n    \"" << p << "\":\"" << text << "\"";
    }

    f << "\n} }";
}

metawriter_properties metawriter::parse_properties(boost::optional<std::string> str)
{
    metawriter_properties properties;
    if (str) {
        std::string str_ = boost::algorithm::erase_all_copy(*str, " ");
        boost::split(properties, str_, boost::is_any_of(","), boost::token_compress_on);
    }
    return properties;
}
};
