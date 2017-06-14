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

#include <mapnik/performance_stats.hpp>

namespace mapnik {

template class MAPNIK_DECL singleton<timer_stats, CreateStatic>;

void timer_stats::add(std::string const& metric_name, double cpu_elapsed, double wall_clock_elapsed)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(metrics_mutex_);
#endif
    timer_metrics& metrics = metrics_[metric_name];
    metrics.cpu_elapsed += cpu_elapsed;
    metrics.wall_clock_elapsed += wall_clock_elapsed;
    metrics_[metric_name] = metrics;
}

timer_metrics timer_stats::get(std::string const& metric_name)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(metrics_mutex_);
#endif
    return metrics_[metric_name];
}

void timer_stats::reset(std::string metric_name)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(metrics_mutex_);
#endif
    metrics_.erase(metric_name);
}

void timer_stats::reset_all()
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(metrics_mutex_);
#endif
    metrics_.clear();
}

metrics_hash_t timer_stats::dump() {
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(metrics_mutex_);
#endif
    return metrics_;
}

metrics_hash_t timer_stats::flush() {
    metrics_hash_t out = dump();
    reset_all();
    return out;
}



stats_timer::stats_timer(std::string const& metric_name)
    : metric_name_(metric_name)
{}

stats_timer::~stats_timer()
{
    if (! stopped_)
        {
            stop();
        }
}

void stats_timer::stop() const
{
    timer::stop();
    try
        {
            timer_stats::instance().add(metric_name_, cpu_elapsed(), wall_clock_elapsed());
        }
    catch (...) {} // eat any exceptions
}

void stats_timer::discard()
{
    stopped_ = true;
}

} // end of ns
