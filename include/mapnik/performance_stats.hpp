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

#ifdef MAPNIK_THREADSAFE
#include <mutex>
#endif


// mapnik
#include <mapnik/config.hpp>            // for MAPNIK_DECL
#include <mapnik/timer.hpp>
#include <mapnik/util/singleton.hpp>

using mapnik::singleton;
using mapnik::CreateStatic;

namespace mapnik {

struct timer_metrics {
    double cpu_elapsed;
    double wall_clock_elapsed;
};

typedef std::unordered_map<std::string, timer_metrics> metrics_hash_t;

class MAPNIK_DECL timer_stats : public singleton<timer_stats, CreateStatic>
{
public:
    void add(std::string const& metric_name, double cpu_elapsed, double wall_clock_elapsed);
    timer_metrics get(std::string const& metric_name);
    void reset(std::string metric_name);
    void reset_all();
    metrics_hash_t dump();
    metrics_hash_t flush();

private:
    friend class CreateStatic<timer_stats>;
    metrics_hash_t metrics_;
#ifdef MAPNIK_THREADSAFE
    std::mutex metrics_mutex_;
#endif
};

extern template class MAPNIK_DECL singleton<timer_stats, CreateStatic>;


//  A stats_timer behaves like a timer except that the destructor stores
//  elapsed and CPU times for later retrieval from singleton timer_stats
class MAPNIK_DECL stats_timer : public timer
{
public:
    stats_timer(std::string const& metric_name);
    ~stats_timer();
    void stop() const;
    void discard();

private:
    std::string metric_name_;
};

}

#endif // MAPNIK_PERFORMANCE_STATS_HPP
