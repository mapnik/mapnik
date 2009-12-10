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
#include <mapnik/envelope.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/scale_denominator.hpp>
#ifdef MAPNIK_DEBUG
#include <mapnik/wall_clock_timer.hpp>
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
         feature_style_processor(Map const& m)
            : m_(m) {}
	 
         void apply()
         {
#ifdef MAPNIK_DEBUG           
            mapnik::wall_clock_progress_timer t(std::clog, "map rendering took: ");
#endif          
            Processor & p = static_cast<Processor&>(*this);
            p.start_map_processing(m_);
                       
            try
            {
               projection proj(m_.srs()); // map projection
               double scale_denom = mapnik::scale_denominator(m_,proj.is_geographic());
#ifdef MAPNIK_DEBUG
               std::clog << "scale denominator = " << scale_denom << "\n";
#endif
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
               std::clog << "proj_init_error:" << ex.what() << "\n"; 
            }
            
            p.end_map_processing(m_);
         }	
      private:
         void apply_to_layer(Layer const& lay, Processor & p, 
                             projection const& proj0,double scale_denom)
         {
#ifdef MAPNIK_DEBUG
   wall_clock_progress_timer timer(clog, "end layer rendering: ");
#endif
            p.start_layer_processing(lay);
            boost::shared_ptr<datasource> ds=lay.datasource();
            if (ds)
            {
               Envelope<double> ext = m_.get_buffered_extent();
               projection proj1(lay.srs());
               proj_transform prj_trans(proj0,proj1);
               
               double mx0 = ext.minx();
               double my0 = ext.miny();
               double mz0 = 0.0;
               double mx1 = ext.maxx();
               double my1 = ext.maxy();
               double mz1 = 0.0;
               
               // project main map projection into layers extent
               prj_trans.forward(mx0,my0,mz0);
               prj_trans.forward(mx1,my1,mz1);

               // if no intersection then nothing to do for layer
               Envelope<double> layer_ext = lay.envelope();
               if ( mx0 > layer_ext.maxx() || mx1 < layer_ext.minx() || my0 > layer_ext.maxy() || my1 < layer_ext.miny() )
               {
                  return;
               }
               
               // clip query bbox
               mx0 = std::max(layer_ext.minx(),mx0);
               my0 = std::max(layer_ext.miny(),my0);
               mx1 = std::min(layer_ext.maxx(),mx1);
               my1 = std::min(layer_ext.maxy(),my1);
               
               Envelope<double> bbox(mx0,my0,mx1,my1);
               
               double resolution = m_.getWidth()/bbox.width();
               query q(bbox,resolution,scale_denom); //BBOX query
               
               std::vector<std::string> const& style_names = lay.styles();
               std::vector<std::string>::const_iterator stylesIter = style_names.begin();
               std::vector<std::string>::const_iterator stylesEnd = style_names.end(); 
               for (;stylesIter != stylesEnd; ++stylesIter)
               {
                  std::set<std::string> names;
                  attribute_collector<Feature> collector(names);
                  std::vector<rule_type*> if_rules;
                  std::vector<rule_type*> else_rules;
                    
                  bool active_rules=false;
                  
                  boost::optional<feature_type_style const&> style=m_.find_style(*stylesIter);
                  if (!style) continue;
                  
                  const std::vector<rule_type>& rules=(*style).get_rules();
                  std::vector<rule_type>::const_iterator ruleIter=rules.begin();
                  std::vector<rule_type>::const_iterator ruleEnd=rules.end();
                                        
                  for (;ruleIter!=ruleEnd;++ruleIter)
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
                              filter_ptr const& filter=(*itr)->get_filter();    
                              if (filter->pass(*feature))
                              {   
                                 do_else=false;
                                 const symbolizers& symbols = (*itr)->get_symbolizers();
                                 symbolizers::const_iterator symIter=symbols.begin();
                                 symbolizers::const_iterator symEnd =symbols.end();
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
                                 const symbolizers& symbols = (*itr)->get_symbolizers();
                                 symbolizers::const_iterator symIter= symbols.begin();
                                 symbolizers::const_iterator symEnd = symbols.end();
                                        
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
   };
}

#endif //FEATURE_STYLE_PROCESSOR_HPP
