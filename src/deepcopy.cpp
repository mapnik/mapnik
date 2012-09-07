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
#include <mapnik/feature_type_style.hpp>
#include <mapnik/map.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/params.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/util/deepcopy.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/foreach.hpp>


namespace mapnik { namespace util {

// poor man's deepcopy implementation

    void deepcopy(Map const& map_in, Map & map_out)
    {
//  *   width_(rhs.width_),
//  *   height_(rhs.height_),
//  *   srs_(rhs.srs_),
//  *   buffer_size_(rhs.buffer_size_),
//  *   background_(rhs.background_),
//  *   background_image_(rhs.background_image_),
//  *   styles_(rhs.styles_),
//      fontsets_(rhs.fontsets_),
//  *   layers_(rhs.layers_),
//      aspectFixMode_(rhs.aspectFixMode_),
//      current_extent_(rhs.current_extent_),
//  *   maximum_extent_(rhs.maximum_extent_),
//  *   base_path_(rhs.base_path_),
//      extra_attr_(rhs.extra_attr_),
//      extra_params_(rhs.extra_params_)

        // width, height
        map_out.resize(map_in.width(), map_in.height());
        // srs
        map_out.set_srs(map_in.srs());
        // buffer_size
        map_out.set_buffer_size(map_in.buffer_size());
        // background
        boost::optional<color> background = map_in.background();
        if  (background)
        {
            map_out.set_background(*background);
        }
        // background_image
        boost::optional<std::string> background_image = map_in.background_image();
        if (background_image)
        {
            map_out.set_background_image(*background_image);
        }
        // maximum extent
        boost::optional<box2d<double> > max_extent = map_in.maximum_extent();
        if (max_extent)
        {
            map_out.set_maximum_extent(*max_extent);
        }
        // base_path
        map_out.set_base_path(map_in.base_path());

        // fontsets
        typedef std::map<std::string,font_set> fontsets;
        BOOST_FOREACH ( fontsets::value_type const& kv,map_in.fontsets())
        {
            map_out.insert_fontset(kv.first,kv.second);
        }

        BOOST_FOREACH ( layer const& lyr_in, map_in.layers())
        {
            layer lyr_out(lyr_in);
            datasource_ptr ds_in = lyr_in.datasource();
            if (ds_in)
            {
                parameters p(ds_in->params());

                // TODO : re-use datasource extent if already set.
                datasource_ptr ds_out = datasource_cache::instance().create(p);
                if (ds_out)
                {
                    lyr_out.set_datasource(ds_out);
                }
            }
            map_out.addLayer(lyr_out);
        }
        typedef std::map<std::string, feature_type_style> style_cont;
        typedef style_cont::value_type value_type;

        style_cont const& styles = map_in.styles();
        BOOST_FOREACH ( value_type const& kv, styles )
        {
            feature_type_style const& style_in = kv.second;
            feature_type_style style_out(style_in,true); // deep copy
            map_out.insert_style(kv.first, style_out);
        }

    }

    }}
