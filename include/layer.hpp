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
//$Id: layer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef LAYER_HPP
#define LAYER_HPP

#include <vector>
#include "feature.hpp"
#include "datasource.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>

namespace mapnik
{
    class MAPNIK_DECL Layer
    {
        friend class boost::serialization::access;
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int /*version*/)
        {
            ar  & boost::serialization::make_nvp("name",name_)
                & boost::serialization::make_nvp("params",params_)
                & boost::serialization::make_nvp("min_zoom",minZoom_)
                & boost::serialization::make_nvp("max_zoom",maxZoom_)
                & boost::serialization::make_nvp("active",active_)
                & boost::serialization::make_nvp("selectable",selectable_)
                & boost::serialization::make_nvp("styles",styles_)
                ;
        }
        parameters params_;
        std::string name_;
        double minZoom_;
        double maxZoom_;
        bool active_;
        bool selectable_;
	
        std::vector<std::string>  styles_;
        std::string selection_style_;

        mutable std::vector<boost::shared_ptr<Feature> > selection_;
        mutable datasource_p ds_;
        
    public:
        Layer();
        explicit Layer(const parameters& params);
        Layer(Layer const& l);
        Layer& operator=(Layer const& l);
        bool operator==(Layer const& other) const;
        parameters const& params() const;
        void set_name(std::string const& name);
        const std::string& name() const;
        void add_style(std::string const& stylename);
        std::vector<std::string> const& styles() const;
        void selection_style(const std::string& name);
        const std::string& selection_style() const;
        void setMinZoom(double minZoom);
        void setMaxZoom(double maxZoom);
        double getMinZoom() const;
        double getMaxZoom() const;
        void setActive(bool active);
        bool isActive() const;
        void setSelectable(bool selectable);
        bool isSelectable() const;
        bool isVisible(double scale) const;
        void add_to_selection(boost::shared_ptr<Feature>& feature) const;
        std::vector<boost::shared_ptr<Feature> >& selection() const;
        void clear_selection() const;
        void set_datasource(datasource_p const& ds);
        datasource_p const& datasource() const;
        Envelope<double> envelope() const;
        virtual ~Layer();
    private:
        void swap(const Layer& other);
    };
}

BOOST_CLASS_IMPLEMENTATION(std::vector<std::string>, boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(std::vector<std::string>, boost::serialization::track_never)

BOOST_CLASS_IMPLEMENTATION(mapnik::Layer, boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(mapnik::Layer, boost::serialization::track_never)

#endif //LAYER_HPP
