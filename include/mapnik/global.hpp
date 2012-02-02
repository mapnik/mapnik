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

#ifndef MAPNIK_GLOBAL_HPP
#define MAPNIK_GLOBAL_HPP

// boost
#include <boost/cstdint.hpp>
#include <boost/detail/endian.hpp>

// stl
#include <cstring>

namespace mapnik
{

#ifdef BOOST_BIG_ENDIAN
#define MAPNIK_BIG_ENDIAN
#endif

#define int2net(A)  (int16_t) (((boost::uint16_t) ((boost::uint8_t) (A)[1]))      | \
                               (((boost::uint16_t) ((boost::uint8_t) (A)[0])) << 8))

#define int4net(A)  (int32_t)  (((boost::uint32_t) ((boost::uint8_t) (A)[3]))        | \
                                (((boost::uint32_t) ((boost::uint8_t) (A)[2])) << 8)  | \
                                (((boost::uint32_t) ((boost::uint8_t) (A)[1])) << 16) | \
                                (((boost::uint32_t) ((boost::uint8_t) (A)[0])) << 24))

#define int8net(A)  (int64_t)  (((boost::uint64_t) ((boost::uint8_t) (A)[7]))        | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[6])) << 8)  | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[5])) << 16) | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[4])) << 24) | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[3])) << 32) | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[2])) << 40) | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[1])) << 48) | \
                                (((boost::uint64_t) ((boost::uint8_t) (A)[0])) << 56))

typedef boost::uint8_t byte;
#define float8net(V,M)   do { double def_temp;  \
        ((byte*) &def_temp)[0]=(M)[7];          \
        ((byte*) &def_temp)[1]=(M)[6];          \
        ((byte*) &def_temp)[2]=(M)[5];          \
        ((byte*) &def_temp)[3]=(M)[4];          \
        ((byte*) &def_temp)[4]=(M)[3];          \
        ((byte*) &def_temp)[5]=(M)[2];          \
        ((byte*) &def_temp)[6]=(M)[1];          \
        ((byte*) &def_temp)[7]=(M)[0];          \
        (V) = def_temp; } while(0)
#define float4net(V,M)   do { float def_temp;   \
        ((byte*) &def_temp)[0]=(M)[3];          \
        ((byte*) &def_temp)[1]=(M)[2];          \
        ((byte*) &def_temp)[2]=(M)[1];          \
        ((byte*) &def_temp)[3]=(M)[0];          \
        (V)=def_temp; } while(0)


// read int16_t NDR (little endian)
inline void read_int16_ndr(const char* data, boost::int16_t & val)
{
#ifndef MAPNIK_BIG_ENDIAN
    std::memcpy(&val,data,2);
#else
    val = (data[0]&0xff) |
        ((data[1]&0xff)<<8);
#endif
}

// read int32_t NDR (little endian)
inline void read_int32_ndr(const char* data, boost::int32_t & val)
{
#ifndef MAPNIK_BIG_ENDIAN
    std::memcpy(&val,data,4);
#else
    val = (data[0]&0xff)     |
        ((data[1]&0xff)<<8)  |
        ((data[2]&0xff)<<16) |
        ((data[3]&0xff)<<24);
#endif
}

// read double NDR (little endian)
inline void read_double_ndr(const char* data, double & val)
{
#ifndef MAPNIK_BIG_ENDIAN
    std::memcpy(&val,&data[0],8);
#else
    boost::int64_t bits = ((boost::int64_t)data[0] & 0xff) |
        ((boost::int64_t)data[1] & 0xff) << 8   |
        ((boost::int64_t)data[2] & 0xff) << 16  |
        ((boost::int64_t)data[3] & 0xff) << 24  |
        ((boost::int64_t)data[4] & 0xff) << 32  |
        ((boost::int64_t)data[5] & 0xff) << 40  |
        ((boost::int64_t)data[6] & 0xff) << 48  |
        ((boost::int64_t)data[7] & 0xff) << 56  ;
    std::memcpy(&val,&bits,8);
#endif
}

// read int16_t XDR (big endian)
inline void read_int16_xdr(const char* data, boost::int16_t & val)
{
#ifndef MAPNIK_BIG_ENDIAN
    val = (data[3]&0xff) | ((data[2]&0xff)<<8);
#else
    std::memcpy(&val,data,2);
#endif
}

// read int32_t XDR (big endian)
inline void read_int32_xdr(const char* data, boost::int32_t & val)
{
#ifndef MAPNIK_BIG_ENDIAN
    val = (data[3]&0xff) | ((data[2]&0xff)<<8) | ((data[1]&0xff)<<16) | ((data[0]&0xff)<<24);
#else
    std::memcpy(&val,data,4);
#endif
}

// read double XDR (big endian)
inline void read_double_xdr(const char* data, double & val)
{
#ifndef MAPNIK_BIG_ENDIAN
    boost::int64_t bits = ((boost::int64_t)data[7] & 0xff) |
        ((boost::int64_t)data[6] & 0xff) << 8   |
        ((boost::int64_t)data[5] & 0xff) << 16  |
        ((boost::int64_t)data[4] & 0xff) << 24  |
        ((boost::int64_t)data[3] & 0xff) << 32  |
        ((boost::int64_t)data[2] & 0xff) << 40  |
        ((boost::int64_t)data[1] & 0xff) << 48  |
        ((boost::int64_t)data[0] & 0xff) << 56  ;
    std::memcpy(&val,&bits,8);
#else
    std::memcpy(&val,&data[0],8);
#endif
}

#ifdef _WINDOWS
// msvc doesn't have rint in <cmath>
inline int rint( double val)
{
    return int(floor(val + 0.5));
}

inline double round (double val)
{
    return floor(val);
}

#define  _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif

}



#endif // MAPNIK_GLOBAL_HPP
