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

//$Id: feature.hh 58 2004-10-31 16:21:26Z artem $

#ifndef FEATURE_HH
#define FEATURE_HH

#include "geometry.hh"
#include "raster.hh"
#include "attribute_container.hh"

namespace mapnik
{
    typedef ref_ptr<raster> RasterPtr;
       
    struct Feature
    {
        virtual int id() const=0;
        virtual bool isRaster() const=0;
        virtual geometry_ptr& getGeometry()=0;
        virtual const RasterPtr& getRaster() const=0;
        virtual void add(const std::string& name,const attribute_base& a)=0;
        virtual const attribute_base& get(const std::string& name) const=0;
        virtual const attribute_container& attributes() const=0;
        virtual ~Feature() {};
    };

    class FeatureImpl : public Feature
    {
    private:
	int id_;
	attribute_container attributes_;
    public:
	FeatureImpl(int id);
	virtual ~FeatureImpl();
	int id () const;
	void add(const std::string& name,const attribute_base& a);
	const attribute_base& get(const std::string& name) const;
	const attribute_container& attributes() const;
    };
}
#endif                                            //FEATURE_HH
