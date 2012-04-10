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
    namespace logger {

        class MAPNIK_DECL severity :
            public singleton<severity,CreateStatic>,
            private boost::noncopyable
        {
        public:
            enum type
            {
                info,
                debug,
                warn,
                error,
                fatal,
                none
            };

            typedef boost::unordered_map<std::string, type> severity_map;

            // globally get security level
            static type get()
            {
                return severity_level_;
            }

            // globally set security level
            static void set(const type& severity_level)
            {
#ifdef MAPNIK_THREADSAFE
                boost::mutex::scoped_lock lock(mutex_);
#endif

                severity_level_ = severity_level;
            }

            // per object get security level
            static type get_object(const std::string& object_name)
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

            // per object set security level
            static void set_object(const std::string& object_name,
                                   const type& security_level)
            {
#ifdef MAPNIK_THREADSAFE
                boost::mutex::scoped_lock lock(mutex_);
#endif
                if (! object_name.empty())
                {
                    object_severity_level_[object_name] = security_level;
                }
            }

        private:
            static type severity_level_;
            static severity_map object_severity_level_;

#ifdef MAPNIK_THREADSAFE
            static boost::mutex mutex_;
#endif
        };


        class MAPNIK_DECL format :
            public singleton<format,CreateStatic>,
            private boost::noncopyable
        {
        public:

            static std::string get()
            {
                return format_;
            }

            static void set(const std::string& format)
            {
#ifdef MAPNIK_THREADSAFE
                boost::mutex::scoped_lock lock(mutex_);
#endif
                format_ = format;
            }

            static std::string str();

        private:
            static std::string format_;

#ifdef MAPNIK_THREADSAFE
            static boost::mutex mutex_;
#endif
        };


        class MAPNIK_DECL output :
            public singleton<output,CreateStatic>,
            private boost::noncopyable
        {
        public:
            static void use_file(const std::string& filepath);
            static void use_console();

        private:
            static std::ofstream file_output_;
            static std::string file_name_;
            static std::streambuf* saved_buf_;
        };


        template<class Ch, class Tr, class A>
        class clog_sink
        {
        public:
            typedef std::basic_ostringstream<Ch, Tr, A> stream_buffer;

            void operator()(const stream_buffer &s)
            {
#ifdef MAPNIK_THREADSAFE
                static boost::mutex mutex;
                boost::mutex::scoped_lock lock(mutex);
#endif
                std::clog << format::str() << " " << s.str() << std::endl;
            }
        };


        template<template <class Ch, class Tr, class A> class OutputPolicy,
                 severity::type Severity,
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
                    output_policy()(streambuf_);
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
                return Severity >= severity::get_object(object_name_);
            }

            typename output_policy::stream_buffer streambuf_;
            std::string object_name_;
#endif
        };

        typedef base_log<clog_sink, severity::info> base_log_info;
        typedef base_log<clog_sink, severity::debug> base_log_debug;
        typedef base_log<clog_sink, severity::warn> base_log_warn;
        typedef base_log<clog_sink, severity::error> base_log_error;
        typedef base_log<clog_sink, severity::fatal> base_log_fatal;
    }


    class MAPNIK_DECL info : public logger::base_log_info {
    public:
        info() : logger::base_log_info() {}
        info(const char* object_name) : logger::base_log_info(object_name) {}
    };

    class MAPNIK_DECL debug : public logger::base_log_debug {
    public:
        debug() : logger::base_log_debug() {}
        debug(const char* object_name) : logger::base_log_debug(object_name) {}
    };

    class MAPNIK_DECL warn : public logger::base_log_warn {
    public:
        warn() : logger::base_log_warn() {}
        warn(const char* object_name) : logger::base_log_warn(object_name) {}
    };

    class MAPNIK_DECL error : public logger::base_log_error {
    public:
        error() : logger::base_log_error() {}
        error(const char* object_name) : logger::base_log_error(object_name) {}
    };

    class MAPNIK_DECL fatal : public logger::base_log_fatal {
    public:
        fatal() : logger::base_log_fatal() {}
        fatal(const char* object_name) : logger::base_log_fatal(object_name) {}
    };


    #define MAPNIK_LOG_INFO(s) mapnik::info(#s)
    #define MAPNIK_LOG_DEBUG(s) mapnik::debug(#s)
    #define MAPNIK_LOG_WARN(s) mapnik::warn(#s)
    #define MAPNIK_LOG_ERROR(s) mapnik::error(#s)
    #define MAPNIK_LOG_FATAL(s) mapnik::fatal(#s)
}

#endif // MAPNIK_DEBUG_HPP
