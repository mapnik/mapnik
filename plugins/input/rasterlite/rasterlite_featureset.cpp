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

#include "rasterlite_featureset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/feature_factory.hpp>

// boost
#include <boost/make_shared.hpp>


using mapnik::query;
using mapnik::coord2d;
using mapnik::box2d;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::geometry_type;
using mapnik::query;
using mapnik::feature_factory;


rasterlite_featureset::rasterlite_featureset(void* dataset,
                                             rasterlite_query q)
    : dataset_(dataset),
      gquery_(q),
      first_(true),
      ctx_(boost::make_shared<mapnik::context_type>())
{
    rasterliteSetBackgroundColor(dataset_, 255, 0, 255);
    rasterliteSetTransparentColor(dataset_, 255, 0, 255);
}

rasterlite_featureset::~rasterlite_featureset()
{
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Closing";

    rasterliteClose(dataset_);
}

feature_ptr rasterlite_featureset::next()
{
    if (first_)
    {
        first_ = false;

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

feature_ptr rasterlite_featureset::get_feature(mapnik::query const& q)
{
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Running get_feature";

    feature_ptr feature(feature_factory::create(ctx_,1));

    double x0, y0, x1, y1;
    rasterliteGetExtent (dataset_, &x0, &y0, &x1, &y1);

    box2d<double> raster_extent(x0, y0, x1, y1);
    box2d<double> intersect = raster_extent.intersect(q.get_bbox());

    const int width = static_cast<int>(boost::get<0>(q.resolution()) * intersect.width() + 0.5);
    const int height = static_cast<int>(boost::get<0>(q.resolution()) * intersect.height() + 0.5);

    const double pixel_size = (intersect.width() >= intersect.height()) ?
        (intersect.width() / (double) width) : (intersect.height() / (double) height);

    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Raster extent=" << raster_extent;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: View extent=" << q.get_bbox();
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Intersect extent=" << intersect;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Query resolution=" << boost::get<0>(q.resolution())  << "," << boost::get<1>(q.resolution());
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Size=" << width << " " << height;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Pixel Size=" << pixel_size;

    if (width > 0 && height > 0)
    {
        int size = 0;
        void* raster = 0;

        if (rasterliteGetRawImageByRect(dataset_,
                                        intersect.minx(),
                                        intersect.miny(),
                                        intersect.maxx(),
                                        intersect.maxy(),
                                        pixel_size,
                                        width,
                                        height,
                                        GAIA_RGBA_ARRAY,
                                        &raster,
                                        &size) == RASTERLITE_OK)
        {
            if (size > 0)
            {
                mapnik::image_data_32 image(width, height);
                image.set(0xffffffff);

                unsigned char* raster_data = static_cast<unsigned char*>(raster);
                unsigned char* image_data = image.getBytes();

                memcpy (image_data, raster_data, size);

                feature->set_raster(boost::make_shared<mapnik::raster>(intersect,image));

                free (raster);

                MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_featureset: Done";
            }
            else
            {
                MAPNIK_LOG_ERROR(rasterlite) << "Rasterlite Plugin: Error " << rasterliteGetLastError (dataset_);
            }
        }

        return feature;
    }
    return feature_ptr();
}

feature_ptr rasterlite_featureset::get_feature_at_point(mapnik::coord2d const& pt)
{
    return feature_ptr();
}
