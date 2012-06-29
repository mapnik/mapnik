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

#ifndef MAPNIK_GRID_HPP
#define MAPNIK_GRID_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/grid/grid_view.hpp>
#include <mapnik/global.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/cstdint.hpp>

// stl
#include <map>
#include <set>
#include <cmath>
#include <string>
#include <cassert>
#include <vector>

namespace mapnik
{

template <typename T>
class MAPNIK_DECL hit_grid
{
public:
    typedef T value_type;
    typedef mapnik::ImageData<value_type> data_type;
    typedef std::string lookup_type;
    // mapping between pixel id and key
    typedef std::map<value_type, lookup_type> feature_key_type;
    typedef std::map<lookup_type, value_type> key_type;
    typedef std::map<lookup_type, mapnik::feature_ptr> feature_type;
    static const value_type base_mask;

private:
    unsigned width_;
    unsigned height_;
    std::string key_;
    data_type data_;
    unsigned int resolution_;
    std::string id_name_;
    bool painted_;
    std::set<std::string> names_;
    feature_key_type f_keys_;
    feature_type features_;
    mapnik::context_ptr ctx_;

public:

    hit_grid(int width, int height, std::string const& key, unsigned int resolution)
        :width_(width),
        height_(height),
        key_(key),
        data_(width,height),
        resolution_(resolution),
        id_name_("__id__"),
        painted_(false),
        names_(),
        f_keys_(),
        features_(),
        ctx_(boost::make_shared<mapnik::context_type>())
        {
            f_keys_[base_mask] = "";
            data_.set(base_mask);
        }

    hit_grid(const hit_grid<T>& rhs)
        :width_(rhs.width_),
        height_(rhs.height_),
        key_(rhs.key_),
        data_(rhs.data_),
        resolution_(rhs.resolution_),
        id_name_("__id__"),
        painted_(rhs.painted_),
        names_(rhs.names_),
        f_keys_(rhs.f_keys_),
        features_(rhs.features_),
        ctx_(rhs.ctx_)
        {
            f_keys_[base_mask] = "";
            data_.set(base_mask);
        }

    ~hit_grid() {}

    inline void painted(bool painted)
    {
        painted_ = painted;
    }

    inline bool painted() const
    {
        return painted_;
    }

    inline std::string const& key_name() const
    {
        return id_name_;
    }

    inline void add_feature(mapnik::feature_impl & feature)
    {
        int feature_id = feature.id();
        // avoid adding duplicate features (e.g. in the case of both a line symbolizer and a polygon symbolizer)
        typename feature_key_type::const_iterator feature_pos = f_keys_.find(feature_id);
        if (feature_pos != f_keys_.end())
        {
            return;
        }

        if (ctx_->size() == 0) {
            mapnik::feature_impl::iterator itr = feature.begin();
            mapnik::feature_impl::iterator end = feature.end();
            for ( ;itr!=end; ++itr)
            {
                ctx_->push(boost::get<0>(*itr));
            }
        }
        // NOTE: currently lookup keys must be strings,
        // but this should be revisited
        lookup_type lookup_value;
        if (key_ == id_name_)
        {
            mapnik::util::to_string(lookup_value,feature_id);
        }
        else
        {
            if (feature.has_key(key_))
            {
                lookup_value = feature.get(key_).to_string();
            }
            else
            {
                MAPNIK_LOG_DEBUG(grid) << "hit_grid: Should not get here: key '" << key_ << "' not found in feature properties";
            }
        }

        if (!lookup_value.empty())
        {
            // TODO - consider shortcutting f_keys if feature_id == lookup_value
            // create a mapping between the pixel id and the feature key
            f_keys_.insert(std::make_pair(feature_id,lookup_value));
            // if extra fields have been supplied, push them into grid memory
            if (!names_.empty())
            {
                // it is ~ 2x faster to copy feature attributes compared
                // to building up a in-memory cache of feature_ptrs
                // https://github.com/mapnik/mapnik/issues/1198
                mapnik::feature_ptr feature2(mapnik::feature_factory::create(ctx_,feature_id));
                feature2->set_data(feature.get_data());
                features_.insert(std::make_pair(lookup_value,feature2));
            }
        }
        else
        {
            MAPNIK_LOG_DEBUG(grid) << "hit_grid: Warning - key '" << key_ << "' was blank for " << feature;
        }
    }

