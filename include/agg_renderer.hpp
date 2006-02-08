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

namespace mapnik
{
    struct agg_renderer : public feature_style_processor<agg_renderer>,
			  private boost::noncopyable
    {
	agg_renderer(Map const& m, Image32 & pixmap);
	void process(point_symbolizer const& sym,Feature const& feature);	    	       
	void process(line_symbolizer const& sym,Feature const& feature);
	void process(line_pattern_symbolizer const& sym,Feature const& feature);
	void process(polygon_symbolizer const& sym,Feature const& feature);
	void process(polygon_pattern_symbolizer const& sym,Feature const& feature);
	void process(raster_symbolizer const& sym,Feature const& feature);
    private:
	Image32 & pixmap_;
	CoordTransform t_;
    };
}

#endif //AGG_RENDERER_HPP
