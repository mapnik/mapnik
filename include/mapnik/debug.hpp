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

// mapnik
#include <mapnik/global.hpp>

// boost
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

#ifdef MAPNIK_DEBUG
#define MAPNIK_DEBUG_AS_BOOL true
#else
#define MAPNIK_DEBUG_AS_BOOL false
#endif

#ifndef MAPNIK_LOG_FORMAT
#error Must run configure again to regenerate the correct log format string. See LOG_FORMAT_STRING scons option.
#endif

namespace mapnik {

    namespace logger {

#define __xstr__(s) __str__(s)
#define __str__(s) #s

        static inline std::string format_logger() {
            char buf[256];
            const time_t tm = time(0);
            strftime(buf, sizeof(buf), __xstr__(MAPNIK_LOG_FORMAT), localtime(&tm));
            return buf;
        }

#undef __xstr__
#undef __str__

        template<class Ch, class Tr, class A>
        class no_output {
        private:
            struct null_buffer {
                template<class T>
                null_buffer &operator<<(const T &) {
                    return *this;
                }
            };
        public:
            typedef null_buffer stream_buffer;

        public:
            void operator()(const stream_buffer &) {
            }
        };

        template<class Ch, class Tr, class A>
        class output_to_clog {
        public:
            typedef std::basic_ostringstream<Ch, Tr, A> stream_buffer;
        public:
            void operator()(const stream_buffer &s) {
#ifdef MAPNIK_THREADSAFE
                static boost::mutex mutex;
                boost::mutex::scoped_lock lock(mutex);
#endif
                std::clog << format_logger() << " " << s.str() << std::endl;
            }
        };

        template<template <class Ch, class Tr, class A> class OutputPolicy,
                 class Ch = char,
                 class Tr = std::char_traits<Ch>,
                 class A = std::allocator<Ch> >
        class base_log {
            typedef OutputPolicy<Ch, Tr, A> output_policy;
        public:
            ~base_log() {
                output_policy()(streambuf_);
            }
        public:
            template<class T>
            base_log &operator<<(const T &x) {
                streambuf_ << x;
                return *this;
            }
        private:
            typename output_policy::stream_buffer streambuf_;
        };
    }

    class log : public logger::base_log<logger::output_to_clog>
    {
    };

}

#endif
