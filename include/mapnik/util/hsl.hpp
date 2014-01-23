/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_HSL_HPP
#define MAPNIK_HSL_HPP

#include <algorithm>

namespace mapnik {

inline void rgb2hsl(double r, double g, double b,
                    double & h, double & s, double & l) {
    double max = std::max(r,std::max(g,b));
    double min = std::min(r,std::min(g,b));
    double delta = max - min;
    double gamma = max + min;
    h = 0.0, s = 0.0, l = gamma / 2.0;
    if (delta > 0.0) {
        s = l > 0.5 ? delta / (2.0 - gamma) : delta / gamma;
        if (max == r && max != g) h = (g - b) / delta + (g < b ? 6.0 : 0.0);
        else if (max == g && max != b) h = (b - r) / delta + 2.0;
        else if (max == b && max != r) h = (r - g) / delta + 4.0;
        h /= 6.0;
    }
}

// http://www.w3.org/TR/css3-color/#hsl-color
inline double hue_to_rgb(double m1, double m2, double h)
{
    if (h < 0.0) h = h + 1.0;
    else if (h > 1.0) h = h - 1.0;
    if (h * 6 < 1.0)
        return m1 + (m2 - m1) * h * 6.0;
    if (h * 2 < 1.0)
        return m2;
    if (h * 3 < 2.0)
        return m1 + (m2 - m1)* (2.0/3.0 - h) * 6.0;
    return m1;
}

inline void hsl2rgb(double h, double s, double l,
                    double & r, double & g, double & b) {
    if (!s) {
        r = g = b = l;
    }
    double m2 = (l <= 0.5) ? l * (s + 1.0) : l + s - l * s;
    double m1 = l * 2.0 - m2;
    r = hue_to_rgb(m1, m2, h + 1.0/3.0);
    g = hue_to_rgb(m1, m2, h);
    b = hue_to_rgb(m1, m2, h - 1.0/3.0);
}

}

#endif // end MAPNIK_HSL_HPP