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

#define HEADER_NOT_WRITTEN -1
#define STOPPED -2
#define STARTED 0

namespace mapnik {

UnicodeString const& metawriter_property_map::operator[](std::string const& key) const
{
    std::map<std::string, UnicodeString>::const_iterator it;
    it = m_.find(key);
    if (it == m_.end()) return not_found_;
    return (*it).second;
}

metawriter_properties::metawriter_properties(boost::optional<std::string> str)
{
    if (str) {
        boost::split(*this, *str, boost::is_any_of(", "), boost::token_compress_on);
    }
}

/********************************************************************************************/

void metawriter_json_stream::start(metawriter_property_map const& properties)
{
    if (!only_nonempty_) {
        write_header();
    } else {
        count_ = HEADER_NOT_WRITTEN;
    }
}

void metawriter_json_stream::write_header()
{
    assert(f_);
    *f_ << "{ \"type\": \"FeatureCollection\", \"features\": [\n";
    count_ = STARTED;
}

void metawriter_json_stream::stop()
{
    if (count_ >= STARTED && f_) {
        *f_ << " ] }\n";
    }
    count_ = STOPPED;
}

metawriter_json_stream::~metawriter_json_stream()
{
    if (count_ >= STARTED) {
#ifdef MAPNIK_DEBUG
        std::cerr << "WARNING: GeoJSON metawriter not stopped before destroying it.";
#endif
        stop();
    }
}

metawriter_json_stream::metawriter_json_stream(metawriter_properties dflt_properties)
    : metawriter(dflt_properties), count_(-1), only_nonempty_(true), f_(0) {}

void metawriter_json_stream::add_box(box2d<double> box, Feature const &feature,
                              proj_transform const& prj_trans, CoordTransform const &t,
                              const metawriter_properties& properties)
{
    /* Check if feature is in bounds. */
    if (box.maxx() < 0 || box.maxy() < 0 || box.minx() > width_ || box.miny() > height_) return;

    if (count_ == HEADER_NOT_WRITTEN) write_header();
#ifdef MAPNIK_DEBUG
    if (count_ == STOPPED)
    {
        std::cerr << "WARNING: Metawriter not started before using it.\n";
    }
#endif

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

    if (count_++) *f_ << ",\n";
    *f_ << std::fixed << std::setprecision(8) << "{ \"type\": \"Feature\",\n  \"geometry\": { \"type\": \"Polygon\",\n    \"coordinates\": [ [ [" <<
            minx << ", " << miny << "], [" <<
            maxx << ", " << miny << "], [" <<
            maxx << ", " << maxy << "], [" <<
            minx << ", " << maxy << "] ] ] }," <<
            "\n  \"properties\": {";
    std::map<std::string, value> fprops = feature.props();
    int i = 0;
    BOOST_FOREACH(std::string p, properties)
    {
        std::map<std::string, value>::const_iterator itr = fprops.find(p);
        std::string text;
        if (itr != fprops.end())
        {
            //Property found
            text = boost::replace_all_copy(boost::replace_all_copy(itr->second.to_string(), "\\", "\\\\"), "\"", "\\\"");
        }
        if (i++) *f_ << ",";
        *f_ << "\n    \"" << p << "\":\"" << text << "\"";
    }

    *f_ << "\n} }";
}


/********************************************************************************************/

metawriter_json::metawriter_json(metawriter_properties dflt_properties, path_expression_ptr fn)
    : metawriter_json_stream(dflt_properties), fn_(fn) {}


void metawriter_json::start(metawriter_property_map const& properties)
{
    filename_ =
        path_processor<metawriter_property_map>::evaluate(*fn_, properties);
#ifdef MAPNIK_DEBUG
    std::clog << "Metawriter JSON: filename=" << filename_ << "\n";
#endif
    metawriter_json_stream::start(properties);
}

void metawriter_json::write_header()
{
    f_.open(filename_.c_str(), std::fstream::out | std::fstream::trunc);
    if (f_.fail()) perror((std::string("Metawriter JSON: Failed to open file ") + filename_).c_str());
    set_stream(&f_);
    metawriter_json_stream::write_header();
}


void metawriter_json::stop()
{
    metawriter_json_stream::stop();
    if (f_.is_open()) {
        f_.close();
    }
#ifdef MAPNIK_DEBUG
    else {
        std::clog << "WARNING: File not open in metawriter_json::stop()!\n";
    }
#endif
}

void metawriter_json::set_filename(path_expression_ptr fn)
{
    fn_ = fn;
}

 path_expression_ptr metawriter_json::get_filename() const
 {
     return fn_;
 }

};
