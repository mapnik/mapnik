/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef AGG_RENDERER_HPP
#define AGG_RENDERER_HPP

#include "feature_style_processor.hpp"
#include <boost/utility.hpp>
#include "font_engine_freetype.hpp"
#include "label_collision_detector.hpp"

namespace mapnik
{
    template <typename T>
    struct agg_renderer : public feature_style_processor<agg_renderer<T> >,
			  private boost::noncopyable
    {
	agg_renderer(Map const& m, T & pixmap);
	void start_map_processing(Map const& map);
	void end_map_processing(Map const& map);
	void start_layer_processing(Layer const& lay);
	void end_layer_processing(Layer const& lay);
	void process(point_symbolizer const& sym,Feature const& feature);	    	       
	void process(line_symbolizer const& sym,Feature const& feature);
	void process(line_pattern_symbolizer const& sym,Feature const& feature);
	void process(polygon_symbolizer const& sym,Feature const& feature);
	void process(polygon_pattern_symbolizer const& sym,Feature const& feature);
	void process(raster_symbolizer const& sym,Feature const& feature);
	void process(text_symbolizer const& sym,Feature const& feature);
    private:
	T & pixmap_;
	CoordTransform t_;
	face_manager<freetype_engine> font_manager_;
	label_collision_detector detector_;
    };
}

#endif //AGG_RENDERER_HPP
