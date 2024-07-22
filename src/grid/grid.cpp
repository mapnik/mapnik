/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/grid/grid.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/value.hpp>

// boost

namespace mapnik {

template<typename T>
const typename hit_grid<T>::value_type hit_grid<T>::base_mask = std::numeric_limits<typename T::type>::min();

template<typename T>
hit_grid<T>::hit_grid(std::size_t width, std::size_t height, std::string const& key)
    : width_(width)
    , height_(height)
    , key_(key)
    , data_(width, height)
    , id_name_("__id__")
    , painted_(false)
    , names_()
    , f_keys_()
    , features_()
    , ctx_(std::make_shared<mapnik::context_type>())
{
    f_keys_[base_mask] = "";
    data_.set(base_mask);
}

template<typename T>
hit_grid<T>::hit_grid(hit_grid<T> const& rhs)
    : width_(rhs.width_)
    , height_(rhs.height_)
    , key_(rhs.key_)
    , data_(rhs.data_)
    , id_name_("__id__")
    , painted_(rhs.painted_)
    , names_(rhs.names_)
    , f_keys_(rhs.f_keys_)
    , features_(rhs.features_)
    , ctx_(rhs.ctx_)
{
    f_keys_[base_mask] = "";
    data_.set(base_mask);
}

template<typename T>
void hit_grid<T>::clear()
{
    painted_ = false;
    f_keys_.clear();
    features_.clear();
    names_.clear();
    f_keys_[base_mask] = "";
    data_.set(base_mask);
    ctx_ = std::make_shared<mapnik::context_type>();
}

template<typename T>
void hit_grid<T>::add_feature(mapnik::feature_impl const& feature)
{
    value_type feature_id = feature.id();
    // avoid adding duplicate features (e.g. in the case of both a line symbolizer and a polygon symbolizer)
    typename feature_key_type::const_iterator feature_pos = f_keys_.find(feature_id);
    if (feature_pos != f_keys_.end())
    {
        return;
    }

    if (ctx_->size() == 0)
    {
        context_type::map_type::const_iterator itr = feature.context()->begin();
        context_type::map_type::const_iterator end = feature.context()->end();
        for (; itr != end; ++itr)
        {
            ctx_->add(itr->first, itr->second);
        }
    }
    // NOTE: currently lookup keys must be strings,
    // but this should be revisited
    lookup_type lookup_value;
    if (key_ == id_name_)
    {
        mapnik::util::to_string(lookup_value, feature_id);
    }
    else
    {
        if (feature.has_key(key_))
        {
            lookup_value = feature.get(key_).to_string();
        }
        else
        {
            MAPNIK_LOG_DEBUG(grid) << "hit_grid: Should not get here: key '" << key_
                                   << "' not found in feature properties";
        }
    }

    if (!lookup_value.empty())
    {
        // TODO - consider shortcutting f_keys if feature_id == lookup_value
        // create a mapping between the pixel id and the feature key
        f_keys_.emplace(feature_id, lookup_value);
        // if extra fields have been supplied, push them into grid memory
        if (!names_.empty())
        {
            // it is ~ 2x faster to copy feature attributes compared
            // to building up a in-memory cache of feature_ptrs
            // https://github.com/mapnik/mapnik/issues/1198
            mapnik::feature_ptr feature2(mapnik::feature_factory::create(ctx_, feature_id));
            feature2->set_data(feature.get_data());
            features_.emplace(lookup_value, feature2);
        }
    }
    else
    {
        MAPNIK_LOG_DEBUG(grid) << "hit_grid: Warning - key '" << key_ << "' was blank for " << feature;
    }
}

template class MAPNIK_DECL hit_grid<mapnik::value_integer_pixel>;

} // namespace mapnik

#endif
