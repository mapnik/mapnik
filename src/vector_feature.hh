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

//$Id: vector_feature.hh 58 2004-10-31 16:21:26Z artem $

#ifndef VECTOR_FEATURE_HH
#define VECTOR_FEATURE_HH

#include "feature.hh"
#include "attribute.hh"
#include "attribute_container.hh"

namespace mapnik
{

    class VectorFeature : public FeatureImpl
    {
        private:
            geometry_ptr geom_;
        public:
            VectorFeature(int id,const geometry_ptr& geom);
            geometry_ptr& getGeometry();
            const RasterPtr& getRaster() const;
            bool isRaster() const;
            virtual ~VectorFeature();
        private:
            VectorFeature(const VectorFeature&);
            VectorFeature& operator=(const VectorFeature&);
    };
}
#endif                                            //VECTOR_FEATURE_HH
