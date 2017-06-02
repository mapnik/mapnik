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

#include "timer.hpp"


namespace mapnik {

struct timer_metrics {
    double cpu_elapsed;
    double wall_clock_elapsed;
};

typedef std::unordered_map<std::string, timer_metrics> metrics_hash_t;

class timer_stats_
{
public:
    void add(std::string const& metric_name, double cpu_elapsed, double wall_clock_elapsed)
    {
        timer_metrics& metrics = timer_stats_[metric_name];
        metrics.cpu_elapsed += cpu_elapsed;
        metrics.wall_clock_elapsed += wall_clock_elapsed;
        timer_stats_[metric_name] = metrics;
    }

    timer_metrics get(std::string const& metric_name)
    {
        return timer_stats_[metric_name];
    }

    void reset(std::string metric_name) {
        timer_stats_.erase(metric_name);
    }

    void reset_all() {
        timer_stats_.clear();
    }

    metrics_hash_t::iterator begin() {
        return timer_stats_.begin();
    }

    metrics_hash_t::iterator end() {
        return timer_stats_.end();
    }

    std::string dump() {
        std::stringstream out;
        for(auto metric : timer_stats_) {
            out << metric.first << "\tcpu_time = " << metric.second.cpu_elapsed << " ms\twall_time = " << metric.second.wall_clock_elapsed << " ms" << std::endl;
        }
        return out.str();
    }

private:
    metrics_hash_t timer_stats_;
};

timer_stats_ timer_stats;



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
            timer_stats.add(metric_name_, cpu_elapsed(), wall_clock_elapsed());
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
