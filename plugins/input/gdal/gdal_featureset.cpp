/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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

#include "gdal_featureset.hpp"
#include <gdal_priv.h>

using mapnik::query;
using mapnik::Envelope;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::CoordTransform;

gdal_featureset::gdal_featureset(GDALDataset & dataset, query const& q)
   : dataset_(dataset),
     query_extent_(q.get_bbox()),
     first_(true) {}

gdal_featureset::~gdal_featureset() {}

feature_ptr gdal_featureset::next()
{
   if (first_)
   {
      first_ = false;
      feature_ptr feature(new Feature(1));
      GDALRasterBand * red = 0;
      GDALRasterBand * green = 0;
      GDALRasterBand * blue = 0;
      GDALRasterBand * alpha = 0;
      GDALRasterBand * grey = 0; 
      for (int i = 0; i < dataset_.GetRasterCount() ;++i)
      {
         GDALRasterBand * band = dataset_.GetRasterBand(i+1);
         //if (band->GetOverviewCount() > 0)
         //{
         //  band = band->GetOverview(0);
         //}
         
#ifdef MAPNIK_DEBUG         
         int bsx,bsy;
         band->GetBlockSize(&bsx,&bsy);
         std::cout << boost::format("Block=%dx%d Type=%s Color=%s \n") % bsx % bsy
            % GDALGetDataTypeName(band->GetRasterDataType())
            % GDALGetColorInterpretationName(band->GetColorInterpretation());   
#endif
         GDALColorInterp color_interp = band->GetColorInterpretation();
         switch (color_interp)
         {
            case GCI_RedBand:
               red = band;
               break;
            case GCI_GreenBand:
               green = band;
               break;
            case GCI_BlueBand:
               blue = band;
               break;
            case GCI_AlphaBand:
               alpha = band;
               break;
            case GCI_GrayIndex:
               grey = band;
               break;
            case GCI_PaletteIndex:
            {	
               grey = band;
               GDALColorTable *color_table = band->GetColorTable();
				
               if ( color_table)
               {
                  int count = color_table->GetColorEntryCount();
#ifdef MAPNIK_DEBUG
                  std::cout << "Color Table count = " << count << "\n";
#endif 
                  for ( int i = 0; i < count; i++ )
                  {
                     const GDALColorEntry *ce = color_table->GetColorEntry ( i );
                     if (!ce ) continue;
#ifdef MAPNIK_DEBUG
                     std::cout << "color entry RGB (" << ce->c1 <<"," <<ce->c2 << "," << ce->c3 << ")\n"; 
#endif
                  }
               }	
               break;
            }
            case GCI_Undefined:
               grey = band;
               break;	
            default:
               break;
               ;
         }
      }
      
      unsigned raster_xsize = dataset_.GetRasterXSize();
      unsigned raster_ysize = dataset_.GetRasterYSize();
      double tr[6];
      dataset_.GetGeoTransform(tr);
      double x0 = tr[0];
      double y0 = tr[3];
      double x1 = tr[0] + raster_xsize * tr[1] + raster_ysize * tr[2];
      double y1 = tr[3] + raster_xsize * tr[4] + raster_ysize * tr[5];
      Envelope<double> raster_extent(x0,y0,x1,y1);
      CoordTransform t (raster_xsize,raster_ysize,raster_extent,0,0);
      Envelope<double> intersection = raster_extent.intersect(query_extent_);
      Envelope<double> box = t.forward(intersection);
      
      int start_x = int(box.minx()+0.5);
      int start_y = int(box.miny()+0.5);
      int width = int(box.width()+0.5);
      int height = int(box.height()+0.5);
      
      if (width > 0 && height > 0)
      {
         mapnik::ImageData32 image(width,height);
         image.set(0xffffffff);
         
         if (red && green && blue)
         {
            red->RasterIO  (GF_Read,start_x,start_y,width,height, image.getBytes() + 0, width,height, GDT_Byte,4,4*width);
            green->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 1, width,height, GDT_Byte,4,4*width);
            blue->RasterIO (GF_Read,start_x,start_y,width,height, image.getBytes() + 2, width,height, GDT_Byte,4,4*width);
            if (alpha)
            {
               alpha->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 3, width,height, GDT_Byte,4,4*width);
            }
         }
         else if (grey)
         {
            grey->RasterIO(GF_Read,start_x,start_y,width,height,image.getBytes() + 0, width, height,GDT_Byte,4,4*width);
            grey->RasterIO(GF_Read,start_x,start_y,width,height,image.getBytes() + 1, width, height,GDT_Byte,4,4*width);
            grey->RasterIO(GF_Read,start_x,start_y,width,height,image.getBytes() + 2, width, height,GDT_Byte,4,4*width);
            
            if (alpha)
            {
               alpha->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 3, width,height, GDT_Byte,4,4*width);
            }
         }
         feature->set_raster(mapnik::raster_ptr(new mapnik::raster(intersection,image)));
         return feature;
      }
   }
   return feature_ptr();
}
