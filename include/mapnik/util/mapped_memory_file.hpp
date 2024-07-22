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

#ifndef MAPNIK_MEMORY_MAPPED_FILE_HPP
#define MAPNIK_MEMORY_MAPPED_FILE_HPP

#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <mapnik/mapped_memory_cache.hpp>
#else
#include <fstream>
#endif

#include <string>

namespace mapnik {
namespace util {

/**
 * @brief memory mapped file abstraction. Implementation depends on MAPNIK_MEMORY_MAPPED_FILE.
 * Might be a simple file wrapper, if MAPNIK_MEMORY_MAPPED_FILE=0
 *
 */
class MAPNIK_DECL mapped_memory_file : public noncopyable
{
  public:
#ifdef MAPNIK_MEMORY_MAPPED_FILE
    using file_source_type = boost::interprocess::ibufferstream;
#else
    using file_source_type = std::ifstream;
#endif

  public:
    mapped_memory_file();
    explicit mapped_memory_file(std::string const& file_name);
    virtual ~mapped_memory_file();

    file_source_type& file();
    bool is_open() const;
    void skip(std::streampos bytes);

    /**
     * @brief deletes the file identified by file_name. Might also remove the file from any caches.
     */
    static void deleteFile(std::string const& file_name);

  protected:
    const std::string file_name_;
#ifdef MAPNIK_MEMORY_MAPPED_FILE
    mapnik::mapped_region_ptr mapped_region_;
#endif
    file_source_type file_;
};

} // namespace util
} // namespace mapnik

#endif
