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

// Mapnik
#include <mapnik/debug.hpp>
#include <mapnik/metawriter.hpp>
#include <mapnik/metawriter_json.hpp>
#include <mapnik/text_placements/base.hpp>
#include <mapnik/text_path.hpp>

// Boost
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

// STL
#include <iomanip>

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

std::string metawriter_properties::to_string() const
{
    return boost::algorithm::join(*this, ",");
}

/********************************************************************************************/

void metawriter_json_stream::start(metawriter_property_map const& /*properties*/)
{
    assert(trans_);
    if (output_empty_) {
        write_header();
    } else {
        count_ = HEADER_NOT_WRITTEN;
    }
}

void metawriter_json_stream::write_header()
{
    assert(f_);
    *f_ << "{ \"type\": \"FeatureCollection\", \"features\": [\n" << std::fixed << std::setprecision(pixel_coordinates_ ? 0 : 8);
    count_ = STARTED;
}

void metawriter_json_stream::stop()
{
    if (count_ >= STARTED && f_)
    {
        *f_ << " ] }\n";
    }
    count_ = STOPPED;
}

metawriter_json_stream::~metawriter_json_stream()
{
    if (count_ >= STARTED)
    {
        MAPNIK_LOG_WARN(metawriter) << "WARNING: GeoJSON metawriter not stopped before destroying it.";

        stop();
    }
    if (trans_) delete trans_;
}


