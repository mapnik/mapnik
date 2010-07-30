/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

// mapnik
#include <mapnik/svg_renderer.hpp>

namespace mapnik
{
    template <typename T>
    void svg_renderer<T>::process(polygon_symbolizer const& sym,
				  Feature const& feature,
				  proj_transform const& prj_trans)
    {
	typedef coord_transform2<CoordTransform, geometry2d> path_type;

	for(unsigned i=0; i<feature.num_geometries(); ++i)
	{
	    geometry2d const& geom = feature.get_geometry(i);
	    if(geom.num_points() > 2)
	    {
		path_type path(t_, geom, prj_trans);
		generator_.generate_path(path, sym.get_fill());
	    }
	}
    }

    template void svg_renderer<std::ostream_iterator<char> >::process(polygon_symbolizer const& sym,
								      Feature const& feature,
								      proj_transform const& prj_trans);
}
