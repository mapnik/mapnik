// SPDX-License-Identifier: BSD-3-Clause
/**
 * Copyright (c) MapBox
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * - Neither the name "MapBox" nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VECTOR_TILE_COMPRESSION_HPP_
#define VECTOR_TILE_COMPRESSION_HPP_

#include <cstdint>
#include <string>
// zlib
#include <zlib.h>

namespace mapnik 
{
 
namespace vector_tile_impl 
{

inline bool is_zlib_compressed(const char * data, std::size_t size)
{
    return size > 2 &&
           static_cast<uint8_t>(data[0]) == 0x78 &&
           (
               static_cast<uint8_t>(data[1]) == 0x9C ||
               static_cast<uint8_t>(data[1]) == 0x01 ||
               static_cast<uint8_t>(data[1]) == 0xDA ||
               static_cast<uint8_t>(data[1]) == 0x5E
           );
}

inline bool is_gzip_compressed(const char * data, std::size_t size)
{
    return size > 2 && static_cast<uint8_t>(data[0]) == 0x1F && static_cast<uint8_t>(data[1]) == 0x8B;
}

// decodes both zlib and gzip
// http://stackoverflow.com/a/1838702/2333354
void zlib_decompress(const char * data,
                                          std::size_t size, 
                                          std::string & output);

} // end ns vector_tile_impl

} // end ns mapnik

#endif /* VECTOR_TILE_COMPRESSION_HPP_ */