metawriter_json_stream::metawriter_json_stream(metawriter_properties dflt_properties)
    : metawriter(dflt_properties), count_(-1), output_empty_(true),
      trans_(0), output_srs_("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
      pixel_coordinates_(false), f_(0)
{
}

void metawriter_json_stream::write_properties(Feature const& feature, metawriter_properties const& properties)
{
    *f_ << "}," << //Close coordinates object
        "\n  \"properties\": {";

    int i = 0;
    BOOST_FOREACH(std::string const& p, properties)
    {
        if (feature.has_key(p))
        {
            mapnik::value const& val = feature.get(p);
            std::string str = val.to_string();
            if (str.size() == 0) continue; // ignore empty attributes

            //Property found
            std::string text = boost::replace_all_copy(boost::replace_all_copy(str, "\\", "\\\\"), "\"", "\\\"");
            if (i++) *f_ << ",";
            *f_ << "\n    \"" << p << "\":\"" << text << "\"";
        }
    }

    *f_ << "\n} }";
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
   intermediate: map srs
   trans_.forward()
   output: WGS84
*/


void metawriter_json_stream::add_box(box2d<double> const &box, Feature const& feature,
                                     CoordTransform const& t, metawriter_properties const& properties)
{
    /* Check if feature is in bounds. */
    if (box.maxx() < 0 || box.maxy() < 0 || box.minx() > width_ || box.miny() > height_) return;
    double minx, miny, maxx, maxy;
    if (pixel_coordinates_) {
        minx = box.minx();
        miny = box.miny();
        maxx = box.maxx();
        maxy = box.maxy();
    } else {
        //t_ coord transform has transform for box2d combined with proj_transform
        box2d<double> transformed = t.backward(box, *trans_);

        minx = transformed.minx();
        miny = transformed.miny();
        maxx = transformed.maxx();
        maxy = transformed.maxy();
    }

    write_feature_header("Polygon");

    *f_ << " [ [ [" <<
        minx << ", " << miny << "], [" <<
        maxx << ", " << miny << "], [" <<
        maxx << ", " << maxy << "], [" <<
        minx << ", " << maxy << "] ] ]";

    write_properties(feature, properties);

}

void metawriter_json_stream::add_text(
    boost::ptr_vector<text_path> &placements, box2d<double> const& extents,
    Feature const& feature, CoordTransform const& t,
    metawriter_properties const& properties)
{
    /* Note:
       Map coordinate system (and starting_{x,y}) starts in upper left corner
       and grows towards lower right corner.
       Font + placement vertex coordinate system starts in lower left corner
       and grows towards upper right corner.
       Therefore y direction is different. Keep this in mind while doing calculations.

       The y value returned by vertex() is always the baseline.
       Lowest y = baseline of bottom line
       Hightest y = baseline of top line

    */
    for (unsigned n = 0; n < placements.size(); n++)
    {
        text_path &current_placement = placements[n];

        bool inside = false; /* Part of text is inside rendering region */
        bool straight = true;
        char_info_ptr c;
        double x, y, angle;
        current_placement.rewind();
        for (int i = 0; i < current_placement.num_nodes(); ++i) {
            int cx = current_placement.center.x;
            int cy = current_placement.center.y;
            current_placement.vertex(&c, &x, &y, &angle);
            if (cx+x >= 0 && cx+x < width_ && cy-y >= 0 && cy-y < height_) inside = true;
            if (angle > 0.001 || angle < -0.001) straight = false;
            if (inside && !straight) break;
        }
        if (!inside) continue;

        current_placement.rewind();

        if (straight) {
            //Reduce number of polygons
            double minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
            for (int i = 0; i < current_placement.num_nodes(); ++i) {
                current_placement.vertex(&c, &x, &y, &angle);
                minx = std::min(minx, x);
                maxx = std::max(maxx, x+c->width);
                maxy = std::max(maxy, y+c->ymax);
                miny = std::min(miny, y+c->ymin);
            }
            add_box(box2d<double>(current_placement.center.x+minx,
                                  current_placement.center.y-miny,
                                  current_placement.center.x+maxx,
                                  current_placement.center.y-maxy), feature, t, properties);
            continue;
        }

        write_feature_header("MultiPolygon");
        *f_ << "[";
        for (int i = 0; i < current_placement.num_nodes(); ++i) {
            current_placement.vertex(&c, &x, &y, &angle);
            if (c->c == ' ') continue;
            *f_ << ",";

            double x0, y0, x1, y1, x2, y2, x3, y3;
            double sina = sin(angle);
            double cosa = cos(angle);
            x0 = current_placement.center.x + x - sina*c->ymin;
            y0 = current_placement.center.y - y - cosa*c->ymin;
            x1 = x0 + c->width * cosa;
            y1 = y0 - c->width * sina;
            x2 = x0 + (c->width * cosa - c->height() * sina);
            y2 = y0 - (c->width * sina + c->height() * cosa);
            x3 = x0 - c->height() * sina;
            y3 = y0 - c->height() * cosa;

            *f_ << "\n     [[";
            write_point(t, x0, y0);
            write_point(t, x1, y1);
            write_point(t, x2, y2);
            write_point(t, x3, y3, true);
            *f_ << "]]";
        }
        *f_ << "]";
        write_properties(feature, properties);
    }
}

void metawriter_json_stream::add_polygon(path_type & path,
                                         Feature const& feature,
                                         CoordTransform const& t,
                                         metawriter_properties const& properties)
{
    write_feature_header("Polygon");
    write_line_polygon(path, t, true);
    write_properties(feature, properties);
}

void metawriter_json_stream::add_line(path_type & path,
                                      Feature const& feature,
                                      CoordTransform const& t,
                                      metawriter_properties const& properties)
{
    write_feature_header("MultiLineString");
    write_line_polygon(path, t, false);
    write_properties(feature, properties);
}

void metawriter_json_stream::write_line_polygon(path_type & path, CoordTransform const& t, bool /*polygon*/){
    *f_ << " [";
    double x, y, last_x=0.0, last_y=0.0;
    unsigned cmd, last_cmd = SEG_END;
    path.rewind(0);

    int polygon_count = 0;
    while ((cmd = path.vertex(&x, &y)) != SEG_END) {
        if (cmd == SEG_LINETO) {
            if (last_cmd == SEG_MOVETO) {
                //Start new polygon/line
                if (polygon_count++) *f_ << "], ";
                *f_ << "[";
                write_point(t, last_x, last_y, true);
            }
            *f_ << ",";
            write_point(t, x, y, true);
        }
        last_x = x;
        last_y = y;
        last_cmd = cmd;
    }
    *f_ << "]]";
}


void metawriter_json_stream::set_map_srs(projection const& input_srs_)
{
    if (trans_) delete trans_;
    trans_ = new proj_transform(input_srs_, output_srs_);
}


/********************************************************************************************/

metawriter_json::metawriter_json(metawriter_properties dflt_properties, path_expression_ptr fn)
    : metawriter_json_stream(dflt_properties), fn_(fn) {}


void metawriter_json::start(metawriter_property_map const& properties)
{
    filename_ = path_processor<metawriter_property_map>::evaluate(*fn_, properties);

    MAPNIK_LOG_DEBUG(metawriter) << "metawriter_json: Filename=" << filename_;

    metawriter_json_stream::start(properties);
}

void metawriter_json::write_header()
{
    f_.open(filename_.c_str(), std::fstream::out | std::fstream::trunc);
    if (f_.fail())
    {
        MAPNIK_LOG_DEBUG(metawriter) << "metawriter_json: Failed to open file " << filename_;
    }
    set_stream(&f_);
    metawriter_json_stream::write_header();
}


void metawriter_json::stop()
{
    metawriter_json_stream::stop();
    if (f_.is_open())
    {
        f_.close();
    }
    else if (count_ >= STARTED)
    {
        MAPNIK_LOG_DEBUG(metawriter) << "metawriter_json: File not open when stopping";
    }
}

void metawriter_json::set_filename(path_expression_ptr fn)
{
    fn_ = fn;
}

path_expression_ptr metawriter_json::get_filename() const
{
    return fn_;
}

}
