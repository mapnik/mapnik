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

//stl
#include <vector>
// boost
#include <boost/progress.hpp>
// mapnik
#include <mapnik/envelope.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/scale_denominator.hpp>

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
        feature_style_processor(Map const& m)
            : m_(m) {}
	
        void apply()
        {
            boost::progress_timer t(std::clog);            
            Processor & p = static_cast<Processor&>(*this);
            p.start_map_processing(m_);
                       
            try
            {
                projection proj(m_.srs()); // map projection
                double scale_denom = scale_denominator(m_,proj.is_geographic());
                std::clog << "scale denominator = " << scale_denom << "\n";
                
                std::vector<Layer>::const_iterator itr = m_.layers().begin();
                std::vector<Layer>::const_iterator end = m_.layers().end();
            
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
                std::clog << ex.what() << "\n"; 
            }

            p.end_map_processing(m_);
        }	
    private:
        void apply_to_layer(Layer const& lay, Processor & p, 
                            projection const& proj0,double scale_denom)
        {
            p.start_layer_processing(lay);
            boost::shared_ptr<datasource> ds=lay.datasource();
            if (ds)
            {
                Envelope<double> const& ext=m_.getCurrentExtent();
              
                projection proj1(lay.srs());
                proj_transform prj_trans(proj0,proj1);
                
                double x0 = ext.minx();
                double y0 = ext.miny();
                double z0 = 0.0;
                double x1 = ext.maxx();
                double y1 = ext.maxy();
                double z1 = 0.0;
                prj_trans.forward(x0,y0,z0);
                prj_trans.forward(x1,y1,z1);
                Envelope<double> bbox(x0,y0,x1,y1);
                std::clog << bbox << "\n";
                
                std::vector<std::string> const& style_names = lay.styles();
                std::vector<std::string>::const_iterator stylesIter = style_names.begin();
                std::vector<std::string>::const_iterator stylesEnd = style_names.end();
                
                while (stylesIter != stylesEnd)
                {
                    std::set<std::string> names;
                    attribute_collector<Feature> collector(names);
                    std::vector<rule_type*> if_rules;
                    std::vector<rule_type*> else_rules;
                    
                    bool active_rules=false;
                    
                    feature_type_style const& style=m_.find_style(*stylesIter++);
                        
                    query q(bbox); //BBOX query

                    const std::vector<rule_type>& rules=style.get_rules();
                    std::vector<rule_type>::const_iterator ruleIter=rules.begin();
                    std::vector<rule_type>::const_iterator ruleEnd=rules.end();
                                        
                    while (ruleIter!=ruleEnd)
                    {
                        if (ruleIter->active(scale_denom))
                        {
                            active_rules=true;
                            ruleIter->accept(collector);

                            if (ruleIter->has_else_filter())
                            {
                                else_rules.push_back(const_cast<rule_type*>(&(*ruleIter)));
                            }
                            else
                            {
                                if_rules.push_back(const_cast<rule_type*>(&(*ruleIter))); 		    
                            }
                        }
                        ++ruleIter;
                    }
                    std::set<std::string>::const_iterator namesIter=names.begin();
                    std::set<std::string>::const_iterator namesEnd =names.end();
                    
                    // push all property names
                    while (namesIter!=namesEnd)
                    {
                        q.add_property_name(*namesIter);
                        ++namesIter;
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
                                while (itr != end)
                                {
                                    filter_ptr const& filter=(*itr)->get_filter();    
                                    if (filter->pass(*feature))
                                    {   
                                        do_else=false;
                                        const symbolizers& symbols = (*itr)->get_symbolizers();
                                        symbolizers::const_iterator symIter=symbols.begin();
                                        symbolizers::const_iterator symEnd =symbols.end();
                                        while (symIter != symEnd)
                                        {   
                                            boost::apply_visitor
                                                (symbol_dispatch(p,*feature,prj_trans),*symIter++);
                                        }
                                    }			    
                                    ++itr;
                                }
                                if (do_else)
                                {
                                    //else filter
                                    std::vector<rule_type*>::const_iterator itr=
                                        else_rules.begin();
                                    std::vector<rule_type*>::const_iterator end=
                                        else_rules.end();
                                    while (itr != end)
                                    {
                                        const symbolizers& symbols = (*itr)->get_symbolizers();
                                        symbolizers::const_iterator symIter= symbols.begin();
                                        symbolizers::const_iterator symEnd = symbols.end();
                                        
                                        while (symIter!=symEnd)
                                        {
                                            boost::apply_visitor
                                                (symbol_dispatch(p,*feature,prj_trans),
                                                 *symIter++);
                                        }
                                        ++itr;
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
    };
}

#endif //FEATURE_STYLE_PROCESSOR_HPP
