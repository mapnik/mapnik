/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_FEATURE_HPP
#define MAPNIK_FEATURE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/value.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/envelope.hpp>
//
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <memory>
#include <vector>
#include <map>
#include <ostream>                      // for basic_ostream, operator<<, etc
#include <sstream>                      // for basic_stringstream
#include <stdexcept>                    // for out_of_range
#include <iostream>

namespace mapnik {

class raster;
class feature_impl;

using raster_ptr = std::shared_ptr<raster>;

template <typename T>
class context : private util::noncopyable

{
    friend class feature_impl;
public:
    using map_type = T;
    using value_type = typename map_type::value_type;
    using key_type = typename map_type::key_type;
    using size_type = typename map_type::size_type;
    using difference_type = typename map_type::difference_type;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

    context()
        : mapping_() {}

    inline size_type push(key_type const& name)
    {
        size_type index = mapping_.size();
        mapping_.emplace(name, index);
        return index;
    }

    inline void add(key_type const& name, size_type index)
    {
        mapping_.emplace(name, index);
    }

    inline size_type size() const { return mapping_.size(); }
    inline const_iterator begin() const { return mapping_.begin();}
    inline const_iterator end() const { return mapping_.end();}

private:
    map_type mapping_;
};

using context_type = context<std::map<std::string,std::size_t> >;
using context_ptr = std::shared_ptr<context_type>;

static const value default_feature_value{};

class MAPNIK_DECL feature_impl : private util::noncopyable
{
    friend class feature_kv_iterator;
public:

    using value_type = mapnik::value;
    using cont_type = std::vector<value_type>;
    using iterator = feature_kv_iterator;

    feature_impl(context_ptr const& ctx, mapnik::value_integer _id)
        : id_(_id),
        ctx_(ctx),
        data_(ctx_->mapping_.size()),
        geom_(geometry::geometry_empty()),
        raster_() {}

    inline mapnik::value_integer id() const { return id_;}
    inline void set_id(mapnik::value_integer _id) { id_ = _id;}
    template <typename T>
    inline void put(context_type::key_type const& key, T const& val)
    {
        put(key, value(val));
    }

    template <typename T>
    inline void put_new(context_type::key_type const& key, T const& val)
    {
        put_new(key, value(val));
    }

    inline void put(context_type::key_type const& key, value && val)
    {
        context_type::map_type::const_iterator itr = ctx_->mapping_.find(key);
        if (itr != ctx_->mapping_.end()
            && itr->second < data_.size())
        {
            data_[itr->second] = std::move(val);
        }
        else
        {
            throw std::out_of_range(std::string("Key does not exist: '") + key + "'");
        }
    }

    inline void put_new(context_type::key_type const& key, value && val)
    {
        context_type::map_type::const_iterator itr = ctx_->mapping_.find(key);
        if (itr != ctx_->mapping_.end()
            && itr->second < data_.size())
        {
            data_[itr->second] = std::move(val);
        }
        else
        {
            cont_type::size_type index = ctx_->push(key);
            if (index == data_.size())
                data_.push_back(std::move(val));
        }
    }

    inline bool has_key(context_type::key_type const& key) const
    {
        return (ctx_->mapping_.count(key) == 1);
    }

    inline value_type const& get(context_type::key_type const& key) const
    {
        context_type::map_type::const_iterator itr = ctx_->mapping_.find(key);
        if (itr != ctx_->mapping_.end())
            return get(itr->second);
        else
            return default_feature_value;
    }

    inline value_type const& get(std::size_t index) const
    {
        if (index < data_.size())
            return data_[index];
        return default_feature_value;
    }

    inline std::size_t size() const
    {
        return data_.size();
    }

    inline cont_type const& get_data() const
    {
        return data_;
    }

    inline void set_data(cont_type const& data)
    {
        data_ = data;
    }

    inline context_ptr context() const
    {
        return ctx_;
    }

    inline void set_geometry(geometry::geometry<double> && geom)
    {
        geom_ = std::move(geom);
    }

    inline void set_geometry_copy(geometry::geometry<double> const& geom)
    {
        geom_ = geom;
    }

    inline geometry::geometry<double> const& get_geometry() const
    {
        return geom_;
    }

    inline geometry::geometry<double> & get_geometry()
    {
        return geom_;
    }

    inline box2d<double> envelope() const
    {
        return mapnik::geometry::envelope(geom_);
    }

    inline raster_ptr const& get_raster() const
    {
        return raster_;
    }

    inline void set_raster(raster_ptr const& raster)
    {
        raster_ = raster;
    }

    inline feature_kv_iterator begin() const
    {
        return feature_kv_iterator(*this,true);
    }

    inline feature_kv_iterator end() const
    {
        return feature_kv_iterator(*this);
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "Feature ( id=" << id_ << std::endl;
        for (auto const& kv : ctx_->mapping_)
        {
            std::size_t index = kv.second;
            if (index < data_.size())
            {
                if (data_[kv.second] == mapnik::value_null())
                {
                    ss << "  " << kv.first  << ":null" << std::endl;
                }
                else
                {
                    ss << "  " << kv.first  << ":" <<  data_[kv.second] << std::endl;
                }
            }
        }
        ss << ")" << std::endl;
        return ss.str();
    }

private:
    mapnik::value_integer id_;
    context_ptr ctx_;
    cont_type data_;
    geometry::geometry<double> geom_;
    raster_ptr raster_;
};


inline std::ostream& operator<< (std::ostream & out,feature_impl const& f)
{
    out << f.to_string();
    return out;
}

using feature_ptr = std::shared_ptr<feature_impl>;

}

#endif // MAPNIK_FEATURE_HPP
