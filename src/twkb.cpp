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

// mapnik
#include <mapnik/wkb.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/correct.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <cmath>

namespace mapnik {
namespace detail {

struct twkb_reader : mapnik::util::noncopyable
{
  private:
    const char* twkb_;
    size_t size_;
    unsigned int pos_;
    // Metadata on the geometry we are parsing
    uint8_t twkb_type_;
    uint8_t has_bbox_;
    uint8_t has_size_;
    uint8_t has_idlist_;
    uint8_t has_z_;
    uint8_t has_m_;
    uint8_t is_empty_;
    // Precision factors to convert ints to double
    double factor_xy_;
    double factor_z_;
    double factor_m_;
    // An array to keep delta values from 4 dimensions
    int64_t coord_x_;
    int64_t coord_y_;
    int64_t coord_z_;
    int64_t coord_m_;

  public:
    enum twkbGeometryType : std::uint8_t {
        twkbPoint = 1,
        twkbLineString = 2,
        twkbPolygon = 3,
        twkbMultiPoint = 4,
        twkbMultiLineString = 5,
        twkbMultiPolygon = 6,
        twkbGeometryCollection = 7
    };

    twkb_reader(char const* twkb, size_t size)
        : twkb_(twkb),
          size_(size),
          pos_(0),
          twkb_type_(0),   // Geometry type
          has_bbox_(0),    // Bounding box?
          has_size_(0),    // Size attribute?
          has_idlist_(0),  // Presence of X/Y
          has_z_(0),       // Presence of Z
          has_m_(0),       // Presence of M
          is_empty_(0),    // Empty?
          factor_xy_(0.0), // Expansion factor for X/Y
          factor_z_(0.0),  // Expansion factor for Z
          factor_m_(0.0)   // Expansion factor for M
    {}

    mapnik::geometry::geometry<double> read()
    {
        mapnik::geometry::geometry<double> geom = mapnik::geometry::geometry_empty();
        // Read the metadata bytes, populating all the
        // information about optional fields, extended (z/m) dimensions
        // expansion factors and so on
        read_header();

        // Each new read call has to reset the coordinate accumulators
        coord_x_ = 0; // Accumulation register (x)
        coord_y_ = 0; // Accumulation register (y)
        coord_z_ = 0; // Accumulation register (z)
        coord_m_ = 0; // Accumulation register (m)

        // If the geometry is empty, add nothing to the paths array
        if (is_empty_)
            return geom;

        // Read the [optional] size information
        if (has_size_)
            size_ = read_unsigned_integer();

        // Read the [optional] bounding box information
        if (has_bbox_)
            read_bbox();

        switch (twkb_type_)
        {
            case twkbPoint:
                geom = read_point();
                break;
            case twkbLineString:
                geom = read_linestring();
                break;
            case twkbPolygon:
                geom = read_polygon();
                break;
            case twkbMultiPoint:
                geom = read_multipoint();
                break;
            case twkbMultiLineString:
                geom = read_multilinestring();
                break;
            case twkbMultiPolygon:
                geom = read_multipolygon();
                break;
            case twkbGeometryCollection:
                geom = read_collection();
            default:
                break;
        }
        return geom;
    }

  private:
    int64_t unzigzag64(uint64_t val)
    {
        if (val & 0x01)
            return -1 * (int64_t)((val + 1) >> 1);
        else
            return (int64_t)(val >> 1);
    }

    int32_t unzigzag32(uint32_t val)
    {
        if (val & 0x01)
            return -1 * (int32_t)((val + 1) >> 1);
        else
            return (int32_t)(val >> 1);
    }

    int8_t unzigzag8(uint8_t val)
    {
        if (val & 0x01)
            return -1 * (int8_t)((val + 1) >> 1);
        else
            return (int8_t)(val >> 1);
    }

    // Read from signed 64bit varint
    int64_t read_signed_integer() { return unzigzag64(read_unsigned_integer()); }

    // Read from unsigned 64bit varint
    uint64_t read_unsigned_integer()
    {
        uint64_t nVal = 0;
        int nShift = 0;
        uint8_t nByte;

        // Check so we don't read beyond the twkb
        while (pos_ < size_)
        {
            nByte = twkb_[pos_];
            // We get here when there is more to read in the input varInt
            // Here we take the least significant 7 bits of the read
            // byte and put it in the most significant place in the result variable.
            nVal |= ((uint64_t)(nByte & 0x7f)) << nShift;
            // move the "cursor" of the input buffer step (8 bits)
            pos_++;
            // move the cursor in the resulting variable (7 bits)
            nShift += 7;
            // Hibit isn't set, so this is the last byte
            if (!(nByte & 0x80))
            {
                return nVal;
            }
        }
        return 0;
    }

    // Every TWKB geometry starts with a metadata header
    //
    // type_and_dims     byte
    // metadata_header   byte
    // [extended_dims]   byte
    // [size]            uvarint
    // [bounds]          bbox
    //
    void read_header()
    {
        uint8_t type_precision = twkb_[pos_++];
        uint8_t metadata = twkb_[pos_++];
        twkb_type_ = type_precision & 0x0F;
        int8_t precision = unzigzag8((type_precision & 0xF0) >> 4);
        factor_xy_ = std::pow(10, static_cast<double>(precision));
        has_bbox_ = metadata & 0x01;
        has_size_ = (metadata & 0x02) >> 1;
        has_idlist_ = (metadata & 0x04) >> 2;
        uint8_t zm = (metadata & 0x08) >> 3;
        is_empty_ = (metadata & 0x10) >> 4;

        // Flag for higher dimensions means read a third byte
        // of extended dimension information
        if (zm)
        {
            zm = twkb_[pos_++];
            // Strip Z/M presence and precision from ext byte
            has_z_ = (zm & 0x01);
            has_m_ = (zm & 0x02) >> 1;
            // Convert the precision into factor
            int8_t precision_z = (zm & 0x1C) >> 2;
            int8_t precision_m = (zm & 0xE0) >> 5;
            factor_z_ = pow(10, (double)precision_z);
            factor_m_ = pow(10, (double)precision_m);
        }
    }

