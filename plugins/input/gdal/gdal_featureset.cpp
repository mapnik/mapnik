/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/datasource.hpp>
#include <boost/optional/optional_io.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/image.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>

// stl
#include <cmath>
#include <memory>
#include <sstream>

#include "gdal_featureset.hpp"
#include <gdal_priv.h>

using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::view_transform;
using mapnik::datasource_exception;
using mapnik::feature_factory;

#ifdef MAPNIK_LOG
namespace {

void get_overview_meta(GDALRasterBand* band)
{
    int band_overviews = band->GetOverviewCount();
    if (band_overviews > 0)
    {
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: " << band_overviews << " overviews found!";

        for (int b = 0; b < band_overviews; b++)
        {
            GDALRasterBand * overview = band->GetOverview(b);
            MAPNIK_LOG_DEBUG(gdal) << "Overview= " << b
              << " Width=" << overview->GetXSize()
              << " Height=" << overview->GetYSize();
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

    MAPNIK_LOG_DEBUG(gdal) << "Block=" << bsx << "x" << bsy
        << " Scale=" << scale
        << " Type=" << GDALGetDataTypeName(band->GetRasterDataType())
        << "Color=" << GDALGetColorInterpretationName(band->GetColorInterpretation());
}
} // anonymous ns
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
                                 boost::optional<double> const& nodata,
                                 double nodata_tolerance)
    : dataset_(dataset),
      ctx_(std::make_shared<mapnik::context_type>()),
      band_(band),
      gquery_(q),
      raster_extent_(extent),
      raster_width_(width),
      raster_height_(height),
      dx_(dx),
      dy_(dy),
      nbands_(nbands),
      nodata_value_(nodata),
      nodata_tolerance_(nodata_tolerance),
      first_(true)
{
    ctx_->push("nodata");
}

gdal_featureset::~gdal_featureset()
{
}

GDALRIOResampleAlg gdal_featureset::get_gdal_resample_alg(mapnik::scaling_method_e mapnik_scaling_method)
{

  switch (mapnik_scaling_method)
    {
    case mapnik::SCALING_NEAR: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Nearest Neighbor resampling"; return GRIORA_NearestNeighbour;
    case mapnik::SCALING_BILINEAR: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Bilinear resampling"; return GRIORA_Bilinear;
    case mapnik::SCALING_BICUBIC: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Cubic resampling"; return GRIORA_Cubic;
    case mapnik::SCALING_SPLINE16: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL CubicSpline resampling"; return GRIORA_CubicSpline;
    case mapnik::SCALING_SPLINE36: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL CubicSpline resampling"; return GRIORA_CubicSpline;
    case mapnik::SCALING_HANNING: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Gauss resampling"; return GRIORA_Gauss;
    case mapnik::SCALING_HAMMING: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Gauss resampling"; return GRIORA_Gauss;
    case mapnik::SCALING_HERMITE: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Cubic resampling"; return GRIORA_Cubic;
    case mapnik::SCALING_KAISER: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Lanczos resampling"; return GRIORA_Lanczos;
    case mapnik::SCALING_QUADRIC: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Lanczos resampling"; return GRIORA_Lanczos;
    case mapnik::SCALING_CATROM: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Cubic resampling"; return GRIORA_Cubic;
    case mapnik::SCALING_GAUSSIAN: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Gauss resampling"; return GRIORA_Gauss;
    case mapnik::SCALING_BESSEL: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Lanczos resampling"; return GRIORA_Lanczos;
    case mapnik::SCALING_MITCHELL: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Cubic resampling"; return GRIORA_Cubic;
    case mapnik::SCALING_SINC: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Lanczos resampling"; return GRIORA_Lanczos;
    case mapnik::SCALING_LANCZOS: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Lanczos resampling"; return GRIORA_Lanczos;
    case mapnik::SCALING_BLACKMAN: MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Using GDAL Lanczos resampling"; return GRIORA_Lanczos;
    }

  MAPNIK_LOG_WARN(gdal) << "No valid resampling method found using Nearest Neighbor";
  return GRIORA_NearestNeighbour;

}

feature_ptr gdal_featureset::next()
{
    if (first_)
    {
        first_ = false;
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Next feature in Dataset=" << &dataset_;
        return mapnik::util::apply_visitor(query_dispatch(*this), gquery_);
    }
    return feature_ptr();
}

feature_ptr gdal_featureset::get_feature(mapnik::query const& q)
{
    feature_ptr feature = feature_factory::create(ctx_,1);
    int raster_has_nodata = 0;
    double raster_nodata = 0;
    GDALRasterBand * red = 0;
    GDALRasterBand * green = 0;
    GDALRasterBand * blue = 0;
    GDALRasterBand * alpha = 0;
    GDALRasterBand * grey = 0;
    CPLErr raster_io_error = CE_None;

    /*
#ifdef MAPNIK_LOG
      double tr[6];
      dataset_.GetGeoTransform(tr);

      const double dx = tr[1];
      const double dy = tr[5];
      MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: dx_=" << dx_ << " dx=" << dx << " dy_=" << dy_ << "dy=" << dy;
#endif
    */

    view_transform t(raster_width_, raster_height_, raster_extent_, 0, 0);
    box2d<double> intersect = raster_extent_.intersect(q.get_bbox());
    box2d<double> box = t.forward(intersect);
    double filter_factor = q.get_filter_factor();

    //size of resized output pixel in source image domain
    double margin_x = 1.0 / (std::fabs(dx_) * std::get<0>(q.resolution()));
    double margin_y = 1.0 / (std::fabs(dy_) * std::get<1>(q.resolution()));
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: margin_x=" << margin_x;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: margin_y=" << margin_y;
    if (margin_x < 1)
    {
        margin_x = 1.0;
    }
    if (margin_y < 1)
    {
        margin_y = 1.0;
    }

    //select minimum raster containing whole box
    int x_off = static_cast<int>(std::floor((box.minx() - margin_x) + .5));
    int y_off = static_cast<int>(std::floor((box.miny() - margin_y) + .5));
    int end_x = static_cast<int>(std::floor((box.maxx() + margin_x) + .5));
    int end_y = static_cast<int>(std::floor((box.maxy() + margin_y) + .5));
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: x_off=" << x_off;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: y_off=" << y_off;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: end_x=" << end_x;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: end_y=" << end_y;

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

    // width and height of the portion of the source image we are requesting
    int width = end_x - x_off;
    int height = end_y - y_off;

    double im_offset_x = x_off;
    double im_offset_y = y_off;

    // resolution is 1 / requested pixel size
    const double width_res = std::get<0>(q.resolution());
    const double height_res = std::get<1>(q.resolution());
    int im_width = int(width_res * intersect.width() + 0.5);
    int im_height = int(height_res * intersect.height() + 0.5);

    // no upsampling in gdal
    if (im_width > width)
      {im_width=width;}
    if (im_height > height)
      {im_height=height;}

    //calculate actual box2d of returned raster
    box2d<double> feature_raster_extent(x_off, y_off, x_off + width, y_off + height);

    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Feature Raster extent=" << feature_raster_extent;
    feature_raster_extent = t.backward(feature_raster_extent);

    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Raster extent=" << raster_extent_;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Feature Raster extent=" << feature_raster_extent;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: View extent=" << intersect;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Query resolution=" << std::get<0>(q.resolution()) << "," << std::get<1>(q.resolution());
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: StartX=" << x_off << " StartY=" << y_off << " Width=" << width << " Height=" << height;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: IM StartX=" << im_offset_x << " StartY=" << im_offset_y << " Width=" << im_width << " Height=" << im_height;

    if (width > 0 && height > 0)
    {

        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Requested Size from gdal=(" << width << "," << height << ")";
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Internal Image Size=(" << im_width << "," << im_height << ")";
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Reading band=" << band_;
        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: requested resampling method=" << scaling_method_to_string(q.get_scaling_method());
        GDALRasterIOExtraArg psExtraArg;
        INIT_RASTERIO_EXTRA_ARG(psExtraArg);
        psExtraArg.eResampleAlg = get_gdal_resample_alg(q.get_scaling_method());
        if (band_ > 0) // we are querying a single band
        {
            GDALRasterBand * band = dataset_.GetRasterBand(band_);
            if (band_ > nbands_)
            {
                std::ostringstream s;
                s << "GDAL Plugin: " << band_ << " is an invalid band, dataset only has " << nbands_ << "bands";
                throw datasource_exception(s.str());
            }
            GDALDataType band_type = band->GetRasterDataType();
            switch (band_type)
            {
            case GDT_Byte:
            {
                mapnik::image_gray8 image(im_width, im_height);
                image.set(std::numeric_limits<std::uint8_t>::max());
                raster_nodata = band->GetNoDataValue(&raster_has_nodata);
                raster_io_error = band->RasterIO(GF_Read, x_off, y_off, width, height,
                                                 image.data(), image.width(), image.height(),
                                                 GDT_Byte, 0, 0, &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }
                mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, image, filter_factor);
                // set nodata value to be used in raster colorizer
                if (nodata_value_) raster->set_nodata(*nodata_value_);
                else raster->set_nodata(raster_nodata);
                feature->set_raster(raster);
                break;
            }
            case GDT_Float64:
            case GDT_Float32:
            {
                mapnik::image_gray32f image(im_width, im_height);
                image.set(std::numeric_limits<float>::max());
                raster_nodata = band->GetNoDataValue(&raster_has_nodata);
                raster_io_error = band->RasterIO(GF_Read, x_off, y_off, width, height,
                                                 image.data(), image.width(), image.height(),
                                                 GDT_Float32, 0, 0, &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }
                mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, image, filter_factor);
                // set nodata value to be used in raster colorizer
                if (nodata_value_) raster->set_nodata(*nodata_value_);
                else raster->set_nodata(raster_nodata);
                feature->set_raster(raster);
                break;
            }
            case GDT_UInt16:
            {
                mapnik::image_gray16 image(im_width, im_height);
                image.set(std::numeric_limits<std::uint16_t>::max());
                raster_nodata = band->GetNoDataValue(&raster_has_nodata);
                raster_io_error = band->RasterIO(GF_Read, x_off, y_off, width, height,
                                                 image.data(), image.width(), image.height(),
                                                 GDT_UInt16, 0, 0, &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }
                mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, image, filter_factor);
                // set nodata value to be used in raster colorizer
                if (nodata_value_) raster->set_nodata(*nodata_value_);
                else raster->set_nodata(raster_nodata);
                feature->set_raster(raster);
                break;
            }
            case GDT_Int32:
            {
                mapnik::image_gray32s image(im_width, im_height);
                image.set(std::numeric_limits<std::int32_t>::max());
                raster_nodata = band->GetNoDataValue(&raster_has_nodata);
                raster_io_error = band->RasterIO(GF_Read, x_off, y_off, width, height,
                                                 image.data(), image.width(), image.height(),
                                                 GDT_Int32, 0, 0, &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }
                mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, image, filter_factor);
                // set nodata value to be used in raster colorizer
                if (nodata_value_) raster->set_nodata(*nodata_value_);
                else raster->set_nodata(raster_nodata);
                feature->set_raster(raster);
                break;
            }
            default:
            case GDT_Int16:
            {
                mapnik::image_gray16s image(im_width, im_height);
                image.set(std::numeric_limits<std::int16_t>::max());
                raster_nodata = band->GetNoDataValue(&raster_has_nodata);
                raster_io_error = band->RasterIO(GF_Read, x_off, y_off, width, height,
                                                 image.data(), image.width(), image.height(),
                                                 GDT_Int16, 0, 0, &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }
                mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, image, filter_factor);
                // set nodata value to be used in raster colorizer
                if (nodata_value_) raster->set_nodata(*nodata_value_);
                else raster->set_nodata(raster_nodata);
                feature->set_raster(raster);
                break;
            }
            }
        }
        else // working with all bands
        {
            mapnik::image_rgba8 image(im_width, im_height);
            image.set(std::numeric_limits<std::uint32_t>::max());
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
#ifdef MAPNIK_LOG
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
#endif
                    break;
                }
                case GCI_Undefined:
