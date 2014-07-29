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

enum pgraster_color_interp {
  // Automatic color interpretation:
  // uses grayscale for single band, rgb for 3 bands
  // rgba for 4 bands
  pgr_auto,
  // Grayscale interpretation
  pgr_grayscale,
  pgr_indexed,
  pgr_rgb,
  pgr_rgba
};

class pgraster_wkb_reader
{
public:

  pgraster_wkb_reader(const uint8_t* wkb, int size, int bnd=0)
    : wkbsize_(size), wkb_(wkb), wkbend_(wkb+size), ptr_(wkb), bandno_(bnd) 
  {}

  mapnik::raster_ptr get_raster();

  /// @param bnd band number. If 0 (default) it'll try to read all bands
  ///            with automatic color interpretation (rgb for 3 bands,
  ///            rgba for 4 bands, grayscale for 1 band).
  ///            Any other value results in pixel
  ///            values being copied verbatim into the returned raster
  ///            for interpretation by the caller.
  static mapnik::raster_ptr read(const uint8_t* wkb, int size, int bnd=0)
  {
    pgraster_wkb_reader reader(wkb,size,bnd);
    return reader.get_raster();
  }

private:
  void read_indexed(mapnik::raster_ptr raster);
  void read_grayscale(mapnik::raster_ptr raster);
  void read_rgba(mapnik::raster_ptr raster);

  int wkbsize_;
  const uint8_t* wkb_;
  const uint8_t* wkbend_;
  const uint8_t* ptr_;
  uint8_t endian_;
  int bandno_;
  uint16_t numBands_;
  uint16_t width_;
  uint16_t height_;
};


#endif // PGRASTER_WKB_READER_HPP
