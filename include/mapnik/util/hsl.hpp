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

#include <cmath>

namespace mapnik {

static inline void rgb2hsl(unsigned char red, unsigned char green, unsigned char blue,
             double & h, double & s, double & l) {
    double r = red/255.0;
    double g = green/255.0;
    double b = blue/255.0;
    double max = std::max(r,std::max(g,b));
    double min = std::min(r,std::min(g,b));
    double delta = max - min;
    double gamma = max + min;
    h = s = l = gamma / 2.0;
    if (delta > 0.0) {
        s = l > 0.5 ? delta / (2.0 - gamma) : delta / gamma;
        if (max == r && max != g) h = (g - b) / delta + (g < b ? 6.0 : 0.0);
        else if (max == g && max != b) h = (b - r) / delta + 2.0;
        else if (max == b && max != r) h = (r - g) / delta + 4.0;
        h /= 6.0;
    }
}

static inline double hueToRGB(double m1, double m2, double h) {
    // poor mans fmod
    if(h < 0) h += 1;
    if(h > 1) h -= 1;
    if (h * 6 < 1) return m1 + (m2 - m1) * h * 6;
    if (h * 2 < 1) return m2;
    if (h * 3 < 2) return m1 + (m2 - m1) * (0.66666 - h) * 6;
    return m1;
}

static inline void hsl2rgb(double h, double s, double l,
             unsigned char & r, unsigned char & g, unsigned char & b) {
    if (!s) {
        r = g = b = static_cast<unsigned char>(l * 255);
    }
    double m2 = (l <= 0.5) ? l * (s + 1) : l + s - l * s;
    double m1 = l * 2 - m2;
    r = static_cast<unsigned char>(hueToRGB(m1, m2, h + 0.33333) * 255);
    g = static_cast<unsigned char>(hueToRGB(m1, m2, h) * 255);
    b = static_cast<unsigned char>(hueToRGB(m1, m2, h - 0.33333) * 255);
}

}

#endif // end MAPNIK_HSL_HPP