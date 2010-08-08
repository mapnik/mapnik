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


#ifndef METAWRITER_JSON_HPP
#define METAWRITER_JSON_HPP

// Mapnik
#include <mapnik/metawriter.hpp>
#include <mapnik/parse_path.hpp>

// STL
#include <fstream>

namespace mapnik {


/** Write JSON data to a stream object. */
class metawriter_json_stream : public metawriter, private boost::noncopyable
{
public:
    metawriter_json_stream(metawriter_properties dflt_properties);
    ~metawriter_json_stream();
    virtual void add_box(box2d<double> const& box, Feature const& feature,
                         CoordTransform const& t,
                         metawriter_properties const& properties);

    virtual void start(metawriter_property_map const& properties);
    virtual void stop();
    /** Set output stream. This function has to be called before the first output is made. */
    void set_stream(std::ostream *f) { f_ = f; }
    /** Get output stream. */
    std::ostream *get_stream() const { return f_; }
    /** Only write header/footer to file with one or more features. */
    void set_only_nonempty(bool only_nonempty) { only_nonempty_ = only_nonempty; }
    /** See set_only_nonempty(). */
    bool get_only_nonempty() { return only_nonempty_; }
    virtual void set_map_srs(projection const& proj);
protected:
    /** Features written. */
    int count_;
    bool only_nonempty_;
    /** Transformation from map srs to output srs. */
    proj_transform *trans_;
    projection output_srs_;
    virtual void write_header();
private:
    std::ostream *f_;
};

/** Shared pointer to metawriter_json_stream object. */
typedef boost::shared_ptr<metawriter_json_stream> metawriter_json_stream_ptr;

/** JSON writer. */
class metawriter_json : public metawriter_json_stream
{
public:
    metawriter_json(metawriter_properties dflt_properties, path_expression_ptr fn);

    virtual void start(metawriter_property_map const& properties);
    virtual void stop();
    /** Set filename template.
      *
      * This template is processed with values from Map's metawriter properties to
      * create the actual filename during start() call.
      */
    void set_filename(path_expression_ptr fn);
    /** Get filename template. */
    path_expression_ptr get_filename() const;
private:
    path_expression_ptr fn_;
    std::fstream f_;
    std::string filename_;
protected:
    virtual void write_header();
};

/** Shared pointer to metawriter_json object. */
typedef boost::shared_ptr<metawriter_json> metawriter_json_ptr;

};

#endif
