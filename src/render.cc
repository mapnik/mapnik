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

//$Id: render.cc 67 2004-11-23 10:04:32Z artem $

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

namespace mapnik
{  
    template <typename Image>
    void Renderer<Image>::renderLayer(const Layer& l,const CoordTransform& t,
				      const Envelope<double>& extent,Image& image)
    {
        const datasource_p& ds=l.datasource();
        if (!ds) return; 
        FeaturesetPtr fs=ds->featuresInBox(t,extent);
        if (fs)
        {     
	    volatile style_cache* styles=style_cache::instance();
	    const Style& style=styles->find(l.getStyle());
	    //TODO fix scale 
            Style::Iterator itr = style.find(1.0/t.scale());
	    if (itr!=style.end()) {
		Feature* feature=0;
		while (feature=fs->next())
		{
		    // TODO refactor!!!!
		    if (feature->isRaster())
		    {
			const RasterPtr& ras=feature->getRaster();
			image.set_rectangle(ras->x_,ras->y_,ras->data_);
		    }
		    else
		    {
			geometry_ptr& geom=feature->getGeometry();
			geom->transform(t);
			(*itr)->render(*geom,image);
		    }
		    delete feature,feature=0; 
		}
	    }
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
		    geometry_ptr& geom=(*itr)->getGeometry();
		    geom->transform(t);
		    (*pos)->render(*geom,image);
		    ++itr;
		} 
	    }
	    l.clear_selection();
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
        
	int width=map.getWidth();
        int height=map.getHeight();

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
