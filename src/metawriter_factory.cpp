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
//$Id$

#include <mapnik/metawriter_factory.hpp>
#include <mapnik/ptree_helpers.hpp>

#include <mapnik/metawriter_json.hpp>
#include <mapnik/metawriter_inmem.hpp>

#include <boost/optional.hpp>

using boost::property_tree::ptree;
using boost::optional;
using std::string;

namespace mapnik
{

metawriter_ptr
metawriter_create(const boost::property_tree::ptree &pt) {
  metawriter_ptr writer;
  string type = get_attr<string>(pt, "type");

  optional<string> properties = get_opt_attr<string>(pt, "default-output");
  if (type == "json") {
    string file = get_attr<string>(pt, "file");
    metawriter_json_ptr json = metawriter_json_ptr(new metawriter_json(properties, parse_path(file)));
    optional<boolean> output_empty = get_opt_attr<boolean>(pt, "output-empty");
    if (output_empty) {
      json->set_output_empty(*output_empty);
    }

    optional<boolean> pixel_coordinates = get_opt_attr<boolean>(pt, "pixel-coordinates");
    if (pixel_coordinates) {
      json->set_pixel_coordinates(*pixel_coordinates);
    }
    writer = json;
    
  } else if (type == "inmem") {
    metawriter_inmem_ptr inmem = metawriter_inmem_ptr(new metawriter_inmem(properties));
    writer = inmem;
  } else {
    throw config_error(string("Unknown type '") + type + "'");
  }

  return writer;
}

void 
metawriter_save(const metawriter_ptr &metawriter, ptree &metawriter_node, bool explicit_defaults) {
  
  metawriter_json *json = dynamic_cast<metawriter_json *>(metawriter.get());
  if (json) {
    set_attr(metawriter_node, "type", "json");
    std::string const& filename = path_processor_type::to_string(*(json->get_filename()));
    if (!filename.empty() || explicit_defaults) {
      set_attr(metawriter_node, "file", filename);
    }
  }
  
  metawriter_inmem *inmem = dynamic_cast<metawriter_inmem *>(metawriter.get());
  if (inmem) {
    set_attr(metawriter_node, "type", "inmem");
  }
  
  if (!metawriter->get_default_properties().empty() || explicit_defaults) {
    set_attr(metawriter_node, "default-output", metawriter->get_default_properties().to_string());
  }
}

} // namespace mapnik
