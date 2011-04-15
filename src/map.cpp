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

//$Id: map.cpp 17 2005-03-08 23:58:43Z pavlenko $,
#include <mapnik/map.hpp>

#include <mapnik/style.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/filter_featureset.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/scale_denominator.hpp>

namespace mapnik
{

/** Call cache_metawriters for each symbolizer.*/
struct metawriter_cache_dispatch : public boost::static_visitor<>
{
    metawriter_cache_dispatch (Map const &m) : m_(m) {}

    template <typename T> void operator () (T &sym) const
    {
        sym.cache_metawriters(m_);
    }

    Map const &m_;
};

static const char * aspect_fix_mode_strings[] = {
    "GROW_BBOX",
    "GROW_CANVAS",
    "SHRINK_BBOX",
    "SHRINK_CANVAS",
    "ADJUST_BBOX_WIDTH",
    "ADJUST_BBOX_HEIGHT",
    "ADJUST_CANVAS_WIDTH",
    "ADJUST_CANVAS_HEIGHT",
    ""
};
   
IMPLEMENT_ENUM( aspect_fix_mode_e, aspect_fix_mode_strings )

Map::Map()
    : width_(400),
      height_(400),
      srs_("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
      buffer_size_(0),
      aspectFixMode_(GROW_BBOX) {}
    
Map::Map(int width,int height, std::string const& srs)
    : width_(width),
      height_(height),
      srs_(srs),
      buffer_size_(0),
      aspectFixMode_(GROW_BBOX) {}
   
Map::Map(const Map& rhs)
    : width_(rhs.width_),
      height_(rhs.height_),
      srs_(rhs.srs_),
      buffer_size_(rhs.buffer_size_),
      background_(rhs.background_),
      background_image_(rhs.background_image_),
      styles_(rhs.styles_),
      metawriters_(rhs.metawriters_),
      layers_(rhs.layers_),
      aspectFixMode_(rhs.aspectFixMode_),
      current_extent_(rhs.current_extent_),
      maximum_extent_(rhs.maximum_extent_),
      extra_attr_(rhs.extra_attr_) {}
    
Map& Map::operator=(const Map& rhs)
{
    if (this==&rhs) return *this;
    width_=rhs.width_;
    height_=rhs.height_;
    srs_=rhs.srs_;
    buffer_size_ = rhs.buffer_size_;
    background_=rhs.background_;
    background_image_=rhs.background_image_;
    styles_=rhs.styles_;
    metawriters_ = rhs.metawriters_;
    layers_=rhs.layers_;
    aspectFixMode_=rhs.aspectFixMode_;
    maximum_extent_=rhs.maximum_extent_;
    extra_attr_=rhs.extra_attr_;
    return *this;
}
   
std::map<std::string,feature_type_style> const& Map::styles() const
{
    return styles_;
}
   
std::map<std::string,feature_type_style> & Map::styles()
{
    return styles_;
}
   
Map::style_iterator Map::begin_styles()
{
    return styles_.begin();
}
    
Map::style_iterator Map::end_styles()
{
    return styles_.end();
}
    
Map::const_style_iterator  Map::begin_styles() const
{
    return styles_.begin();
}
    
Map::const_style_iterator  Map::end_styles() const
{
    return styles_.end();
}
    
bool Map::insert_style(std::string const& name,feature_type_style const& style) 
{
    return styles_.insert(make_pair(name,style)).second;
}
    
void Map::remove_style(std::string const& name) 
{
    styles_.erase(name);
}

boost::optional<feature_type_style const&> Map::find_style(std::string const& name) const
{
    std::map<std::string,feature_type_style>::const_iterator itr = styles_.find(name);
    if (itr != styles_.end())
        return boost::optional<feature_type_style const&>(itr->second);
    else
        return boost::optional<feature_type_style const&>() ;
}

bool Map::insert_metawriter(std::string const& name, metawriter_ptr const& writer)
{
    return metawriters_.insert(make_pair(name, writer)).second;
}

void Map::remove_metawriter(std::string const& name)
{
    metawriters_.erase(name);
}

metawriter_ptr Map::find_metawriter(std::string const& name) const
{
    std::map<std::string, metawriter_ptr>::const_iterator itr = metawriters_.find(name);
    if (itr != metawriters_.end())
        return itr->second;
    else
        return metawriter_ptr();
}

std::map<std::string,metawriter_ptr> const& Map::metawriters() const
{
    return metawriters_;
}

Map::const_metawriter_iterator Map::begin_metawriters() const
{
    return metawriters_.begin();
}

Map::const_metawriter_iterator Map::end_metawriters() const
{
    return metawriters_.end();
}

bool Map::insert_fontset(std::string const& name, font_set const& fontset) 
{
    return fontsets_.insert(make_pair(name, fontset)).second;
}
         
font_set const& Map::find_fontset(std::string const& name) const
{
    std::map<std::string,font_set>::const_iterator itr = fontsets_.find(name);
    if (itr!=fontsets_.end())
        return itr->second;
    static font_set default_fontset;
    return default_fontset;
}

std::map<std::string,font_set> const& Map::fontsets() const
{
    return fontsets_;
}

std::map<std::string,font_set> & Map::fontsets()
{
    return fontsets_;
}

size_t Map::layer_count() const
{
    return layers_.size();
}
    
void Map::addLayer(const layer& l)
{
    layers_.push_back(l);
}

void Map::removeLayer(size_t index)
{
    layers_.erase(layers_.begin()+index);
}
    
void Map::remove_all() 
{
    layers_.clear();
    styles_.clear();
    metawriters_.clear();
}
    
const layer& Map::getLayer(size_t index) const
{
    return layers_[index];
}

layer& Map::getLayer(size_t index)
{
    return layers_[index];
}

std::vector<layer> const& Map::layers() const
{
    return layers_;
}

std::vector<layer> & Map::layers()
{
    return layers_;
}

unsigned Map::width() const
{
    return width_;
}

unsigned Map::height() const
{
    return height_;
}
    
void Map::set_width(unsigned width)
{
    if (width >= MIN_MAPSIZE && width <= MAX_MAPSIZE)
    {
        width_=width;
        fixAspectRatio();
    }   
}

void Map::set_height(unsigned height)
{
    if (height >= MIN_MAPSIZE && height <= MAX_MAPSIZE)
    {
        height_=height;
        fixAspectRatio();
    }
}
    
void Map::resize(unsigned width,unsigned height)
{
    if (width >= MIN_MAPSIZE && width <= MAX_MAPSIZE &&
        height >= MIN_MAPSIZE && height <= MAX_MAPSIZE)
    {
        width_=width;
        height_=height;
        fixAspectRatio();
    }
}

std::string const&  Map::srs() const
{
    return srs_;
}
    
void Map::set_srs(std::string const& srs)
{
    srs_ = srs;
}
   
void Map::set_buffer_size( int buffer_size)
{
    buffer_size_ = buffer_size;
}

int Map::buffer_size() const
{
    return buffer_size_;
}
   
boost::optional<color> const& Map::background() const
{
    return background_;
}
   
void Map::set_background(const color& c)
{
    background_ = c;
}
   
boost::optional<std::string> const& Map::background_image() const
{
    return background_image_;
}
   
void Map::set_background_image(std::string const& image_filename)
{
    background_image_ = image_filename;
}

void Map::set_maximum_extent(box2d<double> const& box)
{
    maximum_extent_ = box;
}
        
boost::optional<box2d<double> > const& Map::maximum_extent() const
{
    return maximum_extent_;
}

void Map::zoom(double factor)
{
    coord2d center = current_extent_.center();
    double w = factor * current_extent_.width();
    double h = factor * current_extent_.height();
    current_extent_ = box2d<double>(center.x - 0.5 * w, 
                                   center.y - 0.5 * h,
                                   center.x + 0.5 * w, 
                                   center.y + 0.5 * h);
    fixAspectRatio();
}
    
void Map::zoom_all() 
{
    if (maximum_extent_) {
        zoom_to_box(*maximum_extent_);
    }
    else
    {
        try 
        {
            projection proj0(srs_);
            box2d<double> ext;
            bool success = false;
            bool first = true;
            std::vector<layer>::const_iterator itr = layers_.begin();
            std::vector<layer>::const_iterator end = layers_.end();
            while (itr != end)
            {
                if (itr->isActive())
                {
                    std::string const& layer_srs = itr->srs();
                    projection proj1(layer_srs);
                    
                    proj_transform prj_trans(proj0,proj1);
                        
                    box2d<double> layer_ext = itr->envelope();
                    if (prj_trans.backward(layer_ext))
                    {
                        success = true;
            #ifdef MAPNIK_DEBUG
                        std::clog << " layer " << itr->name() << " original ext: " << itr->envelope() << "\n";
                        std::clog << " layer " << itr->name() << " transformed to map srs: " << layer_ext << "\n";
            #endif                
                        if (first)
                        {
                            ext = layer_ext;
                            first = false;
                        }
                        else 
                        {
                            ext.expand_to_include(layer_ext);
                        }
                    }
                }
                ++itr;
            }
            if (success) {
                zoom_to_box(ext);
            } else {
                std::ostringstream s;
                s << "could not zoom to combined layer extents "
                  << "using zoom_all because proj4 could not "
                  << "back project any layer extents into the map srs "
                  << "(set map 'maximum-extent' to override layer extents)";
                throw std::runtime_error(s.str());
            }
        }
        catch (proj_init_error & ex)
        {
            std::clog << "proj_init_error:" << ex.what() << "\n";
        }    
    }
}

void Map::zoom_to_box(const box2d<double> &box)
{
    current_extent_=box;
    fixAspectRatio();
}

void Map::fixAspectRatio()
{
    double ratio1 = (double) width_ / (double) height_;
    double ratio2 = current_extent_.width() / current_extent_.height();
    if (ratio1 == ratio2) return;

    switch(aspectFixMode_) 
    {
    case ADJUST_BBOX_HEIGHT:
        current_extent_.height(current_extent_.width() / ratio1);
        break;
    case ADJUST_BBOX_WIDTH:
        current_extent_.width(current_extent_.height() * ratio1);
        break;
    case ADJUST_CANVAS_HEIGHT:
        height_ = int (width_ / ratio2 + 0.5); 
        break;
    case ADJUST_CANVAS_WIDTH:
        width_ = int (height_ * ratio2 + 0.5); 
        break;
    case GROW_BBOX:
        if (ratio2 > ratio1)
            current_extent_.height(current_extent_.width() / ratio1);
        else 
            current_extent_.width(current_extent_.height() * ratio1);
        break;  
    case SHRINK_BBOX:
        if (ratio2 < ratio1)
            current_extent_.height(current_extent_.width() / ratio1);
        else 
            current_extent_.width(current_extent_.height() * ratio1);
        break;  
    case GROW_CANVAS:
        if (ratio2 > ratio1)
            width_ = (int) (height_ * ratio2 + 0.5);
        else
            height_ = int (width_ / ratio2 + 0.5); 
        break;
    case SHRINK_CANVAS:
        if (ratio2 > ratio1)
            height_ = int (width_ / ratio2 + 0.5); 
        else
            width_ = (int) (height_ * ratio2 + 0.5);
        break;
    default:
        if (ratio2 > ratio1)
            current_extent_.height(current_extent_.width() / ratio1);
        else 
            current_extent_.width(current_extent_.height() * ratio1);
        break;  
    }
}
   
const box2d<double>& Map::get_current_extent() const
{
    return current_extent_;
}

box2d<double> Map::get_buffered_extent() const
{
    double extra = 2.0 * scale() * buffer_size_;
    box2d<double> ext(current_extent_);
    ext.width(current_extent_.width() + extra);
    ext.height(current_extent_.height() + extra);
    return ext;
}
   
void Map::pan(int x,int y)
{
    int dx = x - int(0.5 * width_);
    int dy = int(0.5 * height_) - y;
    double s = width_/current_extent_.width();
    double minx  = current_extent_.minx() + dx/s;
    double maxx  = current_extent_.maxx() + dx/s;
    double miny  = current_extent_.miny() + dy/s;
    double maxy  = current_extent_.maxy() + dy/s;
    current_extent_.init(minx,miny,maxx,maxy);
}

void Map::pan_and_zoom(int x,int y,double factor)
{
    pan(x,y);
    zoom(factor);
}

double Map::scale() const
{
    if (width_>0)
        return current_extent_.width()/width_;
    return current_extent_.width();
}

double Map::scale_denominator() const 
{
    projection map_proj(srs_);
    return mapnik::scale_denominator( *this, map_proj.is_geographic());    
}

CoordTransform Map::view_transform() const
{
    return CoordTransform(width_,height_,current_extent_);
}
    
featureset_ptr Map::query_point(unsigned index, double x, double y) const
{
    if ( index< layers_.size())
    {
        mapnik::layer const& layer = layers_[index];    
        try
        {
            double z = 0;
            mapnik::projection dest(srs_);
            mapnik::projection source(layer.srs());
            proj_transform prj_trans(source,dest);
            prj_trans.backward(x,y,z);
                
            double minx = current_extent_.minx();
            double miny = current_extent_.miny();
            double maxx = current_extent_.maxx();
            double maxy = current_extent_.maxy();
                
            prj_trans.backward(minx,miny,z);
            prj_trans.backward(maxx,maxy,z);
            double tol = (maxx - minx) / width_ * 3;
            mapnik::datasource_ptr ds = layer.datasource();
            if (ds)
            {
#ifdef MAPNIK_DEBUG
                std::clog << " query at point tol = " << tol << " (" << x << "," << y << ")\n";
#endif    
                featureset_ptr fs = ds->features_at_point(mapnik::coord2d(x,y));
                if (fs) 
                    return featureset_ptr(new filter_featureset<hit_test_filter>(fs,hit_test_filter(x,y,tol)));
            }
        }
        catch (...)
        {
#ifdef MAPNIK_DEBUG
            std::clog << "exception caught in \"query_map_point\"\n";
#endif
        }
    }
    return featureset_ptr();
}
    
featureset_ptr Map::query_map_point(unsigned index, double x, double y) const
{
    if ( index< layers_.size())
    {
        mapnik::layer const& layer = layers_[index];
        CoordTransform tr = view_transform();
        tr.backward(&x,&y);
            
        try
        {
            mapnik::projection dest(srs_);
            mapnik::projection source(layer.srs());
            proj_transform prj_trans(source,dest);
            double z = 0;
            prj_trans.backward(x,y,z);
                
            double minx = current_extent_.minx();
            double miny = current_extent_.miny();
            double maxx = current_extent_.maxx();
            double maxy = current_extent_.maxy();
                
            prj_trans.backward(minx,miny,z);
            prj_trans.backward(maxx,maxy,z);
            double tol = (maxx - minx) / width_ * 3;
            mapnik::datasource_ptr ds = layer.datasource();
            if (ds)
            {
#ifdef MAPNIK_DEBUG
                std::clog << " query at point tol = " << tol << " (" << x << "," << y << ")\n";
#endif
                featureset_ptr fs = ds->features_at_point(mapnik::coord2d(x,y));
                if (fs) 
                    return featureset_ptr(new filter_featureset<hit_test_filter>(fs,hit_test_filter(x,y,tol)));
            }
        }
        catch (...)
        {
#ifdef MAPNIK_DEBUG
            std::clog << "exception caught in \"query_map_point\"\n";
#endif
        }
    }
    return featureset_ptr();
}

Map::~Map() {}

void Map::init_metawriters()
{
    metawriter_cache_dispatch d(*this);
    Map::style_iterator styIter = begin_styles();
    Map::style_iterator styEnd = end_styles();
    for (; styIter!=styEnd; ++styIter) {
        std::vector<rule>& rules = styIter->second.get_rules_nonconst();
        std::vector<rule>::iterator ruleIter = rules.begin();
        std::vector<rule>::iterator ruleEnd = rules.end();
        for (; ruleIter!=ruleEnd; ++ruleIter) {
            rule::symbolizers::iterator symIter = ruleIter->begin();
            rule::symbolizers::iterator symEnd = ruleIter->end();
            for (; symIter!=symEnd; ++symIter) {
                boost::apply_visitor(d, *symIter);
            }
        }
    }
}

void Map::set_metawriter_property(std::string name, std::string value)
{
    metawriter_output_properties[name] = UnicodeString::fromUTF8(value);
}

std::string Map::get_metawriter_property(std::string name) const
{
    std::string result;
    to_utf8(metawriter_output_properties[name], result);
    return result;
}

parameters const& Map::get_extra_attributes() const
{
    return extra_attr_;
}

void Map::set_extra_attributes(parameters& params)
{
    extra_attr_ = params;
}

}
