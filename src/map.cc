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

//$Id: map.cc 69 2004-11-24 11:08:45Z artem $

#include "style.hh"
#include "datasource.hh"
#include "layer.hh"
#include "map.hh"

namespace mapnik
{

    Map::Map(int width,int height,int srid)
        : width_(width),
        height_(height),
        srid_(srid),
        background_(Color(255,255,255)) {}

    Map::Map(const Map& rhs)
        : width_(rhs.width_),
        height_(rhs.height_),
        srid_(rhs.srid_),
        background_(rhs.background_),
        layers_(rhs.layers_),
        currentExtent_(rhs.currentExtent_){}

    Map& Map::operator=(const Map& rhs)
    {
        if (this==&rhs) return *this;
        width_=rhs.width_;
        height_=rhs.height_;
        srid_=rhs.srid_;
        background_=rhs.background_;
        layers_=rhs.layers_;
        return *this;
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
    void Map::removeLayer(const char* lName)
    {
        //todo
    }

    const Layer& Map::getLayer(size_t index) const
    {
        return layers_[index];
    }

    int Map::getWidth() const
    {
        return width_;
    }

    int Map::getHeight() const
    {
        return height_;
    }

    int Map::srid() const
    {
        return srid_;
    }

    void Map::setBackground(const Color& c)
    {
        background_=c;
    }

    const Color& Map::getBackground() const
    {
        return background_;
    }

    void Map::zoom(double factor)
    {
        coord2d center = currentExtent_.center();
        double w = factor * currentExtent_.width();
        double h = factor * currentExtent_.height();
        currentExtent_ = Envelope<double>(center.x - 0.5 * w, center.y - 0.5 * h,
            center.x + 0.5 * w, center.y + 0.5 * h);
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
        if (ratio1 >= 1.0)
        {
            if (ratio2 > ratio1)
            {
                currentExtent_.height(currentExtent_.width() / ratio1);
            }
            else
            {
                currentExtent_.width(currentExtent_.height() * ratio1);
            }
        }
        else
        {
            if (ratio2 > ratio1)
            {
                currentExtent_.width(currentExtent_.height() * ratio1);
            }
            else
            {
                currentExtent_.height(currentExtent_.width() / ratio1);
            }
        }
    }

    const Envelope<double>& Map::getCurrentExtent() const
    {
        return currentExtent_;
    }

    void Map::pan(int x,int y)
    {
        CoordTransform t(width_,height_,currentExtent_);
        coord2d pt(x,y);
        t.backward(pt);
        currentExtent_.re_center(pt.x,pt.y);
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

    Map::~Map() {}
}
