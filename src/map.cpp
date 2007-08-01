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

#include <mapnik/style.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/filter_featureset.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/map.hpp>

namespace mapnik
{
    Map::Map()
        : width_(400),
          height_(400),
          srs_("+proj=latlong +datum=WGS84") {}
    
    Map::Map(int width,int height, std::string const& srs)
        : width_(width),
          height_(height),
          srs_(srs) {}
   
    Map::Map(const Map& rhs)
        : width_(rhs.width_),
          height_(rhs.height_),
          srs_(rhs.srs_),
          background_(rhs.background_),
          styles_(rhs.styles_),
          layers_(rhs.layers_),
          currentExtent_(rhs.currentExtent_) {}
    
    Map& Map::operator=(const Map& rhs)
    {
        if (this==&rhs) return *this;
        width_=rhs.width_;
        height_=rhs.height_;
        srs_=rhs.srs_;
        background_=rhs.background_;
        styles_=rhs.styles_;
        layers_=rhs.layers_;
        return *this;
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
    
    feature_type_style const& Map::find_style(std::string const& name) const
    {
        std::map<std::string,feature_type_style>::const_iterator itr = styles_.find(name);
        if (itr!=styles_.end()) 
            return itr->second;
        static feature_type_style default_style;
        return default_style;
    }
    
    size_t Map::layerCount() const
    {
        return layers_.size();
    }
    
    void Map::addLayer(const Layer& l)
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
    }
    
    const Layer& Map::getLayer(size_t index) const
    {
        return layers_[index];
    }

    Layer& Map::getLayer(size_t index)
    {
        return layers_[index];
    }

    std::vector<Layer> const& Map::layers() const
    {
        return layers_;
    }

    std::vector<Layer> & Map::layers()
    {
        return layers_;
    }

    unsigned Map::getWidth() const
    {
        return width_;
    }

    unsigned Map::getHeight() const
    {
        return height_;
    }
    
    void Map::setWidth(unsigned width)
    {
        if (width >= MIN_MAPSIZE && width <= MAX_MAPSIZE)
        {
            width_=width;
            fixAspectRatio();
        }	
    }

    void Map::setHeight(unsigned height)
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
  
   boost::optional<Color> const& Map::background() const
   {
      return background_;
   }
   
   void Map::set_background(const Color& c)
   {
      background_ = c;
   }
   
    void Map::zoom(double factor)
    {
        coord2d center = currentExtent_.center();
        double w = factor * currentExtent_.width();
        double h = factor * currentExtent_.height();
        currentExtent_ = Envelope<double>(center.x - 0.5 * w, 
                                          center.y - 0.5 * h,
                                          center.x + 0.5 * w, 
                                          center.y + 0.5 * h);
        fixAspectRatio();
    }
    
    void Map::zoom_all() 
    {
        try 
        {
            projection proj0(srs_);
            Envelope<double> ext;
            bool first = true;
            std::vector<Layer>::const_iterator itr = layers_.begin();
            std::vector<Layer>::const_iterator end = layers_.end();
            while (itr != end)
            {
                std::string const& layer_srs = itr->srs();
                projection proj1(layer_srs);
                proj_transform prj_trans(proj0,proj1);
                
                Envelope<double> layerExt = itr->envelope();
                double x0 = layerExt.minx();
                double y0 = layerExt.miny();
                double z0 = 0.0;
                double x1 = layerExt.maxx();
                double y1 = layerExt.maxy();
                double z1 = 0.0;
                prj_trans.backward(x0,y0,z0);
                prj_trans.backward(x1,y1,z1);
                
                Envelope<double> layerExt2(x0,y0,x1,y1);
#ifdef MAPNIK_DEBUG
                std::clog << " layer1 - > " << layerExt << "\n";
                std::clog << " layer2 - > " << layerExt2 << "\n";
#endif                
                if (first)
                {
                    ext = layerExt2;
                    first = false;
                }
                else 
                {
                    ext.expand_to_include(layerExt2);
                }
                ++itr;
            }
            zoomToBox(ext);
        }
        catch (proj_init_error & ex)
        {
            std::clog << ex.what() << '\n';
        }
    }

    void Map::zoomToBox(const Envelope<double> &box)
    {
        currentExtent_=box;
        fixAspectRatio();
    }

    void Map::fixAspectRatio()
    {
        double ratio1 = (double) width_ / (double) height_;
        double ratio2 = currentExtent_.width() / currentExtent_.height();
         
        if (ratio2 > ratio1)
        {
            currentExtent_.height(currentExtent_.width() / ratio1);
        }
        else if (ratio2 < ratio1)
        {
            currentExtent_.width(currentExtent_.height() * ratio1);
        }       
    }

    const Envelope<double>& Map::getCurrentExtent() const
    {
        return currentExtent_;
    }

    void Map::pan(int x,int y)
    {
        int dx = x - int(0.5 * width_);
        int dy = int(0.5 * height_) - y;
        double s = width_/currentExtent_.width();
        double minx  = currentExtent_.minx() + dx/s;
        double maxx  = currentExtent_.maxx() + dx/s;
        double miny  = currentExtent_.miny() + dy/s;
        double maxy  = currentExtent_.maxy() + dy/s;
        currentExtent_.init(minx,miny,maxx,maxy);
    }

    void Map::pan_and_zoom(int x,int y,double factor)
    {
        pan(x,y);
        zoom(factor);
    }

    double Map::scale() const
    {
        if (width_>0)
            return currentExtent_.width()/width_;
        return currentExtent_.width();
    }

    CoordTransform Map::view_transform() const
    {
        return CoordTransform(width_,height_,currentExtent_);
    }
    
    featureset_ptr Map::query_point(unsigned index, double x, double y) const
    {
        if ( index< layers_.size())
        {
            mapnik::Layer const& layer = layers_[index];    
            try
            {
                double z = 0;
                mapnik::projection dest(srs_);
                mapnik::projection source(layer.srs());
                proj_transform prj_trans(source,dest);
                prj_trans.backward(x,y,z);
                
                double minx = currentExtent_.minx();
                double miny = currentExtent_.miny();
                double maxx = currentExtent_.maxx();
                double maxy = currentExtent_.maxy();
                
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
            mapnik::Layer const& layer = layers_[index];
            CoordTransform tr = view_transform();
            tr.backward(&x,&y);
	    
            try
            {
                mapnik::projection dest(srs_);
                mapnik::projection source(layer.srs());
                proj_transform prj_trans(source,dest);
                double z = 0;
                prj_trans.backward(x,y,z);
                
                double minx = currentExtent_.minx();
                double miny = currentExtent_.miny();
                double maxx = currentExtent_.maxx();
                double maxy = currentExtent_.maxy();
                
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
}
