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
#include <mapnik/datasource.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_factory.hpp>

#include "kismet_featureset.hpp"

using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::geometry_type;
using mapnik::geometry_utils;
using mapnik::transcoder;
using mapnik::feature_factory;

kismet_featureset::kismet_featureset(std::list<kismet_network_data> const& knd_list,
                                     std::string const& srs,
                                     std::string const& encoding)
    : knd_list_(knd_list),
      tr_(new transcoder(encoding)),
      feature_id_(1),
      knd_list_it(knd_list_.begin()),
      source_(srs),
      ctx_(boost::make_shared<mapnik::context_type>())
{
    ctx_->push("internet_access");
}

kismet_featureset::~kismet_featureset()
{
}

feature_ptr kismet_featureset::next()
{
    if (knd_list_it != knd_list_.end ())
    {
        const kismet_network_data& knd = *knd_list_it;
        const std::string key = "internet_access";

        std::string value;
        if (knd.crypt() == crypt_none)
        {
            value = "wlan_uncrypted";
        }
        else if (knd.crypt() == crypt_wep)
        {
            value = "wlan_wep";
        }
        else
        {
            value = "wlan_crypted";
        }

        feature_ptr feature(feature_factory::create(ctx_,feature_id_));
        ++feature_id_;

        geometry_type* pt = new geometry_type(mapnik::Point);
        pt->move_to(knd.bestlon(), knd.bestlat());
        feature->add_geometry(pt);

        feature->put(key, tr_->transcode(value.c_str()));

        ++knd_list_it;

        return feature;
    }

    return feature_ptr();
}
