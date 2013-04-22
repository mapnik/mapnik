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
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_factory.hpp>

// ogr
#include "sqlite_featureset.hpp"
#include "sqlite_utils.hpp"

using mapnik::query;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::transcoder;
using mapnik::feature_factory;

sqlite_featureset::sqlite_featureset(boost::shared_ptr<sqlite_resultset> rs,
                                     mapnik::context_ptr const& ctx,
                                     std::string const& encoding,
                                     mapnik::box2d<double> const& bbox,
                                     mapnik::wkbFormat format,
                                     bool spatial_index,
                                     bool using_subquery)
    : rs_(rs),
      ctx_(ctx),
      tr_(new transcoder(encoding)),
      bbox_(bbox),
      format_(format),
      spatial_index_(spatial_index),
      using_subquery_(using_subquery)
{}

sqlite_featureset::~sqlite_featureset() {}

feature_ptr sqlite_featureset::next()
{
    while (rs_->is_valid () && rs_->step_next ())
    {
        int size;
        const char* data = (const char*) rs_->column_blob(0, size);
        if (data == 0)
        {
            return feature_ptr();
        }

        // null feature id is not acceptable
        if (rs_->column_type(1) == SQLITE_NULL)
        {
            MAPNIK_LOG_ERROR(postgis) << "sqlite_featureset: null value encountered for key_field";
            continue;
        }

        feature_ptr feature = feature_factory::create(ctx_,rs_->column_integer64(1));
        if (!geometry_utils::from_wkb(feature->paths(), data, size, format_))
            continue;

        if (!spatial_index_)
        {
            // we are not using r-tree index, check if feature intersects bounding box
            if (!bbox_.intersects(feature->envelope()))
                continue;
        }

        for (int i = 2; i < rs_->column_count(); ++i)
        {
            const int type_oid = rs_->column_type(i);
            const char* fld_name = rs_->column_name(i);

            if (! fld_name)
                continue;

            std::string fld_name_str(fld_name);

            // subqueries in sqlite lead to field double quoting which we need to strip
            if (using_subquery_)
            {
                sqlite_utils::dequote(fld_name_str);
            }

            switch (type_oid)
            {
            case SQLITE_INTEGER:
            {
                feature->put<mapnik::value_integer>(fld_name_str, rs_->column_integer64(i));
                break;
            }

            case SQLITE_FLOAT:
            {
                feature->put(fld_name_str, rs_->column_double(i));
                break;
            }

            case SQLITE_TEXT:
            {
                int text_col_size;
                const char * text_data = rs_->column_text(i, text_col_size);
                UnicodeString ustr = tr_->transcode(text_data, text_col_size);
                feature->put(fld_name_str, ustr);
                break;
            }

            case SQLITE_NULL:
            {
                // NOTE: we intentionally do not store null here
                // since it is equivalent to the attribute not existing
                break;
            }

            case SQLITE_BLOB:
                break;

            default:
                MAPNIK_LOG_WARN(sqlite) << "sqlite_featureset: Field=" << fld_name_str << " unhandled type_oid=" << type_oid;
                break;
            }
        }

        return feature;
    }

    return feature_ptr();
}
