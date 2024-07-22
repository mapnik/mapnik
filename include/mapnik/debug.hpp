/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_DEBUG_HPP
#define MAPNIK_DEBUG_HPP

// mapnik (should not depend on anything that need to use this)
#include <mapnik/config.hpp>
#include <mapnik/util/singleton.hpp>
#include <mapnik/util/noncopyable.hpp>

// std
#include <iostream>
#include <sstream>
#include <ostream>
#include <iosfwd>
#include <string>
#include <unordered_map>
#ifdef MAPNIK_THREADSAFE
#include <mutex>
#include <atomic>
#endif

namespace mapnik {

// Global logger class that holds the configuration of severity, format
// and file/console redirection.

class MAPNIK_DECL logger : public singleton<logger, CreateStatic>,
                           private util::noncopyable
{
  public:
    enum severity_type { debug = 0, warn = 1, error = 2, none = 3 };

    using severity_map = std::unordered_map<std::string, severity_type>;

    // global security level
    static severity_type get_severity() { return severity_level_; }

    static void set_severity(severity_type severity_level) { severity_level_ = severity_level; }

    // per object security levels
    static severity_type get_object_severity(std::string const& object_name)
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(severity_mutex_);
#endif
        severity_map::iterator it = object_severity_level_.find(object_name);
        if (object_name.empty() || it == object_severity_level_.end())
        {
            return severity_level_;
        }
        else
        {
            return it->second;
        }
    }

    static void set_object_severity(std::string const& object_name, severity_type const& security_level)
    {
        if (!object_name.empty())
        {
#ifdef MAPNIK_THREADSAFE
            std::lock_guard<std::mutex> lock(severity_mutex_);
#endif
            object_severity_level_[object_name] = security_level;
        }
    }

    static void clear_object_severity()
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(severity_mutex_);
#endif

        object_severity_level_.clear();
    }

    // format
    static std::string const& get_format() { return format_; }

    static void set_format(std::string const& format)
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(format_mutex_);
#endif
        format_ = format;
    }

    // interpolate the format string for output
    static std::string str();

    // output
    static void use_file(std::string const& filepath);
    static void use_console();

  private:
    static severity_map object_severity_level_;
    static std::string format_;
    static std::ofstream file_output_;
    static std::string file_name_;
    static std::streambuf* saved_buf_;

#ifdef MAPNIK_THREADSAFE
    static std::atomic<severity_type> severity_level_;
    static std::atomic<bool> severity_env_check_;
    static std::atomic<bool> format_env_check_;
    static std::mutex severity_mutex_;
    static std::mutex format_mutex_;
#else
    static severity_type severity_level_;
    static bool severity_env_check_;
    static bool format_env_check_;
#endif
};

namespace detail {

// Default sink, it regulates access to clog

template<class Ch, class Tr, class A>
class clog_sink
{
  public:
    using stream_buffer = std::basic_ostringstream<Ch, Tr, A>;

    void operator()(logger::severity_type const& /*severity*/, stream_buffer const& s)
    {
#ifdef MAPNIK_THREADSAFE
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
#endif
        std::clog << logger::str() << " " << s.str() << std::endl;
    }
};

// Base log class, should not log anything when no MAPNIK_LOG is defined
//
// This is used for debug/warn reporting that should not output
// anything when not compiling for speed.

template<template<class Ch, class Tr, class A> class OutputPolicy,
         logger::severity_type Severity,
         class Ch = char,
         class Tr = std::char_traits<Ch>,
         class A = std::allocator<Ch>>
class base_log : public util::noncopyable
{
  public:
    using output_policy = OutputPolicy<Ch, Tr, A>;

    base_log() {}

#ifdef MAPNIK_LOG
    base_log(const char* object_name)
    {
        if (object_name != nullptr)
        {
            object_name_ = object_name;
        }
    }
#else
    base_log(const char* /*object_name*/) {}
#endif

    ~base_log()
    {
#ifdef MAPNIK_LOG
        if (check_severity())
        {
            output_policy()(Severity, streambuf_);
        }
#endif
    }

    template<class T>
#ifdef MAPNIK_LOG
    base_log& operator<<(T const& x)
    {
        streambuf_ << x;
        return *this;
    }
#else
    base_log& operator<<(T const& /*x*/)
    {
        return *this;
    }
#endif

  private:
#ifdef MAPNIK_LOG
    inline bool check_severity() { return Severity >= logger::get_object_severity(object_name_); }

    typename output_policy::stream_buffer streambuf_;
    std::string object_name_;
#endif
};

// Base log class that always log, regardless of MAPNIK_LOG.
// This is used for error reporting that should always log something

template<template<class Ch, class Tr, class A> class OutputPolicy,
         logger::severity_type Severity,
         class Ch = char,
         class Tr = std::char_traits<Ch>,
         class A = std::allocator<Ch>>
class base_log_always : public util::noncopyable
{
  public:
    using output_policy = OutputPolicy<Ch, Tr, A>;

    base_log_always() {}

    base_log_always(const char* object_name)
    {
        if (object_name != nullptr)
        {
            object_name_ = object_name;
        }
    }

    ~base_log_always()
    {
        if (check_severity())
        {
            output_policy()(Severity, streambuf_);
        }
    }

    template<class T>
    base_log_always& operator<<(T const& x)
    {
        streambuf_ << x;
        return *this;
    }

  private:
    inline bool check_severity() { return Severity >= logger::get_object_severity(object_name_); }

    typename output_policy::stream_buffer streambuf_;
    std::string object_name_;
};

using base_log_debug = base_log<clog_sink, logger::debug>;
using base_log_warn = base_log<clog_sink, logger::warn>;
using base_log_error = base_log_always<clog_sink, logger::error>;

} // namespace detail

// real interfaces
class MAPNIK_DECL warn : public detail::base_log_warn
{
  public:
    warn()
        : detail::base_log_warn()
    {}
    warn(const char* object_name)
        : detail::base_log_warn(object_name)
    {}
};

class MAPNIK_DECL debug : public detail::base_log_debug
{
  public:
    debug()
        : detail::base_log_debug()
    {}
    debug(const char* object_name)
        : detail::base_log_debug(object_name)
    {}
};

class MAPNIK_DECL error : public detail::base_log_error
{
  public:
    error()
        : detail::base_log_error()
    {}
    error(const char* object_name)
        : detail::base_log_error(object_name)
    {}
};

// logging helpers
#define MAPNIK_LOG_DEBUG(s) mapnik::debug(#s)
#define MAPNIK_LOG_WARN(s)  mapnik::warn(#s)
#define MAPNIK_LOG_ERROR(s) mapnik::error(#s)
} // namespace mapnik

#endif // MAPNIK_DEBUG_HPP
