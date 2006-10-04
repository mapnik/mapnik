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
#ifndef SPATIAL_HPP
#define SPATIAL_HPP

#include <mapnik/filter.hpp>
#include <mapnik/filter_visitor.hpp>

namespace mapnik
{    

    template <typename FeatureT>
    struct equals_ : public filter<FeatureT>
    {

        bool pass(const FeatureT& feature) const
        {
            return false;
        }
	
        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
    };
    
    template <typename FeatureT>
    struct disjoint : public filter<FeatureT>
    {
	  
	
        bool pass(const FeatureT& feature) const
        {
            return false;
        }
	
        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
    };
  
    template <typename FeatureT>
    struct touches : public filter<FeatureT>
    {

	
        bool pass(const FeatureT& feature) const
        {
            return false;
        }
	
        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
    };

    template <typename FeatureT>
    struct within : public filter<FeatureT>
    {

        bool pass(const FeatureT& feature) const
        {
            return false;
        }

        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
    };

    template <typename FeatureT>
    struct overlaps : public filter<FeatureT>
    {

        bool pass(const FeatureT& feature) const
        {
            return false;
        }
	
        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
    };

    template <typename FeatureT>
    struct crosses : public filter<FeatureT>
    {

	
        bool pass(const FeatureT& feature) const
        {
            return false;
        }
	
        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
    };
    
    template <typename FeatureT>
    struct bbox  : public filter<FeatureT> 
    {
    private:
        Envelope<double> box_;
    public:
        bbox(const Envelope<double>& box)
            : box_(box) {}

	
        bool pass(const FeatureT& feature) const
        {
            return box_.contains(feature.get_geometry()->bbox());
        }
	

        filter<FeatureT>* clone() const
        {
            return new bbox<FeatureT>(box_);
        }
        void accept(const filter_visitor<FeatureT>& v)
        {
            v.visit(*this);
        }
	
        virtual ~bbox() {}
    };
}

#endif //SPATIAL_HPP
