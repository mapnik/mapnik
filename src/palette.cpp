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

#include <mapnik/palette.hpp>
#include <mapnik/config_error.hpp>

namespace mapnik
{

rgb::rgb(rgba const& c)
    : r(c.r), g(c.g), b(c.b) {}

// ordering by mean(a,r,g,b), a, r, g, b
bool rgba::mean_sort_cmp::operator() (const rgba& x, const rgba& y) const
{
    int t1 = (int)x.a + x.r + x.g + x.b;
    int t2 = (int)y.a + y.r + y.g + y.b;
    if (t1 != t2) return t1 < t2;

    // https://github.com/mapnik/mapnik/issues/1087
    if (x.a != y.a) return x.a < y.a;
    if (x.r != y.r) return x.r < y.r;
    if (x.g != y.g) return x.g < y.g;
    return x.b < y.b;
}

std::size_t rgba::hash_func::operator()(rgba const& p) const
{
    return ((std::size_t)p.r * 33023 + (std::size_t)p.g * 30013 +
            (std::size_t)p.b * 27011 + (std::size_t)p.a * 24007) % 21001;
}


rgba_palette::rgba_palette(std::string const& pal, palette_type type)
    : colors_(0)
{
    parse(pal, type);
}

rgba_palette::rgba_palette()
    : colors_(0) {}

const std::vector<rgb>& rgba_palette::palette() const
{
    return rgb_pal_;
}

const std::vector<unsigned>& rgba_palette::alphaTable() const
{
    return alpha_pal_;
}

bool rgba_palette::valid() const
{
    return colors_ > 0;
}

// return color index in returned earlier palette
unsigned rgba_palette::quantize(rgba const& c) const
{
    unsigned index = 0;
    if (colors_ == 1) return index;

    rgba_hash_table::iterator it = color_hashmap_.find(c);
    if (it != color_hashmap_.end())
    {
        index = it->second;
    }
    else
    {
        int dr, dg, db, da;
        int dist, newdist;

        // find closest match based on mean of r,g,b,a
        std::vector<rgba>::const_iterator pit =
            std::lower_bound(sorted_pal_.begin(), sorted_pal_.end(), c, rgba::mean_sort_cmp());
        index = pit - sorted_pal_.begin();
        if (index == sorted_pal_.size()) index--;

        dr = sorted_pal_[index].r - c.r;
        dg = sorted_pal_[index].g - c.g;
        db = sorted_pal_[index].b - c.b;
        da = sorted_pal_[index].a - c.a;
        dist = dr*dr + dg*dg + db*db + da*da;
        int poz = index;

        // search neighbour positions in both directions for better match
        for (int i = poz - 1; i >= 0; i--)
        {
            dr = sorted_pal_[i].r - c.r;
            dg = sorted_pal_[i].g - c.g;
            db = sorted_pal_[i].b - c.b;
            da = sorted_pal_[i].a - c.a;
            // stop criteria based on properties of used sorting
            if ((dr+db+dg+da) * (dr+db+dg+da) / 4 > dist)
            {
                break;
            }
            newdist = dr*dr + dg*dg + db*db + da*da;
            if (newdist < dist)
            {
                index = i;
                dist = newdist;
            }
        }

        for (unsigned i = poz + 1; i < sorted_pal_.size(); i++)
        {
            dr = sorted_pal_[i].r - c.r;
            dg = sorted_pal_[i].g - c.g;
            db = sorted_pal_[i].b - c.b;
            da = sorted_pal_[i].a - c.a;
            // stop criteria based on properties of used sorting
            if ((dr+db+dg+da) * (dr+db+dg+da) / 4 > dist)
            {
                break;
            }
            newdist = dr*dr + dg*dg + db*db + da*da;
            if (newdist < dist)
            {
                index = i;
                dist = newdist;
            }
        }

        // Cache found index for the color c into the hashmap.
        color_hashmap_[c] = index;
    }

    return index;
}

void rgba_palette::parse(std::string const& pal, palette_type type)
{
    unsigned length = pal.length();

    if ((type == PALETTE_RGBA && length % 4 > 0) ||
        (type == PALETTE_RGB && length % 3 > 0) ||
        (type == PALETTE_ACT && length != 772))
    {
        throw config_error("invalid palette length");
    }

    if (type == PALETTE_ACT)
    {
        length = (pal[768] << 8 | pal[769]) * 3;
    }

    sorted_pal_.clear();
    color_hashmap_.clear();
    rgb_pal_.clear();
    alpha_pal_.clear();

    if (type == PALETTE_RGBA)
    {
        for (unsigned i = 0; i < length; i += 4)
        {
            sorted_pal_.push_back(rgba(pal[i], pal[i + 1], pal[i + 2], pal[i + 3]));
        }
    }
    else
    {
        for (unsigned i = 0; i < length; i += 3)
        {
            sorted_pal_.push_back(rgba(pal[i], pal[i + 1], pal[i + 2], 0xFF));
        }
    }

    // Make sure we have at least one entry in the palette.
    if (sorted_pal_.size() == 0)
    {
        sorted_pal_.push_back(rgba(0, 0, 0, 0));
    }

    colors_ = sorted_pal_.size();

    // Sort palette for binary searching in quantization
    std::sort(sorted_pal_.begin(), sorted_pal_.end(), rgba::mean_sort_cmp());

    // Insert all palette colors into the hashmap and into the palette vectors.
    for (unsigned i = 0; i < colors_; i++)
    {
        rgba c = sorted_pal_[i];
        color_hashmap_[c] = i;
        rgb_pal_.push_back(rgb(c));
        if (c.a < 0xFF)
        {
            alpha_pal_.push_back(c.a);
        }
    }
}

} // namespace mapnik
