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

//mapnik
#include <mapnik/stats_processor.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/scale_denominator.hpp>

// boost
#include <boost/foreach.hpp>

//stl
#include <vector>

#include <mapnik/timer.hpp>
#include <iomanip>
#include <sstream>

namespace mapnik
{


stats_processor::stats_processor(Map const& m)
    : m_(m),
      tree(),
      s(new stats())
    {}

void stats_processor::apply()
{

    mapnik::timer t;
    projection proj(m_.srs());

    double scale_denom = mapnik::scale_denominator(m_,proj.is_geographic());

    BOOST_FOREACH ( layer const& lyr, m_.layers() )
    {
        if (lyr.isVisible(scale_denom))
        {
            std::set<std::string> names;
            apply_to_layer(lyr, proj, scale_denom, names);
        }
    }
    //tree.put("wall_clock_elapsed", t.wall_clock_elapsed());
    //tree.put("cpu_elapsed", t.cpu_elapsed());
}

void stats_processor::apply_to_layer(layer const& lay, projection const& proj0,
                                                        double scale_denom,
                                                        std::set<std::string>& names)
{
    boost::property_tree::ptree ltree;
    mapnik::timer t;

    std::vector<std::string> const& style_names = lay.styles();

    unsigned int num_styles = style_names.size();
    if (!num_styles) {
        std::clog << "WARNING: No style for layer '" << lay.name() << "'\n";
        return;
    }

    mapnik::datasource_ptr ds = lay.datasource();
    if (!ds)
    {
        std::clog << "WARNING: No datasource for layer '" << lay.name() << "'\n";
        return;
    }

    projection proj1(lay.srs());
    proj_transform prj_trans(proj0,proj1);

    ltree.put("reprojected",!prj_trans.equal());

    box2d<double> map_ext = m_.get_buffered_extent();

    // clip buffered extent by maximum extent, if supplied
    boost::optional<box2d<double> > const& maximum_extent = m_.maximum_extent();
    if (maximum_extent) {
        map_ext.clip(*maximum_extent);
    }

    box2d<double> layer_ext = lay.envelope();

    // first, try intersection of map extent forward projected into layer srs
    if (prj_trans.forward(map_ext, PROJ_ENVELOPE_POINTS) && map_ext.intersects(layer_ext))
    {
        layer_ext.clip(map_ext);
    }
    // if no intersection and projections are also equal, early return
    else if (prj_trans.equal())
    {
        return;
    }
    // next try intersection of layer extent back projected into map srs
    else if (prj_trans.backward(layer_ext, PROJ_ENVELOPE_POINTS) && map_ext.intersects(layer_ext))
    {
        layer_ext.clip(map_ext);
        // forward project layer extent back into native projection
        if (!prj_trans.forward(layer_ext, PROJ_ENVELOPE_POINTS))
            std::clog << "WARNING: layer " << lay.name()
                      << " extent " << layer_ext << " in map projection "
                      << " did not reproject properly back to layer projection\n";
    }
    else
    {
        return;
    }

    ltree.put("clipped_extent",layer_ext);

    box2d<double> query_ext = m_.get_current_extent();
    box2d<double> unbuffered_extent = m_.get_current_extent();
    prj_trans.forward(query_ext, PROJ_ENVELOPE_POINTS);
    query::resolution_type res(m_.width()/query_ext.width(),
                               m_.height()/query_ext.height());

    query q(layer_ext,res,scale_denom,unbuffered_extent);

    std::vector<feature_type_style*> active_styles;
    attribute_collector collector(names);
    double filt_factor = 1;
    directive_collector d_collector(&filt_factor);

    // iterate through all named styles collecting active styles and attribute names
    BOOST_FOREACH(std::string const& style_name, style_names)
    {
        boost::optional<feature_type_style const&> style=m_.find_style(style_name);
        if (!style)
        {
            std::clog << "WARNING: style '" << style_name << "' required for layer '"
                      << lay.name() << "' does not exist.\n";
            continue;
        }

        const std::vector<rule>& rules=(*style).get_rules();
        bool active_rules=false;

        BOOST_FOREACH(rule const& r, rules)
        {
            if (r.active(scale_denom))
            {
                active_rules = true;
                if (ds->type() == datasource::Vector)
                {
                    collector(r);
                }
                // TODO - in the future rasters should be able to be filtered.
            }
        }
        if (active_rules)
        {
            active_styles.push_back(const_cast<feature_type_style*>(&(*style)));
        }
    }

    // Don't even try to do more work if there are no active styles.
    if (active_styles.size() > 0)
    {
        // push all property names
        BOOST_FOREACH(std::string const& name, names)
        {
            q.add_property_name(name);
        }

        int i = 0;
        BOOST_FOREACH (feature_type_style * style, active_styles)
        {
            boost::property_tree::ptree stree;
            stree.put("name",style_names[i++]);
            featureset_ptr features = ds->features(q);
            if (features) {
                feature_ptr feature;
                while ((feature = features->next()))
                {
                    BOOST_FOREACH(rule * r, style->get_if_rules(scale_denom) )
                    {
                        expression_ptr const& expr=r->get_filter();
                        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(*feature),*expr);
                        if (result.to_bool())
                        {
                        // todo
                        }
                    }
                }
            }
            // http://stackoverflow.com/questions/2114466/creating-json-arrays-in-boost-using-property-trees
            ltree.push_back( std::make_pair("", stree ) );
        }
    }

    ltree.put("wall_clock_elapsed", t.wall_clock_elapsed());
    ltree.put("cpu_elapsed", t.cpu_elapsed());

    tree.put_child(lay.name(),ltree);
}

}
