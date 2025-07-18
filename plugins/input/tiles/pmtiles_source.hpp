/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_PMTILES_SOURCE_HPP
#define MAPNIK_PMTILES_SOURCE_HPP

#include <mapnik/global.hpp>
#include "tiles_source.hpp"
// stl
#include <iostream>
#include <tuple>
#include <fstream>

// boost
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
// mapnik_vector_tile
#include "vector_tile_compression.hpp"

namespace mapnik {

enum class compression_type : std::uint8_t { UNKNOWN = 0x0, NONE = 0x1, GZIP = 0x2, BROTLI = 0x3, ZSTD = 0x4 };

enum class tile_type : std::uint8_t { UNKNOWN = 0x00, MVT = 0x01, PNG = 0x02, JPEG = 0x03, WEBP = 0x04, AVIF = 0x05 };

struct entryv3
{
    uint64_t tile_id;
    uint64_t offset;
    uint32_t length;
    uint32_t run_length;

    entryv3()
        : tile_id(0),
          offset(0),
          length(0),
          run_length(0)
    {}

    entryv3(uint64_t _tile_id, uint64_t _offset, uint32_t _length, uint32_t _run_length)
        : tile_id(_tile_id),
          offset(_offset),
          length(_length),
          run_length(_run_length)
    {}
};
namespace {
struct varint_too_long_exception : std::exception
{
    char const* what() const noexcept override { return "varint too long exception"; }
};

struct end_of_buffer_exception : std::exception
{
    char const* what() const noexcept override { return "end of buffer exception"; }
};
struct malformed_directory_exception : std::exception
{
    char const* what() const noexcept override { return "malformed directory exception"; }
};

constexpr int8_t const max_varint_length = sizeof(uint64_t) * 8 / 7 + 1;
// from https://github.com/mapbox/protozero/blob/master/include/protozero/varint.hpp
uint64_t decode_varint_impl(char const** data, char const* end)
{
    auto const* begin = reinterpret_cast<int8_t const*>(*data);
    auto const* iend = reinterpret_cast<int8_t const*>(end);
    int8_t const* p = begin;
    uint64_t val = 0;
    if (iend - begin >= max_varint_length)
    { // fast path
        do
        {
            int64_t b = *p++;
            val = ((uint64_t(b) & 0x7fU));
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 7U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 14U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 21U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 28U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 35U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 42U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 49U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 56U);
            if (b >= 0)
            {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x01U) << 63U);
            if (b >= 0)
            {
                break;
            }
            throw varint_too_long_exception{};
        } while (false);
    }
    else
    {
        unsigned int shift = 0;
        while (p != iend && *p < 0)
        {
            val |= (uint64_t(*p++) & 0x7fU) << shift;
            shift += 7;
        }
        if (p == iend)
        {
            throw end_of_buffer_exception{};
        }
        val |= uint64_t(*p++) << shift;
    }

    *data = reinterpret_cast<char const*>(p);
    return val;
}

uint64_t decode_varint(char const** data, char const* end)
{
    // If this is a one-byte varint, decode it here.
    if (end != *data && ((static_cast<uint64_t>(**data) & 0x80U) == 0))
    {
        auto const val = static_cast<uint64_t>(**data);
        ++(*data);
        return val;
    }
    // If this varint is more than one byte, defer to complete implementation.
    return decode_varint_impl(data, end);
}

inline std::vector<entryv3> deserialize_directory(std::string const& decompressed)
{
    char const* t = decompressed.data();
    char const* end = t + decompressed.size();

    uint64_t const num_entries_64bit = decode_varint(&t, end);
    // Sanity check to avoid excessive memory allocation attempt:
    // each directory entry takes at least 4 bytes
    if (num_entries_64bit / 4U > decompressed.size())
    {
        throw malformed_directory_exception();
    }
    size_t const num_entries = static_cast<size_t>(num_entries_64bit);
    // std::cerr << "Decompressed size" << decompressed.size() << std::endl;
    std::vector<entryv3> result;
    result.resize(num_entries);

    uint64_t last_id = 0;
    for (std::size_t i = 0; i < num_entries; ++i)
    {
        uint64_t const val = decode_varint(&t, end);
        if (val > std::numeric_limits<uint64_t>::max() - last_id)
        {
            throw malformed_directory_exception();
        }
        uint64_t const tile_id = last_id + val;
        result[i].tile_id = tile_id;
        last_id = tile_id;
    }

    for (std::size_t i = 0; i < num_entries; ++i)
    {
        uint64_t const val = decode_varint(&t, end);
        if (val > std::numeric_limits<uint32_t>::max())
        {
            throw malformed_directory_exception();
        }
        result[i].run_length = static_cast<uint32_t>(val);
    }

    for (size_t i = 0; i < num_entries; i++)
    {
        uint64_t const val = decode_varint(&t, end);
        if (val > std::numeric_limits<uint32_t>::max())
        {
            throw malformed_directory_exception();
        }
        result[i].length = static_cast<uint32_t>(val);
    }

    for (size_t i = 0; i < num_entries; i++)
    {
        uint64_t tmp = decode_varint(&t, end);

        if (i > 0 && tmp == 0)
        {
            if (result[i - 1].offset > std::numeric_limits<uint64_t>::max() - result[i - 1].length)
            {
                throw malformed_directory_exception();
            }
            result[i].offset = result[i - 1].offset + result[i - 1].length;
        }
        else
        {
            result[i].offset = tmp - 1;
        }
    }

    // assert the directory has been fully consumed
    if (t != end)
    {
        throw malformed_directory_exception();
    }
    return result;
}

