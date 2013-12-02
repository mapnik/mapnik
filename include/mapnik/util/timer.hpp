/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef MAPNIK_UTIL_TIMER_HPP
#define MAPNIK_UTIL_TIMER_HPP

#include <string>
#include <chrono>

namespace mapnik {

class auto_cpu_timer
{
public:
    auto_cpu_timer(std::ostream & os, std::string const& message)
        : start_(std::chrono::system_clock::now()),
          os_(os),
          message_(message) {}

    ~auto_cpu_timer()
    {
        std::chrono::duration<double,std::milli> elapsed = std::chrono::system_clock::now() - start_;
        os_ << message_ << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms" << std::endl;
    }
private:
    std::chrono::time_point<std::chrono::system_clock> start_;
    std::ostream & os_;
    std::string message_;
};

// NOTE : add more timers here

}

#endif // MAPNIK_UTIL_TIMER_HPP
