/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_MINIZ_PNG_HPP
#define MAPNIK_MINIZ_PNG_HPP

// mapnik
#include <mapnik/palette.hpp>

// stl
#include <vector>
#include <iostream>
#include <stdexcept>

/* miniz.c porting issues:
  - duplicate symbols in python bindings require moving miniz.c include to just cpp file
  - due to http://code.google.com/p/miniz/issues/detail?id=7
  - avoiding including miniz.c here requires fwd declaring the two structs below
  - being able to fwd declare requires removing typedef from struct declarations in miniz.c
  - being able to fwd declare also requires using pointers to structs
*/

// TODO: try using #define MINIZ_HEADER_FILE_ONLY
struct tdefl_output_buffer;
struct tdefl_compressor;

namespace mapnik { namespace MiniZ {

using mapnik::rgb;

class PNGWriter {

public:
    PNGWriter(int level, int strategy);
    ~PNGWriter();
private:
    inline void writeUInt32BE(unsigned char *target, unsigned int value);
    size_t startChunk(const unsigned char header[], size_t length);
    void finishChunk(size_t start);
public:
    void writeIHDR(unsigned int width, unsigned int height, unsigned char pixel_depth);
    void writePLTE(std::vector<rgb> const& palette);
    void writetRNS(std::vector<unsigned> const& alpha);
    template<typename T>
    void writeIDAT(T const& image);
    template<typename T>
    void writeIDATStripAlpha(T const& image);
    void writeIEND();
    void toStream(std::ostream& stream);

private:
    tdefl_compressor *compressor;
    tdefl_output_buffer *buffer;
    static const unsigned char preamble[];
    static const unsigned char IHDR_tpl[];
    static const unsigned char PLTE_tpl[];
    static const unsigned char tRNS_tpl[];
    static const unsigned char IDAT_tpl[];
    static const unsigned char IEND_tpl[];
};

}}

#endif // MAPNIK_MINIZ_PNG_HPP
