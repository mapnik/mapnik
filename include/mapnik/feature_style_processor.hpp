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

#ifdef MAPNIK_DEBUG
//#include <mapnik/wall_clock_timer.hpp>
#endif
// boost
#include <boost/foreach.hpp>
//stl
#include <vector>

namespace mapnik
{     
  
template <typename Processor>
class feature_style_processor 
{
    /** Calls the renderer's process function,
      * \param output     Renderer
      * \param f          Feature to process
      * \param prj_trans  Projection
      * \param sym        Symbolizer object
      */
    struct symbol_dispatch : public boost::static_visitor<>
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
public:
    explicit feature_style_processor(Map const& m, double scale_factor = 1.0)
        : m_(m),
          scale_factor_(scale_factor) {}
    
    void apply()
    {
#ifdef MAPNIK_DEBUG           
        //mapnik::wall_clock_progress_timer t(std::clog, "map rendering took: ");
#endif          
        Processor & p = static_cast<Processor&>(*this);
        p.start_map_processing(m_);
                       
        try
        {
            projection proj(m_.srs()); // map projection

            Map::const_metawriter_iterator metaItr = m_.begin_metawriters();
            Map::const_metawriter_iterator metaItrEnd = m_.end_metawriters();
            for (;metaItr!=metaItrEnd; ++metaItr)
            {
                metaItr->second->set_size(m_.width(), m_.height());
                metaItr->second->set_map_srs(proj);
                metaItr->second->start(m_.metawriter_output_properties);
            }

            double scale_denom = mapnik::scale_denominator(m_,proj.is_geographic());
            scale_denom *= scale_factor_;
#ifdef MAPNIK_DEBUG
            std::clog << "scale denominator = " << scale_denom << "\n";
#endif
            BOOST_FOREACH ( layer const& lyr, m_.layers() )
            {
                if (lyr.isVisible(scale_denom))
                {
                    apply_to_layer(lyr, p, proj, scale_denom);
                }
            }

            metaItr = m_.begin_metawriters();
            for (;metaItr!=metaItrEnd; ++metaItr)
            {
                metaItr->second->stop();
            }
        }
        catch (proj_init_error& ex)
        {
            std::clog << "proj_init_error:" << ex.what() << "\n"; 
        }
        
        p.end_map_processing(m_);
    }   
private:
    void apply_to_layer(layer const& lay, Processor & p, 
                        projection const& proj0, double scale_denom)
    {
#ifdef MAPNIK_DEBUG
        //wall_clock_progress_timer timer(clog, "end layer rendering: ");
#endif
        boost::shared_ptr<datasource> ds = lay.datasource();
        if (!ds) 
        {
            std::clog << "WARNING: No datasource for layer '" << lay.name() << "'\n";
            return;
        }
        
        p.start_layer_processing(lay);
        
        if (ds)
        {
            
            box2d<double> ext = m_.get_buffered_extent();

            // clip buffered extent by maximum extent, if supplied
            boost::optional<box2d<double> > const& maximum_extent = m_.maximum_extent();
            if (maximum_extent) {
                ext.clip(*maximum_extent);
            }

            projection proj1(lay.srs());
            proj_transform prj_trans(proj0,proj1);

            // todo: only display raster if src and dest proj are matched
            // todo: add raster re-projection as an optional feature 
            if (ds->type() == datasource::Raster && !prj_trans.equal())
            {
                std::clog << "WARNING: Map srs does not match layer srs, skipping raster layer '" << lay.name() << "' as raster re-projection is not currently supported (http://trac.mapnik.org/ticket/663)\n";
                std::clog << "map srs: '" << m_.srs() << "'\nlayer srs: '" << lay.srs() << "' \n";       
                return;
            }
            
            // 
            
            box2d<double> layer_ext = lay.envelope();
               
            double lx0 = layer_ext.minx();
            double ly0 = layer_ext.miny();
            double lz0 = 0.0;
            double lx1 = layer_ext.maxx();
            double ly1 = layer_ext.maxy();
            double lz1 = 0.0;
            // back project layers extent into main map projection
            prj_trans.backward(lx0,ly0,lz0);
            prj_trans.backward(lx1,ly1,lz1);
               
            // if no intersection then nothing to do for layer
            if ( lx0 > ext.maxx() || lx1 < ext.minx() || ly0 > ext.maxy() || ly1 < ext.miny() )
            {
                return;
            }
            
            // clip query bbox
            lx0 = std::max(ext.minx(),lx0);
            ly0 = std::max(ext.miny(),ly0);
            lx1 = std::min(ext.maxx(),lx1);
            ly1 = std::min(ext.maxy(),ly1);
            
            prj_trans.forward(lx0,ly0,lz0);
            prj_trans.forward(lx1,ly1,lz1);
            box2d<double> bbox(lx0,ly0,lx1,ly1);
            
            query::resolution_type res(m_.width()/m_.get_current_extent().width(),m_.height()/m_.get_current_extent().height());
            query q(bbox,res,scale_denom); //BBOX query
                           
            std::vector<feature_type_style*> active_styles;
            std::set<std::string> names;
            attribute_collector collector(names);
            double filt_factor = 1;
            directive_collector d_collector(&filt_factor);
            
            std::vector<std::string> const& style_names = lay.styles();
            // iterate through all named styles collecting active styles and attribute names
            BOOST_FOREACH(std::string const& style_name, style_names)
            {
                boost::optional<feature_type_style const&> style=m_.find_style(style_name);
                if (!style) 
                {
                    std::clog << "WARNING: style '" << style_name << "' required for layer '" << lay.name() << "' does not exist.\n";
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
            bool cache_features = lay.cache_features() && style_names.size()>1?true:false;
            bool first = true;
            
            BOOST_FOREACH (feature_type_style * style, active_styles)
            {
                std::vector<rule*> if_rules;
                std::vector<rule*> else_rules;

                std::vector<rule> const& rules=style->get_rules();
                
                BOOST_FOREACH(rule const& r, rules)
                {
                    if (r.active(scale_denom))
                    {
                        if (r.has_else_filter())
                        {
                            else_rules.push_back(const_cast<rule*>(&r));
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
                        bool do_else=true;
                        
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
                                do_else=false;
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
                    }
                }
                cache_features = false;
            }
        }
        
        p.end_layer_processing(lay);
    } 
    
    Map const& m_;
    double scale_factor_;
};
}

#endif //FEATURE_STYLE_PROCESSOR_HPP
