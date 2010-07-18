/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Hermann Kraus
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


#ifndef METAWRITER_JSON_HPP
#define METAWRITER_JSON_HPP

// Mapnik
#include <mapnik/metawriter.hpp>

// STL
#include <ostream>

namespace mapnik {
/** JSON writer. */
class metawriter_json : public metawriter, private boost::noncopyable
{
    public:
        metawriter_json(metawriter_properties dflt_properties, std::string fn);
        metawriter_json(metawriter_properties dflt_properties, std::ostream);
        ~metawriter_json();
        virtual void add_box(box2d<double> box, Feature const &feature,
                             proj_transform const& prj_trans,
                             CoordTransform const& t,
                             metawriter_properties const& properties);
        virtual void start();
        virtual void stop();
    private:
        std::ostream *f;
        std::string fn_;
        int count;
};

};

#endif
