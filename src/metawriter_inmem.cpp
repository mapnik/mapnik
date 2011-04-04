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
 
// Mapnik
#include <mapnik/metawriter.hpp>
#include <mapnik/metawriter_inmem.hpp>
#include <mapnik/placement_finder.hpp>

// Boost
#include <boost/foreach.hpp>

using std::map;
using std::string;

namespace {

using mapnik::value;
using mapnik::Feature;
using mapnik::metawriter_properties;

// intersect a set of properties with those in the feature descriptor
map<string,value> intersect_properties(const Feature &feature, const metawriter_properties &properties) {
  const map<string, value> &fprops = feature.props();
  map<string,value> nprops;

  BOOST_FOREACH(string p, properties) {
    map<string, value>::const_iterator itr = fprops.find(p);
    if (itr != fprops.end()) {
      nprops.insert(*itr);
    }
  }

  return nprops;
}
} // end anonymous namespace

namespace mapnik {

metawriter_inmem::metawriter_inmem(metawriter_properties dflt_properties) 
  : metawriter(dflt_properties) {
  // ???
}

metawriter_inmem::~metawriter_inmem() {
}

void 
metawriter_inmem::add_box(box2d<double> const& box, Feature const& feature,
                          CoordTransform const& /*t*/,
                          metawriter_properties const& properties) {
  meta_instance inst;
  inst.box = box;
  inst.properties = intersect_properties(feature, properties);
  instances_.push_back(inst);
}

void 
metawriter_inmem::add_text(placement const& p,
                           face_set_ptr /*face*/,
                           Feature const& feature,
                           CoordTransform const& /*t*/,
                           metawriter_properties const& properties) {
  // there's more than one bbox for the text (one for each char), so keeping it
  // simple for the moment and merging them all together...
  if (p.envelopes.size() > 0) {
    // stupid queue - doesn't expose begin() and end(), so forced to iterate by
    // taking a copy...
    std::queue<box2d<double> > env_copy = p.envelopes;
    meta_instance inst;
    box2d<double> box = env_copy.front();
    while (env_copy.size() > 1) {
      env_copy.pop();
      box.expand_to_include(env_copy.front());
    }
    inst.properties = intersect_properties(feature, properties);
    inst.box = box;
    instances_.push_back(inst);
  }
}

void 
metawriter_inmem::add_polygon(path_type & path,
                              Feature const& feature,
                              CoordTransform const& t,
                              metawriter_properties const& properties) {
  add_vertices(path, feature, t, properties);
}

void 
metawriter_inmem::add_line(path_type & path,
                           Feature const& feature,
                           CoordTransform const& t,
                           metawriter_properties const& properties) {
  add_vertices(path, feature, t, properties);
}

void 
metawriter_inmem::add_vertices(path_type & path,
                               Feature const& feature,
                               CoordTransform const& /*t*/,
                               metawriter_properties const& properties) {
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

void 
metawriter_inmem::start(metawriter_property_map const& /*properties*/) {
  instances_.clear();
}

void 
metawriter_inmem::stop() {
}

void 
metawriter_inmem::set_map_srs(projection const& /*proj*/) {
  // currently unused, since the inmem metawriter keeps everything in
  // image coordinates.
}

const std::list<metawriter_inmem::meta_instance> &
metawriter_inmem::instances() const {
  return instances_;
}

metawriter_inmem::meta_instance_list::const_iterator 
metawriter_inmem::inst_begin() const {
  return instances_.begin();
}

metawriter_inmem::meta_instance_list::const_iterator 
metawriter_inmem::inst_end() const {
  return instances_.end();
}


}

