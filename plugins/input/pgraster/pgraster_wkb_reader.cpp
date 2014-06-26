/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Sandro Santilli <strk@keybit.net>
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

#include "pgraster_wkb_reader.hpp"

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/global.hpp> // for int2net
#include <mapnik/box2d.hpp> // for box2d

// boost
#include <boost/cstdint.hpp> // for boost::int8_t
#include <boost/make_shared.hpp>

namespace {

uint16_t
read_uint16(const boost::uint8_t** from, boost::uint8_t littleEndian) {
    uint16_t ret = 0;

    if (littleEndian) {
        ret = (*from)[0] |
                (*from)[1] << 8;
    } else {
        /* big endian */
        ret = (*from)[0] << 8 |
                (*from)[1];
    }
    *from += 2;
    return ret;
}

double
read_float64(const boost::uint8_t** from, boost::uint8_t littleEndian) {

    union {
        double d;
        uint64_t i;
    } ret;

    if (littleEndian) {
        ret.i = (uint64_t) ((*from)[0] & 0xff) |
                (uint64_t) ((*from)[1] & 0xff) << 8 |
                (uint64_t) ((*from)[2] & 0xff) << 16 |
                (uint64_t) ((*from)[3] & 0xff) << 24 |
                (uint64_t) ((*from)[4] & 0xff) << 32 |
                (uint64_t) ((*from)[5] & 0xff) << 40 |
                (uint64_t) ((*from)[6] & 0xff) << 48 |
                (uint64_t) ((*from)[7] & 0xff) << 56;
    } else {
        /* big endian */
        ret.i = (uint64_t) ((*from)[7] & 0xff) |
                (uint64_t) ((*from)[6] & 0xff) << 8 |
                (uint64_t) ((*from)[5] & 0xff) << 16 |
                (uint64_t) ((*from)[4] & 0xff) << 24 |
                (uint64_t) ((*from)[3] & 0xff) << 32 |
                (uint64_t) ((*from)[2] & 0xff) << 40 |
                (uint64_t) ((*from)[1] & 0xff) << 48 |
                (uint64_t) ((*from)[0] & 0xff) << 56;
    }

    *from += 8;
    return ret.d;
}

uint32_t
read_uint32(const boost::uint8_t** from, boost::uint8_t littleEndian) {
    uint32_t ret = 0;

    if (littleEndian) {
        ret = (uint32_t) ((*from)[0] & 0xff) |
                (uint32_t) ((*from)[1] & 0xff) << 8 |
                (uint32_t) ((*from)[2] & 0xff) << 16 |
                (uint32_t) ((*from)[3] & 0xff) << 24;
    } else {
        /* big endian */
        ret = (uint32_t) ((*from)[3] & 0xff) |
                (uint32_t) ((*from)[2] & 0xff) << 8 |
                (uint32_t) ((*from)[1] & 0xff) << 16 |
                (uint32_t) ((*from)[0] & 0xff) << 24;
    }

    *from += 4;
    return ret;
}

int32_t
read_int32(const boost::uint8_t** from, boost::uint8_t littleEndian) {

    return read_uint32(from, littleEndian);
}


}

using mapnik::box2d;

mapnik::raster_ptr
pgraster_wkb_reader::get_raster() {

    /* Read endianness */
    boost::uint8_t endian = *ptr_;
    endian = *ptr_;
    ptr_ += 1;

    /* Read version of protocol */
    uint16_t version = read_uint16(&ptr_, endian);
    if (version != 0) {
       MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: WKB version "
          << version << " unsupported";
      return mapnik::raster_ptr();
    }

    uint16_t numBands = read_uint16(&ptr_, endian);
    double scaleX = read_float64(&ptr_, endian);
    double scaleY = read_float64(&ptr_, endian);
    double ipX = read_float64(&ptr_, endian);
    double ipY = read_float64(&ptr_, endian);
    double skewX = read_float64(&ptr_, endian);
    double skewY = read_float64(&ptr_, endian);
    int32_t srid = read_int32(&ptr_, endian);
    uint16_t width = read_uint16(&ptr_, endian);
    uint16_t height = read_uint16(&ptr_, endian);

    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: numBands=" << numBands;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: scaleX=" << scaleX;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: scaleY=" << scaleY;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: ipX=" << ipX;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: ipY=" << ipY;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: skewX=" << skewX;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: skewY=" << skewY;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: srid=" << srid;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: size="
      << width << "x" << height;


    if ( skewX || skewY ) {
       MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: raster rotation is not supported";
    }

    box2d<double> ext(ipX,ipY,ipX+(width*scaleX),ipY+(height*scaleY)); 
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_featureset: Raster extent=" << ext;

    mapnik::raster_ptr raster = boost::make_shared<mapnik::raster>(ext, width, height);
    //raster->set_nodata(0x11111111);
    mapnik::image_data_32 & image = raster->data_;
    image.set(0xff0000ff); // this is red

    return raster;

}


