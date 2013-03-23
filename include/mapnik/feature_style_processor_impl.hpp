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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard. To
//    create a custom feature_style_processor, include this file and instantiate
//    the template with the desired template arguments.

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/rule_cache.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>

// boost
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/foreach.hpp>
#include <boost/concept_check.hpp>

// stl
#include <vector>


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
    : m_(m),
      scale_factor_(scale_factor)
{
    // https://github.com/mapnik/mapnik/issues/1100
    if (scale_factor_ <= 0)
    {
        throw std::runtime_error("scale_factor must be greater than 0.0");
    }
}

template <typename Processor>
void feature_style_processor<Processor>::apply(double scale_denom)
{
#if defined(RENDERING_STATS)
    std::clog << "\n//-- starting rendering timer...\n";
    mapnik::progress_timer t(std::clog, "total map rendering");
#endif

    Processor & p = static_cast<Processor&>(*this);
    p.start_map_processing(m_);

    try
    {
        projection proj(m_.srs(),true);
        if (scale_denom <= 0.0)
            scale_denom = mapnik::scale_denominator(m_.scale(),proj.is_geographic());
        scale_denom *= scale_factor_;

        BOOST_FOREACH ( layer const& lyr, m_.layers() )
        {
            if (lyr.visible(scale_denom))
            {
                std::set<std::string> names;
                apply_to_layer(lyr,
                               p,
                               proj,
                               m_.scale(),
                               scale_denom,
                               m_.width(),
                               m_.height(),
                               m_.get_current_extent(),
                               m_.buffer_size(),
                               names);

            }
        }
    }
    catch (proj_init_error const& ex)
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
void feature_style_processor<Processor>::apply(mapnik::layer const& lyr,
                                               std::set<std::string>& names,
                                               double scale_denom)
{
    Processor & p = static_cast<Processor&>(*this);
    p.start_map_processing(m_);
    try
    {
        projection proj(m_.srs(),true);
        if (scale_denom <= 0.0)
            scale_denom = mapnik::scale_denominator(m_.scale(),proj.is_geographic());
        scale_denom *= scale_factor_;

        if (lyr.visible(scale_denom))
        {
            apply_to_layer(lyr,
                           p,
                           proj,
                           m_.scale(),
                           scale_denom,
                           m_.width(),
                           m_.height(),
                           m_.get_current_extent(),
                           m_.buffer_size(),
                           names);
        }
    }
    catch (proj_init_error const& ex)
    {
        MAPNIK_LOG_ERROR(feature_style_processor) << "feature_style_processor: proj_init_error=" << ex.what();
    }
    p.end_map_processing(m_);
}

template <typename Processor>
void feature_style_processor<Processor>::apply_to_layer(layer const& lay, Processor & p,
                                                        projection const& proj0,
                                                        double scale,
                                                        double scale_denom,
                                                        unsigned width,
                                                        unsigned height,
                                                        box2d<double> const& extent,
                                                        int buffer_size,
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

    projection proj1(lay.srs(),true);
    proj_transform prj_trans(proj0,proj1);

#if defined(RENDERING_STATS)
    if (! prj_trans.equal())
    {
        std::clog << "notice: reprojecting layer: '" << lay.name() << "' from/to:\n\t'"
                  << lay.srs() << "'\n\t'"
                  << m_.srs() << "'\n";
    }
#endif


    box2d<double> query_ext = extent; // unbuffered
    box2d<double> buffered_query_ext(query_ext);  // buffered

    double buffer_padding = 2.0 * scale;
    boost::optional<int> layer_buffer_size = lay.buffer_size();
    if (layer_buffer_size) // if layer overrides buffer size, use this value to compute buffered extent
    {
        buffer_padding *= *layer_buffer_size;
    }
    else
    {
        buffer_padding *= buffer_size;
    }
    buffered_query_ext.width(query_ext.width() + buffer_padding);
    buffered_query_ext.height(query_ext.height() + buffer_padding);

    // clip buffered extent by maximum extent, if supplied
    boost::optional<box2d<double> > const& maximum_extent = m_.maximum_extent();
    if (maximum_extent) {
        buffered_query_ext.clip(*maximum_extent);
    }

    box2d<double> layer_ext = lay.envelope();
    bool fw_success = false;
    bool early_return = false;

    // first, try intersection of map extent forward projected into layer srs
    if (prj_trans.forward(buffered_query_ext, PROJ_ENVELOPE_POINTS) && buffered_query_ext.intersects(layer_ext))
    {
        fw_success = true;
        layer_ext.clip(buffered_query_ext);
    }
    // if no intersection and projections are also equal, early return
    else if (prj_trans.equal())
    {
        early_return = true;
    }
    // next try intersection of layer extent back projected into map srs
    else if (prj_trans.backward(layer_ext, PROJ_ENVELOPE_POINTS) && buffered_query_ext.intersects(layer_ext))
    {
        layer_ext.clip(buffered_query_ext);
        // forward project layer extent back into native projection
        if (! prj_trans.forward(layer_ext, PROJ_ENVELOPE_POINTS))
        {
            MAPNIK_LOG_ERROR(feature_style_processor)
                    << "feature_style_processor: Layer=" << lay.name()
                    << " extent=" << layer_ext << " in map projection "
                    << " did not reproject properly back to layer projection";
        }
    }
    else
    {
        // if no intersection then nothing to do for layer
        early_return = true;
    }

    if (early_return)
    {
        // check for styles needing compositing operations applied
        // https://github.com/mapnik/mapnik/issues/1477
        BOOST_FOREACH(std::string const& style_name, style_names)
        {
            boost::optional<feature_type_style const&> style=m_.find_style(style_name);
            if (!style)
            {
                continue;
            }
            if (style->comp_op() || style->image_filters().size() > 0)
            {
                if (style->active(scale_denom))
                {
                    // trigger any needed compositing ops
                    p.start_style_processing(*style);
                    p.end_style_processing(*style);
                }
            }
        }
#if defined(RENDERING_STATS)
        layer_timer.discard();
#endif
        return;
    }

    // if we've got this far, now prepare the unbuffered extent
    // which is used as a bbox for clipping geometries
    if (maximum_extent)
    {
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
    query::resolution_type res(width/qw,
                               height/qh);

    query q(layer_ext,res,scale_denom,extent);
    std::vector<feature_type_style const*> active_styles;
    attribute_collector collector(names);
    double filt_factor = 1.0;
    directive_collector d_collector(filt_factor);
    boost::ptr_vector<rule_cache> rule_caches;

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

        std::vector<rule> const& rules = style->get_rules();
        bool active_rules = false;
        std::auto_ptr<rule_cache> rc(new rule_cache);
        BOOST_FOREACH(rule const& r, rules)
        {
            if (r.active(scale_denom))
            {
                rc->add_rule(r);
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
            rule_caches.push_back(rc);
            active_styles.push_back(&(*style));
        }
    }

    // Don't even try to do more work if there are no active styles.
    if (active_styles.size() > 0)
    {
        if (p.attribute_collection_policy() == COLLECT_ALL)
        {
            layer_descriptor lay_desc = ds->get_descriptor();
            BOOST_FOREACH(attribute_descriptor const& desc, lay_desc.get_descriptors())
            {
                q.add_property_name(desc.get_name());
            }
        }
        else
        {
            BOOST_FOREACH(std::string const& name, names)
            {
                q.add_property_name(name);
            }
        }

        // Update filter_factor for all enabled raster layers.
        BOOST_FOREACH (feature_type_style const* style, active_styles)
        {
            BOOST_FOREACH(rule const& r, style->get_rules())
            {
                if (r.active(scale_denom) &&
                    ds->type() == datasource::Raster &&
                    ds->params().get<double>("filter_factor",0.0) == 0.0)
                {
                    BOOST_FOREACH (rule::symbolizers::value_type sym,  r.get_symbolizers())
                    {
                        // if multiple raster symbolizers, last will be respected
                        // should we warn or throw?
                        boost::apply_visitor(d_collector,sym);
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
                memory_datasource cache(ds->type(),false);
                feature_ptr feature, prev;

                while ((feature = features->next()))
                {
                    if (prev && prev->get(group_by) != feature->get(group_by))
                    {
                        // We're at a value boundary, so render what we have
                        // up to this point.
                        int i = 0;
                        BOOST_FOREACH (feature_type_style const* style, active_styles)
                        {
                            render_style(lay, p, style, rule_caches[i], style_names[i],
                                         cache.features(q), prj_trans);
                            i++;
                        }
                        cache.clear();
                    }
                    cache.push(feature);
                    prev = feature;
                }

                int i = 0;
                BOOST_FOREACH (feature_type_style const* style, active_styles)
                {
                    render_style(lay, p, style, rule_caches[i], style_names[i],
                                 cache.features(q), prj_trans);
                    i++;
                }
            }
        }
        else if (cache_features)
        {
            memory_datasource cache(ds->type(),false);
            featureset_ptr features = ds->features(q);
            if (features) {
                // Cache all features into the memory_datasource before rendering.
                feature_ptr feature;
                while ((feature = features->next()))
                {
                    cache.push(feature);
                }
            }
            int i = 0;
            BOOST_FOREACH (feature_type_style const* style, active_styles)
            {
                render_style(lay, p, style, rule_caches[i], style_names[i],
                             cache.features(q), prj_trans);
                i++;
            }
        }
        // We only have a single style and no grouping.
        else
        {
            int i = 0;
            BOOST_FOREACH (feature_type_style const* style, active_styles)
            {
                render_style(lay, p, style, rule_caches[i], style_names[i],
                             ds->features(q), prj_trans);
                i++;
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
    feature_type_style const* style,
    rule_cache const& rc,
    std::string const& style_name,
    featureset_ptr features,
    proj_transform const& prj_trans)
{
    p.start_style_processing(*style);
    if (!features)
    {
        p.end_style_processing(*style);
        return;
    }

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

        BOOST_FOREACH(rule const* r, rc.get_if_rules() )
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
                    do_also=false;
                    break;
                }
            }
        }
        if (do_else)
        {
            BOOST_FOREACH( rule const* r, rc.get_else_rules() )
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
            BOOST_FOREACH( rule const* r, rc.get_also_rules() )
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

}
