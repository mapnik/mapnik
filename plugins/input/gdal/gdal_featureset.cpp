/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/feature_factory.hpp>

// boost
#include <boost/format.hpp>

#include "gdal_featureset.hpp"
#include <gdal_priv.h>

using mapnik::query;
using mapnik::coord2d;
using mapnik::box2d;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::CoordTransform;
using mapnik::geometry_type;
using mapnik::datasource_exception;
using mapnik::feature_factory;

#ifdef _WINDOWS
using mapnik::rint;
#endif

gdal_featureset::gdal_featureset(GDALDataset& dataset,
                                 int band,
                                 gdal_query q,
                                 mapnik::box2d<double> extent,
                                 unsigned width,
                                 unsigned height,
                                 int nbands,
                                 double dx,
                                 double dy,
                                 double filter_factor,
                                 boost::optional<double> const& nodata)
    : dataset_(dataset),
      ctx_(boost::make_shared<mapnik::context_type>()),
      band_(band),
      gquery_(q),
      raster_extent_(extent),
      raster_width_(width),
      raster_height_(height),
      dx_(dx),
      dy_(dy),
      nbands_(nbands),
      filter_factor_(filter_factor),
      nodata_value_(nodata),
      first_(true)
{
    ctx_->push("value");
    ctx_->push("NODATA");
}

gdal_featureset::~gdal_featureset()
{
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Closing Dataset=" << &dataset_;

    GDALClose(&dataset_);
}

feature_ptr gdal_featureset::next()
{
    if (first_)
    {
        first_ = false;

        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Next feature in Dataset=" << &dataset_;

        query *q = boost::get<query>(&gquery_);
        if (q)
        {
            return get_feature(*q);
        }
        else
        {
            coord2d *p = boost::get<coord2d>(&gquery_);
            if (p)
            {
                return get_feature_at_point(*p);
            }
        }
        // should never reach here
    }
    return feature_ptr();
}

