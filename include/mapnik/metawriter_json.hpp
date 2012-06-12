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

#ifndef MAPNIK_METAWRITER_JSON_HPP
#define MAPNIK_METAWRITER_JSON_HPP

// mapnik
#include <mapnik/metawriter.hpp>
#include <mapnik/parse_path.hpp>

// boost
#include <boost/shared_ptr.hpp>

// stl
#include <fstream>

namespace mapnik {

class metawriter_json_stream : public metawriter_base, 
                               private boost::noncopyable
{
public:
    explicit metawriter_json_stream(metawriter_properties dflt_properties);
    ~metawriter_json_stream();
    void add_box(box2d<double> const& box, Feature const& feature,
                 CoordTransform const& t,
                 metawriter_properties const& properties);
    void add_text(boost::ptr_vector<text_path> &placements,
                  box2d<double> const& extents,
                  Feature const& feature,
                  CoordTransform const& t,
                  metawriter_properties const& properties);
    template <typename T>
    void add_polygon(T & path,
                     Feature const& feature,
                     CoordTransform const& t,
                     metawriter_properties const& properties);   
    
    template <typename T>
    void add_line(T & path,
                  Feature const& feature,
                  CoordTransform const& t,
                  metawriter_properties const& properties)
    {
        write_feature_header("MultiLineString");
        *f_ << " [";
        
        path.rewind(0);    
        double x, y;
        unsigned cmd;        
        int ring_count = 0;
        while ((cmd = path.vertex(&x, &y)) != SEG_END) 
        {
            if (cmd == SEG_MOVETO) 
            {
                if (ring_count++ != 0) *f_ << "], ";
                *f_ << "[";
                write_point(t, x, y, true);
            }
            else if (cmd == SEG_LINETO) 
            {            
                *f_ << ",";
                write_point(t, x, y, true);
            }
        }
        if (ring_count != 0) *f_ << "]";
        *f_ << "]";
            
        write_properties(feature, properties);
    }
    
    void start(metawriter_property_map const& properties);
    void stop();    
    void set_stream(std::ostream *f) { f_ = f; }    
    std::ostream *get_stream() const { return f_; }    
    void set_output_empty(bool output_empty) { output_empty_ = output_empty; }    
    bool get_output_empty() { return output_empty_; }
    void set_pixel_coordinates(bool on) { pixel_coordinates_ = on; }
    bool get_pixel_coordinates() { return pixel_coordinates_; }
    void set_map_srs(projection const& proj);
protected:
    enum {
        HEADER_NOT_WRITTEN = -1,
        STOPPED = -2,
        STARTED = 0
    };

    /** Features written. */
    int count_;
    bool output_empty_;
    /** Transformation from map srs to output srs. */
    proj_transform *trans_;
    projection output_srs_;
    bool pixel_coordinates_;
    
    void write_header();
    inline void write_feature_header(std::string type) 
    {
        if (count_ == STOPPED)
        {
            MAPNIK_LOG_WARN(metawrite_json) << "Metawriter: instance not started before using it.";
        }

        if (count_ == HEADER_NOT_WRITTEN) write_header();
        if (count_++) *f_ << ",\n";

        *f_  << "{ \"type\": \"Feature\",\n  \"geometry\": { \"type\": \"" << type << "\",\n    \"coordinates\":";
    }

    void write_properties(Feature const& feature, metawriter_properties const& properties);

    inline void write_point(CoordTransform const& t, double x, double y, bool last = false)
    {
        double z = 0.0;
        if (!pixel_coordinates_) {
            t.backward(&x, &y);
            trans_->forward(x, y, z);
        }
        *f_ << "[" << x << "," << y << "]";
        if (!last) {
            *f_ << ",";
        }
    }
    
private:
    std::ostream *f_;
};

//typedef boost::shared_ptr<metawriter_json_stream> metawriter_json_stream_ptr;

// JSON writer.
class metawriter_json : public metawriter_json_stream
{
public:
    metawriter_json(metawriter_properties dflt_properties, path_expression_ptr fn);
    void start(metawriter_property_map const& properties);
    void stop();
    void set_filename(path_expression_ptr fn);
    path_expression_ptr get_filename() const;
    
private:
    path_expression_ptr fn_;
    std::fstream f_;
    std::string filename_;

protected:
    void write_header();
};

typedef boost::shared_ptr<metawriter_json> metawriter_json_ptr;

}

#endif // MAPNIK_METAWRITER_JSON_HPP
