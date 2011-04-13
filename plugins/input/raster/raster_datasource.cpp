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
//$Id: raster_datasource.cc 44 2005-04-22 18:53:54Z pavlenko $

// boost
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

// mapnik
#include <mapnik/image_reader.hpp>

#include "raster_featureset.hpp"
#include "raster_info.hpp"
#include "raster_datasource.hpp"

using mapnik::datasource;
using mapnik::parameters;
using mapnik::image_reader;

DATASOURCE_PLUGIN(raster_datasource)

using boost::lexical_cast;
using boost::bad_lexical_cast;
using mapnik::layer_descriptor;
using mapnik::featureset_ptr;
using mapnik::query;
using mapnik::coord2d;
using mapnik::datasource_exception;

raster_datasource::raster_datasource(const parameters& params, bool bind)
    : datasource(params),
      desc_(*params.get<std::string>("type"),"utf-8"),
      extent_initialized_(false)
{
#ifdef MAPNIK_DEBUG
    std::clog << "Raster Plugin: Initializing..." << std::endl;
#endif

    boost::optional<std::string> file = params.get<std::string>("file");
    if (! file) throw datasource_exception("Raster Plugin: missing <file> parameter ");
   
    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
       filename_ = *base + "/" + *file;
    else
       filename_ = *file;

    format_=*params_.get<std::string>("format","tiff");
   
    boost::optional<double> lox = params_.get<double>("lox");
    boost::optional<double> loy = params_.get<double>("loy");
    boost::optional<double> hix = params_.get<double>("hix");
    boost::optional<double> hiy = params_.get<double>("hiy");
    boost::optional<std::string> ext = params_.get<std::string>("extent");

    if (lox && loy && hix && hiy)
    {
        extent_.init(*lox, *loy, *hix, *hiy);
        extent_initialized_ = true;
    }
    else if (ext)
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    if (! extent_initialized_)
        throw datasource_exception("Raster Plugin: valid <extent> or <lox> <loy> <hix> <hiy> are required");

    if (bind) 
    {
        this->bind();
    }
}

void raster_datasource::bind() const
{
    if (is_bound_) return;
   
    if (! boost::filesystem::exists(filename_))
        throw datasource_exception("Raster Plugin: " + filename_ + " does not exist");
    
    try
    {         
        std::auto_ptr<image_reader> reader(mapnik::get_image_reader(filename_, format_));
        if (reader.get())
        {
            width_ = reader->width();
            height_ = reader->height();

#ifdef MAPNIK_DEBUG
            std::clog << "Raster Plugin: RASTER SIZE(" << width_ << "," << height_ << ")" << std::endl;
#endif
        }
    }
    catch (mapnik::image_reader_exception const& ex)
    {
        std::cerr << "Raster Plugin: image reader exception caught: " << ex.what() << std::endl;
        throw;
    }
    catch (...)
    {
        std::cerr << "Raster Plugin: exception caught" << std::endl;
        throw;
    }
    
    is_bound_ = true;
}

raster_datasource::~raster_datasource()
{
}

int raster_datasource::type() const
{
    return datasource::Raster;
}

std::string raster_datasource::name()
{
    return "raster";
}

mapnik::box2d<double> raster_datasource::envelope() const
{
    return extent_;
}

layer_descriptor raster_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr raster_datasource::features(query const& q) const
{
    if (! is_bound_) bind();
   
    mapnik::CoordTransform t(width_, height_, extent_, 0, 0);
    mapnik::box2d<double> intersect = extent_.intersect(q.get_bbox());
    mapnik::box2d<double> ext = t.forward(intersect);
   
    const int width  = int(ext.maxx() + 0.5) - int(ext.minx() + 0.5);
    const int height = int(ext.maxy() + 0.5) - int(ext.miny() + 0.5);

#ifdef MAPNIK_DEBUG
    std::clog << "Raster Plugin: BOX SIZE(" << width << " " << height << ")" << std::endl;
#endif

    if (width * height > 512*512)
    {
#ifdef MAPNIK_DEBUG
        std::clog << "Raster Plugin: TILED policy" << std::endl;
#endif

        tiled_file_policy policy(filename_, format_, 256, extent_, q.get_bbox(), width_, height_);
        return featureset_ptr(new raster_featureset<tiled_file_policy>(policy, extent_, q));
    }
    else
    {
#ifdef MAPNIK_DEBUG
        std::clog << "Raster Plugin: SINGLE FILE" << std::endl;
#endif

        raster_info info(filename_, format_, extent_, width_, height_);
        single_file_policy policy(info);
        return featureset_ptr(new raster_featureset<single_file_policy>(policy, extent_, q));
    }
}

featureset_ptr raster_datasource::features_at_point(coord2d const&) const
{
    return featureset_ptr();
}

