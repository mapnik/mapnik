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

#ifndef MAPNIK_TIMER_INCLUDED
#define MAPNIK_TIMER_INCLUDED

#include <cstdlib>
#include <string>
#include <sys/time.h> 

namespace mapnik {

// Measure times in both wall clock time and CPU times. Results are returned in milliseconds.
class timer
{
public:
    timer()
    {
        restart();
    }

    void restart() 
    {   
        _stopped = false;
        gettimeofday(&_wall_clock_start, NULL);
        _cpu_start = clock();
    }

    virtual void stop() const
    {
        _stopped = true;
        _cpu_end = clock();
        gettimeofday(&_wall_clock_end, NULL);
    }

    double cpu_elapsed() const
    {
        // return elapsed CPU time in ms
        if (!_stopped)
            stop();

        return ((double) (_cpu_end - _cpu_start)) / CLOCKS_PER_SEC * 1000.0;
    }

    double wall_clock_elapsed() const
    {
        // return elapsed wall clock time in ms
        if (!_stopped)
            stop();

        long seconds  = _wall_clock_end.tv_sec  - _wall_clock_start.tv_sec;
        long useconds = _wall_clock_end.tv_usec - _wall_clock_start.tv_usec;

        return ((seconds) * 1000 + useconds / 1000.0) + 0.5;
    }
protected:
    mutable timeval _wall_clock_start, _wall_clock_end;
    mutable clock_t _cpu_start, _cpu_end;
    mutable bool _stopped;
};

//  A progress_timer behaves like a timer except that the destructor displays
//  an elapsed time message at an appropriate place in an appropriate form.
class progress_timer : public timer
{
public:
    progress_timer(std::ostream & os, std::string const& base_message):
      os_(os),
      base_message_(base_message)
      {}

    ~progress_timer()
    {
        if (!_stopped)
            stop();
    }
    
    void stop() const
    {
        timer::stop();
        try
        {
            std::ostringstream s;
            s.precision(2);
            s << std::fixed; 
            s << wall_clock_elapsed() << "ms (cpu " << cpu_elapsed() << "ms)";
            s << std::setw(30 - (int)s.tellp()) << std::right << "| " << base_message_ << "\n"; 
            os_ << s.str();
        }
        catch (...) {} // eat any exceptions
    }

    void discard() {
        _stopped = true;
    }

private:
    std::ostream & os_;
    std::string base_message_;
};
    
};
#endif // MAPNIK_TIMER_INCLUDED