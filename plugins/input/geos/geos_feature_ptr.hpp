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

#ifndef GEOS_FEATURE_PTR_HPP
#define GEOS_FEATURE_PTR_HPP

// geos
#include <geos_c.h>
#include <cstdlib>

class geos_feature_ptr
{
public:
    geos_feature_ptr ()
        : feat_ (NULL)
    {
    }

    explicit geos_feature_ptr (GEOSGeometry* const feat)
        : feat_ (feat)
    {
    }

    ~geos_feature_ptr ()
    {
        if (feat_ != NULL)
            GEOSGeom_destroy(feat_);
    }

    void set_feature (GEOSGeometry* const feat)
    {
        if (feat_ != NULL)
            GEOSGeom_destroy(feat_);

        feat_ = feat;
    }

    GEOSGeometry* operator*()
    {
        return feat_;
    }

private:
    GEOSGeometry* feat_;
};


class geos_wkb_ptr
{
public:
    geos_wkb_ptr (GEOSGeometry* const geometry)
        : data_ (NULL),
          size_ (0)
    {
        data_ = GEOSGeomToWKB_buf(geometry, &size_);
    }

    ~geos_wkb_ptr ()
    {
        if (data_ != NULL)
        {
            // We use std::free here instead of GEOSFree(data_) to support geos 3.1.0
            std::free(data_);
        }
    }

    bool is_valid() const
    {
        return (data_ != NULL) && (size_ > 0);
    }

    unsigned int size() const
    {
        return (unsigned int) size_;
    }

    const char* data()
    {
        return reinterpret_cast<const char*>(data_);
    }

private:
    unsigned char* data_;
    size_t size_;
};


#endif // GEOS_FEATURE_PTR_HPP

