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

#ifdef MAPNIK_DEBUG
#define MAPNIK_DEBUG_AS_BOOL true
#else
#define MAPNIK_DEBUG_AS_BOOL false
#endif

#ifndef MAPNIK_LOG_FORMAT
#define MAPNIK_LOG_FORMAT "Mapnik LOG> %Y-%m-%d %H:%M:%S:"
#endif

// mapnik (should not depend on anything else)
#include <mapnik/config.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// std
#include <iostream>
#include <cassert>
#include <sstream>
#include <ctime>
#include <ostream>
#include <fstream>
#include <string>


namespace mapnik {
    namespace logger {

        class severity
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


#define __xstr__(s) __str__(s)
#define __str__(s) #s

        static inline std::string format_logger()
        {
            char buf[256];
            const time_t tm = time(0);
            strftime(buf, sizeof(buf), __xstr__(MAPNIK_LOG_FORMAT), localtime(&tm));
            return buf;
        }

#undef __xstr__
#undef __str__


#if 0
        template<class Ch, class Tr, class A>
        class no_output
        {
        private:
            struct null_buffer
            {
                template<class T>
                null_buffer &operator<<(const T &)
                {
                    return *this;
                }
            };

            typedef null_buffer stream_buffer;

            void operator()(const stream_buffer &)
            {
            }
        };
#endif


        template<class Ch, class Tr, class A>
        class output_to_clog
        {
        public:
            typedef std::basic_ostringstream<Ch, Tr, A> stream_buffer;

            void operator()(const stream_buffer &s)
            {
#ifdef MAPNIK_THREADSAFE
                static boost::mutex mutex;
                boost::mutex::scoped_lock lock(mutex);
#endif
                std::clog << format_logger() << " " << s.str() << std::endl;
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
                if (object_name != NULL)
                {
                    object_name_ = object_name;
                }
            }

            ~base_log()
            {
                if (check_severity())
                {
                    output_policy()(streambuf_);
                }
            }

            template<class T>
            base_log &operator<<(const T &x)
            {
                streambuf_ << x;
                return *this;
            }

        private:
            inline bool check_severity()
            {
                return Severity >= severity::get_object(object_name_);
            }

            typename output_policy::stream_buffer streambuf_;
            std::string object_name_;
        };
    }


    class MAPNIK_DECL log : public logger::base_log<logger::output_to_clog,
            logger::severity::debug> {
    public:
        typedef logger::base_log<logger::output_to_clog, logger::severity::debug> base_class;
        log(const char* object_name) : base_class(object_name) {}
        log() : base_class() {}
    };

    class MAPNIK_DECL info : public logger::base_log<logger::output_to_clog,
            logger::severity::info> {
    public:
        typedef logger::base_log<logger::output_to_clog, logger::severity::info> base_class;
        info(const char* object_name) : base_class(object_name) {}
        info() : base_class() {}
    };

    class MAPNIK_DECL debug : public logger::base_log<logger::output_to_clog,
            logger::severity::debug> {
    public:
        typedef logger::base_log<logger::output_to_clog, logger::severity::debug> base_class;
        debug(const char* object_name) : base_class(object_name) {}
        debug() : base_class() {}
    };

    class MAPNIK_DECL warn : public logger::base_log<logger::output_to_clog,
            logger::severity::warn> {
    public:
        typedef logger::base_log<logger::output_to_clog, logger::severity::warn> base_class;
        warn(const char* object_name) : base_class(object_name) {}
        warn() : base_class() {}
    };

    class MAPNIK_DECL error : public logger::base_log<logger::output_to_clog,
            logger::severity::error> {
    public:
        typedef logger::base_log<logger::output_to_clog, logger::severity::error> base_class;
        error(const char* object_name) : base_class(object_name) {}
        error() : base_class() {}
    };

    class MAPNIK_DECL fatal : public logger::base_log<logger::output_to_clog,
            logger::severity::fatal> {
    public:
        typedef logger::base_log<logger::output_to_clog, logger::severity::fatal> base_class;
        fatal(const char* object_name) : base_class(object_name) {}
        fatal() : base_class() {}
    };


    #define MAPNIK_LOG_INFO(s) mapnik::info(#s)
    #define MAPNIK_LOG_DEBUG(s) mapnik::debug(#s)
    #define MAPNIK_LOG_WARN(s) mapnik::warn(#s)
    #define MAPNIK_LOG_ERROR(s) mapnik::error(#s)
    #define MAPNIK_LOG_FATAL(s) mapnik::fatal(#s)

}

#endif // MAPNIK_DEBUG_HPP
