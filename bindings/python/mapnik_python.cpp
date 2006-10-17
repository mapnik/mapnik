/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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
//$Id: mapnik_python.cc 27 2005-03-30 21:45:40Z pavlenko $

#include <boost/python.hpp>
#include <boost/get_pointer.hpp>
#include <boost/python/detail/api_placeholder.hpp>

void export_color();
void export_coord();
void export_layer();
void export_parameters();
void export_envelope();
void export_query();
void export_image();
void export_map();
void export_python();
void export_filter();
void export_rule();
void export_style();
void export_stroke();
void export_datasource();
void export_datasource_cache();
void export_point_symbolizer();
void export_line_symbolizer();
void export_line_pattern_symbolizer();
void export_polygon_symbolizer();
void export_polygon_pattern_symbolizer();
void export_raster_symbolizer();
void export_shield_symbolizer();
void export_text_symbolizer();
void export_font_engine();
void export_projection();

#include <mapnik/map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/save_map.hpp>

void render_to_file(const mapnik::Map& map,
                    const std::string& file,
                    const std::string& format)
{
    mapnik::Image32 image(map.getWidth(),map.getHeight());
    mapnik::agg_renderer<mapnik::Image32> ren(map,image);
    ren.apply();
    image.saveToFile(file,format);
}

void render(const mapnik::Map& map,mapnik::Image32& image)
{
    mapnik::agg_renderer<mapnik::Image32> ren(map,image);
    ren.apply();
}


BOOST_PYTHON_MODULE(_mapnik)
{
    using namespace boost::python;
    using mapnik::Featureset;
    using mapnik::featureset_ptr;
    using mapnik::datasource;
    using mapnik::coord;
    using mapnik::filter_ptr;
    using mapnik::load_map;
    using mapnik::save_map;
    
    export_query();
    
    class_<Featureset,featureset_ptr,boost::noncopyable>("FeatureSet",no_init)
      ;
    
    export_datasource();
    export_parameters();
    export_color(); 
    export_envelope();   
    export_image();
    export_filter();
    export_rule();
    export_style();    
    export_layer();
    export_stroke();
    export_datasource_cache();
    export_point_symbolizer();
    export_line_symbolizer();
    export_line_pattern_symbolizer();
    export_polygon_symbolizer();
    export_polygon_pattern_symbolizer();
    export_raster_symbolizer();
    export_shield_symbolizer();
    export_text_symbolizer();
    export_font_engine();
    export_projection();
    export_coord();
    export_map();
    
    def("render_to_file",&render_to_file);
    def("render",&render);
    
    def("load_map",&load_map,"load Map object from XML");
    def("save_map",&load_map,"sace Map object to XML");
    
    register_ptr_to_python<filter_ptr>();
}
