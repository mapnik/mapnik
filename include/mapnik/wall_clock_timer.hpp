/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef MAPNIK_WALL_CLOCK_TIMER_INCLUDED
#define MAPNIK_WALL_CLOCK_TIMER_INCLUDED

#include <cstdlib>
#include <string>
#include <sys/time.h> 

namespace mapnik {

// This is a class with a similar signature to boost::timer, but which measures
// times in wall clock time. Results are returned in milliseconds.
class wall_clock_timer
{
public:
    wall_clock_timer()
    { 
        gettimeofday(&_start_time, NULL);
    }

    void restart() 
    {   
        gettimeofday(&_start_time, NULL);
    }

    double elapsed() const
    { 
        timeval end;
        gettimeofday(&end, NULL);

        long seconds  = end.tv_sec  - _start_time.tv_sec;
        long useconds = end.tv_usec - _start_time.tv_usec;

        return ((seconds) * 1000 + useconds / 1000.0) + 0.5;
    }
private:
    timeval _start_time;
};

//  A progress_timer behaves like a timer except that the destructor displays
//  an elapsed time message at an appropriate place in an appropriate form.
class wall_clock_progress_timer : public wall_clock_timer
{
public:
    wall_clock_progress_timer(std::ostream & os,
                              std::string const& base_message):
      os_(os),
      base_message_(base_message),
      stopped_(false) {}

    ~wall_clock_progress_timer()
    {
        if (!stopped_)
            stop();
    }
    
    void stop() {
        stopped_ = true;
        try
        {
            std::ostringstream s;
            s.precision(2);
            s << std::fixed; 
            s << elapsed() << " ms";
            s << std::setw(15 - (int)s.tellp()) << std::right << "| " << base_message_ << "\n"; 
            os_ << s.str();
        }
        catch (...) {} // eat any exceptions
    }

    void discard() {
        stopped_ = true;
    }

private:
    std::ostream & os_;
    std::string base_message_;
    bool stopped_;
};
    
};
#endif // MAPNIK_WALL_CLOCK_TIMER_INCLUDED
