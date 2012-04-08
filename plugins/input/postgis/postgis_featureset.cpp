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

#include "postgis_featureset.hpp"
#include "resultset.hpp"
#include "cursorresultset.hpp"

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>

// stl
#include <sstream>
#include <string>

using mapnik::geometry_type;
using mapnik::byte;
using mapnik::geometry_utils;
using mapnik::feature_factory;
using mapnik::context_ptr;

postgis_featureset::postgis_featureset(boost::shared_ptr<IResultSet> const& rs,
                                       context_ptr const& ctx,
                                       std::string const& encoding,
                                       bool key_field)
    : rs_(rs),
      ctx_(ctx),
      tr_(new transcoder(encoding)),
      totalGeomSize_(0),
      feature_id_(1),
      key_field_(key_field)
{
}

feature_ptr postgis_featureset::next()
{
    if (rs_->next())
    {
        // new feature
        unsigned pos = 1;
        feature_ptr feature;

        if (key_field_)
        {
            // create feature with user driven id from attribute
            int oid = rs_->getTypeOID(pos);
            const char* buf = rs_->getValue(pos);
            std::string name = rs_->getFieldName(pos);

            // validation happens of this type at bind()
            int val;
            if (oid == 20)
            {
                val = int8net(buf);
            }
            else if (oid == 21)
            {
                val = int2net(buf);
            }
            else
            {
                val = int4net(buf);
            }

            feature = feature_factory::create(ctx_, val);
            // TODO - extend feature class to know
            // that its id is also an attribute to avoid
            // this duplication
            feature->put(name,val);
            ++pos;
        }
        else
        {
            // fallback to auto-incrementing id
            feature = feature_factory::create(ctx_, feature_id_);
            ++feature_id_;
        }

        // parse geometry
        int size = rs_->getFieldLength(0);
        const char *data = rs_->getValue(0);
        geometry_utils::from_wkb(feature->paths(), data, size);
        totalGeomSize_ += size;

        int num_attrs = ctx_->size() + 1;
        for (; pos < num_attrs; ++pos)
        {
            std::string name = rs_->getFieldName(pos);

            if (rs_->isNull(pos))
            {
                feature->put(name, mapnik::value_null());
            }
            else
            {
                const char* buf = rs_->getValue(pos);
                const int oid = rs_->getTypeOID(pos);
                switch (oid)
                {
                    case 16: //bool
                    {
                        feature->put(name, (buf[0] != 0));
                        break;
                    }

                    case 23: //int4
                    {
                        int val = int4net(buf);
                        feature->put(name, val);
                        break;
                    }

                    case 21: //int2
                    {
                        int val = int2net(buf);
                        feature->put(name, val);
                        break;
                    }

                    case 20: //int8/BigInt
                    {
                        // TODO - need to support boost::uint64_t in mapnik::value
                        // https://github.com/mapnik/mapnik/issues/895
                        int val = int8net(buf);
                        feature->put(name, val);
                        break;
                    }

                    case 700: //float4
                    {
                        float val;
                        float4net(val, buf);
                        feature->put(name, val);
                        break;
                    }

                    case 701: //float8
                    {
                        double val;
                        float8net(val, buf);
                        feature->put(name, val);
                        break;
                    }

                    case 25:   //text
                    case 1043: //varchar
                    {
                        feature->put(name, tr_->transcode(buf));
                        break;
                    }

                    case 1042: //bpchar
                    {
                        std::string str(buf);
                        boost::trim(str);
                        feature->put(name, tr_->transcode(str.c_str()));
                        break;
                    }

                    case 1700: //numeric
                    {
                        double val;
                        std::string str = mapnik::sql_utils::numeric2string(buf);
                        if (mapnik::util::string2double(str, val))
                        {
                            feature->put(name, val);
                        }
                        break;
                    }

                    default:
                    {
#ifdef MAPNIK_LOG
                        mapnik::log() << "postgis_featureset: Uknown type_oid=" << oid;
#endif
                        break;
                    }
                }
            }
        }
        return feature;
    }
    else
    {
        rs_->close();
        return feature_ptr();
    }
}


postgis_featureset::~postgis_featureset()
{
    rs_->close();
}