    void read_bbox()
    {
        // we have nowhere to store this box information
        // for now, so we'll just move the read head forward
        // an appropriate number of times
        if (has_bbox_)
        {
            read_signed_integer(); // uint64_t xmin
            read_signed_integer(); // uint64_t xdelta
            read_signed_integer(); // uint64_t ymin
            read_signed_integer(); // uint64_t ydelta
            if (has_z_)
            {
                read_signed_integer(); // uint64_t zmin
                read_signed_integer(); // uint64_t zdelta
            }
            if (has_m_)
            {
                read_signed_integer(); // uint64_t mmin
                read_signed_integer(); // uint64_t mdelta
            }
        }
    }

    void read_idlist(unsigned int num_ids)
    {
        // we have nowhere to store this id information
        // for now, so we'll just move the read head
        // forward an appropriate number of times
        if (has_idlist_)
        {
            for (unsigned int i = 0; i < num_ids; ++i)
            {
                read_signed_integer(); // uint64_t id
            }
        }
    }

    template<typename Ring>
    void read_coords(Ring& ring, std::size_t num_points)
    {
        for (std::size_t i = 0; i < num_points; ++i)
        {
            coord_x_ += read_signed_integer();
            coord_y_ += read_signed_integer();
            ring.emplace_back(coord_x_ / factor_xy_, coord_y_ / factor_xy_);
            // Skip Z and M
            if (has_z_)
                coord_z_ += read_signed_integer();
            if (has_m_)
                coord_m_ += read_signed_integer();
        }
    }

    mapnik::geometry::point<double> read_point()
    {
        coord_x_ += read_signed_integer();
        coord_y_ += read_signed_integer();
        double x = coord_x_ / factor_xy_;
        double y = coord_y_ / factor_xy_;
        return mapnik::geometry::point<double>(x, y);
    }

    mapnik::geometry::multi_point<double> read_multipoint()
    {
        mapnik::geometry::multi_point<double> multi_point;
        unsigned int num_points = read_unsigned_integer();
        if (has_idlist_)
            read_idlist(num_points);

        if (num_points > 0)
        {
            multi_point.reserve(num_points);
            for (unsigned int i = 0; i < num_points; ++i)
            {
                multi_point.emplace_back(read_point());
            }
        }
        return multi_point;
    }

    mapnik::geometry::line_string<double> read_linestring()
    {
        mapnik::geometry::line_string<double> line;
        unsigned int num_points = read_unsigned_integer();
        if (num_points > 0)
        {
            line.reserve(num_points);
            read_coords<mapnik::geometry::line_string<double>>(line, num_points);
        }
        return line;
    }

    mapnik::geometry::multi_line_string<double> read_multilinestring()
    {
        mapnik::geometry::multi_line_string<double> multi_line;
        unsigned int num_lines = read_unsigned_integer();
        if (has_idlist_)
            read_idlist(num_lines);
        multi_line.reserve(num_lines);
        for (unsigned int i = 0; i < num_lines; ++i)
        {
            multi_line.push_back(read_linestring());
        }
        return multi_line;
    }

    mapnik::geometry::polygon<double> read_polygon()
    {
        unsigned int num_rings = read_unsigned_integer();
        mapnik::geometry::polygon<double> poly;
        poly.reserve(num_rings);
        for (unsigned int i = 0; i < num_rings; ++i)
        {
            mapnik::geometry::linear_ring<double> ring;
            unsigned int num_points = read_unsigned_integer();
            if (num_points > 0)
            {
                ring.reserve(num_points);
                read_coords<mapnik::geometry::linear_ring<double>>(ring, num_points);
            }
            poly.push_back(std::move(ring));
        }
        return poly;
    }

    mapnik::geometry::multi_polygon<double> read_multipolygon()
    {
        mapnik::geometry::multi_polygon<double> multi_poly;
        unsigned int num_polys = read_unsigned_integer();
        if (has_idlist_)
            read_idlist(num_polys);
        for (unsigned int i = 0; i < num_polys; ++i)
        {
            multi_poly.push_back(read_polygon());
        }
        return multi_poly;
    }

    mapnik::geometry::geometry_collection<double> read_collection()
    {
        unsigned int num_geometries = read_unsigned_integer();
        mapnik::geometry::geometry_collection<double> collection;
        if (has_idlist_)
            read_idlist(num_geometries);
        for (unsigned int i = 0; i < num_geometries; ++i)
        {
            collection.push_back(read());
        }
        return collection;
    }
};

} // namespace detail

mapnik::geometry::geometry<double> geometry_utils::from_twkb(const char* wkb, std::size_t size)
{
    detail::twkb_reader reader(wkb, size);
    mapnik::geometry::geometry<double> geom(reader.read());
    // note: this will only be applied to polygons
    mapnik::geometry::correct(geom);
    return geom;
}

} // namespace mapnik
