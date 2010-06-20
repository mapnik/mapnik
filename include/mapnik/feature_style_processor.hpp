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

#ifdef MAPNIK_DEBUG
//#include <mapnik/wall_clock_timer.hpp>
#endif
//stl
#include <vector>

namespace mapnik
{       
template <typename Processor>
class feature_style_processor 
{
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
            double scale_denom = mapnik::scale_denominator(m_,proj.is_geographic());
            scale_denom *= scale_factor_;
#ifdef MAPNIK_DEBUG
            std::clog << "scale denominator = " << scale_denom << "\n";
#endif
            std::vector<layer>::const_iterator itr = m_.layers().begin();
            std::vector<layer>::const_iterator end = m_.layers().end();
            
            while (itr != end)
            {
                if (itr->isVisible(scale_denom))
                {
                    apply_to_layer(*itr, p, proj, scale_denom);
                }
                ++itr;
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
                        projection const& proj0,double scale_denom)
    {
#ifdef MAPNIK_DEBUG
        //wall_clock_progress_timer timer(clog, "end layer rendering: ");
#endif
        p.start_layer_processing(lay);
        boost::shared_ptr<datasource> ds=lay.datasource();
        if (ds)
        {
            box2d<double> ext = m_.get_buffered_extent();
            projection proj1(lay.srs());
            proj_transform prj_trans(proj0,proj1);

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
      
            query::resolution_type res(m_.getWidth()/m_.getCurrentExtent().width(),m_.getHeight()/m_.getCurrentExtent().height());
            query q(bbox,res,scale_denom); //BBOX query
               
            std::vector<std::string> const& style_names = lay.styles();
            std::vector<std::string>::const_iterator stylesIter = style_names.begin();
            std::vector<std::string>::const_iterator stylesEnd = style_names.end(); 
            for (;stylesIter != stylesEnd; ++stylesIter)
            {
                std::set<std::string> names;
                attribute_collector collector(names);
                std::vector<rule_type*> if_rules;
                std::vector<rule_type*> else_rules;
                    
                bool active_rules=false;
                  
                boost::optional<feature_type_style const&> style=m_.find_style(*stylesIter);
                if (!style) {
                    std::clog << "WARNING: style '" << *stylesIter << "' required for layer '" << lay.name() << "' does not exist.\n";
                    continue;
                }

                const std::vector<rule_type>& rules=(*style).get_rules();
                std::vector<rule_type>::const_iterator ruleIter=rules.begin();
                std::vector<rule_type>::const_iterator ruleEnd=rules.end();
                                        
                for (;ruleIter!=ruleEnd;++ruleIter)
                {
                    if (ruleIter->active(scale_denom))
                    {
                        active_rules=true;
                        // collect unique attribute names
                        // TODO - in the future rasters should be able to be filtered...
                        if (ds->type() == datasource::Vector)
                        {
                            collector(*ruleIter);
                        }
                        if (ruleIter->has_else_filter())
                        {
                            else_rules.push_back(const_cast<rule_type*>(&(*ruleIter)));
                        }
                        else
                        {
                            if_rules.push_back(const_cast<rule_type*>(&(*ruleIter)));               
                        }
                        if (ds->type() == datasource::Raster)
                        {
                            if (ds->params().get<double>("filter_factor",0.0) == 0.0)
                            {
                                const rule_type::symbolizers& symbols = ruleIter->get_symbolizers();
                                rule_type::symbolizers::const_iterator symIter = symbols.begin();
                                rule_type::symbolizers::const_iterator symEnd = symbols.end();
                                for (;symIter != symEnd;++symIter)
                                {   
                                    try
                                    {
                                        raster_symbolizer sym = boost::get<raster_symbolizer>(*symIter);
                                        std::string scaling = sym.get_scaling();
                                        if (scaling == "bilinear" || scaling == "bilinear8" )
                                        {
                                            // todo - allow setting custom value in symbolizer property?
                                            q.filter_factor(2.0);
                                        }
                                    }
                                    catch (const boost::bad_get &v)
                                    {
                                        // case where useless symbolizer is attached to raster layer
                                        //throw config_error("Invalid Symbolizer type supplied, only RasterSymbolizer is supported");
                                    }
                                }
                            }
                            
                        }
                    }
                }
                std::set<std::string>::const_iterator namesIter=names.begin();
                std::set<std::string>::const_iterator namesEnd =names.end();
                    
                // push all property names
                for (;namesIter!=namesEnd;++namesIter)
                {
                    q.add_property_name(*namesIter);
                }
                if (active_rules)
                {
                    featureset_ptr fs=ds->features(q);
                    if (fs)
                    {               
                        feature_ptr feature;
                        while ((feature = fs->next()))
                        {                  
                            bool do_else=true;              
                            std::vector<rule_type*>::const_iterator itr=if_rules.begin();
                            std::vector<rule_type*>::const_iterator end=if_rules.end();
                            for (;itr != end;++itr)
                            {
                                expression_ptr const& expr=(*itr)->get_filter();    
                                value_type result = boost::apply_visitor(evaluate<Feature,value_type>(*feature),*expr);
                                if (result.to_bool())
                                {   
                                    do_else=false;
                                    const rule_type::symbolizers& symbols = (*itr)->get_symbolizers();
                                    rule_type::symbolizers::const_iterator symIter=symbols.begin();
                                    rule_type::symbolizers::const_iterator symEnd =symbols.end();
                                    for (;symIter != symEnd;++symIter)
                                    {   
                                        boost::apply_visitor
                                            (symbol_dispatch(p,*feature,prj_trans),*symIter);
                                    }
                                }                           
                            }
                            if (do_else)
                            {
                                //else filter
                                std::vector<rule_type*>::const_iterator itr=
                                    else_rules.begin();
                                std::vector<rule_type*>::const_iterator end=
                                    else_rules.end();
                                for (;itr != end;++itr)
                                {
                                    const rule_type::symbolizers& symbols = (*itr)->get_symbolizers();
                                    rule_type::symbolizers::const_iterator symIter= symbols.begin();
                                    rule_type::symbolizers::const_iterator symEnd = symbols.end();
                                        
                                    for (;symIter!=symEnd;++symIter)
                                    {
                                        boost::apply_visitor
                                            (symbol_dispatch(p,*feature,prj_trans),*symIter);
                                    }
                                }
                            }     
                        }
                    }
                }
            }               
        }
        p.end_layer_processing(lay);
    }   
    Map const& m_;
    double scale_factor_;
};
}

#endif //FEATURE_STYLE_PROCESSOR_HPP
