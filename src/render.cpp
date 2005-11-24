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

//$Id: render.cpp 44 2005-04-22 18:53:54Z pavlenko $


#include "render.hpp"
#include "line_aa.hpp"
#include "scanline.hpp"
#include "scanline_aa.hpp"
#include "text.hpp"
#include "image_util.hpp"
#include "utils.hpp"
#include "style_cache.hpp"
#include "image_reader.hpp"
#include "polygon_symbolizer.hpp"
#include "line_symbolizer.hpp"
#include "query.hpp"
#include "feature_layer_desc.hpp"
#include "attribute_collector.hpp"
#include "property_index.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace mapnik
{  
    
    template <typename Image>
    void Renderer<Image>::render_vector_layer(const Layer& l,unsigned width,unsigned height,
				      const Envelope<double>& bbox,Image& image)
    {
        const datasource_p& ds=l.datasource();
        if (!ds) return;
	CoordTransform t(width,height,bbox);
	std::vector<std::string> const& namedStyles=l.styles();
	std::vector<std::string>::const_iterator stylesIter=namedStyles.begin();
	while (stylesIter!=namedStyles.end())
	{
	    feature_type_style style=named_style_cache::instance()->find(*stylesIter++);
	    
	    std::set<std::string> names;
	    attribute_collector<Feature> collector(names);
	    property_index<Feature> indexer(names);

	    query q(bbox,width,height);
	    double scale = 1.0/t.scale();
	    std::vector<rule_type*> if_rules;
	    std::vector<rule_type*> else_rules;
	    
	    bool active_rules=false;
	    const std::vector<rule_type>& rules=style.get_rules();
	    std::vector<rule_type>::const_iterator ruleIter=rules.begin();
	    
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
	    //only query datasource if there are active rules
	    if (active_rules)
	    {
		featureset_ptr fs=ds->features(q);
		if (fs)
		{   	    
		    feature_ptr feature;
		    while ((feature = fs->next()))
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
				    break;// not sure !!
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
		    }
		}
		
		//if (l.isSelectable() && l.selection().size()) //TODO !!!
		//{
		  //volatile style_cache* styles=style_cache::instance();
		  //  const Style& style=style_cache::instance()->find(l.selection_style());
		
		//	    std::vector<ref_ptr<Feature> >& selection=l.selection();
		
		//	    Style::Iterator pos = style.begin();
		//   if (pos!=style.end()) {
		//	std::vector<ref_ptr<Feature> >::iterator itr=selection.begin();
		//   
		//	while (itr!=selection.end())
		//	{
		//	    geometry_ptr& geom=(*itr)->get_geometry();
		//	    geom->transform(t);
		//	    (*pos)->render(*geom,image);
		//	    ++itr;
		//	} 
		//   }
		//    l.clear_selection();
		//} 
	    }
	}
    }
    
    template <typename Image>
    void Renderer<Image>::render_raster_layer(const Layer& l,unsigned width,unsigned height,
				      const Envelope<double>& bbox,Image& image)
    {	
	const datasource_p& ds=l.datasource();
        if (!ds) return;
	query q(bbox,width,height);
	featureset_ptr fs=ds->features(q);
	if (fs)
	{   	    
	    feature_ptr feature;
	    while ((feature = fs->next()))
	    {
		raster_ptr const& raster=feature->get_raster();
		if (raster)
		{
		    image.set_rectangle(raster->x_,raster->y_,raster->data_);
		}
	    }
	}		   	
    }

    template <typename Image>
    void Renderer<Image>::render(const Map& map,Image& image)
    {
        timer clock;
        //////////////////////////////////////////////////////
        const Envelope<double>& extent=map.getCurrentExtent();
	std::cout<<"BBOX:"<<extent<<std::endl;
        double scale=map.scale();
        std::cout<<" scale="<<scale<<std::endl;
        
	unsigned width=map.getWidth();
        unsigned height=map.getHeight();

        Color const& background=map.getBackground();
        image.setBackground(background);
	
        for (size_t n=0;n<map.layerCount();++n)
        {
            const Layer& l=map.getLayer(n);
            if (l.isVisible(scale)) // TODO: extent check
	    {
                if (l.datasource()->type() == datasource::Vector)
		{
		    render_vector_layer(l,width,height,extent,image);
		}
		else if (l.datasource()->type() == datasource::Raster)
		{
		    render_raster_layer(l,width,height,extent,image);
		}
            }
        }        
        clock.stop();
    }

    template class Renderer<Image32>;
}
