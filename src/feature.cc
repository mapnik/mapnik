/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: feature.cc 58 2004-10-31 16:21:26Z artem $

#include "feature.hh"

namespace mapnik
{
    FeatureImpl::FeatureImpl(int id)
	: id_(id) {}

    FeatureImpl::~FeatureImpl() {}

    int FeatureImpl::id() const 
    {
	return id_;
    }
    
    void FeatureImpl::add(const std::string& name,const attribute_base& a)
    {
        attributes_.add(name,a);
    }

    const attribute_base& FeatureImpl::get(const std::string& name) const
    {
        return attributes_.get(name);
    }

    const attribute_container& FeatureImpl::attributes() const
    {
        return attributes_;
    }
}
