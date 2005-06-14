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

#include "raster_featureset.hh"
#include "image_reader.hh"

template <typename LookupPolicy>
RasterFeatureset<LookupPolicy>::RasterFeatureset(const LookupPolicy& policy,
						 const Envelope<double>& box,
						 const CoordTransform& t)
    : policy_(policy),
      id_(1),
      extent_(box),
      t_(t),
      curIter_(policy_.query(box)),
      endIter_(policy_.end()) 

{}

template <typename LookupPolicy>
RasterFeatureset<LookupPolicy>::~RasterFeatureset() {}

template <typename LookupPolicy>
Feature* RasterFeatureset<LookupPolicy>::next()
{
    Feature* f=0;
    if (curIter_!=endIter_)
    {
        try
        {
	    std::cout<<"RasterFeatureset "<<curIter_->format()<<" "<<curIter_->file()<<std::endl;
            std::auto_ptr<ImageReader> reader(get_image_reader(curIter_->format(),curIter_->file()));
	    std::cout<<reader.get()<<std::endl;
	    if (reader.get())
            {
                int image_width=reader->width();
                int image_height=reader->height();
                if (image_width>0 && image_height>0)
                {
                    CoordTransform t(image_width,image_height,curIter_->envelope());
                    Envelope<double> intersect=extent_.intersect(curIter_->envelope());
                    Envelope<double> ext=t.forward(intersect);
                    
		    Envelope<double> image_ext=t_.forward(intersect);
                    
		    ImageData32 image((int)ext.width(),(int)ext.height());
                    reader->read((int)ext.minx(),(int)ext.miny(),image);
                    ImageData32 target((int)(image_ext.width()+0.5),(int)(image_ext.height()+0.5));
                    scale_image<ImageData32>(target,image);

                    f=new RasterFeature(++id_,RasterPtr(new raster((int)(image_ext.minx()+0.5),
								   (int)(image_ext.miny()+0.5),target)));
                }
            }
        }
        catch (...)
        {
        }
        ++curIter_;
    }
    return f;
}


template class RasterFeatureset<single_file_policy>;
//template RasterFeatureset<os_name_policy>;
