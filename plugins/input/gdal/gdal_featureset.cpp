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
using mapnik::coord2d;
using mapnik::Envelope;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::CoordTransform;
using mapnik::point_impl;
using mapnik::geometry2d;


gdal_featureset::gdal_featureset(GDALDataset & dataset, int band, gdal_query q)
   : dataset_(dataset),
     band_(band),
     gquery_(q),
     first_(true) {}

gdal_featureset::~gdal_featureset()
{
#ifdef MAPNIK_DEBUG
      std::clog << "closing dataset = " << &dataset_ << "\n";
#endif
   GDALClose(&dataset_);
}

feature_ptr gdal_featureset::next()
{
   if (first_)
   {
      first_ = false;
#ifdef MAPNIK_DEBUG
      std::clog << "featureset, dataset = " << &dataset_ << "\n";
#endif

      query *q = boost::get<query>(&gquery_);
      if(q) {
          return get_feature(*q);
      } else {
          coord2d *p = boost::get<coord2d>(&gquery_);
          if(p) {
              return get_feature_at_point(*p);
          }
      }
      // should never reach here
   }
   return feature_ptr();
}

       
feature_ptr gdal_featureset::get_feature(mapnik::query const& q)
{
   feature_ptr feature(new Feature(1));

   GDALRasterBand * red = 0;
   GDALRasterBand * green = 0;
   GDALRasterBand * blue = 0;
   GDALRasterBand * alpha = 0;
   GDALRasterBand * grey = 0; 

   unsigned raster_xsize = dataset_.GetRasterXSize();
   unsigned raster_ysize = dataset_.GetRasterYSize();
   int nbands = dataset_.GetRasterCount();

   double tr[6];
   dataset_.GetGeoTransform(tr);
   double dx = tr[1];
   double dy = tr[5];
   double x0 = tr[0];
   double y0 = tr[3];
   double x1 = tr[0] + raster_xsize * dx + raster_ysize * tr[2];
   double y1 = tr[3] + raster_xsize * tr[4] + raster_ysize * dy;
   Envelope<double> raster_extent(x0,y0,x1,y1);
   CoordTransform t (raster_xsize,raster_ysize,raster_extent,0,0);
   Envelope<double> intersection = raster_extent.intersect(q.get_bbox());
   Envelope<double> box = t.forward(intersection);

   int start_x, start_y;
   if (dx>0)
       start_x = int(std::floor(intersection.minx()-raster_extent.minx())/dx);
   else
       // west-left raster
       start_x = -int(std::floor(raster_extent.maxx()-intersection.maxx())/dx);
   if(dy>0)
       start_y = int(std::floor(intersection.miny()-raster_extent.miny())/dy);
   else
       // north-up raster
       start_y = -int(std::floor(raster_extent.maxy()-intersection.maxy())/dy);

   int width = int(box.width()+0.5);
   int height = int(box.height()+0.5);

#ifdef MAPNIK_DEBUG         
   std::cout << "raster_extent=" << raster_extent << "\n";
   std::cout << "intersection=" << intersection << "\n";
   std::cout << boost::format("RasterWidth=%d RasterHeight=%d \n") % raster_xsize % raster_ysize;
   std::cout << boost::format("StartX=%d StartY=%d Width=%d Height=%d \n") % start_x % start_y % width % height;
#endif

   if (width > 0 && height > 0)
   {
      int im_width = q.resolution()*intersection.width();
      int im_height = q.resolution()*intersection.height();

      mapnik::ImageData32 image(im_width, im_height);
      image.set(0xffffffff);
         
#ifdef MAPNIK_DEBUG         
      std::cout << "band=" << band_ << "\n";
      std::cout << "im_size=(" << im_width << "," << im_height << ")\n";
      std::cout << "box_size=(" << width << "," << height << ")\n";
#endif

      if(band_>0)      // we are querying a single band
      {
         float *imageData = (float*)image.getBytes();
         GDALRasterBand * band = dataset_.GetRasterBand(band_);
         band->RasterIO(GF_Read, start_x, start_y, width, height,
                        imageData, image.width(), image.height(),
                        GDT_Float32, 0, 0);

         feature->set_raster(mapnik::raster_ptr(new mapnik::raster(intersection,image)));
      }
      
      else             // working with all bands
      {
         for (int i = 0; i < nbands; ++i)
         {
            GDALRasterBand * band = dataset_.GetRasterBand(i+1);
         
#ifdef MAPNIK_DEBUG
            int band_overviews = band->GetOverviewCount();
            // Note: overviews are picked automatically during GdalDataset::RasterIO below
            // when nXSize > nPixelSpace and nYSize > nLineSpace 
            
            if (band_overviews > 0)
            {
                for (int b = 0; b < band_overviews; b++)
                {
                    GDALRasterBand * overview = band->GetOverview (b);
                    std::cout << boost::format("Overview=%d Width=%d Height=%d \n")
                                           % b % overview->GetXSize() % overview->GetYSize();
                }
            }

            int bsx,bsy;
            double scale;
            band->GetBlockSize(&bsx,&bsy);
            scale = band->GetScale();
            std::cout << boost::format("Block=%dx%d Scale=%f Type=%s Color=%s \n") % bsx % bsy % scale
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
            }
         }

         if (red && green && blue)
         {
            red->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 0, image.width(),image.height(), GDT_Byte,4,4*image.width());
            green->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 1, image.width(),image.height(), GDT_Byte,4,4*image.width());
            blue->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 2, image.width(),image.height(), GDT_Byte,4,4*image.width());
         }
         else if (grey)
         {
            grey->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 0, image.width(),image.height(), GDT_Byte,4,4*image.width());
            grey->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 1, image.width(),image.height(), GDT_Byte,4,4*image.width());
            grey->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 2, image.width(),image.height(), GDT_Byte,4,4*image.width());
         }

         if (alpha)
         {
            alpha->RasterIO(GF_Read,start_x,start_y,width,height, image.getBytes() + 3, image.width(),image.height(), GDT_Byte,4,4*image.width());
         }

         feature->set_raster(mapnik::raster_ptr(new mapnik::raster(intersection,image)));
      }
      return feature;
   }
   return feature_ptr();
}



