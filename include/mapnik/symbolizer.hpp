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
//$Id: symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef MAPNIK_SYMBOLIZER_HPP
#define MAPNIK_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/metawriter.hpp>

// boost
#include <boost/array.hpp>

namespace mapnik 
{

class Map;

class MAPNIK_DECL symbolizer_base {
    public:
        /** Add a metawriter to this symbolizer.
          *
          * expression can be empty the default expression of
          * the writer is used in this case. */
        void add_metawriter(std::string name, metawriter_properties const& properties);
        /** Cache metawriter objects to avoid repeated lookups while processing.
          *
          * This function has to be called before the symbolizer is used, because
          * the map object is not available in renderer::apply().
          */
        void cache_metawriters(Map const &m);
        //TODO: Allow more than one meta writer per symbolizer?
        /** Get the metawriter associated with this symbolizer or a NULL pointer if none exists.
          *
          * This functions requires that cache_metawriters() was called first.
          */
        metawriter_with_properties get_metawriter() const;
        /** Get properties needed for metawriter.
          * \note This function is a helperfunction for class attribute_collector.
          */
        metawriter_properties const& get_metawriter_properties() const { return properties_;}
    private:
        metawriter_properties properties_;
        std::string writer_name_;
        metawriter_ptr writer_ptr_;
};

typedef boost::array<double,6> transform_type;

class MAPNIK_DECL symbolizer_with_image {
public:
    path_expression_ptr get_filename() const;
    void set_filename(path_expression_ptr filename);
    void set_transform(transform_type const& );
    transform_type const& get_transform() const;
    void set_opacity(float opacity);
    float get_opacity() const;
protected:
    symbolizer_with_image(path_expression_ptr filename);
    symbolizer_with_image(symbolizer_with_image const& rhs);
    path_expression_ptr image_filename_;   
    float opacity_;
    transform_type matrix_;
};
}

#endif //MAPNIK_SYMBOLIZER_HPP
