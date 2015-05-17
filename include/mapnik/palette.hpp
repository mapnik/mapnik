/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/util/noncopyable.hpp>

#define USE_DENSE_HASH_MAP

#ifdef USE_DENSE_HASH_MAP
    #include <mapnik/sparsehash/dense_hash_map>
    using rgba_hash_table = google::dense_hash_map<unsigned int, unsigned char>;
#else
    #include <boost/unordered_map.hpp>
    using rgba_hash_table = boost::unordered_map<unsigned int, unsigned char>;
#endif

// stl
#include <vector>

#define U2RED(x) ((x)&0xff)
#define U2GREEN(x) (((x)>>8)&0xff)
#define U2BLUE(x) (((x)>>16)&0xff)
#define U2ALPHA(x) (((x)>>24)&0xff)

namespace mapnik {

struct rgba;

struct MAPNIK_DECL rgb {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;

    inline rgb(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_) : r(r_), g(g_), b(b_) {}
    rgb(rgba const& c);

    inline bool operator==(const rgb& y) const
    {
        return r == y.r && g == y.g && b == y.b;
    }
};

struct MAPNIK_DECL rgba
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    inline rgba(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_)
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

    // ordering by mean(a,r,g,b), a, r, g, b
    struct MAPNIK_DECL mean_sort_cmp
    {
        bool operator() (const rgba& x, const rgba& y) const;
    };

};


class MAPNIK_DECL rgba_palette : private util::noncopyable {
public:
    enum palette_type { PALETTE_RGBA = 0, PALETTE_RGB = 1, PALETTE_ACT = 2 };

    explicit rgba_palette(std::string const& pal, palette_type type = PALETTE_RGBA);
    rgba_palette();

    const std::vector<rgb>& palette() const;
    const std::vector<unsigned>& alphaTable() const;

    unsigned char quantize(unsigned c) const;

    bool valid() const;
    std::string to_string() const;

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
