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

#ifndef RASTER_FEATURESET_HH
#define RASTER_FEATURESET_HH

#include "raster_datasource.hh"
#include "raster_info.hh"
#include <vector>

using std::vector;

class single_file_policy
{
    RasterInfo info_;
public:
    class const_iterator
    {
	enum {start,end};
	bool status_;
	const single_file_policy* p_;
    public:
	explicit const_iterator(const single_file_policy* p)
	    :status_(start),
	     p_(p) {}

	const_iterator()
	    :status_(end){}

	const_iterator(const const_iterator& other)
	    :status_(other.status_),
	     p_(other.p_) {}

	const_iterator& operator++()
	{
	    status_=end;
	    return *this;
	}

	const RasterInfo& operator*() const
	{
	    return p_->info_;
	}

	const RasterInfo* operator->() const
	{
	    return &(p_->info_);
	}

	bool operator!=(const const_iterator& itr)
	{
	    return status_!=itr.status_;
	}
    };

    explicit single_file_policy(const RasterInfo& info)
	:info_(info) {}

    const_iterator begin()
    {
	return const_iterator(this);
    }

    const_iterator query(const Envelope<double>& box)
    {
	if (box.intersects(info_.envelope()))
	{
	    return begin();
	}
	return end();
    }

    const_iterator end()
    {
	return const_iterator();
    }
};

class os_name_policy
{
    //TODO
};

template <typename LookupPolicy>
class RasterFeatureset : public Featureset
{
    typedef typename LookupPolicy::const_iterator iterator_type;
    LookupPolicy policy_;
    size_t id_;
    Envelope<double> extent_;
    CoordTransform t_;
    iterator_type curIter_;
    iterator_type endIter_;
public:
    RasterFeatureset(const LookupPolicy& policy,const Envelope<double>& box,const CoordTransform& t);
    virtual ~RasterFeatureset();
    Feature* next();
};

#endif                                            //RASTER_FEATURESET_HH
