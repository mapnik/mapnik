/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2006 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef FEATURE_STYLE_PROCESSOR_HPP
#define FEATURE_STYLE_PROCESSOR_HPP

#include <vector>
#include <iostream>

#include "envelope.hpp"
#include "datasource.hpp"
#include "layer.hpp"
#include "map.hpp"
#include "attribute_collector.hpp"
#include "property_index.hpp"
#include "utils.hpp"

namespace mapnik
{    
    
    template <typename Processor>
    class feature_style_processor 
    {
	struct symbol_dispatch : public boost::static_visitor<>
	{
	    symbol_dispatch (Processor & output,Feature const& f)
		: output_(output),f_(f) {}
	    
	    void operator () (polygon_symbolizer const& sym) const
	    {
		output_.process(sym,f_);
	    }

	    void operator () (polygon_pattern_symbolizer const& sym) const
	    {
		output_.process(sym,f_);
	    }
	    
	    void operator () (line_symbolizer const& sym) const
	    {
		output_.process(sym,f_);
	    }
	    
	    void operator () (line_pattern_symbolizer const& sym) const
	    {
		output_.process(sym,f_);
	    }
	   
	    void operator () (point_symbolizer const& sym) const
	    {
		output_.process(sym,f_);
	    }

	    void operator () (raster_symbolizer const& sym) const
	    {
		output_.process(sym,f_);
	    }
	    
	    Processor & output_;
	    Feature const& f_;
	};
    public:
	feature_style_processor(Map const& m)
	    : m_(m) {}
	
	void apply()
	{
	    timer clock;
	    
	    Processor & p = static_cast<Processor&>(*this);
	    std::vector<Layer>::const_iterator itr = m_.layers().begin();
	    while (itr != m_.layers().end())
	    {
		if (itr->isVisible(m_.scale()))// && itr->envelope().intersects(extent))
		{
		    apply_to_layer(*itr,p);
		}
		++itr;
	    }
	    clock.stop();
	}	
    private:
	void apply_to_layer(Layer const& lay,Processor & p)
	{
	    datasource *ds=lay.datasource().get();
	    if (ds)
	    {
		Envelope<double> const& bbox=m_.getCurrentExtent();
		double scale = m_.scale();
	
		std::vector<std::string> const& style_names = lay.styles();
		std::vector<std::string>::const_iterator stylesIter = style_names.begin();
		while (stylesIter != style_names.end())
		{
		    std::set<std::string> names;
		    attribute_collector<Feature> collector(names);
		    property_index<Feature> indexer(names);
		    std::vector<rule_type*> if_rules;
		    std::vector<rule_type*> else_rules;
		
		    bool active_rules=false;
		    
		    feature_type_style const& style=m_.find_style(*stylesIter++);
		    
		    const std::vector<rule_type>& rules=style.get_rules();
		    std::vector<rule_type>::const_iterator ruleIter=rules.begin();
		    
		    query q(bbox,m_.getWidth(),m_.getHeight());
		    while (ruleIter!=rules.end())
		    {
			if (ruleIter->active(scale))
			{
			    active_rules=true;
			    filter_ptr& filter=const_cast<filter_ptr&>(ruleIter->get_filter());
			    filter->accept(collector);
			    filter->accept(indexer);
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
		    // push all property names
		    while (namesIter!=names.end())
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
				while (itr!=if_rules.end())
				{
				    filter_ptr const& filter=(*itr)->get_filter();    
				    if (filter->pass(*feature))
				    {   
					do_else=false;
					const symbolizers& symbols = (*itr)->get_symbolizers();
					symbolizers::const_iterator symIter=symbols.begin();
					while (symIter!=symbols.end())
					{   
					    boost::apply_visitor(symbol_dispatch(p,*feature),*symIter++);
					}
				    }			    
				    ++itr;
				}
				if (do_else)
				{
				    //else filter
				    std::vector<rule_type*>::const_iterator itr=else_rules.begin();
				    while (itr != else_rules.end())
				    {
					const symbolizers& symbols = (*itr)->get_symbolizers();
					symbolizers::const_iterator symIter=symbols.begin();
					while (symIter!=symbols.end())
					{
					    boost::apply_visitor(symbol_dispatch(p,*feature),*symIter++);
					}
					++itr;
				    }
				}	  
			    }
			}
		    }
		}
	    }
	}
	
	Map const& m_;
    };
}

#endif //FEATURE_STYLE_PROCESSOR_HPP