    inline void add_property_name(std::string const& name)
    {
        names_.insert(name);
    }

    inline std::set<std::string> const& property_names() const
    {
        return names_;
    }

    inline const feature_type& get_grid_features() const
    {
        return features_;
    }

    inline feature_type& get_grid_features()
    {
        return features_;
    }

    inline const feature_key_type& get_feature_keys() const
    {
        return f_keys_;
    }

    inline feature_key_type& get_feature_keys()
    {
        return f_keys_;
    }

    inline const std::string& get_key() const
    {
        return key_;
    }

    inline void set_key(std::string const& key)
    {
        key_ = key;
    }

    inline unsigned int get_resolution() const
    {
        return resolution_;
    }

    inline void set_resolution(unsigned int res)
    {
        resolution_ = res;
    }

    inline const data_type& data() const
    {
        return data_;
    }

    inline data_type& data()
    {
        return data_;
    }

    inline const T* raw_data() const
    {
        return data_.getData();
    }

    inline T* raw_data()
    {
        return data_.getData();
    }

    inline const value_type* getRow(unsigned row) const
    {
        return data_.getRow(row);
    }

    inline mapnik::grid_view get_view(unsigned x, unsigned y, unsigned w, unsigned h)
    {
        return mapnik::grid_view(x,y,w,h,
                                 data_,key_,id_name_,resolution_,names_,f_keys_,features_);
    }

private:

    inline bool checkBounds(unsigned x, unsigned y) const
    {
        return (x < width_ && y < height_);
    }

    hit_grid& operator=(const hit_grid&);

public:
    inline void setPixel(int x,int y,value_type feature_id)
    {
        if (checkBounds(x,y))
        {
            data_(x,y) = feature_id;
        }
    }
    inline unsigned width() const
    {
        return width_;
    }

    inline unsigned height() const
    {
        return height_;
    }

    inline void blendPixel(value_type feature_id,int x,int y,unsigned int rgba1,int t)
    {
        blendPixel2(feature_id ,x,y,rgba1,t,1.0);  // do not change opacity
    }

    inline void blendPixel2(value_type feature_id,int x,int y,unsigned int rgba1,int t,double opacity)
    {
        if (checkBounds(x,y))
        {

#ifdef MAPNIK_BIG_ENDIAN
            unsigned a = (int)((rgba1 & 0xff) * opacity) & 0xff; // adjust for desired opacity
#else
            unsigned a = (int)(((rgba1 >> 24) & 0xff) * opacity) & 0xff; // adjust for desired opacity
#endif
            // if the pixel is more than a tenth
            // opaque then burn in the feature id
            if (a >= 25)
            {
                data_(x,y) = feature_id;
            }
        }
    }

    inline void set_rectangle(value_type id,image_data_32 const& data,int x0,int y0)
    {
        box2d<int> ext0(0,0,width_,height_);
        box2d<int> ext1(x0,y0,x0+data.width(),y0+data.height());

        if (ext0.intersects(ext1))
        {
            box2d<int> box = ext0.intersect(ext1);
            for (int y = box.miny(); y < box.maxy(); ++y)
            {
                value_type* row_to =  data_.getRow(y);
                unsigned int const * row_from = data.getRow(y-y0);

                for (int x = box.minx(); x < box.maxx(); ++x)
                {
                    unsigned rgba = row_from[x-x0];
#ifdef MAPNIK_BIG_ENDIAN
                    unsigned a = rgba & 0xff;
#else
                    unsigned a = (rgba >> 24) & 0xff;
#endif
                    // if the pixel is more than a tenth
                    // opaque then burn in the feature id
                    if (a >= 25)
                    {
                        row_to[x] = id;
                    }
                }
            }
        }
    }

};

typedef MAPNIK_DECL hit_grid<int> grid;

}
#endif //MAPNIK_GRID_HPP
