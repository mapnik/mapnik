/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef SVG_RENDERER_HPP
#define SVG_RENDERER_HPP

// mapnik
#include <mapnik/feature_style_processor.hpp>

namespace mapnik 
{
    // svg_renderer isn't a template class for now, because 
    // I haven't devised an equivalent of image_32 for svg.
    class MAPNIK_DECL svg_renderer : public feature_style_processor<svg_renderer>, 
				     private boost::noncopyable
    {

    public:
	// the only parameter I'm sure of is the map.
	svg_renderer(Map const& m);
	~svg_renderer();

	void start_map_processing(Map const& map);
	void end_map_processing(Map const& map);
	void start_layer_processing(layer const& lay);
	void end_layer_processing(layer const& lay);
	void process(point_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(line_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(line_pattern_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(polygon_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(polygon_pattern_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(raster_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(shield_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(text_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(building_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(markers_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
	void process(glyph_symbolizer const& sym,
		     Feature const& feature,
		     proj_transform const& prj_trans);
    };
}

#endif //SVG_RENDERER_HPP
