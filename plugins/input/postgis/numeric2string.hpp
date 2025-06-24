/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_NUMERIC_2_STRING_HPP
#define MAPNIK_NUMERIC_2_STRING_HPP

#include <mapnik/global.hpp>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>

static inline std::string numeric2string(const char* buf)
{
    std::int16_t ndigits = int2net(buf);
    std::int16_t weight = int2net(buf + 2);
    std::int16_t sign = int2net(buf + 4);
    std::int16_t dscale = int2net(buf + 6);

    std::unique_ptr<std::int16_t[]> digits(new std::int16_t[ndigits]);
    for (int n = 0; n < ndigits; ++n)
    {
        digits[n] = int2net(buf + 8 + n * 2);
    }

    std::ostringstream ss;

    if (sign == 0x4000)
        ss << "-";

    int i = std::max(weight, std::int16_t(0));
    int d = 0;

    // Each numeric "digit" is actually a value between 0000 and 9999 stored in a 16 bit field.
    // For example, the number 1234567809990001 is stored as four digits: [1234] [5678] [999] [1].
    // Note that the last two digits show that the leading 0's are lost when the number is split.
    // We must be careful to re-insert these 0's when building the string.

    while (i >= 0)
    {
        if (i <= weight && d < ndigits)
        {
            // All digits after the first must be padded to make the field 4 characters long
            if (d != 0)
            {
#ifdef _WIN32
                int dig = digits[d];
                if (dig < 10)
                {
                    ss << "000"; // 0000 - 0009
                }
                else if (dig < 100)
                {
                    ss << "00"; // 0010 - 0099
                }
                else
                {
                    ss << "0"; // 0100 - 0999;
                }
#else
                switch (digits[d])
                {
                    case 0 ... 9:
                        ss << "000"; // 0000 - 0009
                        break;
                    case 10 ... 99:
                        ss << "00"; // 0010 - 0099
                        break;
                    case 100 ... 999:
                        ss << "0"; // 0100 - 0999
                        break;
                }
#endif
            }
            ss << digits[d++];
        }
        else
        {
            if (d == 0)
                ss << "0";
            else
                ss << "0000";
        }

        i--;
    }
    if (dscale > 0)
    {
        ss << '.';
        // dscale counts the number of decimal digits following the point, not the numeric digits
        while (dscale > 0)
        {
            int value;
            if (i <= weight && d < ndigits)
                value = digits[d++];
            else
                value = 0;

            // Output up to 4 decimal digits for this value
            if (dscale > 0)
            {
                ss << (value / 1000);
                value %= 1000;
                dscale--;
            }
            if (dscale > 0)
            {
                ss << (value / 100);
                value %= 100;
                dscale--;
            }
            if (dscale > 0)
            {
                ss << (value / 10);
                value %= 10;
                dscale--;
            }
            if (dscale > 0)
            {
                ss << value;
                dscale--;
            }

            i--;
        }
    }
    return ss.str();
}

#endif
