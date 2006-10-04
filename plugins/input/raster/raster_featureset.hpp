/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
#ifndef RASTER_FEATURESET_HH
#define RASTER_FEATURESET_HH

#include <vector>

#include "raster_datasource.hpp"
#include "raster_info.hpp"

using std::vector;

class single_file_policy
{
    raster_info info_;
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

        const raster_info& operator*() const
        {
            return p_->info_;
        }

        const raster_info* operator->() const
        {
            return &(p_->info_);
        }

        bool operator!=(const const_iterator& itr)
        {
            return status_!=itr.status_;
        }
    };

    explicit single_file_policy(const raster_info& info)
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

template <typename LookupPolicy>
class raster_featureset : public Featureset
{
    typedef typename LookupPolicy::const_iterator iterator_type;
    LookupPolicy policy_;
    size_t id_;
    Envelope<double> extent_;
    iterator_type curIter_;
    iterator_type endIter_;
public:
    raster_featureset(LookupPolicy const& policy,query const& q);
    virtual ~raster_featureset();
    feature_ptr next();
};

#endif //RASTER_FEATURESET_HH
