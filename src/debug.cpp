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

namespace mapnik {

#ifdef MAPNIK_THREADSAFE
boost::mutex logger::severity_mutex_;
boost::mutex logger::format_mutex_;
#endif


// severity

logger::severity_type logger::severity_level_ =
#ifdef MAPNIK_DEBUG
    logger::debug
#else
    logger::error
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
    char buf[256];
    const time_t tm = time(0);
    strftime(buf, sizeof(buf), logger::format_.c_str(), localtime(&tm));
    return buf;
}


// output

std::ofstream logger::file_output_;
std::string logger::file_name_;
std::streambuf* logger::saved_buf_ = 0;

void logger::use_file(const std::string& filepath)
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

    std::clog.rdbuf(saved_buf_);
}

}
