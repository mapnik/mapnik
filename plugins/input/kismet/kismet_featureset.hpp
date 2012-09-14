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

#ifndef KISMET_FEATURESET_HPP
#define KISMET_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/feature.hpp>

// boost
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

//STL
#include <list>

#include "kismet_types.hpp"

class kismet_featureset : public mapnik::Featureset
{
public:
    kismet_featureset(std::list<kismet_network_data> const& knd_list,
                      std::string const& srs,
                      std::string const& encoding);
    virtual ~kismet_featureset();
    mapnik::feature_ptr next();

private:
    std::list<kismet_network_data> const& knd_list_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    int feature_id_;
    std::list<kismet_network_data>::const_iterator knd_list_it;
    mapnik::projection source_;
    mapnik::context_ptr ctx_;
};

#endif // KISMET_FEATURESET_HPP