// use a 0 length entry as a null value.
entryv3 find_tile(std::vector<entryv3> const& entries, uint64_t tile_id)
{
    int m = 0;
    int n = static_cast<int>(entries.size()) - 1;
    while (m <= n)
    {
        int k = (n + m) >> 1;
        if (tile_id > entries[k].tile_id)
        {
            m = k + 1;
        }
        else if (tile_id < entries[k].tile_id)
        {
            n = k - 1;
        }
        else
        {
            return entries[k];
        }
    }

    if (n >= 0)
    {
        if (entries[n].run_length == 0)
        {
            return entries[n];
        }
        if (tile_id - entries[n].tile_id < entries[n].run_length)
        {
            return entries[n];
        }
    }

    return entryv3{0, 0, 0, 0};
}
void rotate(int64_t n, uint32_t& x, uint32_t& y, uint32_t rx, uint32_t ry)
{
    if (ry == 0)
    {
        if (rx != 0)
        {
            x = n - 1 - x;
            y = n - 1 - y;
        }
        uint32_t t = x;
        x = y;
        y = t;
    }
}

inline uint64_t zxy_to_tileid(uint8_t z, uint32_t x, uint32_t y)
{
    if (z > 31)
    {
        throw std::overflow_error("tile zoom exceeds 64-bit limit");
    }
    if (x > (1U << z) - 1U || y > (1U << z) - 1U)
    {
        throw std::overflow_error("tile x/y outside zoom level bounds");
    }
    uint64_t acc = ((1LL << (z * 2U)) - 1) / 3;
    uint32_t tx = x, ty = y;
    int a = z - 1;
    for (uint32_t s = 1LL << a; s > 0; s >>= 1)
    {
        uint32_t rx = s & tx;
        uint32_t ry = s & ty;
        rotate(s, tx, ty, rx, ry);
        acc += ((3LL * rx) ^ ry) << a;
        a--;
    }
    return acc;
}

} // namespace

inline std::int32_t read_int32_ndr(char const* buf, std::size_t pos)
{
    std::int32_t val;
    std::memcpy(&val, &buf[pos], 4);
    return val;
}

inline std::uint64_t read_uint64_xdr(char const* buf, std::size_t pos)
{
    std::uint64_t val;
    std::memcpy(&val, &buf[pos], 8);
    return val;
}

class pmtiles_source : public tiles_source
{
    struct header
    {
        explicit header(char const* data)
            : data_(data)
        {}
        char const* data_;
        bool check_valid() const { return (std::string(data_, data_ + 7) == "PMTiles"); }
        int version() const { return data_[7]; }
        std::uint64_t root_dir_offset() const { return read_uint64_xdr(data_, 8); }
        std::uint64_t root_dir_length() const { return read_uint64_xdr(data_, 16); }
        std::uint64_t metadata_offset() const { return read_uint64_xdr(data_, 24); }
        std::uint64_t metadata_length() const { return read_uint64_xdr(data_, 32); }
        std::uint64_t leaf_directories_offset() const { return read_uint64_xdr(data_, 40); }
        std::uint64_t leaf_directories_length() const { return read_uint64_xdr(data_, 48); }
        std::uint64_t tile_data_offset() const { return read_uint64_xdr(data_, 56); }
        std::uint64_t tile_data_length() const { return read_uint64_xdr(data_, 64); }
        std::uint64_t addressed_tile_count() const { return read_uint64_xdr(data_, 72); }
        std::uint64_t tile_entries_count() const { return read_uint64_xdr(data_, 80); }
        std::uint64_t tile_content_count() const { return read_uint64_xdr(data_, 88); }

        int min_zoom() const { return static_cast<int>(data_[100]); }
        int max_zoom() const { return static_cast<int>(data_[101]); }
        double minx() const { return read_int32_ndr(data_, 102) / 1e7; }
        double miny() const { return read_int32_ndr(data_, 106) / 1e7; }
        double maxx() const { return read_int32_ndr(data_, 110) / 1e7; }
        double maxy() const { return read_int32_ndr(data_, 114) / 1e7; }
        compression_type internal_compression() const { return compression_type(data_[97]); }
        compression_type tile_compression() const { return compression_type(data_[98]); }
        tile_type type() const { return tile_type(data_[99]); }
    };

  public:

