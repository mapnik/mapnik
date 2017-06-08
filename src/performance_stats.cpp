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

timer_stats & timer_stats::instance()
{
    static timer_stats ts;
    return ts;
}

void timer_stats::add(std::string const& metric_name, double cpu_elapsed, double wall_clock_elapsed)
{
    timer_metrics& metrics = metrics_[metric_name];
    metrics.cpu_elapsed += cpu_elapsed;
    metrics.wall_clock_elapsed += wall_clock_elapsed;
    metrics_[metric_name] = metrics;
}

timer_metrics timer_stats::get(std::string const& metric_name)
{
    return metrics_[metric_name];
}

void timer_stats::reset(std::string metric_name) {
    metrics_.erase(metric_name);
}

void timer_stats::reset_all() {
    metrics_.clear();
}

metrics_hash_t::iterator timer_stats::begin() {
    return metrics_.begin();
}

metrics_hash_t::iterator timer_stats::end() {
    return metrics_.end();
}

std::string timer_stats::dump() {
    std::stringstream out;
    for(auto metric : metrics_) {
        out << metric.first << "\tcpu_time = " << metric.second.cpu_elapsed << " ms\twall_time = " << metric.second.wall_clock_elapsed << " ms" << std::endl;
    }
    return out.str();
}

std::string timer_stats::flush() {
    std::string out = dump();
    reset_all();
    return out;
}

}
