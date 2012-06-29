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
#include <mapnik/memory_datasource.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/grid/grid_renderer.hpp>

// boost
#include <boost/foreach.hpp>
#include <boost/concept_check.hpp>
//stl
#include <vector>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo_renderer.hpp>
#endif

#if defined(SVG_RENDERER)
#include <mapnik/svg_renderer.hpp>
#endif

#if defined(RENDERING_STATS)
#include <mapnik/timer.hpp>
#include <iomanip>
#include <sstream>
#endif

namespace mapnik
{

template <typename T0,typename T1> struct has_process;

template <bool>
struct process_impl
{
    template <typename T0, typename T1, typename T2, typename T3>
    static void process(T0 & ren, T1 const& sym, T2 & f, T3 const& tr)
    {
        ren.process(sym,f,tr);
    }
};

template <> // No-op specialization
struct process_impl<false>
{
    template <typename T0, typename T1, typename T2, typename T3>
    static void process(T0 & ren, T1 const& sym, T2 & f, T3 const& tr)
    {
        boost::ignore_unused_variable_warning(ren);
        boost::ignore_unused_variable_warning(f);
        boost::ignore_unused_variable_warning(tr);
#ifdef MAPNIK_DEBUG
        std::clog << "NO-OP ...\n";
#endif
    }
};

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
                     mapnik::feature_impl & f,
                     proj_transform const& prj_trans)
        : output_(output),
          f_(f),
          prj_trans_(prj_trans)  {}
    
    template <typename T>
    void operator () (T const& sym) const
    {
        process_impl<has_process<Processor,T>::value>::process(output_,sym,f_,prj_trans_);
    }
    
    Processor & output_;
    mapnik::feature_impl & f_;
    proj_transform const& prj_trans_;
};

typedef char (&no_tag)[1]; 
typedef char (&yes_tag)[2]; 

template <typename T0, typename T1, void (T0::*)(T1 const&, mapnik::feature_impl &, proj_transform const&) >
struct process_memfun_helper {}; 
    
template <typename T0, typename T1> no_tag  has_process_helper(...); 
template <typename T0, typename T1> yes_tag has_process_helper(process_memfun_helper<T0, T1, &T0::process>* p);
    