feature_ptr gdal_featureset::get_feature(mapnik::query const& q)
{
    feature_ptr feature = feature_factory::create(ctx_,1);

    GDALRasterBand * red = 0;
    GDALRasterBand * green = 0;
    GDALRasterBand * blue = 0;
    GDALRasterBand * alpha = 0;
    GDALRasterBand * grey = 0;

    /*
#ifdef MAPNIK_LOG
      double tr[6];
      dataset_.GetGeoTransform(tr);

      const double dx = tr[1];
      const double dy = tr[5];
      MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: dx_=" << dx_ << " dx=" << dx << " dy_=" << dy_ << "dy=" << dy;
#endif
    */

    CoordTransform t(raster_width_, raster_height_, raster_extent_, 0, 0);
    box2d<double> intersect = raster_extent_.intersect(q.get_bbox());
    box2d<double> box = t.forward(intersect);

    //size of resized output pixel in source image domain
    double margin_x = 1.0 / (fabs(dx_) * boost::get<0>(q.resolution()));
    double margin_y = 1.0 / (fabs(dy_) * boost::get<1>(q.resolution()));
    if (margin_x < 1)
    {
        margin_x = 1.0;
    }
    if (margin_y < 1)
    {
        margin_y = 1.0;
    }

    //select minimum raster containing whole box
    int x_off = rint(box.minx() - margin_x);
    int y_off = rint(box.miny() - margin_y);
    int end_x = rint(box.maxx() + margin_x);
    int end_y = rint(box.maxy() + margin_y);

    //clip to available data
    if (x_off < 0)
    {
        x_off = 0;
    }
    if (y_off < 0)
    {
        y_off = 0;
    }
    if (end_x > (int)raster_width_)
    {
        end_x = raster_width_;
    }
    if (end_y > (int)raster_height_)
    {
        end_y = raster_height_;
    }
    int width = end_x - x_off;
    int height = end_y - y_off;

    // don't process almost invisible data
    if (box.width() < 0.5)
    {
        width = 0;
    }
    if (box.height() < 0.5)
    {
        height = 0;
    }

    //calculate actual box2d of returned raster
    box2d<double> feature_raster_extent(x_off, y_off, x_off + width, y_off + height);
    intersect = t.backward(feature_raster_extent);

    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Raster extent=" << raster_extent_;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: View extent=" << intersect;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Query resolution=" << boost::get<0>(q.resolution()) << "," << boost::get<1>(q.resolution());
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: StartX=" << x_off << " StartY=" << y_off << " Width=" << width << " Height=" << height;

    if (width > 0 && height > 0)
    {
        double width_res = boost::get<0>(q.resolution());
        double height_res = boost::get<1>(q.resolution());
        int im_width = int(width_res * intersect.width() + 0.5);
        int im_height = int(height_res * intersect.height() + 0.5);

        // if layer-level filter_factor is set, apply it
        if (filter_factor_)
        {
            im_width *= filter_factor_;
            im_height *= filter_factor_;

            MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Applying layer filter_factor=" << filter_factor_;
        }
        // otherwise respect symbolizer level factor applied to query, default of 1.0
        else
        {
            double sym_downsample_factor = q.get_filter_factor();
            im_width *= sym_downsample_factor;
            im_height *= sym_downsample_factor;
        }

        // case where we need to avoid upsampling so that the
        // image can be later scaled within raster_symbolizer
        if (im_width >= width || im_height >= height)
        {
            im_width = width;
            im_height = height;
        }

        if (im_width > 0 && im_height > 0)
        {
            mapnik::raster_ptr raster = boost::make_shared<mapnik::raster>(intersect, im_width, im_height);
            feature->set_raster(raster);
            mapnik::image_data_32 & image = raster->data_;
            image.set(0xffffffff);

            MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Image Size=(" << im_width << "," << im_height << ")";
            MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Reading band=" << band_;

            if (band_ > 0) // we are querying a single band
            {
                if (band_ > nbands_)
                {
                    throw datasource_exception((boost::format("GDAL Plugin: '%d' is an invalid band, dataset only has '%d' bands\n") % band_ % nbands_).str());
                }

                float* imageData = (float*)image.getBytes();
                GDALRasterBand * band = dataset_.GetRasterBand(band_);
                int hasNoData(0);
                double nodata(0);
                if (nodata_value_)
                {
                    hasNoData = 1;
                    nodata = *nodata_value_;
                }
                else
                {
                    nodata = band->GetNoDataValue(&hasNoData);
                }
                band->RasterIO(GF_Read, x_off, y_off, width, height,
                               imageData, image.width(), image.height(),
                               GDT_Float32, 0, 0);

                if (hasNoData)
                {
                    feature->put("NODATA",nodata);
                }
            }
            else // working with all bands
            {
                for (int i = 0; i < nbands_; ++i)
                {
                    GDALRasterBand * band = dataset_.GetRasterBand(i + 1);

#ifdef MAPNIK_LOG
                    get_overview_meta(band);
#endif

                    GDALColorInterp color_interp = band->GetColorInterpretation();
                    switch (color_interp)
                    {
                    case GCI_RedBand:
                        red = band;

                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found red band";

                        break;
                    case GCI_GreenBand:
                        green = band;

                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found green band";

                        break;
                    case GCI_BlueBand:
                        blue = band;

                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found blue band";

                        break;
                    case GCI_AlphaBand:
                        alpha = band;

                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found alpha band";

                        break;
                    case GCI_GrayIndex:
                        grey = band;

                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found gray band";

                        break;
                    case GCI_PaletteIndex:
                    {
                        grey = band;

                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found gray band, and colortable...";

                        GDALColorTable *color_table = band->GetColorTable();

                        if (color_table)
                        {
                            int count = color_table->GetColorEntryCount();

                            MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Color Table count=" << count;

                            for (int j = 0; j < count; j++)
                            {
                                const GDALColorEntry *ce = color_table->GetColorEntry (j);
                                if (! ce) continue;

                                MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Color entry RGB=" << ce->c1 << "," <<ce->c2 << "," << ce->c3;
                            }
                        }
                        break;
                    }
                    case GCI_Undefined:
                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found undefined band (assumming gray band)";

                        grey = band;
                        break;
                    default:
                        MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Band type unknown!";

                        break;
                    }
                }

                if (red && green && blue)
                {
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Processing rgb bands...";

                    int hasNoData = 0;
                    double nodata = 0.0;
                    if (nodata_value_)
                    {
                        hasNoData = 1;
                        nodata = *nodata_value_;
                    }
                    else
                    {
                        nodata = red->GetNoDataValue(&hasNoData);
                    }
                    if (hasNoData)
                    {
                        feature->put("NODATA",nodata);
                    }
                    GDALColorTable *color_table = red->GetColorTable();

                    if (! alpha && hasNoData && ! color_table)
                    {
                        // first read the data in and create an alpha channel from the nodata values
                        float* imageData = (float*)image.getBytes();
                        red->RasterIO(GF_Read, x_off, y_off, width, height,
                                      imageData, image.width(), image.height(),
                                      GDT_Float32, 0, 0);

                        int len = image.width() * image.height();

                        for (int i = 0; i < len; ++i)
                        {
                            if (nodata == imageData[i])
                            {
                                *reinterpret_cast<unsigned *>(&imageData[i]) = 0;
                            }
                            else
                            {
                                *reinterpret_cast<unsigned *>(&imageData[i]) = 0xFFFFFFFF;
                            }
                        }

                    }

                    red->RasterIO(GF_Read, x_off, y_off, width, height, image.getBytes() + 0,
                                  image.width(), image.height(), GDT_Byte, 4, 4 * image.width());
                    green->RasterIO(GF_Read, x_off, y_off, width, height, image.getBytes() + 1,
                                    image.width(), image.height(), GDT_Byte, 4, 4 * image.width());
                    blue->RasterIO(GF_Read, x_off, y_off, width, height, image.getBytes() + 2,
                                   image.width(), image.height(), GDT_Byte, 4, 4 * image.width());
                }
                else if (grey)
                {
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Processing gray band...";

                    int hasNoData(0);
                    double nodata(0);
                    if (nodata_value_)
                    {
                        hasNoData = 1;
                        nodata = *nodata_value_;
                    }
                    else
                    {
                        nodata = grey->GetNoDataValue(&hasNoData);
                    }
                    GDALColorTable* color_table = grey->GetColorTable();

                    if (hasNoData && ! color_table)
                    {
                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: No data value for layer=" << nodata;

                        feature->put("NODATA",nodata);
                        // first read the data in and create an alpha channel from the nodata values
                        float* imageData = (float*)image.getBytes();
                        grey->RasterIO(GF_Read, x_off, y_off, width, height,
                                       imageData, image.width(), image.height(),
                                       GDT_Float32, 0, 0);

                        int len = image.width() * image.height();

                        for (int i = 0; i < len; ++i)
                        {
                            if (nodata == imageData[i])
                            {
                                *reinterpret_cast<unsigned *>(&imageData[i]) = 0;
                            }
                            else
                            {
                                *reinterpret_cast<unsigned *> (&imageData[i]) = 0xFFFFFFFF;
                            }
                        }
                    }

                    grey->RasterIO(GF_Read, x_off, y_off, width, height, image.getBytes() + 0,
                                   image.width(), image.height(), GDT_Byte, 4, 4 * image.width());
                    grey->RasterIO(GF_Read,x_off, y_off, width, height, image.getBytes() + 1,
                                   image.width(), image.height(), GDT_Byte, 4, 4 * image.width());
                    grey->RasterIO(GF_Read,x_off, y_off, width, height, image.getBytes() + 2,
                                   image.width(), image.height(), GDT_Byte, 4, 4 * image.width());

                    if (color_table)
                    {
                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Loading colour table...";

                        unsigned nodata_value = static_cast<unsigned>(nodata);
                        if (hasNoData)
                        {
                            feature->put("NODATA",static_cast<int>(nodata_value));
                        }
                        for (unsigned y = 0; y < image.height(); ++y)
                        {
                            unsigned int* row = image.getRow(y);
                            for (unsigned x = 0; x < image.width(); ++x)
                            {
                                unsigned value = row[x] & 0xff;
                                if (hasNoData && (value == nodata_value))
                                {
                                    // make no data fully alpha
                                    row[x] = 0;
                                }
                                else
                                {
                                    const GDALColorEntry *ce = color_table->GetColorEntry(value);
                                    if (ce)
                                    {
                                        // TODO - big endian support
                                        row[x] = (ce->c4 << 24)| (ce->c3 << 16) |  (ce->c2 << 8) | (ce->c1) ;
                                    }
                                    else
                                    {
                                        // make lacking color entry fully alpha
                                        // note - gdal_translate makes black
                                        row[x] = 0;
                                    }
                                }
                            }
                        }
                    }
                }
                if (alpha)
                {
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: processing alpha band...";

                    alpha->RasterIO(GF_Read, x_off, y_off, width, height, image.getBytes() + 3,
                                    image.width(), image.height(), GDT_Byte, 4, 4 * image.width());
                }
            }
            return feature;
        }
    }
    return feature_ptr();
}