#if GDAL_VERSION_NUM <= 1730
                    if (nbands_ == 4)
                    {
                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found undefined band (assumming alpha band)";
                        alpha = band;
                    }
                    else
                    {
                        MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found undefined band (assumming gray band)";
                        grey = band;
                    }
#else
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Found undefined band (assumming gray band)";
                    grey = band;
#endif
                    break;
                default:
                    MAPNIK_LOG_WARN(gdal) << "gdal_featureset: Band type unknown!";
                    break;
                }
            }
            if (red && green && blue)
            {
                MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Processing rgb bands...";
                raster_nodata = red->GetNoDataValue(&raster_has_nodata);
                GDALColorTable *color_table = red->GetColorTable();
                bool has_nodata = nodata_value_ || raster_has_nodata;

                // we can deduce the alpha channel from nodata in the Byte case
                // by reusing the reading of R,G,B bands directly
                if (has_nodata && !color_table && red->GetRasterDataType() == GDT_Byte)
                {
                    double apply_nodata = nodata_value_ ? *nodata_value_ : raster_nodata;
                    // read the data in and create an alpha channel from the nodata values
                    // TODO - we assume here the nodata value for the red band applies to all bands
                    // more details about this at http://trac.osgeo.org/gdal/ticket/2734
                    float* imageData = (float*)image.bytes();
                    raster_io_error = red->RasterIO(GF_Read, x_off, y_off, width, height,
                                                    imageData, image.width(), image.height(),
                                                    GDT_Float32, 0, 0, &psExtraArg);
                    if (raster_io_error == CE_Failure) {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                    int len = image.width() * image.height();
                    for (int i = 0; i < len; ++i)
                    {
                        if (std::fabs(apply_nodata - imageData[i]) < nodata_tolerance_)
                        {
                            *reinterpret_cast<unsigned *>(&imageData[i]) = 0;
                        }
                        else
                        {
                            *reinterpret_cast<unsigned *>(&imageData[i]) = 0xFFFFFFFF;
                        }
                    }
                }

                // Use dataset RasterIO in priority in 99.9% of the cases
                if( red->GetBand() == 1 && green->GetBand() == 2 && blue->GetBand() == 3 )
                {
                    int nBandsToRead = 3;
                    if( alpha != nullptr && alpha->GetBand() == 4 && !raster_has_nodata )
                    {
                        nBandsToRead = 4;
                        alpha = nullptr; // to avoid reading it again afterwards
                    }
                    raster_io_error = dataset_.RasterIO(GF_Read, x_off, y_off, width, height,
                                                        image.bytes(),
                                                        image.width(), image.height(), GDT_Byte,
                                                        nBandsToRead, nullptr,
                                                        4, 4 * image.width(), 1, &psExtraArg);
                    if (raster_io_error == CE_Failure) {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                }
                else
                {
                    raster_io_error = red->RasterIO(GF_Read, x_off, y_off, width, height, image.bytes() + 0,
                                                    image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                    &psExtraArg);
                    if (raster_io_error == CE_Failure) {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                    raster_io_error = green->RasterIO(GF_Read, x_off, y_off, width, height, image.bytes() + 1,
                                                      image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                      &psExtraArg);
                    if (raster_io_error == CE_Failure) {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                    raster_io_error = blue->RasterIO(GF_Read, x_off, y_off, width, height, image.bytes() + 2,
                                                     image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                     &psExtraArg);
                    if (raster_io_error == CE_Failure) {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                }

                // In the case we skipped initializing the alpha channel
                if (has_nodata && !color_table && red->GetRasterDataType() == GDT_Byte)
                {
                    double apply_nodata = nodata_value_ ? *nodata_value_ : raster_nodata;
                    if( apply_nodata >= 0 && apply_nodata <= 255 )
                    {
                        int len = image.width() * image.height();
                        GByte* pabyBytes = (GByte*) image.bytes();
                        for (int i = 0; i < len; ++i)
                        {
                            // TODO - we assume here the nodata value for the red band applies to all bands
                            // more details about this at http://trac.osgeo.org/gdal/ticket/2734
                            if (std::fabs(apply_nodata - pabyBytes[4*i]) < nodata_tolerance_)
                                pabyBytes[4*i + 3] = 0;
                            else
                                pabyBytes[4*i + 3] = 255;
                        }
                    }
                }
            }
            else if (grey)
            {
                MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Processing gray band...";
                raster_nodata = grey->GetNoDataValue(&raster_has_nodata);
                GDALColorTable* color_table = grey->GetColorTable();
                bool has_nodata = nodata_value_ || raster_has_nodata;
                if (!color_table && has_nodata)
                {
                    double apply_nodata = nodata_value_ ? *nodata_value_ : raster_nodata;
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: applying nodata value for layer=" << apply_nodata;
                    // first read the data in and create an alpha channel from the nodata values
                    float* imageData = (float*)image.bytes();
                    raster_io_error = grey->RasterIO(GF_Read, x_off, y_off, width, height,
                                                     imageData, image.width(), image.height(),
                                                     GDT_Float32, 0, 0, &psExtraArg);
                    if (raster_io_error == CE_Failure)
                    {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                    int len = image.width() * image.height();
                    for (int i = 0; i < len; ++i)
                    {
                        if (std::fabs(apply_nodata - imageData[i]) < nodata_tolerance_)
                        {
                            *reinterpret_cast<unsigned *>(&imageData[i]) = 0;
                        }
                        else
                        {
                            *reinterpret_cast<unsigned *>(&imageData[i]) = 0xFFFFFFFF;
                        }
                    }
                }

                raster_io_error = grey->RasterIO(GF_Read, x_off, y_off, width, height, image.bytes() + 0,
                                                 image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                 &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }

                raster_io_error = grey->RasterIO(GF_Read,x_off, y_off, width, height, image.bytes() + 1,
                                                 image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                 &psExtraArg);
                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }

                raster_io_error = grey->RasterIO(GF_Read,x_off, y_off, width, height, image.bytes() + 2,
                                                 image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                 &psExtraArg);

                if (raster_io_error == CE_Failure)
                {
                    throw datasource_exception(CPLGetLastErrorMsg());
                }

                if (color_table)
                {
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Loading color table...";
                    for (unsigned y = 0; y < image.height(); ++y)
                    {
                        unsigned int* row = image.get_row(y);
                        for (unsigned x = 0; x < image.width(); ++x)
                        {
                            unsigned value = row[x] & 0xff;
                            const GDALColorEntry *ce = color_table->GetColorEntry(value);
                            if (ce)
                            {
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
            if (alpha)
            {
                MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: processing alpha band...";
                if (!raster_has_nodata || (red && green && blue))
                {
                    raster_io_error = alpha->RasterIO(GF_Read, x_off, y_off, width, height, image.bytes() + 3,
                                                      image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                      &psExtraArg);
                    if (raster_io_error == CE_Failure) {
                        throw datasource_exception(CPLGetLastErrorMsg());
                    }
                }
                else
                {
                    MAPNIK_LOG_WARN(gdal) << "warning: nodata value (" << raster_nodata << ") used to set transparency instead of alpha band";
                }
            }
            else if( dataset_.GetRasterCount() > 0 && dataset_.GetRasterBand(1) )
            {
                // Check if we have a non-alpha mask band (for example a TIFF internal mask)
                int flags = dataset_.GetRasterBand(1)->GetMaskFlags();
                GDALRasterBand* mask = 0;
                if (flags == GMF_PER_DATASET)
                {
                    mask = dataset_.GetRasterBand(1)->GetMaskBand();
                }
                if (mask)
                {
                    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: found and processing mask band...";
                    if (!raster_has_nodata)
                    {
                        raster_io_error = mask->RasterIO(GF_Read, x_off, y_off, width, height, image.bytes() + 3,
                                                         image.width(), image.height(), GDT_Byte, 4, 4 * image.width(),
                                                         &psExtraArg);
                        if (raster_io_error == CE_Failure) {
                            throw datasource_exception(CPLGetLastErrorMsg());
                        }
                    }
                    else
                    {
                        MAPNIK_LOG_WARN(gdal) << "warning: nodata value (" << raster_nodata << ") used to set transparency instead of mask band";
                    }
                }
            }
            mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, image, filter_factor);
            // set nodata value to be used in raster colorizer
            if (nodata_value_)
            {
                raster->set_nodata(*nodata_value_);
            }
            else if (raster_has_nodata && !std::isnan(raster_nodata))
            {
                raster->set_nodata(raster_nodata);
            }
            feature->set_raster(raster);
        }
        // report actual/original source nodata in feature attributes
        if (raster_has_nodata && !std::isnan(raster_nodata))
        {
            feature->put("nodata", raster_nodata);
        }
        return feature;
    }
    return feature_ptr();
}


feature_ptr gdal_featureset::get_feature_at_point(mapnik::coord2d const& pt)
{
    CPLErr raster_io_error = CE_None;

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
            int raster_has_nodata;
            double nodata = band->GetNoDataValue(&raster_has_nodata);
            double value;
            raster_io_error = band->RasterIO(GF_Read, x, y, 1, 1, &value, 1, 1, GDT_Float64, 0, 0);
            if (raster_io_error == CE_Failure) {
                throw datasource_exception(CPLGetLastErrorMsg());
            }
            if (!raster_has_nodata || value != nodata)
            {
                // construct feature
                feature_ptr feature = feature_factory::create(ctx_,1);
                feature->set_geometry(mapnik::geometry::point<double>(pt.x,pt.y));
                feature->put_new("value",value);
                if (raster_has_nodata && !std::isnan(nodata))
                {
                    feature->put_new("nodata", nodata);
                }
                return feature;
            }
        }
    }
    return feature_ptr();
}
