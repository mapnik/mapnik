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

//$Id: layer.cpp 17 2005-03-08 23:58:43Z pavlenko $


#include "style.hpp"
#include "datasource.hpp"
#include "datasource_cache.hpp"
#include "layer.hpp"

#include <string>
#include <iostream>

namespace mapnik
{
    using namespace std;
    Layer::Layer(std::string const& name)
        : name_(name),
          title_(""),
          abstract_(""),
          minZoom_(0),
          maxZoom_(std::numeric_limits<double>::max()),
          active_(true),
          selectable_(false),
          selection_style_("default_selection")
    {}
    
    Layer::Layer(const Layer& rhs)
        : name_(rhs.name_),
          title_(rhs.title_),
          abstract_(rhs.abstract_),
          minZoom_(rhs.minZoom_),
          maxZoom_(rhs.maxZoom_),
          active_(rhs.active_),
          selectable_(rhs.selectable_),
          styles_(rhs.styles_),
          ds_(rhs.ds_),
          selection_style_(rhs.selection_style_) {}
    
    Layer& Layer::operator=(const Layer& rhs)
    {
        Layer tmp(rhs);
        swap(tmp);
        return *this;
    }

    bool Layer::operator==(Layer const& other) const
    {
        return (this == &other);
    }
    
    void Layer::swap(const Layer& rhs)
    {
        name_=rhs.name_;
        title_=rhs.title_;
        abstract_=rhs.abstract_;
        minZoom_=rhs.minZoom_;
        maxZoom_=rhs.maxZoom_;
        active_=rhs.active_;
        selectable_=rhs.selectable_;
        styles_=rhs.styles_;
        ds_=rhs.ds_;
        selection_style_=rhs.selection_style_;
    }
    
    Layer::~Layer() {}
    
    void Layer::set_name( std::string const& name)
    {
        name_ = name;
    }
 
    string const& Layer::name() const
    {
        return name_;
    }

    void Layer::set_title( std::string const& title)
    {
        title_ = title;
    }
 
    string const& Layer::title() const
    {
        return title_;
    }
    
    void Layer::set_abstract( std::string const& abstract)
    {
        abstract_ = abstract;
    }
 
    string const& Layer::abstract() const
    {
        return abstract_;
    }

    void Layer::add_style(std::string const& stylename)
    {
        styles_.push_back(stylename);
    }

    std::vector<std::string> const& Layer::styles() const
    {
        return styles_;
    }

    void Layer::setMinZoom(double minZoom)
    {
        minZoom_=minZoom;
    }

    void Layer::setMaxZoom(double maxZoom)
    {
        maxZoom_=maxZoom;
    }

    double Layer::getMinZoom() const
    {
        return minZoom_;
    }

    double Layer::getMaxZoom() const
    {
        return maxZoom_;
    }

    void Layer::setActive(bool active)
    {
        active_=active;
    }

    bool Layer::isActive() const
    {
        return active_;
    }

    bool Layer::isVisible(double scale) const
    {
        return isActive() && scale>=minZoom_ && scale<maxZoom_;
    }

    void Layer::setSelectable(bool selectable)
    {
        selectable_=selectable;
    }

    bool Layer::isSelectable() const
    {
        return selectable_;
    }

    datasource_p Layer::datasource() const
    {
        return ds_;
    }
    
    void Layer::set_datasource(datasource_p const& ds)
    {
        ds_ = ds;
    }
    
    Envelope<double> Layer::envelope() const
    {
        if (ds_) return ds_->envelope();
    	return Envelope<double>();
    }
    
    void Layer::selection_style(const std::string& name) 
    {
        selection_style_=name;
    }
    
    const std::string& Layer::selection_style() const 
    {
        return selection_style_;
    }

    void Layer::add_to_selection(shared_ptr<Feature>& feature) const
    {
        selection_.push_back(feature);
    }
 
    vector<shared_ptr<Feature> >& Layer::selection() const
    {
        return selection_;
    }

    void Layer::clear_selection() const 
    {
        selection_.clear();
    }
}
