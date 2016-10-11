/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/rule.hpp>
#include <mapnik/color.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/map.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/filter_featureset.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/config.hpp> // for PROJ_ENVELOPE_POINTS
#include <mapnik/text/font_library.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/font_engine_freetype.hpp>

// stl
#include <stdexcept>

namespace mapnik
{

static const char * aspect_fix_mode_strings[] = {
    "GROW_BBOX",
    "GROW_CANVAS",
    "SHRINK_BBOX",
    "SHRINK_CANVAS",
    "ADJUST_BBOX_WIDTH",
    "ADJUST_BBOX_HEIGHT",
    "ADJUST_CANVAS_WIDTH",
    "ADJUST_CANVAS_HEIGHT",
    "RESPECT",
    ""
};

IMPLEMENT_ENUM( aspect_fix_mode_e, aspect_fix_mode_strings )

Map::Map()
: width_(400),
    height_(400),
    srs_(MAPNIK_LONGLAT_PROJ),
    buffer_size_(0),
    background_image_comp_op_(src_over),
    background_image_opacity_(1.0),
    aspectFixMode_(GROW_BBOX),
    base_path_(""),
    extra_params_(),
    font_directory_(),
    font_file_mapping_(),
    font_memory_cache_() {}

Map::Map(int width,int height, std::string const& srs)
    : width_(width),
      height_(height),
      srs_(srs),
      buffer_size_(0),
      background_image_comp_op_(src_over),
      background_image_opacity_(1.0),
      aspectFixMode_(GROW_BBOX),
      base_path_(""),
      extra_params_(),
      font_directory_(),
      font_file_mapping_(),
      font_memory_cache_() {}

Map::Map(Map const& rhs)
    : width_(rhs.width_),
      height_(rhs.height_),
      srs_(rhs.srs_),
      buffer_size_(rhs.buffer_size_),
      background_(rhs.background_),
      background_image_(rhs.background_image_),
      background_image_comp_op_(rhs.background_image_comp_op_),
      background_image_opacity_(rhs.background_image_opacity_),
      styles_(rhs.styles_),
      fontsets_(rhs.fontsets_),
      layers_(rhs.layers_),
      aspectFixMode_(rhs.aspectFixMode_),
      current_extent_(rhs.current_extent_),
      maximum_extent_(rhs.maximum_extent_),
      base_path_(rhs.base_path_),
      extra_params_(rhs.extra_params_),
      font_directory_(rhs.font_directory_),
      font_file_mapping_(rhs.font_file_mapping_),
      // on copy discard memory cache
      font_memory_cache_() {}


Map::Map(Map && rhs)
    : width_(std::move(rhs.width_)),
      height_(std::move(rhs.height_)),
      srs_(std::move(rhs.srs_)),
      buffer_size_(std::move(rhs.buffer_size_)),
      background_(std::move(rhs.background_)),
      background_image_(std::move(rhs.background_image_)),
      background_image_comp_op_(std::move(rhs.background_image_comp_op_)),
      background_image_opacity_(std::move(rhs.background_image_opacity_)),
      styles_(std::move(rhs.styles_)),
      fontsets_(std::move(rhs.fontsets_)),
      layers_(std::move(rhs.layers_)),
      aspectFixMode_(std::move(rhs.aspectFixMode_)),
      current_extent_(std::move(rhs.current_extent_)),
      maximum_extent_(std::move(rhs.maximum_extent_)),
      base_path_(std::move(rhs.base_path_)),
      extra_params_(std::move(rhs.extra_params_)),
      font_directory_(std::move(rhs.font_directory_)),
      font_file_mapping_(std::move(rhs.font_file_mapping_)),
      font_memory_cache_(std::move(rhs.font_memory_cache_)) {}

Map::~Map() {}

Map& Map::operator=(Map rhs)
{
    swap(*this, rhs);
    return *this;
}

void swap (Map & lhs, Map & rhs)
{
    using std::swap;
    std::swap(lhs.width_,  rhs.width_);
    std::swap(lhs.height_, rhs.height_);
    std::swap(lhs.srs_, rhs.srs_);
    std::swap(lhs.buffer_size_, rhs.buffer_size_);
    std::swap(lhs.background_, rhs.background_);
    std::swap(lhs.background_image_, rhs.background_image_);
    std::swap(lhs.background_image_comp_op_, rhs.background_image_comp_op_);
    std::swap(lhs.background_image_opacity_, rhs.background_image_opacity_);
    std::swap(lhs.styles_, rhs.styles_);
    std::swap(lhs.fontsets_, rhs.fontsets_);
    std::swap(lhs.layers_, rhs.layers_);
    std::swap(lhs.aspectFixMode_, rhs.aspectFixMode_);
    std::swap(lhs.current_extent_, rhs.current_extent_);
    std::swap(lhs.maximum_extent_, rhs.maximum_extent_);
    std::swap(lhs.base_path_, rhs.base_path_);
    std::swap(lhs.extra_params_, rhs.extra_params_);
    std::swap(lhs.font_directory_,rhs.font_directory_);
    std::swap(lhs.font_file_mapping_,rhs.font_file_mapping_);
    // on assignment discard memory cache
    //std::swap(lhs.font_memory_cache_,rhs.font_memory_cache_);
}

bool Map::operator==(Map const& rhs) const
{
    return (width_ == rhs.width_) &&
        (height_ == rhs.height_) &&
        (srs_ == rhs.srs_) &&
        (buffer_size_ == rhs.buffer_size_) &&
        (background_ == rhs.background_) &&
        (background_image_ == rhs.background_image_) &&
        (background_image_comp_op_ == rhs.background_image_comp_op_) &&
        (background_image_opacity_ == rhs.background_image_opacity_) &&
        (styles_ == rhs.styles_) &&
        (fontsets_ == rhs.fontsets_) &&
        (layers_ == rhs.layers_) &&
        (aspectFixMode_ == rhs.aspectFixMode_) &&
        (current_extent_ == rhs.current_extent_) &&
        (maximum_extent_ == rhs.maximum_extent_) &&
        (base_path_ == rhs.base_path_) &&
        (extra_params_ == rhs.extra_params_) &&
        (font_directory_ == rhs.font_directory_) &&
        (font_file_mapping_ == rhs.font_file_mapping_);
        // Note: we don't care about font_memory_cache in comparison
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

Map::const_style_iterator Map::begin_styles() const
{
    return styles_.begin();
}

Map::const_style_iterator Map::end_styles() const
{
    return styles_.end();
}

bool Map::insert_style(std::string const& name, feature_type_style const& style)
{
    return styles_.emplace(name, style).second;
}

bool Map::insert_style(std::string const& name, feature_type_style && style)
{
    return styles_.emplace(name, std::move(style)).second;
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

bool Map::insert_fontset(std::string const& name, font_set const& fontset)
{
    if (fontset.get_name() != name)
    {
        throw mapnik::config_error("Fontset name must match the name used to reference it on the map");
    }
    return fontsets_.emplace(name, fontset).second;
}

bool Map::insert_fontset(std::string const& name, font_set && fontset)
{
    if (fontset.get_name() != name)
    {
        throw mapnik::config_error("Fontset name must match the name used to reference it on the map");
    }
    return fontsets_.emplace(name, std::move(fontset)).second;
}

boost::optional<font_set const&> Map::find_fontset(std::string const& name) const
{
    std::map<std::string,font_set>::const_iterator itr = fontsets_.find(name);
    if (itr != fontsets_.end())
        return boost::optional<font_set const&>(itr->second);
    else
        return boost::optional<font_set const&>() ;
}

std::map<std::string,font_set> const& Map::fontsets() const
{
    return fontsets_;
}

std::map<std::string,font_set> & Map::fontsets()
{
    return fontsets_;
}

bool Map::register_fonts(std::string const& dir, bool recurse)
{
    font_library library;
    return freetype_engine::register_fonts_impl(dir, library, font_file_mapping_, recurse);
}

bool Map::load_fonts()
{
    bool result = false;
    auto const& global_mapping = freetype_engine::get_mapping();
    for (auto const& kv : font_file_mapping_) // for every face-name -> idx/filepath
    {
       auto const& file_path = kv.second.second;
        // do not attemp to re-cache in memory
        if (font_memory_cache_.find(file_path) != font_memory_cache_.end())
        {
            continue;
        }
        auto const& meta = global_mapping.find(kv.first);
        // no need to cache if global mapping already has same font
        if ((meta != global_mapping.end()) && (meta->second.second == file_path))
        {
            continue;
        }
        mapnik::util::file file(file_path);
        if (file)
        {
            auto item = font_memory_cache_.emplace(file_path, std::make_pair(file.data(),file.size()));
            if (item.second) result = true;
        }
    }
    return result;
}

size_t Map::layer_count() const
{
    return layers_.size();
}

void Map::add_layer(layer const& l)
{
    layers_.emplace_back(l);
}

void Map::add_layer(layer && l)
{
    layers_.push_back(std::move(l));
}

void Map::remove_layer(size_t index)
{
    layers_.erase(layers_.begin()+index);
}

void Map::remove_all()
{
    layers_.clear();
    styles_.clear();
    fontsets_.clear();
    font_file_mapping_.clear();
    font_memory_cache_.clear();
}

layer const& Map::get_layer(size_t index) const
{
    return layers_[index];
}

layer& Map::get_layer(size_t index)
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

void Map::set_width(unsigned _width)
{
    if (_width != width_ &&
        _width >= MIN_MAPSIZE &&
        _width <= MAX_MAPSIZE)
    {
        width_=_width;
        fixAspectRatio();
    }
}

void Map::set_height(unsigned _height)
{
    if (_height != height_ &&
        _height >= MIN_MAPSIZE &&
        _height <= MAX_MAPSIZE)
    {
        height_ = _height;
        fixAspectRatio();
    }
}

void Map::resize(unsigned _width, unsigned _height)
{
    if ((_width != width_ ||
         _height != height_) &&
        _width >= MIN_MAPSIZE &&
        _width <= MAX_MAPSIZE &&
        _height >= MIN_MAPSIZE &&
        _height <= MAX_MAPSIZE)
    {
        width_ = _width;
        height_ = _height;
        fixAspectRatio();
    }
}

std::string const&  Map::srs() const
{
    return srs_;
}

void Map::set_srs(std::string const& _srs)
{
    srs_ = _srs;
}

void Map::set_buffer_size(int _buffer_size)
{
    buffer_size_ = _buffer_size;
}

int Map::buffer_size() const
{
    return buffer_size_;
}

boost::optional<color> const& Map::background() const
{
    return background_;
}

void Map::set_background(color const& c)
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

composite_mode_e Map::background_image_comp_op() const
{
    return background_image_comp_op_;
}

void Map::set_background_image_comp_op(composite_mode_e comp_op)
{
    background_image_comp_op_ = comp_op;
}

float Map::background_image_opacity() const
{
    return background_image_opacity_;
}

void Map::set_background_image_opacity(float opacity)
{
    background_image_opacity_ = opacity;
}

void Map::set_maximum_extent(box2d<double> const& box)
{
    maximum_extent_.reset(box);
}

boost::optional<box2d<double> > const& Map::maximum_extent() const
{
    return maximum_extent_;
}

void Map::reset_maximum_extent()
{
    maximum_extent_.reset();
}

std::string const&  Map::base_path() const
{
    return base_path_;
}

void Map::set_base_path(std::string const& base)
{
    base_path_ = base;
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
    try
    {
        if (layers_.empty())
        {
            return;
        }
        projection proj0(srs_);
        box2d<double> ext;
        bool success = false;
        bool first = true;
        for (auto const& layer : layers_)
        {
            if (layer.active())
            {
                std::string const& layer_srs = layer.srs();
                projection proj1(layer_srs);
                proj_transform prj_trans(proj0,proj1);
                box2d<double> layer_ext = layer.envelope();
                if (prj_trans.backward(layer_ext, PROJ_ENVELOPE_POINTS))
                {
                    success = true;
                    MAPNIK_LOG_DEBUG(map) << "map: Layer " << layer.name() << " original ext=" << layer.envelope();
                    MAPNIK_LOG_DEBUG(map) << "map: Layer " << layer.name() << " transformed to map srs=" << layer_ext;
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
        }
        if (success)
        {
            if (maximum_extent_) {
                ext.clip(*maximum_extent_);
            }
            zoom_to_box(ext);
        }
        else
        {
            if (maximum_extent_)
            {
                MAPNIK_LOG_ERROR(map) << "could not zoom to combined layer extents"
                                      << " so falling back to maximum-extent for zoom_all result";
                zoom_to_box(*maximum_extent_);
            }
            else
            {
                std::ostringstream s;
                s << "could not zoom to combined layer extents "
                  << "using zoom_all because proj4 could not "
                  << "back project any layer extents into the map srs "
                  << "(set map 'maximum-extent' to override layer extents)";
                throw std::runtime_error(s.str());
            }
        }
    }
    catch (proj_init_error const& ex)
    {
        throw mapnik::config_error(std::string("Projection error during map.zoom_all: ") + ex.what());
    }
}

void Map::zoom_to_box(box2d<double> const& box)
{
    current_extent_= box;
    fixAspectRatio();
}

void Map::fixAspectRatio()
{
    if (aspectFixMode_ == RESPECT) return;
    if (current_extent_.width() > 0 && current_extent_.height() > 0)
    {
        double ratio1 = static_cast<double>(width_) / static_cast<double>(height_);
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
            height_ = static_cast<unsigned>(std::floor(static_cast<double>(width_) / ratio2 + 0.5));
            break;
        case ADJUST_CANVAS_WIDTH:
            width_ = static_cast<unsigned>(std::floor(static_cast<double>(height_) * ratio2 + 0.5));
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
                width_ = static_cast<unsigned>(std::floor(static_cast<double>(height_) * ratio2 + 0.5));
            else
                height_ = static_cast<unsigned>(std::floor(static_cast<double>(width_) / ratio2 + 0.5));
            break;
        case SHRINK_CANVAS:
            if (ratio2 > ratio1)
                height_ = static_cast<unsigned>(std::floor(static_cast<double>(width_) / ratio2 + 0.5));
            else
                width_ = static_cast<unsigned>(std::floor(static_cast<double>(height_) * ratio2 + 0.5));
            break;
        default:
            if (ratio2 > ratio1)
                current_extent_.height(current_extent_.width() / ratio1);
            else
                current_extent_.width(current_extent_.height() * ratio1);
            break;
        }
    }
}

box2d<double> const& Map::get_current_extent() const
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
    double s = static_cast<double>(width_)/current_extent_.width();
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
        return current_extent_.width()/static_cast<double>(width_);
    return current_extent_.width();
}

double Map::scale_denominator() const
{
    projection map_proj(srs_);
    return mapnik::scale_denominator( scale(), map_proj.is_geographic());
}

view_transform Map::transform() const
{
    return view_transform(width_,height_,current_extent_);
}

featureset_ptr Map::query_point(unsigned index, double x, double y) const
{
    if (!current_extent_.valid())
    {
        throw std::runtime_error("query_point: map extent is not initialized, you need to set a valid extent before querying");
    }
    if (!current_extent_.intersects(x,y))
    {
        throw std::runtime_error("query_point: x,y coords do not intersect map extent");
    }
    if (index < layers_.size())
    {
        mapnik::layer const& layer = layers_[index];
        mapnik::datasource_ptr ds = layer.datasource();
        if (ds)
        {
            mapnik::projection dest(srs_);
            mapnik::projection source(layer.srs());
            proj_transform prj_trans(source,dest);
            double z = 0;
            if (!prj_trans.equal() && !prj_trans.backward(x,y,z))
            {
                throw std::runtime_error("query_point: could not project x,y into layer srs");
            }
            // calculate default tolerance
            mapnik::box2d<double> map_ex = current_extent_;
            if (maximum_extent_)
            {
                map_ex.clip(*maximum_extent_);
            }
            if (!prj_trans.backward(map_ex,PROJ_ENVELOPE_POINTS))
            {
                std::ostringstream s;
                s << "query_point: could not project map extent '" << map_ex
                  << "' into layer srs for tolerance calculation";
                throw std::runtime_error(s.str());
            }
            double tol = (map_ex.maxx() - map_ex.minx()) / static_cast<double>(width_) * 3;
            featureset_ptr fs = ds->features_at_point(mapnik::coord2d(x,y), tol);
            MAPNIK_LOG_DEBUG(map) << "map: Query at point tol=" << tol << "(" << x << "," << y << ")";
            if (fs)
            {
                return std::make_shared<filter_featureset<hit_test_filter> >(fs,
                                                                             hit_test_filter(x,y,tol));
            }
        }
    }
    else
    {
        std::ostringstream s;
        s << "Invalid layer index passed to query_point: '" << index << "'";
        if (!layers_.empty()) s << " for map with " << layers_.size() << " layers(s)";
        else s << " (map has no layers)";
        throw std::out_of_range(s.str());
    }
    return mapnik::make_invalid_featureset();
}

featureset_ptr Map::query_map_point(unsigned index, double x, double y) const
{
    view_transform tr = transform();
    tr.backward(&x,&y);
    return query_point(index,x,y);
}


parameters const& Map::get_extra_parameters() const
{
    return extra_params_;
}

parameters& Map::get_extra_parameters()
{
    return extra_params_;
}

void Map::set_extra_parameters(parameters& params)
{
    extra_params_ = params;
}

}
