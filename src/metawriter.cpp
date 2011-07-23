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
#include <mapnik/placement_finder.hpp>

// Boost
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

// STL
#include <iomanip>
#include <cstdio>

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
    if (trans_) delete trans_;
}


metawriter_json_stream::metawriter_json_stream(metawriter_properties dflt_properties)
    : metawriter(dflt_properties), count_(-1), output_empty_(true),
      trans_(0), output_srs_("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"), pixel_coordinates_(false), f_(0)
{
}

void metawriter_json_stream::write_properties(Feature const& feature, metawriter_properties const& properties)
{
    *f_ << "}," << //Close coordinates object
            "\n  \"properties\": {";
    std::map<std::string, value> const &fprops = feature.props();
    int i = 0;
    BOOST_FOREACH(std::string p, properties)
    {
        std::map<std::string, value>::const_iterator itr = fprops.find(p);
        std::string text;
        if (itr != fprops.end())
        {
            // Skip empty props
            if(itr->second.to_string().size() == 0) continue; // ignore empty
            
            //Property found
            text = boost::replace_all_copy(boost::replace_all_copy(itr->second.to_string(), "\\", "\\\\"), "\"", "\\\"");
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

void metawriter_json_stream::add_text(placement const& p,
    face_set_ptr face,
    Feature const& feature,
    CoordTransform const& t,
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
//    if (p.placements.size()) std::cout << p.info.get_string() << "\n";
    for (unsigned n = 0; n < p.placements.size(); n++) {
        placement_element & current_placement = const_cast<placement_element &>(p.placements[n]);

        bool inside = false;
        bool straight = true;
        int c; double x, y, angle;
        current_placement.rewind();
        for (int i = 0; i < current_placement.num_nodes(); ++i) {
            int cx = current_placement.starting_x;
            int cy = current_placement.starting_y;
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
                font_face_set::dimension_t ci = face->character_dimensions(c);
                if (x < minx) minx = x;
                if (x+ci.width > maxx) maxx = x+ci.width;
                if (y+ci.height+ci.ymin > maxy) maxy = y+ci.height+ci.ymin;
                if (y+ci.ymin < miny) miny = y+ci.ymin;
                //                std::cout << (char) c << " height:" << ci.height << " ymin:" << ci.ymin << " y:" << y << " miny:"<< miny << " maxy:"<< maxy <<"\n";

            }
            add_box(box2d<double>(current_placement.starting_x+minx,
                                  current_placement.starting_y-miny,
                                  current_placement.starting_x+maxx,
                                  current_placement.starting_y-maxy), feature, t, properties);
            continue;
        }

        write_feature_header("MultiPolygon");
        *f_ << "[";
        c = ' ';
        for (int i = 0; i < current_placement.num_nodes(); ++i) {
            if (c != ' ') {
                *f_ << ",";
            }
            current_placement.vertex(&c, &x, &y, &angle);
            if (c == ' ') continue;
            font_face_set::dimension_t ci = face->character_dimensions(c);

            double x0, y0, x1, y1, x2, y2, x3, y3;
            double sina = sin(angle);
            double cosa = cos(angle);
            x0 = current_placement.starting_x + x - sina*ci.ymin;
            y0 = current_placement.starting_y - y - cosa*ci.ymin;
            x1 = x0 + ci.width * cosa;
            y1 = y0 - ci.width * sina;
            x2 = x0 + (ci.width * cosa - ci.height * sina);
            y2 = y0 - (ci.width * sina + ci.height * cosa);
            x3 = x0 - ci.height * sina;
            y3 = y0 - ci.height * cosa;

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
    else if (count_ >= STARTED){
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

}
