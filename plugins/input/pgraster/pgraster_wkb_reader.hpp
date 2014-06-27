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

#ifndef PGRASTER_WKB_READER_HPP
#define PGRASTER_WKB_READER_HPP

// mapnik
#include <mapnik/feature.hpp> // for raster_ptr

// boost
#include <boost/cstdint.hpp> // for boost::uint8_t

class pgraster_wkb_reader
{
public:

  pgraster_wkb_reader(const uint8_t* wkb, int size)
    : wkbsize_(size), wkb_(wkb), wkbend_(wkb+size), ptr_(wkb) 
  {}

  mapnik::raster_ptr get_raster();

  static mapnik::raster_ptr read(const uint8_t* wkb, int size)
  {
    pgraster_wkb_reader reader(wkb,size);
    return reader.get_raster();
  }


private:
  void read_grayscale(mapnik::raster_ptr raster);
  void read_rgb(mapnik::raster_ptr raster);

  int wkbsize_;
  const uint8_t* wkb_;
  const uint8_t* wkbend_;
  const uint8_t* ptr_;
  uint16_t numBands_;
  uint16_t width_;
  uint16_t height_;
};


#endif // PGRASTER_WKB_READER_HPP
