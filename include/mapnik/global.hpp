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
 *****************************************************************************/

#ifndef MAPNIK_GLOBAL_HPP
#define MAPNIK_GLOBAL_HPP

// stl
#include <cstdint>
#include <cstring>
#include <cmath>

namespace mapnik
{

#define int2net(A)  (int16_t) (((std::uint16_t) ((std::uint8_t) (A)[1]))      | \
                               (((std::uint16_t) ((std::uint8_t) (A)[0])) << 8))

#define int4net(A)  (int32_t)  (((std::uint32_t) ((std::uint8_t) (A)[3]))        | \
                                (((std::uint32_t) ((std::uint8_t) (A)[2])) << 8)  | \
                                (((std::uint32_t) ((std::uint8_t) (A)[1])) << 16) | \
                                (((std::uint32_t) ((std::uint8_t) (A)[0])) << 24))

#define int8net(A)  (int64_t)  (((std::uint64_t) ((std::uint8_t) (A)[7]))        | \
                                (((std::uint64_t) ((std::uint8_t) (A)[6])) << 8)  | \
                                (((std::uint64_t) ((std::uint8_t) (A)[5])) << 16) | \
                                (((std::uint64_t) ((std::uint8_t) (A)[4])) << 24) | \
                                (((std::uint64_t) ((std::uint8_t) (A)[3])) << 32) | \
                                (((std::uint64_t) ((std::uint8_t) (A)[2])) << 40) | \
                                (((std::uint64_t) ((std::uint8_t) (A)[1])) << 48) | \
                                (((std::uint64_t) ((std::uint8_t) (A)[0])) << 56))

#define float8net(V,M)   do { double def_temp;  \
        ((std::uint8_t*) &def_temp)[0]=(M)[7];          \
        ((std::uint8_t*) &def_temp)[1]=(M)[6];          \
        ((std::uint8_t*) &def_temp)[2]=(M)[5];          \
        ((std::uint8_t*) &def_temp)[3]=(M)[4];          \
        ((std::uint8_t*) &def_temp)[4]=(M)[3];          \
        ((std::uint8_t*) &def_temp)[5]=(M)[2];          \
        ((std::uint8_t*) &def_temp)[6]=(M)[1];          \
        ((std::uint8_t*) &def_temp)[7]=(M)[0];          \
        (V) = def_temp; } while(0)
#define float4net(V,M)   do { float def_temp;   \
        ((std::uint8_t*) &def_temp)[0]=(M)[3];          \
        ((std::uint8_t*) &def_temp)[1]=(M)[2];          \
        ((std::uint8_t*) &def_temp)[2]=(M)[1];          \
        ((std::uint8_t*) &def_temp)[3]=(M)[0];          \
        (V)=def_temp; } while(0)


// read int16_t NDR (little endian)
inline void read_int16_ndr(const char* data, std::int16_t & val)
{
    std::memcpy(&val,data,2);
}

// read int32_t NDR (little endian)
inline void read_int32_ndr(const char* data, std::int32_t & val)
{
    std::memcpy(&val,data,4);
}

// read double NDR (little endian)
inline void read_double_ndr(const char* data, double & val)
{
    std::memcpy(&val,&data[0],8);
}

// read int16_t XDR (big endian)
inline void read_int16_xdr(const char* data, std::int16_t & val)
{
    val = static_cast<std::int16_t>((data[3]&0xff) | ((data[2]&0xff)<<8));
}

// read int32_t XDR (big endian)
inline void read_int32_xdr(const char* data, std::int32_t & val)
{
    val = (data[3]&0xff) | ((data[2]&0xff)<<8) | ((data[1]&0xff)<<16) | ((data[0]&0xff)<<24);
}

// read double XDR (big endian)
inline void read_double_xdr(const char* data, double & val)
{
    std::int64_t bits = ((std::int64_t)data[7] & 0xff) |
        ((std::int64_t)data[6] & 0xff) << 8   |
        ((std::int64_t)data[5] & 0xff) << 16  |
        ((std::int64_t)data[4] & 0xff) << 24  |
        ((std::int64_t)data[3] & 0xff) << 32  |
        ((std::int64_t)data[2] & 0xff) << 40  |
        ((std::int64_t)data[1] & 0xff) << 48  |
        ((std::int64_t)data[0] & 0xff) << 56  ;
    std::memcpy(&val,&bits,8);
}

#if defined(_MSC_VER) && _MSC_VER < 1800
// msvc doesn't have rint in <cmath>
inline int rint(double val)
{
    return int(std::floor(val + 0.5));
}

inline double round(double val)
{
    return std::floor(val);
}
#endif

#if defined(_MSC_VER)
#define  _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif

}



#endif // MAPNIK_GLOBAL_HPP
