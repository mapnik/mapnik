/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_PERFORMANCE_STATS_HPP
#define MAPNIK_PERFORMANCE_STATS_HPP

// stl
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <iterator>

// mapnik
#include <mapnik/config.hpp>            // for MAPNIK_DECL
#include <mapnik/timer.hpp>


namespace mapnik {

struct timer_metrics {
    double cpu_elapsed;
    double wall_clock_elapsed;
};

typedef std::unordered_map<std::string, timer_metrics> metrics_hash_t;

class MAPNIK_DECL timer_stats
{
public:
    static timer_stats & instance();
    void add(std::string const& metric_name, double cpu_elapsed, double wall_clock_elapsed);
    timer_metrics get(std::string const& metric_name);
    void reset(std::string metric_name);
    void reset_all();
    metrics_hash_t::iterator begin();
    metrics_hash_t::iterator end();
    std::string dump();
    std::string flush();

private:
    metrics_hash_t metrics_;
};


//  A stats_timer behaves like a timer except that the destructor stores
//  elapsed and CPU times for later retrieval from global timer_stats
class stats_timer : public timer
{
public:
    stats_timer(std::string const& metric_name)
        : metric_name_(metric_name)
    {}

    ~stats_timer()
    {
        if (! stopped_)
        {
            stop();
        }
    }

    void stop() const
    {
        timer::stop();
        try
        {
            timer_stats::instance().add(metric_name_, cpu_elapsed(), wall_clock_elapsed());
        }
        catch (...) {} // eat any exceptions
    }

    void discard()
    {
        stopped_ = true;
    }

private:
    std::string metric_name_;
};

}

#endif // MAPNIK_PERFORMANCE_STATS_HPP
