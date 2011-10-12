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

#ifndef FEATURE_STYLE_PROCESSOR_HPP
#define FEATURE_STYLE_PROCESSOR_HPP

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/memory_datasource.hpp>


// stl
#include <set>
#include <string>
#include <vector>

namespace mapnik
{

class Map;
class layer;
class projection;
  
template <typename Processor>
class feature_style_processor 
{
    struct symbol_dispatch;
public:
    explicit feature_style_processor(Map const& m, double scale_factor = 1.0);

    /*!
     * @return apply renderer to all map layers.
     */
    void apply();

    /*!
     * @return apply renderer to a single layer, providing pre-populated set of query attribute names.
     */
    void apply(mapnik::layer const& lyr, std::set<std::string>& names);
private:
    /*!
     * @return initialize metawriters for a given map and projection.
     */
    void start_metawriters(Map const& m_, projection const& proj);
    /*!
     * @return stop metawriters that were previously initialized.
     */
    void stop_metawriters(Map const& m_);

    /*!
     * @return render a layer given a projection and scale.
     */
    void apply_to_layer(layer const& lay,
                        Processor & p,
                        projection const& proj0,
                        double scale_denom,
                        std::set<std::string>& names);

    /*!
     * @return renders a featureset with the given styles.
     */
    void render_style(layer const& lay,
                      Processor & p,
                      feature_type_style* style,
                      std::string const& style_name,
                      featureset_ptr features,
                      proj_transform const& prj_trans,
                      double scale_denom);

    Map const& m_;
    double scale_factor_;
};
}

#endif //FEATURE_STYLE_PROCESSOR_HPP