feature_ptr gdal_featureset::get_feature_at_point(mapnik::coord2d const& pt)
{
   if (band_>0) {

      unsigned raster_xsize = dataset_.GetRasterXSize();
      unsigned raster_ysize = dataset_.GetRasterYSize();

      double gt[6];
      dataset_.GetGeoTransform(gt);

      double det = gt[1]*gt[5] - gt[2]*gt[4];
      // subtract half a pixel width & height because gdal coord reference
      // is the top-left corner of a pixel, not the center.
      double X = pt.x - gt[0] - gt[1]/2;
      double Y = pt.y - gt[3] - gt[5]/2;
      double det1 = gt[1]*Y + gt[4]*X;
      double det2 = gt[2]*Y + gt[5]*X;
      unsigned x = det2/det, y = det1/det;

      if(0<=x && x<raster_xsize && 0<=y && y<raster_ysize) {
#ifdef MAPNIK_DEBUG         
         std::cout << boost::format("pt.x=%f pt.y=%f\n") % pt.x % pt.y;
         std::cout << boost::format("x=%f y=%f\n") % x % y;
#endif
         GDALRasterBand * band = dataset_.GetRasterBand(band_);
         int hasNoData;
         double nodata = band->GetNoDataValue(&hasNoData);
         double value;
         band->RasterIO(GF_Read, x, y, 1, 1, &value, 1, 1, GDT_Float64, 0, 0);
         if(!hasNoData || value!=nodata) {
            // construct feature
            feature_ptr feature(new Feature(1));
            geometry2d * point = new point_impl;
            point->move_to(pt.x, pt.y);
            feature->add_geometry(point);
            (*feature)["value"] = value;
            return feature;
         }
      }
   }
   return feature_ptr();
}
