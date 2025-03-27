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

#ifndef MAPNIK_PMTILES_FILE_HPP
#define MAPNIK_PMTILES_FILE_HPP

#include <mapnik/global.hpp>
#define MAPNIK_MEMORY_MAPPED_FILE
#include <mapnik/util/mapped_memory_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
// stl
#include <iostream>
#include <tuple>
#include <fstream>

// boost
//#include <boost/iostreams/filtering_stream.hpp>
//#include <boost/iostreams/copy.hpp>
//#include <boost/iostreams/filter/gzip.hpp>
#include <boost/json.hpp>
//#include <boost/format.hpp>
#include "vector_tile_compression.hpp"

namespace mapnik {

enum class compression_type : std::uint8_t
{
    UNKNOWN = 0x0,
    NONE = 0x1,
    GZIP = 0x2,
    BROTLI = 0x3,
    ZSTD = 0x4
};

struct entryv3 {
    uint64_t tile_id;
    uint64_t offset;
    uint32_t length;
    uint32_t run_length;

    entryv3()
        : tile_id(0), offset(0), length(0), run_length(0) {}

    entryv3(uint64_t _tile_id, uint64_t _offset, uint32_t _length, uint32_t _run_length)
        : tile_id(_tile_id), offset(_offset), length(_length), run_length(_run_length) {
    }
};
namespace {
struct varint_too_long_exception : std::exception
{
    const char *what() const noexcept override {
        return "varint too long exception";
    }
};

struct end_of_buffer_exception : std::exception
{
    const char *what() const noexcept override
    {
        return "end of buffer exception";
    }
};
struct malformed_directory_exception : std::exception {
    const char *what() const noexcept override {
        return "malformed directory exception";
    }
};

constexpr const int8_t max_varint_length = sizeof(uint64_t) * 8 / 7 + 1;
// from https://github.com/mapbox/protozero/blob/master/include/protozero/varint.hpp
uint64_t decode_varint_impl(const char **data, const char *end)
{
    const auto *begin = reinterpret_cast<const int8_t *>(*data);
    const auto *iend = reinterpret_cast<const int8_t *>(end);
    const int8_t *p = begin;
    uint64_t val = 0;
    if (iend - begin >= max_varint_length) {  // fast path
        do {
            int64_t b = *p++;
            val = ((uint64_t(b) & 0x7fU));
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 7U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 14U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 21U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 28U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 35U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 42U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 49U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x7fU) << 56U);
            if (b >= 0) {
                break;
            }
            b = *p++;
            val |= ((uint64_t(b) & 0x01U) << 63U);
            if (b >= 0) {
                break;
            }
            throw varint_too_long_exception{};
        } while (false);
    } else {
        unsigned int shift = 0;
        while (p != iend && *p < 0) {
            val |= (uint64_t(*p++) & 0x7fU) << shift;
            shift += 7;
        }
        if (p == iend) {
            throw end_of_buffer_exception{};
        }
        val |= uint64_t(*p++) << shift;
    }

    *data = reinterpret_cast<const char *>(p);
    return val;
}

uint64_t decode_varint(const char **data, const char *end) {
    // If this is a one-byte varint, decode it here.
    if (end != *data && ((static_cast<uint64_t>(**data) & 0x80U) == 0)) {
        const auto val = static_cast<uint64_t>(**data);
        ++(*data);
        return val;
    }
    // If this varint is more than one byte, defer to complete implementation.
    return decode_varint_impl(data, end);
}

