/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
 *****************************************************************************
 *
 * Initially developed by Sandro Santilli <strk@keybit.net> for CartoDB
 *
 *****************************************************************************/

#include "pgraster_wkb_reader.hpp"

// mapnik
#include <mapnik/datasource.hpp> // for datasource_exception
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/image.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/box2d.hpp> // for box2d
#include <functional>

#include <cstdint>

namespace {

uint8_t
read_uint8(const uint8_t** from) {
    return *(*from)++;
}

uint16_t
read_uint16(const uint8_t** from, uint8_t littleEndian) {
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

/*
int16_t
read_int16(const uint8_t** from, uint8_t littleEndian) {
    assert(NULL != from);

    return read_uint16(from, littleEndian);
}
*/

double
read_float64(const uint8_t** from, uint8_t littleEndian) {

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
read_uint32(const uint8_t** from, uint8_t littleEndian) {
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
read_int32(const uint8_t** from, uint8_t littleEndian) {

    return read_uint32(from, littleEndian);
}

float
read_float32(const uint8_t** from, uint8_t littleEndian) {

    union {
        float f;
        uint32_t i;
    } ret;

    ret.i = read_uint32(from, littleEndian);

    return ret.f;
}


typedef enum {
    PT_1BB=0,     /* 1-bit boolean            */
    PT_2BUI=1,    /* 2-bit unsigned integer   */
    PT_4BUI=2,    /* 4-bit unsigned integer   */
    PT_8BSI=3,    /* 8-bit signed integer     */
    PT_8BUI=4,    /* 8-bit unsigned integer   */
    PT_16BSI=5,   /* 16-bit signed integer    */
    PT_16BUI=6,   /* 16-bit unsigned integer  */
    PT_32BSI=7,   /* 32-bit signed integer    */
    PT_32BUI=8,   /* 32-bit unsigned integer  */
    PT_32BF=10,   /* 32-bit float             */
    PT_64BF=11,   /* 64-bit float             */
    PT_END=13
} rt_pixtype;

#define BANDTYPE_FLAGS_MASK 0xF0
#define BANDTYPE_PIXTYPE_MASK 0x0F
#define BANDTYPE_FLAG_OFFDB     (1<<7)
#define BANDTYPE_FLAG_HASNODATA (1<<6)
#define BANDTYPE_FLAG_ISNODATA  (1<<5)
#define BANDTYPE_FLAG_RESERVED3 (1<<4)

#define BANDTYPE_PIXTYPE_MASK 0x0F
#define BANDTYPE_PIXTYPE(x) ((x)&BANDTYPE_PIXTYPE_MASK)
#define BANDTYPE_IS_OFFDB(x) ((x)&BANDTYPE_FLAG_OFFDB)
#define BANDTYPE_HAS_NODATA(x) ((x)&BANDTYPE_FLAG_HASNODATA)
#define BANDTYPE_IS_NODATA(x) ((x)&BANDTYPE_FLAG_ISNODATA)

}

template<typename T>
mapnik::raster_ptr read_data_band(mapnik::box2d<double> const& bbox,
                    uint16_t width, uint16_t height,
                    bool hasnodata, T reader)
{
  mapnik::image_gray32f image(width, height);
  float* data = image.data();
  double val;
  val = reader(); // nodata value, need to read anyway
  for (int y=0; y<height; ++y) {
    for (int x=0; x<width; ++x) {
      val = reader();
      int off = y * width + x;
      data[off] = val;
    }
  }
  mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(bbox, image, 1.0);
  if ( hasnodata ) raster->set_nodata(val);
  return raster;
}

mapnik::raster_ptr pgraster_wkb_reader::read_indexed(mapnik::box2d<double> const& bbox,
                                                     uint16_t width, uint16_t height)
{
  uint8_t type = read_uint8(&ptr_);

  int pixtype = BANDTYPE_PIXTYPE(type);
  int offline = BANDTYPE_IS_OFFDB(type) ? 1 : 0;
  int hasnodata = BANDTYPE_HAS_NODATA(type) ? 1 : 0;

  MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: band type:"
        << pixtype << " offline:" << offline
        << " hasnodata:" << hasnodata;

  if ( offline ) {
    MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: offline band "
          " unsupported";
    return mapnik::raster_ptr();
  }

  MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: reading " << height_ << "x" << width_ << " pixels";

  switch (pixtype) {
    case PT_1BB:
    case PT_2BUI:
    case PT_4BUI:
      // all <8BPP values are wrote in full bytes anyway
    case PT_8BSI:
      // mapnik does not support signed anyway
    case PT_8BUI:
      return read_data_band(bbox, width_, height_, hasnodata,
                     std::bind(read_uint8, &ptr_));
      break;
    case PT_16BSI:
      // mapnik does not support signed anyway
    case PT_16BUI:
      return read_data_band(bbox, width_, height_, hasnodata,
                     std::bind(read_uint16, &ptr_, endian_));
      break;
    case PT_32BSI:
      // mapnik does not support signed anyway
    case PT_32BUI:
      return read_data_band(bbox, width_, height_, hasnodata,
                     std::bind(read_uint32, &ptr_, endian_));
      break;
    case PT_32BF:
      return read_data_band(bbox, width_, height_, hasnodata,
                     std::bind(read_float32, &ptr_, endian_));
      break;
    case PT_64BF:
      return read_data_band(bbox, width_, height_, hasnodata,
                     std::bind(read_float64, &ptr_, endian_));
      break;
    default:
      std::ostringstream err;
      err << "pgraster_wkb_reader: data band type " << pixtype << " unsupported";
      // TODO: accept policy to decide on throw-or-skip ?
      //MAPNIK_LOG_WARN(pgraster) << err.str();
      throw mapnik::datasource_exception(err.str());
  }
  return mapnik::raster_ptr();
}

template<typename T>
mapnik::raster_ptr read_grayscale_band(mapnik::box2d<double> const& bbox,
                         uint16_t width, uint16_t height,
                         bool hasnodata, T reader)
{
  mapnik::image_rgba8 image(width,height, true, true);
  // Start with plain white (ABGR or RGBA depending on endiannes)
  // TODO: set to transparent instead?
  image.set(0xffffffff);


  int val;
  int nodataval;
  uint8_t * data = image.bytes();
  int ps = 4; // sizeof(image::pixel_type)
  int off;
  nodataval = reader(); // nodata value, need to read anyway
  for (int y=0; y<height; ++y) {
    for (int x=0; x<width; ++x) {
      val = reader();
      // Apply harsh type clipping rules ala GDAL
      if ( val < 0 ) val = 0;
      if ( val > 255 ) val = 255;
      // Calculate pixel offset
      off = y * width * ps + x * ps;
      // Pixel space is RGBA, fill all w/ same value for Grey
      data[off+0] = val;
      data[off+1] = val;
      data[off+2] = val;
      // Set the alpha channel for transparent nodata values
      // Nodata handling is *manual* at the driver level
      if ( hasnodata && val == nodataval ) {
          data[off+3] = 0x00; // transparent
      } else {
          data[off+3] = 0xFF; // opaque
      }
    }
  }
  mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(bbox, image, 1.0);
  if ( hasnodata ) raster->set_nodata(val);
  return raster;
}

mapnik::raster_ptr pgraster_wkb_reader::read_grayscale(mapnik::box2d<double> const& bbox,
                                                       uint16_t width, uint16_t height)
{
  uint8_t type = read_uint8(&ptr_);

  int pixtype = BANDTYPE_PIXTYPE(type);
  int offline = BANDTYPE_IS_OFFDB(type) ? 1 : 0;
  int hasnodata = BANDTYPE_HAS_NODATA(type) ? 1 : 0;

  MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: band type:"
        << pixtype << " offline:" << offline
        << " hasnodata:" << hasnodata;

  if ( offline ) {
    MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: offline band "
          " unsupported";
    return mapnik::raster_ptr();
  }

  switch (pixtype) {
    case PT_1BB:
    case PT_2BUI:
    case PT_4BUI:
      // all <8BPP values are wrote in full bytes anyway
    case PT_8BSI:
      // mapnik does not support signed anyway
    case PT_8BUI:
      return read_grayscale_band(bbox, width_, height_, hasnodata,
                          std::bind(read_uint8, &ptr_));
      break;
    case PT_16BSI:
      // mapnik does not support signed anyway
    case PT_16BUI:
      return read_grayscale_band(bbox, width_, height_, hasnodata,
                          std::bind(read_uint16, &ptr_, endian_));
      break;
    case PT_32BSI:
      // mapnik does not support signed anyway
    case PT_32BUI:
      return read_grayscale_band(bbox, width_, height_, hasnodata,
                          std::bind(read_uint32, &ptr_, endian_));
      break;
    default:
      std::ostringstream err;
      err << "pgraster_wkb_reader: grayscale band type "
          << pixtype << " unsupported";
      //MAPNIK_LOG_WARN(pgraster) << err.str();
      throw mapnik::datasource_exception(err.str());
  }
  return mapnik::raster_ptr();
}

mapnik::raster_ptr pgraster_wkb_reader::read_rgba(mapnik::box2d<double> const& bbox,
                                                  uint16_t width, uint16_t height)
{
  mapnik::image_rgba8 im(width, height, true, true);
  // Start with plain white (ABGR or RGBA depending on endiannes)
  im.set(0xffffffff);

  uint8_t nodataval;
  for (int bn=0; bn<numBands_; ++bn) {
    uint8_t type = read_uint8(&ptr_);

    int pixtype = BANDTYPE_PIXTYPE(type);
    int offline = BANDTYPE_IS_OFFDB(type) ? 1 : 0;
    int hasnodata = BANDTYPE_HAS_NODATA(type) ? 1 : 0;

    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: band " << bn
          << " type:" << pixtype << " offline:" << offline
          << " hasnodata:" << hasnodata;

    if ( offline ) {
      MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: offline band "
            << bn << " unsupported";
      continue;
    }

    if ( pixtype > PT_8BUI || pixtype < PT_8BSI ) {
      MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: band " << bn
            << " type " << type << " unsupported";
      continue;
    }

    uint8_t tmp = read_uint8(&ptr_);
    if ( ! bn ) nodataval = tmp;
    else if ( tmp != nodataval ) {
      MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: band " << bn
            << " nodataval " << tmp << " != band 0 nodataval " << nodataval;
    }

    int ps = 4; // sizeof(image::pixel_type)
    uint8_t * image_data = im.bytes();
    for (int y=0; y<height_; ++y) {
      for (int x=0; x<width_; ++x) {
        uint8_t val = read_uint8(&ptr_);
        // y * width_ * ps is the row (ps is pixel size)
        // x * ps is the column
        int off = y * width_ * ps + x * ps;
        // Pixel space is RGBA
        image_data[off+bn] = val;
      }
    }
  }
  mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(bbox, im, 1.0);
  raster->set_nodata(0xffffffff);
  return raster;
}

mapnik::raster_ptr
pgraster_wkb_reader::get_raster() {

    /* Read endianness */
    endian_ = *ptr_;
    ptr_ += 1;

    /* Read version of protocol */
    uint16_t version = read_uint16(&ptr_, endian_);
    if (version != 0) {
       MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: WKB version "
          << version << " unsupported";
      return mapnik::raster_ptr();
    }

    numBands_ = read_uint16(&ptr_, endian_);
    double scaleX = read_float64(&ptr_, endian_);
    double scaleY = read_float64(&ptr_, endian_);
    double ipX = read_float64(&ptr_, endian_);
    double ipY = read_float64(&ptr_, endian_);
    double skewX = read_float64(&ptr_, endian_);
    double skewY = read_float64(&ptr_, endian_);
    int32_t srid = read_int32(&ptr_, endian_);
    width_ = read_uint16(&ptr_, endian_);
    height_ = read_uint16(&ptr_, endian_);

    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: numBands=" << numBands_;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: scaleX=" << scaleX;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: scaleY=" << scaleY;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: ipX=" << ipX;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: ipY=" << ipY;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: skewX=" << skewX;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: skewY=" << skewY;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: srid=" << srid;
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: size="
      << width_ << "x" << height_;

    // this is for color interpretation
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: bandno=" << bandno_;

    if ( skewX || skewY ) {
      MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: raster rotation is not supported";
      return mapnik::raster_ptr();
    }

    mapnik::box2d<double> ext(ipX,ipY,ipX+(width_*scaleX),ipY+(height_*scaleY));
    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: Raster extent=" << ext;

    if ( bandno_ )
    {
      if ( bandno_ != 1 )
      {
          MAPNIK_LOG_WARN(pgraster) << "pgraster_wkb_reader: "
              "reading bands other than 1st as indexed is unsupported";
          return mapnik::raster_ptr();
      }
      MAPNIK_LOG_DEBUG(pgraster) << "pgraster_wkb_reader: requested band " << bandno_;
      return read_indexed(ext, width_, height_);
    }
    else
    {
      switch (numBands_)
      {
        case 1:
          return read_grayscale(ext, width_, height_);
          break;
        case 3:
        case 4:
          return read_rgba(ext, width_, height_);
          break;
        default:
          std::ostringstream err;
          err << "pgraster_wkb_reader: raster with "
              << numBands_
              << " bands is not supported, specify a band number";
          //MAPNIK_LOG_WARN(pgraster) << err.str();
          throw mapnik::datasource_exception(err.str());
          return mapnik::raster_ptr();
      }
    }
    return mapnik::raster_ptr();
}
