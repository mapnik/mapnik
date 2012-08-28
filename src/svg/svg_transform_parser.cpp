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

// mapnik
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/svg/svg_transform_grammar.hpp>
// agg
#include "agg_trans_affine.h"
// stl
#include <string>

namespace mapnik { namespace svg {

    template <typename TransformType>
    bool parse_transform(const char * wkt, TransformType & p)
    {
        using namespace boost::spirit;
        typedef const char * iterator_type;
        typedef ascii::space_type skip_type;
        svg_transform_grammar<iterator_type,skip_type,TransformType> g(p);
        iterator_type first = wkt;
        iterator_type last =  wkt + std::strlen(wkt);
        return qi::phrase_parse(first, last, g, skip_type());
    }

/*
  template <typename TransformType>
  bool parse_transform(std::string const& wkt, TransformType & p)
  {
  using namespace boost::spirit;
  typedef std::string::const_iterator iterator_type;
  typedef ascii::space_type skip_type;
  svg_transform_grammar<iterator_type,skip_type,TransformType> g(p);
  iterator_type first = wkt.begin();
  iterator_type last =  wkt.end();
  return qi::phrase_parse(first, last, g, skip_type());
  }
*/

    template MAPNIK_DECL bool parse_transform<agg::trans_affine>(const char*, agg::trans_affine&);
//template bool parse_transform<agg::trans_affine>(std::string const& , agg::trans_affine&);

    }}
