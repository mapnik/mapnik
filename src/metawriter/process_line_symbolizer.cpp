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

// boost
#include <boost/foreach.hpp>
// mapnik
#include <mapnik/metawriter_renderer.hpp>
#include <mapnik/metawriter_factory.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

// stl
#include <string>
#include <cmath>

namespace mapnik {

struct metawriter_wrap
{
    metawriter_wrap(metawriter_with_properties const& w, feature_impl const& f, CoordTransform const& t)
        : writer(w),
          feature(f),
          tr(t) 
    {}
    
    template <typename T>
    void add_path(T & path)
    {        
        if (check_metawriter(writer.first)) 
        {
            std::cout << "ADD PATH " << std::endl;
            add_line(writer.first, path, feature, tr, writer.second);
        }
    }
    
    metawriter_with_properties const& writer;  
    feature_impl const& feature;
    CoordTransform const& tr;
};

void metawriter_renderer::process(line_symbolizer const& sym,
                                  mapnik::feature_ptr const& feature,
                                  proj_transform const& prj_trans)

{
    agg::trans_affine tr;
    evaluate_transform(tr, *feature, sym.get_transform());
    metawriter_with_properties writer = sym.get_metawriter();
    metawriter_wrap wrap(writer,*feature,t_);
    
    typedef boost::mpl::vector<clip_line_tag,
                               transform_tag,
                               offset_transform_tag, 
                               affine_transform_tag, 
                               smooth_tag, 
                               dash_tag, 
                               stroke_tag> conv_types;
    
    vertex_converter<box2d<double>,metawriter_wrap,line_symbolizer,CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_,wrap,sym,t_,prj_trans,tr,scale_factor_);
    
    if (sym.clip()) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    
    BOOST_FOREACH( geometry_type & geom, feature->paths())
    {
        if (geom.num_points() > 1)
        {
            converter.apply(geom);            
        }
    }
}

}
