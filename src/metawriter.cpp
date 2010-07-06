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
#include <mapnik/expression_evaluator.hpp>

// STL
#include <iomanip>

namespace mapnik {

    metawriter_json::metawriter_json(std::string fn, expression_ptr dflt_expr) : dflt_expr_(dflt_expr), count(0)
{
    f.open(fn.c_str(), std::fstream::out | std::fstream::trunc);
    if (f.fail()) perror((std::string("Failed to open file ") + fn).c_str());
    f << "{ \"type\": \"FeatureCollection\", \"features\": [\n";
}

metawriter_json::~metawriter_json()
{
    if (f.is_open()) {
        f << " ] }\n";
        f.close();
    }
    std::clog << "Destroying metawriter\n";
}

void metawriter_json::add_box(box2d<double> box, Feature const &feature, proj_transform const& prj_trans, CoordTransform const &t, expression_ptr expression)
{
    expression_ptr e;
    if (expression) {
        e = expression;
    } else {
        e = dflt_expr_;
    }
    if (!e) {
        std::clog << "WARNING: No expression available for JSON metawriter.\n";
        return;
    }
    value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*e);
    UnicodeString text = result.to_unicode();

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

    if (count++) {
        f << ",\n";
    }
    f << std::fixed << std::setprecision(8) << "{ \"type\": \"Feature\",\n  \"geometry\": { \"type\": \"Polygon\",\n    \"coordinates\": [ [ [" <<
            minx << ", " << miny << "], [" <<
            maxx << ", " << miny << "], [" <<
            maxx << ", " << maxy << "], [" <<
            minx << ", " << maxy << "] ] ] }," <<
            "\n  \"properties\": {" << text << "} }";
}

};
