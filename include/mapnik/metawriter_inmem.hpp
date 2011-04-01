/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 MapQuest
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


#ifndef METAWRITER_INMEM_HPP
#define METAWRITER_INMEM_HPP

// Mapnik
#include <mapnik/metawriter.hpp>

// STL
#include <list>

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
class metawriter_inmem
  : public metawriter, private boost::noncopyable {
public:
  /**
   * Construct an in-memory writer which keeps properties specified by the
   * dflt_properties argument. For example: if dflt_properties contains "name",
   * then the name attribute of rendered features referencing this metawriter
   * will be kept in memory.
   */
  metawriter_inmem(metawriter_properties dflt_properties);
  ~metawriter_inmem();
  
  virtual void add_box(box2d<double> const& box, Feature const& feature,
          CoordTransform const& t,
          metawriter_properties const& properties);
  virtual void add_text(placement const& p,
          face_set_ptr face,
          Feature const& feature,
          CoordTransform const& t,
          metawriter_properties const& properties);
  virtual void add_polygon(path_type & path,
          Feature const& feature,
          CoordTransform const& t,
          metawriter_properties const& properties);
  virtual void add_line(path_type & path,
          Feature const& feature,
          CoordTransform const& t,
          metawriter_properties const& properties);
  
  virtual void start(metawriter_property_map const& properties);
  virtual void stop();
  virtual void set_map_srs(projection const& proj);

  /**
   * An instance of a rendered feature. The box represents the image
   * coordinates of a bounding box around the feature. The properties
   * are the intersection of the features' properties and the "kept"
   * properties of the metawriter.
   */
  struct meta_instance {
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

  void add_vertices(path_type & path,
                    Feature const& feature,
                    CoordTransform const& t,
                    metawriter_properties const& properties);
};
  
/** Shared pointer to metawriter_inmem object. */
typedef boost::shared_ptr<metawriter_inmem> metawriter_inmem_ptr;

}

#endif /* METAWRITER_INMEM_HPP */