template<typename T0,typename T1> 
struct has_process
{      
    typedef typename T0::processor_impl_type processor_impl_type;
    BOOST_STATIC_CONSTANT(bool 
                          , value = sizeof(has_process_helper<processor_impl_type,T1>(0)) == sizeof(yes_tag) 
        ); 
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
            if (lyr.visible(scale_denom))
            {
                std::set<std::string> names;
                apply_to_layer(lyr, p, proj, scale_denom, names);
            }
        }

        stop_metawriters(m_);
    }
    catch (proj_init_error& ex)
    {
        MAPNIK_LOG_ERROR(feature_style_processor) << "feature_style_processor: proj_init_error=" << ex.what();
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

        if (lyr.visible(scale_denom))
        {
            apply_to_layer(lyr, p, proj, scale_denom, names);
        }
    }
    catch (proj_init_error& ex)
    {
        MAPNIK_LOG_ERROR(feature_style_processor) << "feature_style_processor: proj_init_error=" << ex.what();
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
    if (! num_styles)
    {
        MAPNIK_LOG_DEBUG(feature_style_processor) << "feature_style_processor: No style for layer=" << lay.name();

        return;
    }

    mapnik::datasource_ptr ds = lay.datasource();
    if (! ds)
    {
        MAPNIK_LOG_DEBUG(feature_style_processor) << "feature_style_processor: No datasource for layer=" << lay.name();

        return;
    }

#if defined(RENDERING_STATS)
    progress_timer layer_timer(std::clog, "rendering total for layer: '" + lay.name() + "'");
#endif

    projection proj1(lay.srs());
    proj_transform prj_trans(proj0,proj1);

#if defined(RENDERING_STATS)
    if (! prj_trans.equal())
    {
        std::clog << "notice: reprojecting layer: '" << lay.name() << "' from/to:\n\t'"
                  << lay.srs() << "'\n\t'"
                  << m_.srs() << "'\n";
    }
#endif

    box2d<double> buffered_query_ext = m_.get_buffered_extent(); // buffered

    // clip buffered extent by maximum extent, if supplied
    boost::optional<box2d<double> > const& maximum_extent = m_.maximum_extent();
    if (maximum_extent) {
        buffered_query_ext.clip(*maximum_extent);
    }

    box2d<double> layer_ext = lay.envelope();
    bool fw_success = false;

    // first, try intersection of map extent forward projected into layer srs
    if (prj_trans.forward(buffered_query_ext, PROJ_ENVELOPE_POINTS) && buffered_query_ext.intersects(layer_ext))
    {
        fw_success = true;
        layer_ext.clip(buffered_query_ext);
    }
    // if no intersection and projections are also equal, early return
    else if (prj_trans.equal())
    {
#if defined(RENDERING_STATS)
        layer_timer.discard();
#endif
        return;
    }
    // next try intersection of layer extent back projected into map srs
    else if (prj_trans.backward(layer_ext, PROJ_ENVELOPE_POINTS) && buffered_query_ext.intersects(layer_ext))
    {
        layer_ext.clip(buffered_query_ext);
        // forward project layer extent back into native projection
        if (! prj_trans.forward(layer_ext, PROJ_ENVELOPE_POINTS))
        {
            MAPNIK_LOG_DEBUG(feature_style_processor)
                    << "feature_style_processor: Layer=" << lay.name()
                    << " extent=" << layer_ext << " in map projection "
                    << " did not reproject properly back to layer projection";
        }
    }
    else
    {
        // if no intersection then nothing to do for layer
#if defined(RENDERING_STATS)
        layer_timer.discard();
#endif
        return;
    }

    // if we've got this far, now prepare the unbuffered extent
    // which is used as a bbox for clipping geometries
    box2d<double> query_ext = m_.get_current_extent(); // unbuffered
    if (maximum_extent) {
        query_ext.clip(*maximum_extent);
    }
    box2d<double> layer_ext2 = lay.envelope();
    if (fw_success)
    {
        if (prj_trans.forward(query_ext, PROJ_ENVELOPE_POINTS))
        {
            layer_ext2.clip(query_ext);
        }
    }
    else
    {
        if (prj_trans.backward(layer_ext2, PROJ_ENVELOPE_POINTS))
        {
            layer_ext2.clip(query_ext);
            prj_trans.forward(layer_ext2, PROJ_ENVELOPE_POINTS);
        }
    }

    p.start_layer_processing(lay, layer_ext2);

    double qw = query_ext.width()>0 ? query_ext.width() : 1;
    double qh = query_ext.height()>0 ? query_ext.height() : 1;
    query::resolution_type res(m_.width()/qw,
                               m_.height()/qh);

    query q(layer_ext,res,scale_denom,m_.get_current_extent());
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
            MAPNIK_LOG_DEBUG(feature_style_processor)
                    << "feature_style_processor: Style=" << style_name
                    << " required for layer=" << lay.name() << " does not exist.";

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

        // Update filter_factor for all enabled raster layers.
        BOOST_FOREACH (feature_type_style * style, active_styles)
        {
            BOOST_FOREACH(rule const& r, style->get_rules())
            {
                if (r.active(scale_denom) &&
                    ds->type() == datasource::Raster &&
                    ds->params().get<double>("filter_factor",0.0) == 0.0)
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

        // Also query the group by attribute
        std::string group_by = lay.group_by();
        if (group_by != "")
        {
            q.add_property_name(group_by);
        }

        bool cache_features = lay.cache_features() && active_styles.size() > 1;

        // Render incrementally when the column that we group by
        // changes value.
        if (group_by != "")
        {
            featureset_ptr features = ds->features(q);
            if (features) {
                // Cache all features into the memory_datasource before rendering.
                memory_datasource cache;
                feature_ptr feature, prev;

                while ((feature = features->next()))
                {
                    if (prev && prev->get(group_by) != feature->get(group_by))
                    {
                        // We're at a value boundary, so render what we have
                        // up to this point.
                        int i = 0;
                        BOOST_FOREACH (feature_type_style * style, active_styles)
                        {
                            render_style(lay, p, style, style_names[i++],
                                         cache.features(q), prj_trans, scale_denom);
                        }
                        cache.clear();
                    }
                    cache.push(feature);
                    prev = feature;
                }

                int i = 0;
                BOOST_FOREACH (feature_type_style * style, active_styles)
                {
                    render_style(lay, p, style, style_names[i++],
                                 cache.features(q), prj_trans, scale_denom);
                }
            }
        }
        else if (cache_features)
        {
            featureset_ptr features = ds->features(q);
            if (features) {
                // Cache all features into the memory_datasource before rendering.
                memory_datasource cache;
                feature_ptr feature;
                while ((feature = features->next()))
                {
                    cache.push(feature);
                }

                int i = 0;
                BOOST_FOREACH (feature_type_style * style, active_styles)
                {
                    render_style(lay, p, style, style_names[i++],
                                 cache.features(q), prj_trans, scale_denom);
                }
            }
        }
        // We only have a single style and no grouping.
        else
        {
            int i = 0;
            BOOST_FOREACH (feature_type_style * style, active_styles)
            {
                featureset_ptr features = ds->features(q);
                if (features) {
                    render_style(lay, p, style, style_names[i++],
                                 features, prj_trans, scale_denom);
                }
            }
        }
    }

#if defined(RENDERING_STATS)
    layer_timer.stop();
#endif

    p.end_layer_processing(lay);
}


template <typename Processor>
void feature_style_processor<Processor>::render_style(
    layer const& lay,
    Processor & p,
    feature_type_style* style,
    std::string const& style_name,
    featureset_ptr features,
    proj_transform const& prj_trans,
    double scale_denom)
{

    p.start_style_processing(*style);
    
#if defined(RENDERING_STATS)
    std::ostringstream s1;
    s1 << "rendering style for layer: '" << lay.name()
       << "' and style '" << style_name << "'";
    mapnik::progress_timer style_timer(std::clog, s1.str());

    int feature_processed_count = 0;
    int feature_count = 0;
#endif

    feature_ptr feature;
    while ((feature = features->next()))
    {
#if defined(RENDERING_STATS)
        feature_count++;
        bool feat_processed = false;
#endif

        bool do_else = true;
        bool do_also = false;

        BOOST_FOREACH(rule * r, style->get_if_rules(scale_denom) )
        {
            expression_ptr const& expr=r->get_filter();
            value_type result = boost::apply_visitor(evaluate<Feature,value_type>(*feature),*expr);
            if (result.to_bool())
            {
#if defined(RENDERING_STATS)
                feat_processed = true;
#endif

                p.painted(true);

                do_else=false;
                do_also=true;
                rule::symbolizers const& symbols = r->get_symbolizers();

                // if the underlying renderer is not able to process the complete set of symbolizers,
                // process one by one.
                if(!p.process(symbols,*feature,prj_trans))
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
            BOOST_FOREACH( rule * r, style->get_else_rules(scale_denom) )
            {
#if defined(RENDERING_STATS)
                feat_processed = true;
#endif

                p.painted(true);

                rule::symbolizers const& symbols = r->get_symbolizers();
                // if the underlying renderer is not able to process the complete set of symbolizers,
                // process one by one.
                if(!p.process(symbols,*feature,prj_trans))
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
            BOOST_FOREACH( rule * r, style->get_also_rules(scale_denom) )
            {
#if defined(RENDERING_STATS)
                feat_processed = true;
#endif

                p.painted(true);

                rule::symbolizers const& symbols = r->get_symbolizers();
                // if the underlying renderer is not able to process the complete set of symbolizers,
                // process one by one.
                if(!p.process(symbols,*feature,prj_trans))
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

    // done with style
    std::ostringstream s;
    if (feature_count > 0)
    {
        double perc_processed = ((double)feature_processed_count/(double)feature_count)*100.0;

        s << "percent rendered: " << perc_processed << "% - " << feature_processed_count
          << " rendered for " << feature_count << " queried for ";
        s << std::setw(15 - (int)s.tellp()) << " layer '" << lay.name() << "' and style '" << style_name << "'\n";
    }
    else
    {
        s << "" << std::setw(15) << "- no features returned from query for layer '" << lay.name() << "' and style '" << style_name << "'\n";
    }
    std::clog << s.str();
    style_timer.discard();
#endif
    p.end_style_processing(*style);
}


#if defined(HAVE_CAIRO)
template class feature_style_processor<cairo_renderer<Cairo::Context> >;
template class feature_style_processor<cairo_renderer<Cairo::Surface> >;
#endif

#if defined(SVG_RENDERER)
template class feature_style_processor<svg_renderer<std::ostream_iterator<char> > >;
#endif

template class feature_style_processor<grid_renderer<grid> >;
template class feature_style_processor<agg_renderer<image_32> >;

}