    pmtiles_source() {}
    pmtiles_source(std::string const& file_name)
        : file_(file_name.c_str(), boost::interprocess::read_only),
          region_(file_, boost::interprocess::read_only)
    {
        if (!is_good())
        {
            throw mapnik::datasource_exception("Failed to create memory mapping for " + file_name);
        }
        init();
    }

    ~pmtiles_source() = default;

    void init()
    {
        header h(data());
        if (!h.check_valid())
        {
            throw mapnik::datasource_exception("PMTiles: invalid magic number");
        }
        else
        {
            // std::cerr << "Version:" << h.version() << std::endl;
            // std::cerr << "Min zoom:" << h.min_zoom() << std::endl;
            // std::cerr << "Max zoom:" << h.max_zoom() << std::endl;
            // std::cerr << "Min Lon/Lat:" << h.minx() << "," << h.miny() << std::endl;
            // std::cerr << "Max Lon/Lat:" << h.maxx() << "," << h.maxy() << std::endl;

            minzoom_ = h.min_zoom();
            maxzoom_ = h.max_zoom();
            extent_ = mapnik::box2d<double>{h.minx(), h.miny(), h.maxx(), h.maxy()};
            root_dir_offset_ = h.root_dir_offset();
            root_dir_length_ = h.root_dir_length();
            tile_data_offset_ = h.tile_data_offset();
            leaf_directories_offset_ = h.leaf_directories_offset();
            compression_ = h.tile_compression();
            type_ = h.type();
            // std::cerr << "Internal compression:" << (int)h.internal_compression() << std::endl;
            // std::cerr << "Tile compression:" << (int)h.tile_compression() << std::endl;
            // std::cerr << "Addressed tile count:" << h.addressed_tile_count() << std::endl;
        }
    }

    inline bool is_good() const { return (region_.get_size() > 0); }
    inline bool is_raster() const { return type_ != tile_type::MVT; }

  private:
    inline char const* data() const { return reinterpret_cast<char const*>(region_.get_address()); }
    boost::interprocess::file_mapping file_;
    boost::interprocess::mapped_region region_;
    std::uint64_t root_dir_offset_;
    std::uint64_t root_dir_length_;
    std::uint64_t tile_data_offset_;
    std::uint64_t leaf_directories_offset_;
    std::uint8_t minzoom_ = 0;
    std::uint8_t maxzoom_ = 14;
    mapnik::box2d<double> extent_;
    compression_type compression_;
    tile_type type_;

    std::pair<uint64_t, uint32_t> get_tile_position(std::uint8_t z, std::uint32_t x, std::uint32_t y) const
    {
        try
        {
            auto tile_id = zxy_to_tileid(z, x, y);
            std::uint64_t dir_offset = root_dir_offset_;
            std::uint64_t dir_length = root_dir_length_;
            for (std::size_t depth = 0; depth < 4; ++depth)
            {
                std::string decompressed_dir;
                mapnik::vector_tile_impl::zlib_decompress(data() + dir_offset, dir_length, decompressed_dir);
                auto dir_entries = deserialize_directory(decompressed_dir);
                auto entry = find_tile(dir_entries, tile_id);
                if (entry.length > 0)
                {
                    if (entry.run_length > 0)
                    {
                        return std::make_pair(tile_data_offset_ + entry.offset, entry.length);
                    }
                    else
                    {
                        dir_offset = leaf_directories_offset_ + entry.offset;
                        dir_length = entry.length;
                    }
                }
                else
                {
                    return std::make_pair(0, 0);
                }
            }
            return std::make_pair(0, 0);
        }
        catch (std::exception const& ex)
        {
            return std::make_pair(0, 0);
        }
    }

  public:

    std::uint8_t minzoom() const { return minzoom_; }

    std::uint8_t maxzoom() const { return maxzoom_; }

    mapnik::box2d<double> const& extent() const { return extent_; }

    std::string get_tile(std::uint8_t z, std::uint32_t x, std::uint32_t y) const
    {
        auto tile = get_tile_position(z, x, y);
        if (compression_ == compression_type::GZIP)
        {
            if (mapnik::vector_tile_impl::is_gzip_compressed(data() + tile.first, tile.second) ||
                mapnik::vector_tile_impl::is_zlib_compressed(data() + tile.first, tile.second))
            {
                std::string decompressed;
                mapnik::vector_tile_impl::zlib_decompress(data() + tile.first, tile.second, decompressed);
                return decompressed;
            }
        }
        return std::string(data() + tile.first, tile.second);
    }

    boost::json::value metadata() const
    {
        header h(data());
        auto metadata_offset = h.metadata_offset();
        auto metadata_length = h.metadata_length();
        std::string metadata;
        if (h.internal_compression() == compression_type::GZIP)
        {
            mapnik::vector_tile_impl::zlib_decompress(data() + metadata_offset, metadata_length, metadata);
        }
        else
        {
            metadata = {data() + metadata_offset, metadata_length};
        }
        boost::json::value json_value;
        try
        {
            json_value = boost::json::parse(metadata);
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
        return json_value;
    }
};
} // namespace mapnik

#endif // MAPNIK_PMTILES_SOURCE_HPP
