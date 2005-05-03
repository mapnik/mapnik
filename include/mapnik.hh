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

//$Id$

#ifndef MAPNIK_HH
#define MAPNIK_HH

#include <config.hh>

#include <map>
#include <vector>
#include <cassert>
#include "ptr.hh"
#include "factory.hh"
#include "filter.hh"
#include "query.hh"
#include "rule.hh"
#include "spatial.hh"
#include "logical.hh"
#include "comparison.hh"
#include "utils.hh"
#include "style.hh"
#include "symbolizer.hh"
#include "style_cache.hh"
#include "geometry.hh"
#include "geom_util.hh"
#include "raster.hh"
#include "feature.hh"
#include "attribute.hh"
#include "attribute_collector.hh"
#include "render.hh"
#include "graphics.hh"
#include "image_reader.hh"
#include "image_util.hh"
#include "datasource.hh"
#include "layer.hh"
#include "datasource_cache.hh"
#include "wkb.hh"
#include "map.hh"
#include "colorcube.hh"
#include "feature_type_style.hh"
#include "filter_visitor.hh"
#include "math_expr.hh"
#include "value.hh"
#include "expression.hh"
#include "filter_parser.hh"
#include "filter_factory.hh"

namespace mapnik
{
    //typedef geometry_type geometry_type;
}

#endif                                            //MAPNIK_HH
