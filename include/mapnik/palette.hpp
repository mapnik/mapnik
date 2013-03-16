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

#ifndef MAPNIK_PALETTE_HPP
#define MAPNIK_PALETTE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/global.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>

// stl
#include <vector>
#include <map>
#include <iostream>
#include <set>
#include <algorithm>
#include <cmath>


#ifdef MAPNIK_BIG_ENDIAN
#define U2RED(x) (((x)>>24)&0xff)
#define U2GREEN(x) (((x)>>16)&0xff)
#define U2BLUE(x) (((x)>>8)&0xff)
#define U2ALPHA(x) ((x)&0xff)
#else
#define U2RED(x) ((x)&0xff)
#define U2GREEN(x) (((x)>>8)&0xff)
#define U2BLUE(x) (((x)>>16)&0xff)
#define U2ALPHA(x) (((x)>>24)&0xff)
#endif


namespace mapnik {

typedef boost::uint8_t byte;
struct rgba;

struct rgb {
    byte r;
    byte g;
    byte b;

    inline rgb(byte r_, byte g_, byte b_) : r(r_), g(g_), b(b_) {};
    rgb(rgba const& c);

    inline bool operator==(const rgb& y) const
    {
        return r == y.r && g == y.g && b == y.b;
    }
};

struct rgba
{
    byte r;
    byte g;
    byte b;
    byte a;

    inline rgba(byte r_, byte g_, byte b_, byte a_)
        : r(r_),
          g(g_),
          b(b_),
          a(a_) {}

    inline rgba(rgb const& c)
        : r(c.r),
          g(c.g),
          b(c.b),
          a(0xFF) {}

    inline rgba(unsigned const& c)
        : r(U2RED(c)),
          g(U2GREEN(c)),
          b(U2BLUE(c)),
          a(U2ALPHA(c)) {}

    inline bool operator==(const rgba& y) const
    {
        return r == y.r && g == y.g && b == y.b && a == y.a;
    }

    inline operator unsigned() const
    {
#ifdef MAPNIK_BIG_ENDIAN
        return (r << 24) | (g << 16) | (b << 8) | a;
#else
        return r | (g << 8) | (b << 16) | (a << 24);
#endif
    }

    // ordering by mean(a,r,g,b), a, r, g, b
    struct mean_sort_cmp
    {
        bool operator() (const rgba& x, const rgba& y) const;
    };

    struct hash_func : public std::unary_function<rgba, std::size_t>
    {
        std::size_t operator()(rgba const& p) const;
    };
};


typedef boost::unordered_map<unsigned, unsigned> rgba_hash_table;


class MAPNIK_DECL rgba_palette : private boost::noncopyable {
public:
    enum palette_type { PALETTE_RGBA = 0, PALETTE_RGB = 1, PALETTE_ACT = 2 };

    explicit rgba_palette(std::string const& pal, palette_type type = PALETTE_RGBA);
    rgba_palette();

    const std::vector<rgb>& palette() const;
    const std::vector<unsigned>& alphaTable() const;

    unsigned char quantize(rgba const& c) const;
    inline unsigned char quantize(unsigned const& c) const
    {
        rgba_hash_table::const_iterator it = color_hashmap_.find(c);
        if (it != color_hashmap_.end())
        {
            return it->second;
        }
        else {
            return quantize(rgba(U2RED(c), U2GREEN(c), U2BLUE(c), U2ALPHA(c)));
        }
    }

    bool valid() const;

private:
    void parse(std::string const& pal, palette_type type);

private:
    std::vector<rgba> sorted_pal_;
    mutable rgba_hash_table color_hashmap_;

    unsigned colors_;
    std::vector<rgb> rgb_pal_;
    std::vector<unsigned> alpha_pal_;
};

} // namespace mapnik

#endif // MAPNIK_PALETTE_HPP

