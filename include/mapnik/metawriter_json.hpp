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
    virtual void add_box(box2d<double> box, Feature const &feature,
                         proj_transform const& prj_trans,
                         CoordTransform const& t,
                         metawriter_properties const& properties);

    virtual void start(metawriter_property_map const& properties);
    virtual void stop();
    void set_stream(std::ostream *f) { f_ = f; }

private:
    std::ostream *f_;
    int count;
};

/** JSON writer. */
class metawriter_json : public metawriter_json_stream
{
public:
    metawriter_json(metawriter_properties dflt_properties, path_expression_ptr fn);
    ~metawriter_json();

    virtual void start(metawriter_property_map const& properties);
    virtual void stop();

private:
    path_expression_ptr fn_;
    std::fstream f_;
};

};

#endif
