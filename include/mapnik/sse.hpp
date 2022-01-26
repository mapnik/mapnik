/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_SSE_HPP
#define MAPNIK_SSE_HPP

#include <emmintrin.h>
#include <xmmintrin.h>

#define ROUND_DOWN(x, s) ((x) & ~((s)-1))

typedef union {
    __m128i v;
    int32_t i32[4];
    uint32_t u32[4];
    uint16_t u16[8];
    uint8_t u8[16];
} m128_int;

static inline __m128i _mm_cmple_epu16(__m128i x, __m128i y)
{
    // Returns 0xFFFF where x <= y:
    return _mm_cmpeq_epi16(_mm_subs_epu16(x, y), _mm_setzero_si128());
}

static inline __m128i _mm_cmple_epu8(__m128i x, __m128i y)
{
    // Returns 0xFF where x <= y:
    return _mm_cmpeq_epi8(_mm_min_epu8(x, y), x);
}

static inline __m128i _mm_cmpgt_epu16(__m128i x, __m128i y)
{
    // Returns 0xFFFF where x > y:
    return _mm_andnot_si128(_mm_cmpeq_epi16(x, y), _mm_cmple_epu16(y, x));
}

static inline __m128i _mm_cmpgt_epu8(__m128i x, __m128i y)
{
    // Returns 0xFF where x > y:
    return _mm_andnot_si128(_mm_cmpeq_epi8(x, y), _mm_cmpeq_epi8(_mm_max_epu8(x, y), x));
}

static inline __m128i _mm_cmplt_epu16(__m128i x, __m128i y)
{
    // Returns 0xFFFF where x < y:
    return _mm_cmpgt_epu16(y, x);
}

static inline __m128i _mm_cmplt_epu8(__m128i x, __m128i y)
{
    // Returns 0xFF where x < y:
    return _mm_cmpgt_epu8(y, x);
}

static inline __m128i _mm_cmpge_epu16(__m128i x, __m128i y)
{
    // Returns 0xFFFF where x >= y:
    return _mm_cmple_epu16(y, x);
}

static inline __m128i _mm_cmpge_epu8(__m128i x, __m128i y)
{
    // Returns 0xFF where x >= y:
    return _mm_cmple_epu8(y, x);
}

// Its not often that you want to use this!
static inline __m128i _mm_not_si128(__m128i x)
{
    // Returns ~x, the bitwise complement of x:
    return _mm_xor_si128(x, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
}

static inline __m128i _mm_absdiff_epu16(__m128i x, __m128i y)
{
    // Calculate absolute difference: abs(x - y):
    return _mm_or_si128(_mm_subs_epu16(x, y), _mm_subs_epu16(y, x));
}

static inline __m128i _mm_absdiff_epu8(__m128i x, __m128i y)
{
    // Calculate absolute difference: abs(x - y):
    return _mm_or_si128(_mm_subs_epu8(x, y), _mm_subs_epu8(y, x));
}

static inline __m128i _mm_div255_epu16(__m128i x)
{
    // Divide 8 16-bit uints by 255:
    // x := ((x + 1) + (x >> 8)) >> 8:
    return _mm_srli_epi16(_mm_adds_epu16(_mm_adds_epu16(x, _mm_set1_epi16(1)), _mm_srli_epi16(x, 8)), 8);
}

static __m128i _mm_scale_epu8(__m128i x, __m128i y)
{
    // Returns an "alpha blend" of x scaled by y/255;
    //   x := x * (y / 255)
    // Reorder: x := (x * y) / 255

    // Unpack x and y into 16-bit uints:
    __m128i xlo = _mm_unpacklo_epi8(x, _mm_setzero_si128());
    __m128i ylo = _mm_unpacklo_epi8(y, _mm_setzero_si128());
    __m128i xhi = _mm_unpackhi_epi8(x, _mm_setzero_si128());
    __m128i yhi = _mm_unpackhi_epi8(y, _mm_setzero_si128());

    // Multiply x with y, keeping the low 16 bits:
    xlo = _mm_mullo_epi16(xlo, ylo);
    xhi = _mm_mullo_epi16(xhi, yhi);

    // Divide by 255:
    xlo = _mm_div255_epu16(xlo);
    xhi = _mm_div255_epu16(xhi);

    // Repack the 16-bit uints to clamped 8-bit values:
    return _mm_packus_epi16(xlo, xhi);
}

#endif // MAPNIK_SSE_HPP
