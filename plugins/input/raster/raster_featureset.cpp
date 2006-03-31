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
#include "raster_featureset.hpp"
#include "image_reader.hpp"
#include "image_util.hpp"

template <typename LookupPolicy>
raster_featureset<LookupPolicy>::raster_featureset(LookupPolicy const& policy,query const& q)
    : policy_(policy),
      id_(1),
      extent_(q.get_bbox()),
      t_(q.get_width(),q.get_height(),extent_),
      curIter_(policy_.query(extent_)),
      endIter_(policy_.end()) 

{}

template <typename LookupPolicy>
raster_featureset<LookupPolicy>::~raster_featureset() {}

template <typename LookupPolicy>
feature_ptr raster_featureset<LookupPolicy>::next()
{
    if (curIter_!=endIter_)
    {
	feature_ptr feature(new Feature(+id_));
        try
        {
	    std::clog<<"raster_featureset "<<curIter_->format()<<" "<<curIter_->file()<<std::endl;
            std::auto_ptr<ImageReader> reader(get_image_reader(curIter_->format(),curIter_->file()));
	    std::clog<<reader.get()<<std::endl;
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
		    feature->set_raster(raster_ptr(new raster(int(image_ext.minx()+0.5),int(image_ext.miny()+0.5),target)));
                }
            }
        }
        catch (...)
        {
        }
        ++curIter_;
	return feature;
    }
    return feature_ptr();
}


template class raster_featureset<single_file_policy>;
//template raster_featureset<os_name_policy>;