feature_ptr gdal_featureset::get_feature_at_point(mapnik::coord2d const& pt)
{
    if (band_ > 0)
    {
        unsigned raster_xsize = dataset_.GetRasterXSize();
        unsigned raster_ysize = dataset_.GetRasterYSize();

        double gt[6];
        dataset_.GetGeoTransform(gt);

        double det = gt[1] * gt[5] - gt[2] * gt[4];
        // subtract half a pixel width & height because gdal coord reference
        // is the top-left corner of a pixel, not the center.
        double X = pt.x - gt[0] - gt[1]/2;
        double Y = pt.y - gt[3] - gt[5]/2;
        double det1 = gt[1]*Y + gt[4]*X;
        double det2 = gt[2]*Y + gt[5]*X;
        unsigned x = static_cast<unsigned>(det2/det);
        unsigned y = static_cast<unsigned>(det1/det);

        if (x < raster_xsize && y < raster_ysize)
        {
            MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: pt.x=" << pt.x << " pt.y=" << pt.y;
            MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: x=" << x << " y=" << y;

            GDALRasterBand* band = dataset_.GetRasterBand(band_);
            int hasNoData;
            double nodata = band->GetNoDataValue(&hasNoData);
            double value;
            band->RasterIO(GF_Read, x, y, 1, 1, &value, 1, 1, GDT_Float64, 0, 0);

            if (! hasNoData || value != nodata)
            {
                // construct feature
                feature_ptr feature = feature_factory::create(ctx_,1);
                geometry_type * point = new geometry_type(mapnik::Point);
                point->move_to(pt.x, pt.y);
                feature->add_geometry(point);
                feature->put("value",value);
                return feature;
            }
        }
    }
    return feature_ptr();
}

#ifdef MAPNIK_LOG
void gdal_featureset::get_overview_meta(GDALRasterBand* band)
{
    int band_overviews = band->GetOverviewCount();
    if (band_overviews > 0)
    {
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: " << band_overviews << " overviews found!";

        for (int b = 0; b < band_overviews; b++)
        {
            GDALRasterBand * overview = band->GetOverview(b);
            MAPNIK_LOG_DEBUG(gdal) << boost::format("gdal_featureset: Overview=%d Width=%d Height=%d")
                % b % overview->GetXSize() % overview->GetYSize();
        }
    }
    else
    {
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: No overviews found!";
    }

    int bsx,bsy;
    double scale;
    band->GetBlockSize(&bsx, &bsy);
    scale = band->GetScale();

    MAPNIK_LOG_DEBUG(gdal) << boost::format("gdal_featureset: Block=%dx%d Scale=%f Type=%s Color=%s") % bsx % bsy % scale
        % GDALGetDataTypeName(band->GetRasterDataType())
        % GDALGetColorInterpretationName(band->GetColorInterpretation());
}
#endif
