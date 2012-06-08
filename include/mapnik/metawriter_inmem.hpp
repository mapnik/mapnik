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

#ifndef MAPNIK_METAWRITER_INMEM_HPP
#define MAPNIK_METAWRITER_INMEM_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/metawriter.hpp>

// boost
#include <boost/shared_ptr.hpp>

// stl
#include <list>
#include <map>
#include <string>

namespace mapnik {

/**
 * Keeps metadata information in-memory, where it can be retrieved by whatever's
 * calling Mapnik and custom output provided.
 *
 * Stored data is all in image coordinates in the current implementation.
 *
 * This is most useful when Mapnik is being called from Python, and the result
 * of the metawriter can be queried and injected into the (meta)tile or whatever
 * in a very flexible way. E.g: for a GUI app the metawriter can be used to
 * create hit areas, for a web app it could be used to create an HTML image map.
 *
 * Because this is kept in-memory, applying this metawriter to features which are
 * very common in the rendered image will increase memory usage, especially if
 * many attributes are also kept.
 */


namespace {

using mapnik::value;
using mapnik::Feature;
using mapnik::metawriter_properties;

// intersect a set of properties with those in the feature descriptor
std::map<std::string,value> intersect_properties(Feature const& feature, metawriter_properties const& properties) 
{

    std::map<std::string,value> nprops;
    BOOST_FOREACH(std::string p, properties)
    {
        if (feature.has_key(p))
            nprops.insert(std::make_pair(p,feature.get(p)));
    }

    return nprops;
}} // end anonymous namespace

class MAPNIK_DECL metawriter_inmem
    : public metawriter_base, private boost::noncopyable 
{
public:
    metawriter_inmem(metawriter_properties dflt_properties);
    ~metawriter_inmem();

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
        box2d<double> box;
        unsigned cmd;
        double x = 0.0, y = 0.0;
      
        path.rewind(0);
        while ((cmd = path.vertex(&x, &y)) != SEG_END) {
            box.expand_to_include(x, y);
        }
      
        if ((box.width() >= 0.0) && (box.height() >= 0.0)) {
            meta_instance inst;
            inst.properties = intersect_properties(feature, properties);
            inst.box = box;
            instances_.push_back(inst);
        }   
    }
    
    void start(metawriter_property_map const& properties);
    void stop() {};
    void set_map_srs(projection const& proj) {}
    /**
     * An instance of a rendered feature. The box represents the image
     * coordinates of a bounding box around the feature. The properties
     * are the intersection of the features' properties and the "kept"
     * properties of the metawriter.
     */
    struct MAPNIK_DECL meta_instance {
        box2d<double> box;
        std::map<std::string, value> properties;
    };

    typedef std::list<meta_instance> meta_instance_list;

    // const-only access to the instances.
    const meta_instance_list &instances() const;

    // utility iterators for use in the python bindings.
    meta_instance_list::const_iterator inst_begin() const;
    meta_instance_list::const_iterator inst_end() const;

private:

    std::list<meta_instance> instances_;

    template <typename T>
    void add_vertices(T & path,
                      Feature const& feature,
                      CoordTransform const& t,
                      metawriter_properties const& properties);
};

/** Shared pointer to metawriter_inmem object. */
typedef boost::shared_ptr<metawriter_inmem> metawriter_inmem_ptr;

}

#endif // MAPNIK_METAWRITER_INMEM_HPP
