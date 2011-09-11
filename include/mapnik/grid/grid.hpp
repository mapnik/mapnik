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
#include <mapnik/image_data.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/grid/grid_view.hpp>
#include <mapnik/global.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>

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
    typedef std::map<std::string, mapnik::value> feature_properties_type;
    // note: feature_type is not the same as a mapnik::Feature as it lacks a geometry
    typedef std::map<std::string, feature_properties_type > feature_type;
    
private:
    unsigned width_;
    unsigned height_;
    std::string key_;
    feature_key_type f_keys_;
    feature_type features_;
    data_type data_;
    std::set<std::string> names_;
    unsigned int resolution_;
    bool painted_;
    
public:

    std::string id_name_;

    hit_grid(int width, int height, std::string const& key, unsigned int resolution)
        :width_(width),
         height_(height),
         key_(key),
         data_(width,height),
         resolution_(resolution),
         id_name_("__id__") {
             // this only works if each datasource's 
             // feature count starts at 1
             f_keys_[0] = "";
         }
    
    hit_grid(const hit_grid<T>& rhs)
        :width_(rhs.width_),
         height_(rhs.height_),
         key_(rhs.key_),
         data_(rhs.data_),
         resolution_(rhs.resolution_),
         id_name_("__id__")  {
             f_keys_[0] = "";     
         }
    
    ~hit_grid() {}

    void painted(bool painted)
    {
        painted_ = painted;
    }

    bool painted() const
    {
        return painted_;
    }

    void add_feature(mapnik::Feature const& feature)
    {

        // copies feature props
        std::map<std::string,value> fprops = feature.props();
        lookup_type lookup_value;
        if (key_ == id_name_)
        {
            // TODO - this will break if lookup_type is not a string
            std::stringstream s;
            s << feature.id();
            lookup_value = s.str();
            // add this as a proper feature so filtering works later on
            fprops[id_name_] = feature.id();
            //fprops[id_name_] = tr_->transcode(lookup_value));
        }
        else
        {
            std::map<std::string,value>::const_iterator const& itr = fprops.find(key_);
            if (itr != fprops.end())
            {
                lookup_value = itr->second.to_string();
            }
            else
            {
                std::clog << "should not get here: key '" << key_ << "' not found in feature properties\n";
            }    
        }

        // what good is an empty lookup key?
        if (!lookup_value.empty())
        {
            // TODO - consider shortcutting f_keys if feature_id == lookup_value
            // create a mapping between the pixel id and the feature key
            f_keys_.insert(std::make_pair(feature.id(),lookup_value));
            // if extra fields have been supplied, push them into grid memory
            if (!names_.empty()) {
                // TODO - add ability to push WKT/WKB of geometry into grid storage
                features_.insert(std::make_pair(lookup_value,fprops));
            }
        }
        else
        {
            std::clog << "### Warning: key '" << key_ << "' was blank for " << feature << "\n";
        }
    } 
    
    void add_property_name(std::string const& name)
    {
        names_.insert(name);
    } 

    std::set<std::string> property_names() const
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

    // compatibility
    inline const std::string& get_join_field() const
    {
        return key_;
    }

    // compatibility
    inline unsigned int get_step() const
    {
        return resolution_;
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
            data_,key_,resolution_,names_,f_keys_,features_);
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
        blendPixel2(x,y,rgba1,t,1.0);  // do not change opacity
    }

    inline void blendPixel2(value_type feature_id,int x,int y,unsigned int rgba1,int t,double opacity)
    {
        if (checkBounds(x,y))
        {

#ifdef MAPNIK_BIG_ENDIAN
            unsigned a1 = (int)((rgba1 & 0xff) * opacity) & 0xff; // adjust for desired opacity
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

typedef hit_grid<uint16_t> grid;

}
#endif //MAPNIK_GRID_HPP
