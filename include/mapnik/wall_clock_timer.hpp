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

/*
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
        wall_clock_progress_timer(std::ostream & os = std::cout, const char * base_message = "") : _os(os), _base_message(base_message) {}
        ~wall_clock_progress_timer()
        {
            //  A) Throwing an exception from a destructor is a Bad Thing.
            //  B) The progress_timer destructor does output which may throw.
            //  C) A progress_timer is usually not critical to the application.
            //  Therefore, wrap the I/O in a try block, catch and ignore all exceptions.
            try
            {
                // use istream instead of ios_base to workaround GNU problem (Greg Chicares)
                std::istream::fmtflags old_flags = _os.setf( std::istream::fixed,
                                                             std::istream::floatfield );
                std::streamsize old_prec = _os.precision( 2 );
                _os << _base_message << elapsed() << " ms\n" // "s" is System International d'Unites std
                                  << std::endl;
                _os.flags( old_flags );
                _os.precision( old_prec );
            }
            catch (...) {} // eat any exceptions
      }

    private:
        std::ostream & _os;
        const char * _base_message;
    };
    
};

*/
#endif // MAPNIK_WALL_CLOCK_TIMER_INCLUDED
