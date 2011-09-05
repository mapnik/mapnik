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
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/memory_datasource.hpp>

#include <mapnik/agg_renderer.hpp>
#include <mapnik/grid/grid_renderer.hpp>

// boost
#include <boost/foreach.hpp>

//stl
#include <vector>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo_renderer.hpp>
#endif

#if defined(RENDERING_STATS)
#include <mapnik/timer.hpp>
#include <iomanip>
#include <sstream>
#endif

namespace mapnik
{

/** Calls the renderer's process function,
  * \param output     Renderer
  * \param f          Feature to process
  * \param prj_trans  Projection
  * \param sym        Symbolizer object
  */
template <typename Processor>
struct feature_style_processor<Processor>::symbol_dispatch : public boost::static_visitor<>
{
    symbol_dispatch (Processor & output,
                     Feature const& f,
                     proj_transform const& prj_trans)
        : output_(output),
          f_(f),
          prj_trans_(prj_trans)  {}

    template <typename T>
    void operator () (T const& sym) const
    {
        output_.process(sym,f_,prj_trans_);
    }

    Processor & output_;
    Feature const& f_;
    proj_transform const& prj_trans_;
};

template <typename Processor>
feature_style_processor<Processor>::feature_style_processor(Map const& m, double scale_factor)
    : m_(m), scale_factor_(scale_factor)
{
}

template <typename Processor>
void feature_style_processor<Processor>::apply()
{

#if defined(RENDERING_STATS)
    std::clog << "\n//-- starting rendering timer...\n";
    mapnik::progress_timer t(std::clog, "total map rendering");
#endif

    Processor & p = static_cast<Processor&>(*this);
    p.start_map_processing(m_);

    try
    {
        projection proj(m_.srs());

        start_metawriters(m_,proj);

        double scale_denom = mapnik::scale_denominator(m_,proj.is_geographic());
        scale_denom *= scale_factor_;

        BOOST_FOREACH ( layer const& lyr, m_.layers() )
        {
            if (lyr.isVisible(scale_denom))
            {
                std::set<std::string> names;
                apply_to_layer(lyr, p, proj, scale_denom, names);
            }
        }

        stop_metawriters(m_);
    }
    catch (proj_init_error& ex)
    {
        std::clog << "proj_init_error:" << ex.what() << "\n";
    }

    p.end_map_processing(m_);
    
#if defined(RENDERING_STATS)
    t.stop();
    std::clog << "//-- rendering timer stopped...\n\n";
#endif

}

template <typename Processor>
void feature_style_processor<Processor>::apply(mapnik::layer const& lyr, std::set<std::string>& names)
{
    Processor & p = static_cast<Processor&>(*this);
    p.start_map_processing(m_);
    try
    {
        projection proj(m_.srs());
        double scale_denom = mapnik::scale_denominator(m_,proj.is_geographic());
        scale_denom *= scale_factor_;

        if (lyr.isVisible(scale_denom))
        {
            apply_to_layer(lyr, p, proj, scale_denom, names);
        }
    }
    catch (proj_init_error& ex)
    {
        std::clog << "proj_init_error:" << ex.what() << "\n";
    }
    p.end_map_processing(m_);
}

template <typename Processor>
void feature_style_processor<Processor>::start_metawriters(Map const& m_, projection const& proj)
{
    Map::const_metawriter_iterator metaItr = m_.begin_metawriters();
    Map::const_metawriter_iterator metaItrEnd = m_.end_metawriters();
    for (;metaItr!=metaItrEnd; ++metaItr)
    {
        metaItr->second->set_size(m_.width(), m_.height());
        metaItr->second->set_map_srs(proj);
        metaItr->second->start(m_.metawriter_output_properties);
    }
}

template <typename Processor>
void feature_style_processor<Processor>::stop_metawriters(Map const& m_)
{
    Map::const_metawriter_iterator metaItr = m_.begin_metawriters();
    Map::const_metawriter_iterator metaItrEnd = m_.end_metawriters();
    for (;metaItr!=metaItrEnd; ++metaItr)
    {
        metaItr->second->stop();
    }
}

template <typename Processor>
void feature_style_processor<Processor>::apply_to_layer(layer const& lay, Processor & p,
                    projection const& proj0,
                    double scale_denom,
                    std::set<std::string>& names)
{
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

    p.start_layer_processing(lay);

#if defined(RENDERING_STATS)
    progress_timer layer_timer(std::clog, "rendering total for layer: '" + lay.name() + "'");
#endif

    projection proj1(lay.srs());
    proj_transform prj_trans(proj0,proj1);

    // todo: only display raster if src and dest proj are matched
    // todo: add raster re-projection as an optional feature
    if (ds->type() == datasource::Raster && !prj_trans.equal())
    {
        std::clog << "WARNING: Map srs does not match layer srs, skipping raster layer '" << lay.name()
                  << "' as raster re-projection is not currently supported (http://trac.mapnik.org/ticket/663)\n"
                  << "map srs: '" << m_.srs() << "'\nlayer srs: '" << lay.srs() << "' \n";
        return;
    }
    
#if defined(RENDERING_STATS)
    if (!prj_trans.equal())
        std::clog << "notice: reprojecting layer: '" << lay.name() << "' from/to:\n\t'" 
            << lay.srs() << "'\n\t'"
            << m_.srs() << "'\n";
#endif

    box2d<double> map_ext = m_.get_buffered_extent();

    // clip buffered extent by maximum extent, if supplied
    boost::optional<box2d<double> > const& maximum_extent = m_.maximum_extent();
    if (maximum_extent) {
        map_ext.clip(*maximum_extent);
    }

    box2d<double> layer_ext = lay.envelope();

    // first, try intersection of map extent forward projected into layer srs
    if (prj_trans.forward(map_ext) && map_ext.intersects(layer_ext))
    {
        layer_ext.clip(map_ext);
    }
    // if no intersection and projections are also equal, early return
    else if (prj_trans.equal())
    {
        return;
    }
    // next try intersection of layer extent back projected into map srs
    else if (prj_trans.backward(layer_ext) && map_ext.intersects(layer_ext))
    {
        layer_ext.clip(map_ext);
        // forward project layer extent back into native projection
        if (!prj_trans.forward(layer_ext))
            std::clog << "WARNING: layer " << lay.name()
                      << " extent " << layer_ext << " in map projection "
                      << " did not reproject properly back to layer projection\n";
    }
    else
    {
        // if no intersection then nothing to do for layer
        #if defined(RENDERING_STATS)
        layer_timer.discard();
        #endif
        return;
    }

    query::resolution_type res(m_.width()/m_.get_current_extent().width(),
                               m_.height()/m_.get_current_extent().height());
    query q(layer_ext,res,scale_denom); //BBOX query

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

    // push all property names
    BOOST_FOREACH(std::string const& name, names)
    {
        q.add_property_name(name);
    }

    memory_datasource cache;
    bool cache_features = lay.cache_features() && num_styles>1?true:false;
    bool first = true;

    #if defined(RENDERING_STATS)
    int style_index = 0;
    if (!active_styles.size() > 0) {
        layer_timer.discard();
    }
    #endif
    BOOST_FOREACH (feature_type_style * style, active_styles)
    {
        #if defined(RENDERING_STATS)
        std::string s_name = style_names[style_index];
        std::ostringstream s1;
        s1 << "rendering style #" << style_index+1
          << " for layer: '" << lay.name() << "' and style '" << s_name << "'";
        mapnik::progress_timer style_timer(std::clog, s1.str());
        if (!num_styles>1)
            style_timer.discard();
        style_index++;
        #endif

        std::vector<rule*> if_rules;
        std::vector<rule*> else_rules;
        std::vector<rule*> also_rules;

        std::vector<rule> const& rules=style->get_rules();

        #if defined(RENDERING_STATS)
        int feature_count = 0;
        int feature_processed_count = 0;
        #endif

        BOOST_FOREACH(rule const& r, rules)
        {
            if (r.active(scale_denom))
            {
                if (r.has_else_filter())
                {
                    else_rules.push_back(const_cast<rule*>(&r));
                }
                else if (r.has_also_filter())
                {
                    also_rules.push_back(const_cast<rule*>(&r));
                }
                else
                {
                    if_rules.push_back(const_cast<rule*>(&r));
                }

                if ( (ds->type() == datasource::Raster) &&
                        (ds->params().get<double>("filter_factor",0.0) == 0.0) )
                {
                    rule::symbolizers const& symbols = r.get_symbolizers();
                    rule::symbolizers::const_iterator symIter = symbols.begin();
                    rule::symbolizers::const_iterator symEnd = symbols.end();
                    while (symIter != symEnd)
                    {
                        // if multiple raster symbolizers, last will be respected
                        // should we warn or throw?
                        boost::apply_visitor(d_collector,*symIter++);
                    }
                    q.set_filter_factor(filt_factor);
                }
            }
        }

        // process features
        featureset_ptr fs;
        if (first)
        {
            if (cache_features)
                first = false;
            fs = ds->features(q);
        }
        else
        {
            fs = cache.features(q);
        }

        if (fs)
        {
            feature_ptr feature;
            while ((feature = fs->next()))
            {

                #if defined(RENDERING_STATS)
                feature_count++;
                bool feat_processed = false;
                #endif

                bool do_else=true;
                bool do_also=false;

                if (cache_features)
                {
                    cache.push(feature);
                }

                BOOST_FOREACH(rule * r, if_rules )
                {
                    expression_ptr const& expr=r->get_filter();
                    value_type result = boost::apply_visitor(evaluate<Feature,value_type>(*feature),*expr);
                    if (result.to_bool())
                    {
                        #if defined(RENDERING_STATS)
                        feat_processed = true;
                        #endif

                        do_else=false;
                        do_also=true;
                        rule::symbolizers const& symbols = r->get_symbolizers();

                        // if the underlying renderer is not able to process the complete set of symbolizers,
                        // process one by one.
#ifdef SVG_RENDERER
                        if(!p.process(symbols,*feature,prj_trans))
#endif
                        {

                            BOOST_FOREACH (symbolizer const& sym, symbols)
                            {
                                boost::apply_visitor(symbol_dispatch(p,*feature,prj_trans),sym);
                            }
                        }
                        if (style->get_filter_mode() == FILTER_FIRST)
                        {
                            // Stop iterating over rules and proceed with next feature.
                            break;
                        }
                    }
                }
                if (do_else)
                {
                    BOOST_FOREACH( rule * r, else_rules )
                    {
                        #if defined(RENDERING_STATS)
                        feat_processed = true;
                        #endif
 
                        rule::symbolizers const& symbols = r->get_symbolizers();
                        // if the underlying renderer is not able to process the complete set of symbolizers,
                        // process one by one.
#ifdef SVG_RENDERER
                        if(!p.process(symbols,*feature,prj_trans))
#endif
                        {
                            BOOST_FOREACH (symbolizer const& sym, symbols)
                            {
                                boost::apply_visitor(symbol_dispatch(p,*feature,prj_trans),sym);
                            }
                        }
                    }
                }
                if (do_also)
                {
                    BOOST_FOREACH( rule * r, also_rules )
                    {
                        #if defined(RENDERING_STATS)
                        feat_processed = true;
                        #endif
 
                        rule::symbolizers const& symbols = r->get_symbolizers();
                        // if the underlying renderer is not able to process the complete set of symbolizers,
                        // process one by one.
#ifdef SVG_RENDERER
                        if(!p.process(symbols,*feature,prj_trans))
#endif
                        {
                            BOOST_FOREACH (symbolizer const& sym, symbols)
                            {
                                boost::apply_visitor(symbol_dispatch(p,*feature,prj_trans),sym);
                            }
                        }
                    }
                }
                #if defined(RENDERING_STATS)
                if (feat_processed)
                    feature_processed_count++;
                #endif
            }

            #if defined(RENDERING_STATS)
            style_timer.stop();
            layer_timer.stop();
            
            // done with style
            std::ostringstream s;
            if (feature_count > 0) {
                double perc_processed = ((double)feature_processed_count/(double)feature_count)*100.0;
                
                s << "percent rendered: " << perc_processed << "% - " << feature_processed_count 
                  << " rendered for " << feature_count << " queried for ";
                s << std::setw(15 - (int)s.tellp()) << " layer '" << lay.name() << "' and style '" << s_name << "'\n";
        
            } else {
                s << "" << std::setw(15) << "- no features returned from query for layer '" << lay.name() << "' and style '" << s_name << "'\n";
            }
            std::clog << s.str();
            #endif

        }
        #if defined(RENDERING_STATS)
        else {
            style_timer.discard();
            layer_timer.discard();
        }
        #endif
        cache_features = false;
    }

    p.end_layer_processing(lay);
}

#if defined(HAVE_CAIRO)
template class feature_style_processor<cairo_renderer<Cairo::Context> >;
template class feature_style_processor<cairo_renderer<Cairo::Surface> >;
#endif

template class feature_style_processor<grid_renderer<grid> >;
template class feature_style_processor<agg_renderer<image_32> >;

}

