/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
 *****************************************************************************
 *
 * Initially developed by Sandro Santilli <strk@keybit.net> for CartoDB
 *
 *****************************************************************************/

#include "pgraster_featureset.hpp"
#include "pgraster_wkb_reader.hpp"
#include "../postgis/resultset.hpp"
#include "../postgis/cursorresultset.hpp"
#include "../postgis/numeric2string.hpp"

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/image.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/global.hpp> // for int2net

// stl
#include <sstream>
#include <string>

using mapnik::feature_factory;
using mapnik::context_ptr;

pgraster_featureset::pgraster_featureset(std::shared_ptr<IResultSet> const& rs,
                                       context_ptr const& ctx,
                                       std::string const& encoding,
                                       bool key_field, int bandno)
    : rs_(rs),
      ctx_(ctx),
      tr_(new transcoder(encoding)),
      feature_id_(1),
      key_field_(key_field),
      band_(bandno)
{
}

feature_ptr pgraster_featureset::next()
{
    while (rs_->next())
    {
        // new feature
        unsigned pos = 1;
        feature_ptr feature;

        if (key_field_)
        {
            std::string name = rs_->getFieldName(pos);

            // null feature id is not acceptable
            if (rs_->isNull(pos))
            {
                MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: null value encountered for key_field: " << name;
                continue;
            }
            // create feature with user driven id from attribute
            int oid = rs_->getTypeOID(pos);
            const char* buf = rs_->getValue(pos);

            // validation happens of this type at initialization
            mapnik::value_integer val;

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

            MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: feature key: " << val;

            feature = feature_factory::create(ctx_, val);
            // TODO - extend feature class to know
            // that its id is also an attribute to avoid
            // this duplication
            feature->put<mapnik::value_integer>(name,val);
            ++pos;
        }
        else
        {
            // fallback to auto-incrementing id
            MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: feature id: " << feature_id_;

            feature = feature_factory::create(ctx_, feature_id_);
            ++feature_id_;
        }

        // null geometry is not acceptable
        if (rs_->isNull(0))
        {
            MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: null value encountered for raster";
            continue;
        }

        // parse geometry
        int size = rs_->getFieldLength(0);
        const uint8_t *data = (const uint8_t*)rs_->getValue(0);

        mapnik::raster_ptr raster = pgraster_wkb_reader::read(data, size, band_);
        if (!raster)
        {
            MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: could not parse raster wkb";
            // TODO: throw an exception ?
            continue;
        }
        MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: raster of " << raster->data_.width() << "x" << raster->data_.height() << " pixels covering extent " << raster->ext_;
        feature->set_raster(raster);

        unsigned num_attrs = ctx_->size() + 1;
        for (; pos < num_attrs; ++pos)
        {
            std::string name = rs_->getFieldName(pos);

            // NOTE: we intentionally do not store null here
            // since it is equivalent to the attribute not existing
            if (!rs_->isNull(pos))
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
                        feature->put<mapnik::value_integer>(name, int4net(buf));
                        break;
                    }

                    case 21: //int2
                    {
                        feature->put<mapnik::value_integer>(name, int2net(buf));
                        break;
                    }

                    case 20: //int8/BigInt
                    {
                        feature->put<mapnik::value_integer>(name, int8net(buf));
                        break;
                    }

                    case 700: //float4
                    {
                        float val;
                        float4net(val, buf);
                        feature->put(name, static_cast<double>(val));
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
                    case 705:  //literal
                    {
                        feature->put(name, tr_->transcode(buf));
                        break;
                    }

                    case 1042: //bpchar
                    {
                        std::string str = mapnik::util::trim_copy(buf);
                        feature->put(name, tr_->transcode(str.c_str()));
                        break;
                    }

                    case 1700: //numeric
                    {
                        double val;
                        std::string str = numeric2string(buf);
                        if (mapnik::util::string2double(str, val))
                        {
                            feature->put(name, val);
                        }
                        break;
                    }

                    default:
                    {
                        MAPNIK_LOG_WARN(pgraster) << "pgraster_featureset: Unknown type_oid=" << oid;

                        break;
                    }
                }
            }
        }
        return feature;
    }
    return feature_ptr();
}


pgraster_featureset::~pgraster_featureset()
{
    rs_->close();
}
