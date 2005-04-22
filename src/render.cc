/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
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

#include <algorithm>
#include <cmath>

#include "render.hh"
#include "line_aa.hh"
#include "scanline.hh"
#include "scanline_aa.hh"
#include "text.hh"
#include "image_util.hh"
#include "utils.hh"
#include "style_cache.hh"
#include "image_reader.hh"
#include "polygon_symbolizer.hh"
#include "line_symbolizer.hh"
#include "query.hh"

namespace mapnik
{  
    template <typename Image>
    void Renderer<Image>::renderLayer(const Layer& l,const CoordTransform& t,
				      const Envelope<double>& bbox,Image& image)
    {
        const datasource_p& ds=l.datasource();
        if (!ds) return;
	
	volatile named_style_cache* styles=named_style_cache::instance();
	const feature_type_style& style=styles->find(l.getStyle());
	
	const std::vector<rule_type>& rules=style.rules();
	
	std::vector<rule_type*> if_rules;
	std::vector<rule_type*> else_rules;
	
	std::vector<rule_type>::const_iterator ruleIter=rules.begin();
	attribute_collector<Feature> collector;
	
	query q(bbox);
        double scale = 1.0/t.scale();
	while (ruleIter!=rules.end())
	{
	    if (ruleIter->active(scale))
	    {
		filter_ptr& filter=const_cast<filter_ptr&>(ruleIter->get_filter());
		filter->accept(collector);
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

        const std::set<std::string>& names=collector.property_names();
	std::set<std::string>::const_iterator namesIter=names.begin();
	// push all property names we'll need
	while (namesIter!=names.end())
	{
	    q.add_property_name(*namesIter);
	    ++namesIter;
	}
	
	featureset_ptr fs=ds->features(q);
        if (fs)
        {   	    
	    Feature* feature=0;
	    while (feature = fs->next())
	    {
		bool do_else=true;
		geometry_ptr& geom=feature->get_geometry();
		if (geom)
		{
		    geom->transform(t);//todo: transform once
		    std::vector<rule_type*>::const_iterator itr=if_rules.begin();
		    while (itr!=if_rules.end())
		    {
			const filter_ptr& filter=(*itr)->get_filter();    
			if (filter->pass(*feature))
			{   
			    do_else=false;
			    const symbolizers& symbols = (*itr)->get_symbolizers();
			    symbolizers::const_iterator symIter=symbols.begin();
			    while (symIter!=symbols.end())
			    {
				(*symIter)->render(*geom,image);
				++symIter;
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
				(*symIter)->render(*geom,image);
				++symIter;
			    }
			    ++itr;
			}
		    }
		}
		delete feature;
	    }
	    
	    if (l.isSelectable())
	    {
		volatile style_cache* styles=style_cache::instance();
		const Style& style=styles->find(l.selection_style());
		
		std::vector<ref_ptr<Feature> >& selection=l.selection();
		
		Style::Iterator pos = style.find(1.0/t.scale());
		if (pos!=style.end()) {
		    std::vector<ref_ptr<Feature> >::iterator itr=selection.begin();
		    
		    while (itr!=selection.end())
		    {
			geometry_ptr& geom=(*itr)->get_geometry();
			geom->transform(t);
			(*pos)->render(*geom,image);
			++itr;
		    } 
		}
		l.clear_selection();
	    }
	    
	}
    }

    template <typename Image>
    void Renderer<Image>::render(const Map& map,Image& image)
    {
        timer clock;
        //////////////////////////////////////////////////////
        const Envelope<double>& extent=map.getCurrentExtent();
        double scale=map.scale();
        std::cout<<" scale="<<scale<<"\n";
        
	unsigned width=map.getWidth();
        unsigned height=map.getHeight();
        CoordTransform t(width,height,extent);
        const Color& background=map.getBackground();
        image.setBackground(background);
	
        for (size_t n=0;n<map.layerCount();++n)
        {
            const Layer& l=map.getLayer(n);
            if (l.isVisible(scale))
            {
                //TODO make datasource to return its extent!!!
                renderLayer(l,t,extent,image);
            }
        }
        
        clock.stop();
    }

    template class Renderer<Image32>;
}
