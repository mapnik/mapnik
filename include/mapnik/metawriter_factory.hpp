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

#ifndef MAPNIK_METAWRITER_FACTORY_HPP
#define MAPNIK_METAWRITER_FACTORY_HPP

// mapnik
#include <mapnik/metawriter.hpp>
#include <mapnik/metawriter_json.hpp>
#include <mapnik/metawriter_inmem.hpp>
// boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik {


class xml_node;

struct is_valid : boost::static_visitor<bool>
{
    template <typename T>
    bool operator() (T const& writer_ptr) const
    {
        return (writer_ptr.get()!=0) ? true : false;
    }
};

struct add_box_ : boost::static_visitor<>
{
    add_box_(box2d<double> const& box, Feature const& feature,
            CoordTransform const& t,
            metawriter_properties const& properties)
        : box_(box),
          feature_(feature),
          t_(t),
          properties_(properties)
    {}
    
    template <typename T>
    void operator() (T const& writer_ptr) const
    {
        return writer_ptr->add_box(box_,feature_,t_, properties_);
    }
    
    box2d<double> const& box_;
    Feature const& feature_;
    CoordTransform const& t_;
    metawriter_properties const& properties_;
};


template <typename T>
struct add_line_ : boost::static_visitor<>
{
    typedef T path_type;
    
    add_line_(path_type & path, Feature const& feature,
            CoordTransform const& t,
            metawriter_properties const& properties)
        : path_(path),
          feature_(feature),
          t_(t),
          properties_(properties)
    {}
    
    template <typename U>
    void operator() (U const& writer_ptr) const
    {
        return writer_ptr->add_line(path_,feature_,t_, properties_);
    }
    
    path_type & path_;
    Feature const& feature_;
    CoordTransform const& t_;
    metawriter_properties const& properties_;
};


struct start_ : boost::static_visitor<>
{    
    start_(metawriter_property_map const& properties)
        : properties_(properties) {}
    
    template <typename U>
    void operator() (U const& writer_ptr) const
    {
        std::cout << typeid(*writer_ptr).name() << std::endl;
        return writer_ptr->start(properties_);
    }
    metawriter_property_map const& properties_;
};

struct set_size_ : boost::static_visitor<>
{    
    set_size_(unsigned w, unsigned h)
        : w_(w), h_(h) {}
    
    template <typename U>
    void operator() (U const& writer_ptr) const
    {
        return writer_ptr->set_size(w_,h_);
    }
    
    unsigned w_;
    unsigned h_;
};


struct set_map_srs_ : boost::static_visitor<>
{    
    set_map_srs_(projection const& proj)
       : proj_(proj) {}
    
    template <typename U>
    void operator() (U const& writer_ptr) const
    {
        return writer_ptr->set_map_srs(proj_);
    }
    
    projection const& proj_;
};

struct stop_ : boost::static_visitor<>
{    
    template <typename U>
    void operator() (U const& writer_ptr) const
    {
        return writer_ptr->stop();
    }
};


typedef boost::variant<metawriter_json_ptr, metawriter_inmem_ptr> metawriter;

inline bool check_metawriter(metawriter const& m)
{        
    return boost::apply_visitor(is_valid(), m);
}


inline void add_box(metawriter const& m,
                    box2d<double> const& box, Feature const& feature,
                    CoordTransform const& t,
                    metawriter_properties const& properties)
{
    add_box_ v(box,feature,t,properties);
    boost::apply_visitor(v, m);
}

template <typename T>
void add_line(metawriter const& m, 
              T & path, 
              Feature const& feature,
              CoordTransform const& t,
              metawriter_properties const& properties)
{
    add_line_<T> v(path,feature,t,properties);
    boost::apply_visitor(v, m);
}

inline void start(metawriter const& m, metawriter_property_map const& properties )
{
    start_ v(properties);
    boost::apply_visitor(v, m);
}

inline void stop(metawriter const& m)
{
    boost::apply_visitor(stop_(), m);
}

inline void set_size(metawriter const& m, unsigned w, unsigned h)
{
    set_size_ v(w,h);
    boost::apply_visitor(v, m);    
}

inline void set_map_srs(metawriter const& m, projection const& proj)
{
    set_map_srs_ v(proj);
    boost::apply_visitor(v, m);
}

typedef std::pair<metawriter, metawriter_properties> metawriter_with_properties;

/**
 * Creates a metawriter with the properties specified in the property
 * tree argument. Currently, this is hard-coded to the JSON and inmem
 * metawriters, but should provide an easy point to make them a
 * proper factory method if this is wanted in the future.
 */
metawriter metawriter_create(xml_node const& pt);

/**
 * Writes properties into the given property tree representing the
 * metawriter argument, and which can be used to reconstruct it.
 */
void metawriter_save(
    metawriter const& m,
    boost::property_tree::ptree & pt,
    bool explicit_defaults);

}

#endif // MAPNIK_METAWRITER_FACTORY_HPP

