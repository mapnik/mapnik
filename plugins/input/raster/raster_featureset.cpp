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
// mapnik
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>

#include "raster_featureset.hpp"


using mapnik::query;
using mapnik::CoordTransform;
using mapnik::ImageReader;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::ImageData32;
using mapnik::raster;

template <typename LookupPolicy>
raster_featureset<LookupPolicy>::raster_featureset(LookupPolicy const& policy,
                                                   Envelope<double> const& extent,
                                                   query const& q)
   : policy_(policy),
     id_(1),
     extent_(extent),
     bbox_(q.get_bbox()),
     curIter_(policy_.begin()),
     endIter_(policy_.end()) 
{}

template <typename LookupPolicy>
raster_featureset<LookupPolicy>::~raster_featureset() {}

template <typename LookupPolicy>
feature_ptr raster_featureset<LookupPolicy>::next()
{
   if (curIter_!=endIter_)
   {
      feature_ptr feature(new Feature(++id_));
      try
      {         
         std::auto_ptr<ImageReader> reader(mapnik::get_image_reader(curIter_->file(),curIter_->format()));

#ifdef MAPNIK_DEBUG         
         std::cout << "Raster Plugin: READER = " << curIter_->format() << " " << curIter_->file() 
                   << " size(" << curIter_->width() << "," << curIter_->height() << ")\n";
#endif
         if (reader.get())
         {
            int image_width=reader->width();
            int image_height=reader->height();
            
            if (image_width>0 && image_height>0)
            {
               CoordTransform t(image_width,image_height,extent_,0,0);
               Envelope<double> intersect=bbox_.intersect(curIter_->envelope());
               Envelope<double> ext=t.forward(intersect);
               if ( ext.width()>0.5 && ext.height()>0.5 )
               {
                  //select minimum raster containing whole ext
                  int x_off = static_cast<int>(floor(ext.minx()));
                  int y_off = static_cast<int>(floor(ext.miny()));
                  int end_x = static_cast<int>(ceil(ext.maxx()));
                  int end_y = static_cast<int>(ceil(ext.maxy()));
                  //clip to available data
                  if (x_off < 0)
                     x_off = 0;
                  if (y_off < 0)
                     y_off = 0;
                  if (end_x > image_width)
                     end_x = image_width;
                  if (end_y > image_height)
                     end_y = image_height;
                  int width = end_x - x_off;
                  int height = end_y - y_off;
                  //calculate actual envelope of returned raster
                  Envelope<double> feature_raster_extent(x_off, y_off, x_off+width, y_off+height); 
                  intersect = t.backward(feature_raster_extent);

                  ImageData32 image(width,height);
                  reader->read(x_off,y_off,image);

                  feature->set_raster(mapnik::raster_ptr(new raster(intersect,image)));
               }
            }
         }
      }
      catch (...)
      {
         std::cerr << "Raster Plugin: Exception caught\n";
      }
      
      ++curIter_;
      return feature;
   }
   return feature_ptr();
}

template class raster_featureset<single_file_policy>;
template class raster_featureset<tiled_file_policy>;