inline std::vector<entryv3> deserialize_directory(std::string const& decompressed)
{
    const char *t = decompressed.data();
    const char *end = t + decompressed.size();

    const uint64_t num_entries_64bit = decode_varint(&t, end);
    // Sanity check to avoid excessive memory allocation attempt:
    // each directory entry takes at least 4 bytes
    if (num_entries_64bit / 4U > decompressed.size()) {
        throw malformed_directory_exception();
    }
    const size_t num_entries = static_cast<size_t>(num_entries_64bit);
    //std::cerr << "Decompressed size" << decompressed.size() << std::endl;
    std::vector<entryv3> result;
    result.resize(num_entries);

    uint64_t last_id = 0;
    for (std::size_t i = 0; i < num_entries; ++i)
    {
        const uint64_t val = decode_varint(&t, end);
        if (val > std::numeric_limits<uint64_t>::max() - last_id)
        {
            throw malformed_directory_exception();
        }
        const uint64_t tile_id = last_id + val;
        result[i].tile_id = tile_id;
        last_id = tile_id;
    }

    for (std::size_t i = 0; i < num_entries; ++i)
    {
        const uint64_t val = decode_varint(&t, end);
        if (val > std::numeric_limits<uint32_t>::max())
        {
            throw malformed_directory_exception();
        }
        result[i].run_length = static_cast<uint32_t>(val);
    }

    for (size_t i = 0; i < num_entries; i++)
    {
        const uint64_t val = decode_varint(&t, end);
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
void rotate(int64_t n, uint32_t &x, uint32_t &y, uint32_t rx, uint32_t ry)
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
    for (uint32_t s = 1LL << a; s > 0; s >>= 1) {
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

class pmtiles_file : public mapnik::util::mapped_memory_file
{
    struct header
    {
        explicit header(char const* data)
            : data_(data) {}
        char const* data_;
        bool check_valid() const
        {
            return (std::string(data_, data_ + 7) == "PMTiles");
        }
        int version() const { return data_[7]; }
        std::uint64_t root_dir_offset() const
        {
            return read_uint64_xdr(data_, 8);
        }
        std::uint64_t root_dir_length() const
        {
            return read_uint64_xdr(data_, 16);
        }
        std::uint64_t metadata_offset() const
        {
            return read_uint64_xdr(data_, 24);
        }
        std::uint64_t metadata_length() const
        {
            return read_uint64_xdr(data_, 32);
        }
        std::uint64_t leaf_directories_offset() const
        {
            return read_uint64_xdr(data_, 40);
        }
        std::uint64_t leaf_directories_length() const
        {
            return read_uint64_xdr(data_, 48);
        }
        std::uint64_t tile_data_offset() const
        {
            return read_uint64_xdr(data_, 56);
        }
        std::uint64_t tile_data_length() const
        {
            return read_uint64_xdr(data_, 64);
        }
        std::uint64_t addressed_tile_count() const
        {
            return read_uint64_xdr(data_, 72);
        }
        std::uint64_t tile_entries_count() const
        {
            return read_uint64_xdr(data_, 80);
        }
        std::uint64_t tile_content_count() const
        {
            return read_uint64_xdr(data_, 88);
        }

        int min_zoom() const { return static_cast<int>(data_[100]); }
        int max_zoom() const { return static_cast<int>(data_[101]); }
        double minx() const
        {
            return read_int32_ndr(data_, 102)/1e7;
        }
        double miny() const
        {
            return read_int32_ndr(data_, 106)/1e7;
        }
        double maxx() const
        {
            return read_int32_ndr(data_, 110)/1e7;
        }
        double maxy() const
        {
            return read_int32_ndr(data_, 114)/1e7;
        }
        compression_type internal_compression() const
        {
            return compression_type(data_[97]);
        }
        compression_type tile_compression() const
        {
            return compression_type(data_[98]);
        }


    };
public:

    pmtiles_file() {}

    pmtiles_file(std::string const& file_name)
        : mapped_memory_file(file_name)
          //: file_(file_name.c_str(), boost::interprocess::read_only),
          //region_(file_, boost::interprocess::read_only)
    {}

    ~pmtiles_file() {}

    template<typename Datasource>
    void read_header(Datasource & source)
    {
        header h(data());
        if (!h.check_valid())
            std::cerr << "PMTiles: invalid magic number" << std::endl;
        else
        {
            std::cerr << "Version:" << h.version() << std::endl;
            std::cerr << "Min zoom:" << h.min_zoom() << std::endl;
            std::cerr << "Max zoom:" << h.max_zoom() << std::endl;
            std::cerr << "Min Lon/Lat:" << h.minx() << "," << h.miny() << std::endl;
            std::cerr << "Max Lon/Lat:" << h.maxx() << "," << h.maxy() << std::endl;

            source.minzoom_ = h.min_zoom();
            source.maxzoom_ = h.max_zoom();
            source.extent_ = mapnik::box2d<double>{h.minx(), h.miny(),h.maxx(), h.maxy()};
            root_dir_offset_ = h.root_dir_offset();
            root_dir_length_ = h.root_dir_length();
            tile_data_offset_ = h.tile_data_offset();
            leaf_directories_offset_ = h.leaf_directories_offset();
            std::cerr << "Internal compression:" << (int)h.internal_compression() << std::endl;
            std::cerr << "Tile compression:" << (int)h.tile_compression() << std::endl;
            std::cerr << "Addressed tile count:" << h.addressed_tile_count() << std::endl;
        }
    }
    boost::json::value metadata() const
    {
        header h(data());
        auto metadata_offset = h.metadata_offset();
        auto metadata_length = h.metadata_length();
        //using namespace boost::iostreams;
        //namespace io = boost::iostreams;
        std::string metadata;

        //filtering_istream in;
        //if (h.internal_compression() == compression_type::GZIP)
        //{
        //    in.push(gzip_decompressor());
        //}
        //in.push(array_source(data() + metadata_offset, metadata_length));
        //io::copy(in, io::back_inserter(metadata));
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
    inline void seek(std::streampos pos) { file_.seekg(pos, std::ios::beg); }
    inline std::streampos pos() { return file_.tellg(); }
    inline bool is_eof() { return file_.eof(); }
    inline bool is_good() { return file_.good(); }
    //inline bool is_good() { return (region_.get_size() > 0);}
    //inline char const* data() const { return reinterpret_cast<char const*>(region_.get_address()); }
    inline char const* data() const { return file_.buffer().first; }
    //boost::interprocess::file_mapping file_;
    //boost::interprocess::mapped_region region_;
    std::uint64_t root_dir_offset_;
    std::uint64_t root_dir_length_;
    std::uint64_t tile_data_offset_;
    std::uint64_t leaf_directories_offset_;

    std::pair<uint64_t, uint32_t> get_tile(std::uint8_t z, std::uint32_t x, std::uint32_t y)
    {
        auto tile_id = zxy_to_tileid(z, x, y);
        //std::cerr << "TileID:" << tile_id << std::endl;
        //header h(file_.buffer().first);
        //std::cerr << "Root dir offset:" << root_dir_offset_ << std::endl;
        //std::cerr << "Root dir length:" << root_dir_length_ << std::endl;
        std::uint64_t dir_offset = root_dir_offset_;
        std::uint64_t dir_length = root_dir_length_;
        //using namespace boost::iostreams;
        //namespace io = boost::iostreams;
        //filtering_istream in;
        //in.set_auto_close(false);

        //if (h.internal_compression() == compression_type::GZIP)
        //{
        //    in.push(gzip_decompressor());
        //}

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
};
}

#endif //MAPNIK_PMTILES_FILE_HPP
