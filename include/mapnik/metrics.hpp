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

#ifndef MAPNIK_METRICS_HPP
#define MAPNIK_METRICS_HPP

#ifdef MAPNIK_METRICS

#include <mapnik/config.hpp>

#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <chrono>
#include <memory>
#include <string>

#ifdef MAPNIK_THREADSAFE
#include <mutex>
#endif

namespace mapnik {

enum measurement_t : int_fast8_t
{
    UNASSIGNED = 0,
    TIME_MICROSECONDS,
    VALUE,
    CALLS,
    TOTAL_ENUM_SIZE
};

struct MAPNIK_DECL measurement
{
    measurement() = default;
    explicit measurement(int64_t value, measurement_t type = measurement_t::UNASSIGNED);

    int64_t value_ = 0;
    int_fast32_t calls_ = 1;
    measurement_t type_ = measurement_t::UNASSIGNED;
};

class metrics;

class MAPNIK_DECL autochrono
{
    using steady_clock = std::chrono::steady_clock;
    using time_units = std::chrono::microseconds;

public:
    autochrono(metrics* m, std::string name);
    ~autochrono();

    autochrono() = delete;
    autochrono(autochrono const &&) = delete;
    autochrono& operator=(autochrono const &) = delete;
    autochrono& operator=(autochrono &&) = delete;
    autochrono(autochrono const &) = delete;


private:
    metrics* metrics_;
    std::string const name_;
    steady_clock clock_;
    steady_clock::time_point start_;
};


class MAPNIK_DECL metrics
{
    friend autochrono;
public:
    using metrics_tree = boost::property_tree::basic_ptree<std::string,
                                                           struct measurement>;
    /* Whether metrics are enabled or not. If disabled any calls to
     * measure_XXX (add/dec/time) will be ignored */
    bool enabled_ = false;

    /* Prefix to use when storing metrics under this object. Make sure to finish
     * it with a '.' to change the hierarchy level of the metrics. For example,
     * if set to "Render." a new metric "Layer" will be stored as "Render.Layer"
     */
    std::string prefix_ = "";

    /**
     * Default constructor with an empty tree
     */
    metrics() = delete;
    metrics(bool enabled);

    /**
     * Builds with the same shared tree as the passed object
     * enabled_ is also copied but independent
     * The prefix is added to the passed one. So if the previous has "Render.",
     * and the new one is "Layer.", "Render.Layer." will be used
     */
    metrics(metrics const &m, std::string prefix = "");

    /* Move constructor */
    metrics(metrics const &&m);

    /* Copy assignment operator */
    metrics& operator=(metrics const &);

    /* Move assignment operator */
    metrics& operator=(metrics &&);

    ~metrics() = default;

    /**
     * Sets up a timer to measure an event. The timer will be stopped and saved
     * when the return value is destroyed / out of scope
     * @param metric_name - Name to use to store the metric
     * @return Smart pointer to hold the reference to the timer
     */
    inline std::unique_ptr<autochrono> measure_time(std::string const& name)
    {
        if (!enabled_) return nullptr;
        return measure_time_impl(name);
    }

    /**
     * Increment the value of a metric
     * @param name - Name of the metric
     * @param value - Value to increment. Default = 1
     * @param type - Type of the stored metric. Default: VALUE
     */
    inline void measure_add(std::string const& name, int64_t value = 1,
                            measurement_t type = measurement_t::VALUE)
    {
        if (!enabled_) return;
        measure_add_impl(name, value, type);
    }

    /**
     * Find a metric by name
     * @param name - Name of the metric
     * @param ignore_prefix - Whether to ignore stored prefix in the search
     * @return optional value with the measurement
     */
    boost::optional<measurement &> find(std::string const& name,
                                        bool ignore_prefix = false);

    /**
     * Generates a string with the metrics (for the full tree)
     * @return std::string in JSON style
     */
    std::string to_string();

private:
    std::unique_ptr<autochrono> measure_time_impl(std::string const& name);
    void measure_add_impl(std::string const& name, int64_t value, measurement_t type);

    std::shared_ptr<metrics_tree> storage_{new metrics_tree};
#ifdef MAPNIK_THREADSAFE
    std::shared_ptr<std::mutex> lock_ {new std::mutex};
#endif
};

} //namespace mapnik

#endif /* MAPNIK_METRICS */

#endif /* MAPNIK_METRICS_HPP */
