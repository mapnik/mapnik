/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#include "gdal_datasource.hpp"
#include "gdal_featureset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/value_types.hpp>

#include <gdal_version.h>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(gdal_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::datasource_exception;

namespace {
// an error handler for GDAL which translates the error into
// the equivalent mapnik one. this is useful for when you want
// to silence all or some levels of error at the mapnik level,
// as you do not know whether or not the GDAL libraries are
// loaded to do it manually.
void mapnik_gdal_error_handler(CPLErr err_class, int err_no, const char *msg) {
  switch (err_class) {
  case CE_None:
  case CE_Debug:
    MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource: ERROR " << err_no << ": " << msg;
    break;

  case CE_Warning:
    MAPNIK_LOG_WARN(gdal) << "gdal_datasource: ERROR " << err_no << ": " << msg;
    break;

  default:
    MAPNIK_LOG_ERROR(gdal) << "gdal_datasource: ERROR " << err_no << ": " << msg;
  }
}
} // anonymous namespace

gdal_datasource::gdal_datasource(parameters const& params)
    : datasource(params),
      dataset_(nullptr),
      desc_(gdal_datasource::name(), "utf-8"),
      nodata_value_(params.get<double>("nodata")),
      nodata_tolerance_(*params.get<double>("nodata_tolerance",1e-12))
{
    MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource: Initializing...";

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "gdal_datasource::init");
#endif

    GDALAllRegister();

    // wrap GDAL error reporting in mapnik's own
    CPLSetErrorHandler(&mapnik_gdal_error_handler);

    boost::optional<std::string> file = params.get<std::string>("file");
    if (! file) throw datasource_exception("missing <file> parameter");

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
    {
        dataset_name_ = *base + "/" + *file;
    }
    else
    {
        dataset_name_ = *file;
    }

    shared_dataset_ = *params.get<mapnik::boolean_type>("shared", false);
    band_ = *params.get<mapnik::value_integer>("band", -1);

#if GDAL_VERSION_NUM >= 1600
    if (shared_dataset_)
    {
        dataset_ = reinterpret_cast<GDALDataset*>(GDALOpenShared((dataset_name_).c_str(), GA_ReadOnly));
    }
    else
#endif
    {
        dataset_ = reinterpret_cast<GDALDataset*>(GDALOpen((dataset_name_).c_str(), GA_ReadOnly));
    }

    if (! dataset_)
    {
        throw datasource_exception(CPLGetLastErrorMsg());
    }

    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: opened Dataset=" << dataset_;

    nbands_ = dataset_->GetRasterCount();
    width_ = dataset_->GetRasterXSize();
    height_ = dataset_->GetRasterYSize();
    desc_.add_descriptor(mapnik::attribute_descriptor("nodata", mapnik::Double));

    double tr[6];
    bool bbox_override = false;
    boost::optional<std::string> bbox_s = params.get<std::string>("extent");
    if (bbox_s)
    {
        MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource: BBox Parameter=" << *bbox_s;

        bbox_override = extent_.from_string(*bbox_s);
        if (! bbox_override)
        {
            throw datasource_exception("GDAL Plugin: bbox parameter '" + *bbox_s + "' invalid");
        }
    }

    if (bbox_override)
    {
        tr[0] = extent_.minx();
        tr[1] = extent_.width() / (double)width_;
        tr[2] = 0;
        tr[3] = extent_.maxy();
        tr[4] = 0;
        tr[5] = -extent_.height() / (double)height_;
        MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource extent override gives Geotransform="
                               << tr[0] << "," << tr[1] << ","
                               << tr[2] << "," << tr[3] << ","
                               << tr[4] << "," << tr[5];
    }
    else
    {
        if (dataset_->GetGeoTransform(tr) != CPLE_None)
        {
            MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource GetGeotransform failure gives="
                                   << tr[0] << "," << tr[1] << ","
                                   << tr[2] << "," << tr[3] << ","
                                   << tr[4] << "," << tr[5];
        }
        else
        {
            MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource Geotransform="
                                   << tr[0] << "," << tr[1] << ","
                                   << tr[2] << "," << tr[3] << ","
                                   << tr[4] << "," << tr[5];
        }
    }

    // TODO - We should throw for true non-north up images, but the check
    // below is clearly too restrictive.
    // https://github.com/mapnik/mapnik/issues/970
    /*
      if (tr[2] != 0 || tr[4] != 0)
      {
      throw datasource_exception("GDAL Plugin: only 'north up' images are supported");
      }
    */

    dx_ = tr[1];
    dy_ = tr[5];

    if (! bbox_override)
    {
        double x0 = tr[0];
        double y0 = tr[3];
        double x1 = tr[0] + width_ * dx_ + height_ *tr[2];
        double y1 = tr[3] + width_ *tr[4] + height_ * dy_;

        /*
          double x0 = tr[0] + (height_) * tr[2]; // minx
          double y0 = tr[3] + (height_) * tr[5]; // miny

          double x1 = tr[0] + (width_) * tr[1]; // maxx
          double y1 = tr[3] + (width_) * tr[4]; // maxy
        */

        extent_.init(x0, y0, x1, y1);
    }

    MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource: Raster Size=" << width_ << "," << height_;
    MAPNIK_LOG_DEBUG(gdal) << "gdal_datasource: Raster Extent=" << extent_;

}

gdal_datasource::~gdal_datasource()
{
    MAPNIK_LOG_DEBUG(gdal) << "gdal_featureset: Closing Dataset=" << dataset_;
    GDALClose(dataset_);
}

datasource::datasource_t gdal_datasource::type() const
{
    return datasource::Raster;
}

const char * gdal_datasource::name()
{
    return "gdal";
}

box2d<double> gdal_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> gdal_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource_geometry_t>();
}

layer_descriptor gdal_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr gdal_datasource::features(query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "gdal_datasource::features");
#endif

    gdal_query gq = q;

    // TODO - move to std::make_shared, but must reduce # of args to <= 9
    return featureset_ptr(new gdal_featureset(*dataset_,
                                              band_,
                                              gq,
                                              extent_,
                                              width_,
                                              height_,
                                              nbands_,
                                              dx_,
                                              dy_,
                                              nodata_value_,
                                              nodata_tolerance_));
}

featureset_ptr gdal_datasource::features_at_point(coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "gdal_datasource::features_at_point");
#endif

    gdal_query gq = pt;

    // TODO - move to std::make_shared, but must reduce # of args to <= 9
    return featureset_ptr(new gdal_featureset(*dataset_,
                                              band_,
                                              gq,
                                              extent_,
                                              width_,
                                              height_,
                                              nbands_,
                                              dx_,
                                              dy_,
                                              nodata_value_,
                                              nodata_tolerance_));
}
