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

#ifndef MAPNIK_DEBUG_HPP
#define MAPNIK_DEBUG_HPP

// mapnik (should not depend on anything that need to use this)
#include <mapnik/config.hpp>
#include <mapnik/utils.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// std
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <string>


namespace mapnik {

    /*
        Global logger class that holds the configuration of severity, format
        and file/console redirection.
    */
    class MAPNIK_DECL logger :
        public singleton<logger,CreateStatic>,
        private boost::noncopyable
    {
    public:
        enum severity_type
        {
            debug = 0,
            warn = 1,
            error = 2,
            none = 3
        };

        typedef boost::unordered_map<std::string, severity_type> severity_map;

        // global security level
        static severity_type get_severity()
        {
            return severity_level_;
        }

        static void set_severity(const severity_type& severity_level)
        {
#ifdef MAPNIK_THREADSAFE
            boost::mutex::scoped_lock lock(severity_mutex_);
#endif

            severity_level_ = severity_level;
        }

        // per object security levels
        static severity_type get_object_severity(std::string const& object_name)
        {
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

        static void set_object_severity(std::string const& object_name,
                                        const severity_type& security_level)
        {
#ifdef MAPNIK_THREADSAFE
            boost::mutex::scoped_lock lock(severity_mutex_);
#endif
            if (! object_name.empty())
            {
                object_severity_level_[object_name] = security_level;
            }
        }

        static void clear_object_severity()
        {
#ifdef MAPNIK_THREADSAFE
            boost::mutex::scoped_lock lock(severity_mutex_);
#endif

            object_severity_level_.clear();
        }

        // format
        static std::string get_format()
        {
            return format_;
        }

        static void set_format(std::string const& format)
        {
#ifdef MAPNIK_THREADSAFE
            boost::mutex::scoped_lock lock(format_mutex_);
#endif
            format_ = format;
        }

        // interpolate the format string for output
        static std::string str();

        // output
        static void use_file(std::string const& filepath);
        static void use_console();

    private:
        static severity_type severity_level_;
        static severity_map object_severity_level_;
        static bool severity_env_check_;

        static std::string format_;
        static bool format_env_check_;

        static std::ofstream file_output_;
        static std::string file_name_;
        static std::streambuf* saved_buf_;

#ifdef MAPNIK_THREADSAFE
        static boost::mutex severity_mutex_;
        static boost::mutex format_mutex_;
#endif
    };


    namespace detail {

        /*
          Default sink, it regulates access to clog
        */
        template<class Ch, class Tr, class A>
        class clog_sink
        {
        public:
            typedef std::basic_ostringstream<Ch, Tr, A> stream_buffer;

            void operator()(const logger::severity_type& severity, const stream_buffer &s)
            {
#ifdef MAPNIK_THREADSAFE
                static boost::mutex mutex;
                boost::mutex::scoped_lock lock(mutex);
#endif
                std::clog << logger::str() << " " << s.str() << std::endl;
            }
        };


        /*
          Base log class, should not log anything when no MAPNIK_LOG is defined

          This is used for debug/warn reporting that should not output
          anything when not compiling for speed.
        */
        template<template <class Ch, class Tr, class A> class OutputPolicy,
                 logger::severity_type Severity,
                 class Ch = char,
                 class Tr = std::char_traits<Ch>,
                 class A = std::allocator<Ch> >
        class base_log : public boost::noncopyable
        {
        public:
            typedef OutputPolicy<Ch, Tr, A> output_policy;

            base_log() {}

            base_log(const char* object_name)
            {
#ifdef MAPNIK_LOG
                if (object_name != NULL)
                {
                    object_name_ = object_name;
                }
#endif
            }

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
            base_log &operator<<(const T &x)
            {
#ifdef MAPNIK_LOG
                streambuf_ << x;
#endif
                return *this;
            }

        private:
#ifdef MAPNIK_LOG
            inline bool check_severity()
            {
                return Severity >= logger::get_object_severity(object_name_);
            }

            typename output_policy::stream_buffer streambuf_;
            std::string object_name_;
#endif
        };


        /*
          Base log class that always log, regardless of MAPNIK_LOG.

          This is used for error reporting that should always log something
        */
        template<template <class Ch, class Tr, class A> class OutputPolicy,
                 logger::severity_type Severity,
                 class Ch = char,
                 class Tr = std::char_traits<Ch>,
                 class A = std::allocator<Ch> >
        class base_log_always : public boost::noncopyable
        {
        public:
            typedef OutputPolicy<Ch, Tr, A> output_policy;

            base_log_always() {}

            base_log_always(const char* object_name)
            {
                if (object_name != NULL)
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
            base_log_always &operator<<(const T &x)
            {
                streambuf_ << x;
                return *this;
            }

        private:
            inline bool check_severity()
            {
                return Severity >= logger::get_object_severity(object_name_);
            }

            typename output_policy::stream_buffer streambuf_;
            std::string object_name_;
        };


        typedef base_log<clog_sink, logger::debug> base_log_debug;
        typedef base_log<clog_sink, logger::warn> base_log_warn;
        typedef base_log_always<clog_sink, logger::error> base_log_error;

    } // namespace detail


    // real interfaces
    class MAPNIK_DECL warn : public detail::base_log_warn {
    public:
        warn() : detail::base_log_warn() {}
        warn(const char* object_name) : detail::base_log_warn(object_name) {}
    };

    class MAPNIK_DECL debug : public detail::base_log_debug {
    public:
        debug() : detail::base_log_debug() {}
        debug(const char* object_name) : detail::base_log_debug(object_name) {}
    };

    class MAPNIK_DECL error : public detail::base_log_error {
    public:
        error() : detail::base_log_error() {}
        error(const char* object_name) : detail::base_log_error(object_name) {}
    };

    // logging helpers
    #define MAPNIK_LOG_DEBUG(s) mapnik::debug(#s)
    #define MAPNIK_LOG_WARN(s) mapnik::warn(#s)
    #define MAPNIK_LOG_ERROR(s) mapnik::error(#s)
}

#endif // MAPNIK_DEBUG_HPP
