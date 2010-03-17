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

// boost
#include <boost/format.hpp>

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
   std::clog << "GDAL Plugin: closing dataset = " << &dataset_ << "\n";
#endif
   GDALClose(&dataset_);
}

feature_ptr gdal_featureset::next()
{
   if (first_)
   {
      first_ = false;
#ifdef MAPNIK_DEBUG
      std::clog << "GDAL Plugin: featureset, dataset = " << &dataset_ << "\n";
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

   int nbands = dataset_.GetRasterCount();
   
   unsigned raster_width = dataset_.GetRasterXSize();
   unsigned raster_height = dataset_.GetRasterYSize();
   
   // TODO - pull from class attributes...
   double tr[6];
   dataset_.GetGeoTransform(tr);
   double dx = tr[1];
   double dy = tr[5];
   double x0 = tr[0]; // minx
   double y0 = tr[3]; // miny
   double x1 = tr[0] + raster_width * dx + raster_height * tr[2]; // maxx
   double y1 = tr[3] + raster_width * tr[4] + raster_height * dy; // maxy
   Envelope<double> raster_extent(x0,y0,x1,y1); 
   
   CoordTransform t(raster_width,raster_height,raster_extent,0,0);
   Envelope<double> intersect = raster_extent.intersect(q.get_bbox());
   Envelope<double> box = t.forward(intersect);

   //size of resized output pixel in source image domain
   double margin_x = 1.0/(fabs(dx)*q.resolution());
   double margin_y = 1.0/(fabs(dy)*q.resolution());
   if (margin_x < 1)
      margin_x = 1.0;
   if (margin_y < 1)
      margin_y = 1.0;
   //select minimum raster containing whole box
   int x_off = rint(box.minx() - margin_x);
   int y_off = rint(box.miny() - margin_y);
   int end_x = rint(box.maxx() + margin_x);
   int end_y = rint(box.maxy() + margin_y);
   //clip to available data
   if (x_off < 0)
      x_off = 0;
   if (y_off < 0)
      y_off = 0;
   if (end_x > (int)raster_width)
      end_x = raster_width;
   if (end_y > (int)raster_height)
      end_y = raster_height;
   int width = end_x - x_off;
   int height = end_y - y_off;
   // don't process almost invisible data
   if (box.width() < 0.5)
      width = 0;
   if (box.height() < 0.5)
      height = 0;
   //calculate actual envelope of returned raster
   Envelope<double> feature_raster_extent(x_off, y_off, x_off+width, y_off+height); 
   intersect = t.backward(feature_raster_extent);
    
#ifdef MAPNIK_DEBUG         
   std::clog << "GDAL Plugin: Raster extent=" << raster_extent << "\n";
   std::clog << "GDAL Plugin: View extent=" << intersect << "\n";
   std::clog << "GDAL Plugin: Query resolution=" << q.resolution() << "\n";
   std::clog << boost::format("GDAL Plugin: StartX=%d StartY=%d Width=%d Height=%d \n") % x_off % y_off % width % height;
#endif

   if (width > 0 && height > 0)
   {
      int im_width = int(q.resolution() * intersect.width() + 0.5);
      int im_height = int(q.resolution() * intersect.height() + 0.5);

      // case where we need to avoid upsampling so that the
      // image can be later scaled within raster_symbolizer
      if (im_width >= width || im_height >= height)
      {
          im_width = width;
          im_height = height;
      }

      if (im_width > 0 && im_height > 0)
      {
          mapnik::ImageData32 image(im_width, im_height);
          image.set(0xffffffff);
             
    #ifdef MAPNIK_DEBUG
          std::clog << "GDAL Plugin: Image Size=(" << im_width << "," << im_height << ")\n";
          std::clog << "GDAL Plugin: Reading band " << band_ << "\n";
    #endif

          if (band_>0) // we are querying a single band
          {
             float *imageData = (float*)image.getBytes();
             GDALRasterBand * band = dataset_.GetRasterBand(band_);
             band->RasterIO(GF_Read, x_off, y_off, width, height,
                            imageData, image.width(), image.height(),
                            GDT_Float32, 0, 0);

             feature->set_raster(mapnik::raster_ptr(new mapnik::raster(intersect,image)));
          }
          
          else // working with all bands
          {
             for (int i = 0; i < nbands; ++i)
             {
                GDALRasterBand * band = dataset_.GetRasterBand(i+1);
             
    #ifdef MAPNIK_DEBUG
                get_overview_meta(band);  
    #endif

                GDALColorInterp color_interp = band->GetColorInterpretation();
                switch (color_interp)
                {
                   case GCI_RedBand:
                      red = band;
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found red band" << "\n";
    #endif
                      break;
                   case GCI_GreenBand:
                      green = band;
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found green band" << "\n";
    #endif
                      break;
                   case GCI_BlueBand:
                      blue = band;
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found blue band" << "\n";
    #endif
                      break;
                   case GCI_AlphaBand:
                      alpha = band;
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found alpha band" << "\n";
    #endif
                      break;
                   case GCI_GrayIndex:
                      grey = band;
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found gray band" << "\n";
    #endif
                      break;
                   case GCI_PaletteIndex:
                   {
                      grey = band;
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found gray band, and colortable..." << "\n";
    #endif
                      GDALColorTable *color_table = band->GetColorTable();
                
                      if ( color_table)
                      {
                         int count = color_table->GetColorEntryCount();
    #ifdef MAPNIK_DEBUG
                         std::clog << "GDAL Plugin: Color Table count = " << count << "\n";
    #endif 
                         for ( int i = 0; i < count; i++ )
                         {
                            const GDALColorEntry *ce = color_table->GetColorEntry ( i );
                            if (!ce ) continue;
    #ifdef MAPNIK_DEBUG
                            std::clog << "GDAL Plugin: Color entry RGB (" << ce->c1 << "," <<ce->c2 << "," << ce->c3 << ")\n"; 
    #endif
                         }
                      }
                      break;
                   }
                   case GCI_Undefined:
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: Found undefined band (assumming gray band)" << "\n";
    #endif
                      grey = band;
                      break;
                   default:
    #ifdef MAPNIK_DEBUG
                      std::clog << "GDAL Plugin: band type unknown!" << "\n";
    #endif
                      break;
                }
             }

              if (red && green && blue)
             {
    #ifdef MAPNIK_DEBUG
                std::clog << "GDAL Plugin: processing rgb bands..." << "\n";
    #endif
                red->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 0,
                    image.width(),image.height(),GDT_Byte,4,4*image.width());
                green->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 1,
                    image.width(),image.height(),GDT_Byte,4,4*image.width());
                blue->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 2,
                    image.width(),image.height(),GDT_Byte,4,4*image.width());
             }
             else if (grey)
             {
    #ifdef MAPNIK_DEBUG
                std::clog << "GDAL Plugin: processing gray band..." << "\n";
    #endif
                grey->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 0,
                    image.width(),image.height(),GDT_Byte, 4, 4 * image.width());
                grey->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 1,
                    image.width(),image.height(),GDT_Byte, 4, 4 * image.width());
                grey->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 2,
                    image.width(),image.height(),GDT_Byte, 4, 4 * image.width());
             }

             if (alpha)
             {
    #ifdef MAPNIK_DEBUG
                std::clog << "GDAL Plugin: processing alpha band..." << "\n";
    #endif
                alpha->RasterIO(GF_Read,x_off,y_off,width,height,image.getBytes() + 3,
                    image.width(),image.height(),GDT_Byte,4,4*image.width());
             }

             feature->set_raster(mapnik::raster_ptr(new mapnik::raster(intersect,image)));
          }
          return feature;
      }
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
         std::clog << boost::format("GDAL Plugin: pt.x=%f pt.y=%f\n") % pt.x % pt.y;
         std::clog << boost::format("GDAL Plugin: x=%f y=%f\n") % x % y;
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

void gdal_featureset::get_overview_meta(GDALRasterBand * band)
{
    int band_overviews = band->GetOverviewCount();    
    if (band_overviews > 0)
    {
        std::clog << "GDAL Plugin: "<< band_overviews << " overviews found!" << "\n";
        for (int b = 0; b < band_overviews; b++)
        {
            GDALRasterBand * overview = band->GetOverview (b);
            std::clog << boost::format("GDAL Plugin: Overview=%d Width=%d Height=%d \n")
                                   % b % overview->GetXSize() % overview->GetYSize();
        }
    }
    else
    {
        std::clog << "GDAL Plugin: No overviews found!" << "\n";
    }
    
    int bsx,bsy;
    double scale;
    band->GetBlockSize(&bsx,&bsy);
    scale = band->GetScale();
    std::clog << boost::format("GDAL Plugin: Block=%dx%d Scale=%f Type=%s Color=%s \n") % bsx % bsy % scale
       % GDALGetDataTypeName(band->GetRasterDataType())
       % GDALGetColorInterpretationName(band->GetColorInterpretation()); 
}
