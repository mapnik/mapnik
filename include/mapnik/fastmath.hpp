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

#ifndef MAPNIK_FASTMATH_HPP
#define MAPNIK_FASTMATH_HPP

/* Timings:
 * fast_sin(not inlined) 8.95s
 * fast_sin(inlined) 6.64s
 * sin 28.7s
 * => 4.3x speedup
 * worst case accuracy abs(fast_sin(x)/sin(x) - 1) = 0.000018 (at 0.664, M_PI-0.664, M_PI+0.664; 2*M_PI-0.664)
 */
static inline double fast_sin(double x)
{
    bool negative = false;
    double result;
    while (x > 2*M_PI) x -= 2*M_PI;
    while (x < 0) x += 2*M_PI;
    if (x > M_PI) {
        x -= M_PI;
        negative = true;
    }

    if (x < 0.664 || x > M_PI-0.664) {
        //series expansion at x=0:  x-x^3/6+x^5/120-x^7/5040+...
        if (x > M_PI-0.664) x = M_PI - x;
        result = x*(1. + x*x*(-1/6. + x*x/120.));
    } else {
        //series expansion at x=pi/2
        //1-x^2/2+x^4/24-x^6/720+x^8/40320+...
        x -= M_PI/2;
        result =  1. + x*x*(-1/2.+x*x*(1/24. + x*x*(-1/720.)));
    }
    return negative?-result:result;
}

static inline double fast_cos(double x)
{
    return fast_sin(x + M_PI/2);
}

static inline  double atan_helper(double x)
{
    //Series expansion at x=0:
    // x-x^3/3+x^5/5-x^7/7+...
    if (x < 0.30) {
        return x * (1 + x*x*(-1/3. + x*x*(1/5.) + x*x*(-1/7. + x*x*(1/9.))));
    }
    else if (x < 0.71) {
        //Series expansion at x=0.5
        //atan(1/2)+(4 x)/5-(8 x^2)/25-(16 x^3)/375+(96 x^4)/625 +...
        x -= 0.5;
        return 0.463647609000806116 /*atan(0.5) */ + x *(4./5. + x *(-8./25. + (-16./375.*x)));
    } else {
        //series expansion at x=1:
        //pi/4+x/2-x^2/4+x^3/12-x^5/40+...
        x -= 1;
        return (M_PI/4.) + x * (1/2. + x*(-1/4. +x*(1/12. + x * (-1/40.))));
    }
}

/*
 * fast_atan(not inlined) 6.74s
 * fast_atan(everything inlined) 6.78s
 * fast_atan(only helper inlined) 6.75
 * atan 27.5s
 * => 4x speedup
 * worst case accuracy abs(fast_atan(x)/atan(x) - 1) = 0.000271 (at 1.411)
 */
double inline fast_atan(double x)
{
    double negative = false;
    double result;
    if (x < 0) {
        x = -x;
        negative = true;
    }
    if (x <= 1) result = atan_helper(x); else result = M_PI/2 - atan_helper(1/x);
    return negative?-result:result;
}

static inline double fast_atan2(double y, double x)
{
    double result = M_PI/2;
    if (x == 0 && y == 0) return 0;
    if (x != 0) result = fast_atan(y/x);
    if (x < 0 && y >= 0) return result + M_PI;
    if (x <= 0 && y < 0) return -M_PI + result;
    return result;
}

#endif // MAPNIK_FASTMATH_HPP
