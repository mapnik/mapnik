/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: mapnik.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef MAPNIK_HPP
#define MAPNIK_HPP

#include <map>
#include <vector>
#include <cassert>
#include "global.hpp"
#include "factory.hpp"
#include "filter.hpp"
#include "query.hpp"
#include "rule.hpp"
#include "spatial.hpp"
#include "logical.hpp"
#include "comparison.hpp"
#include "regex_filter.hpp"
#include "utils.hpp"
#include "style.hpp"
#include "symbolizer.hpp"
#include "style_cache.hpp"
#include "geometry.hpp"
#include "geom_util.hpp"
#include "raster.hpp"
#include "feature.hpp"
#include "attribute.hpp"
#include "attribute_collector.hpp"
#include "render.hpp"
#include "graphics.hpp"
#include "image_reader.hpp"
#include "line_symbolizer.hpp"
#include "polygon_symbolizer.hpp"
//#include "image_symbolizer.hpp"
#include "image_util.hpp"
#include "datasource.hpp"
#include "layer.hpp"
#include "datasource_cache.hpp"
#include "wkb.hpp"
#include "map.hpp"
#include "feature_type_style.hpp"
#include "feature_factory.hpp"
#include "math_expr.hpp"
#include "value.hpp"
#include "expression.hpp"
#include "filter_visitor.hpp"
#include "filter_parser.hpp"
#include "filter_factory.hpp"
#include "text_symbolizer.hpp"
#include "label_placement.hpp"
#include "feature_layer_desc.hpp"
#include "css_color_parser.hpp"
#include "color_factory.hpp"

namespace mapnik
{
    void save_to_xml(Map const& map,const char* filename);  
    void save_to_text(Map const& map,const char* filename);

    void load_from_xml(Map & map, const char * filename);
    void load_from_text(Map & map, const char * filename);
}

#endif //MAPNIK_HPP
