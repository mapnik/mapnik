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

#ifdef MAPNIK_METRICS

#include <mapnik/metrics.hpp>

#include <mapnik/make_unique.hpp>

#include <boost/next_prior.hpp>
#include <sstream>
#include <utility>

namespace mapnik {

const std::string measurement_str[TOTAL_ENUM_SIZE] =
{
    "UNKNOWN",
    "Time (us)",
    "Value",
    "Calls"
};

measurement::measurement(int64_t value, measurement_t type)
    : value_(value),
      type_(type)
{
}

autochrono::autochrono(metrics* m, std::string name)
    : metrics_(m),
      name_(std::move(name))
{
    start_ = clock_.now();
}

autochrono::~autochrono()
{
    auto ns = std::chrono::duration_cast<time_units>(clock_.now() - start_);
    metrics_->measure_add(name_, ns.count(), measurement_t::TIME_MICROSECONDS);
}

metrics::metrics(bool enabled)
    : enabled_(enabled)
{
}

metrics::metrics(metrics const &m, std::string prefix)
    : enabled_(m.enabled_),
      prefix_(m.prefix_ + prefix),
      storage_(m.storage_)
#ifdef MAPNIK_THREADSAFE
     ,lock_(m.lock_)
#endif
{
}

/* Move constructor */
metrics::metrics(metrics const &&m)
{
    *this = m;
}

/* Copy assignment operator */
metrics& metrics::operator=(metrics const& m)
{
    enabled_ = m.enabled_;
    prefix_ = m.prefix_;
    storage_ = m.storage_;
    lock_ = m.lock_;
    return *this;
}

/* Move assignment operator */
metrics& metrics::operator=(metrics&& m)
{
    return *this = m;
}

std::unique_ptr<autochrono> metrics::measure_time_impl(const std::string& name)
{
    return std::make_unique<autochrono>(this, name);
}

void metrics::measure_add_impl(std::string const& name, int64_t value, measurement_t type)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(*this->lock_);
#endif
    auto child = storage_->get_child_optional(prefix_ + name);
    if (!child)
    {
        storage_->put(prefix_ + name, measurement(value, type));
    }
    else
    {
        auto &measure = child->data();
        measure.value_ += value;
        if (measure.type_ == measurement_t::UNASSIGNED)
        {
            measure.type_ = type;
        }
        else
        {
            measure.calls_++;
        }
    }
}


boost::optional<measurement &> metrics::find(std::string const& name, bool ignore_prefix)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(*this->lock_);
#endif
    auto m = storage_->get_child_optional(ignore_prefix ? name : prefix_ + name);
    return (m ? m->data() : boost::optional<measurement &>());
}

/**
 * Recursively generate a JSON string from a metrics tree.
 * It doesn't use boost::property_tree::json_parser because we can have
 * nodes with data and children
 * @param buf
 * @param t
 */
void to_string_helper(std::ostringstream &buf, metrics::metrics_tree t)
{
    measurement &m = t.data();
    if (t.empty() && m.type_ == measurement_t::VALUE)
    {
        buf << m.value_;
        return;
    }

    buf << "{";
    if (m.type_ != measurement_t::UNASSIGNED)
    {
        buf << R"(")" << measurement_str[m.type_] << R"(":)" << m.value_;
        if (m.type_ == measurement_t::TIME_MICROSECONDS)
        {
            buf << R"(,")" << measurement_str[measurement_t::CALLS];
            buf << R"(":)" << m.calls_;
        }
        if (!t.empty())
        {
            buf << ",";
        }
    }

    for (auto it = t.begin(); it != t.end(); it++)
    {
        buf << R"(")" << it->first << R"(":)";
        to_string_helper(buf, it->second);
        if (boost::next(it) != t.end())
        {
            buf << ",";
        }
    }
    buf << "}";
}

std::string metrics::to_string()
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(*this->lock_);
#endif
    std::ostringstream buf;

    to_string_helper(buf, *storage_);
    return buf.str();
}

} //namespace mapnik

#endif //MAPNIK_METRICS