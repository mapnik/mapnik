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

// mapnik
#include <mapnik/debug.hpp>

// stl
#include <ctime>

#ifndef MAPNIK_LOG_FORMAT
#define MAPNIK_LOG_FORMAT "Mapnik LOG> %Y-%m-%d %H:%M:%S:"
#endif

#ifndef MAPNIK_DEFAULT_LOG_SEVERITY
  #ifdef MAPNIK_DEBUG
    #define MAPNIK_DEFAULT_LOG_SEVERITY 1
  #else
    #define MAPNIK_DEFAULT_LOG_SEVERITY 3
  #endif
#endif

namespace mapnik {

// mutexes

#ifdef MAPNIK_THREADSAFE
boost::mutex logger::severity_mutex_;
boost::mutex logger::format_mutex_;
#endif


// first time checks

bool logger::severity_env_check_ = true;
bool logger::format_env_check_ = true;


// severity

logger::severity_type logger::severity_level_ =
    #if MAPNIK_DEFAULT_LOG_SEVERITY == 0
        logger::debug
    #elif MAPNIK_DEFAULT_LOG_SEVERITY == 1
        logger::info
    #elif MAPNIK_DEFAULT_LOG_SEVERITY == 2
        logger::warn
    #elif MAPNIK_DEFAULT_LOG_SEVERITY == 3
        logger::error
    #elif MAPNIK_DEFAULT_LOG_SEVERITY == 4
        logger::fatal
    #elif MAPNIK_DEFAULT_LOG_SEVERITY == 5
        logger::none
    #else
        #error "Wrong default log severity level specified!"
    #endif
;

logger::severity_map logger::object_severity_level_ = logger::severity_map();


// format

#define __xstr__(s) __str__(s)
#define __str__(s) #s
std::string logger::format_ = __xstr__(MAPNIK_LOG_FORMAT);
#undef __xstr__
#undef __str__

std::string logger::str()
{
#if 0
    // update the format from getenv if this is the first time
    if (logger::format_env_check_)
    {
        logger::format_env_check_ = false;

        const char* log_format = getenv("MAPNIK_LOG_FORMAT");
        if (log_format != NULL)
        {
            logger::format_ = log_format;
        }
    }
#endif

    char buf[256];
    const time_t tm = time(0);
    strftime(buf, sizeof(buf), logger::format_.c_str(), localtime(&tm));
    return buf;
}


// output

std::ofstream logger::file_output_;
std::string logger::file_name_;
std::streambuf* logger::saved_buf_ = 0;

void logger::use_file(std::string const& filepath)
{
    // save clog rdbuf
    if (saved_buf_ == 0)
    {
        saved_buf_ = std::clog.rdbuf();
    }

    // use a file to output as clog rdbuf
    if (file_name_ != filepath)
    {
        file_name_ = filepath;

        if (file_output_.is_open())
        {
            file_output_.close();
        }

        file_output_.open(file_name_.c_str(), std::ios::out | std::ios::app);
        if (file_output_)
        {
            std::clog.rdbuf(file_output_.rdbuf());
        }
        else
        {
            std::stringstream s;
            s << "cannot redirect log to file " << file_output_;
            throw std::runtime_error(s.str());
        }
    }
}

void logger::use_console()
{
    // save clog rdbuf
    if (saved_buf_ == 0)
    {
        saved_buf_ = std::clog.rdbuf();
    }

    // close the file to force a flush
    if (file_output_.is_open())
    {
        file_output_.close();
    }

    std::clog.rdbuf(saved_buf_);
}


} // namespace mapnik
