/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_SYMBOLIZER_HPP
#define MAPNIK_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/metawriter.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/transform_expression.hpp>

// boost
#include <boost/array.hpp>
#include <boost/optional.hpp>

namespace mapnik
{

typedef transform_list_ptr transform_type;

MAPNIK_DECL void evaluate_transform(agg::trans_affine& tr, Feature const& feature,
                                    transform_type const& trans_expr);

class Map;

class MAPNIK_DECL symbolizer_base 
{
public:
    symbolizer_base();    
    symbolizer_base(symbolizer_base const& other);
    
    /** Add a metawriter to this symbolizer using a name. */
    void add_metawriter(std::string const& name, metawriter_properties const& properties);
    /** Add a metawriter to this symbolizer using a pointer.
     * The name is only needed if you intend to call save_map() some time.
     * You don't need to call cache_metawriters() when using this function.
     * Call this function with an NULL writer_ptr to remove a metawriter.
     */
    void add_metawriter(metawriter_ptr writer_ptr,
                        metawriter_properties const& properties = metawriter_properties(),
                        std::string const& name = "");
    /** Cache metawriter objects to avoid repeated lookups while processing.
     *
     * If the metawriter was added using a symbolic name (instead of a pointer)
     * this function has to be called before the symbolizer is used, because
     * the map object is not available in renderer::apply() to resolve the reference.
     */
    void cache_metawriters(Map const &m);
    /** Get the metawriter associated with this symbolizer or a NULL pointer if none exists.
     *
     * This functions requires that cache_metawriters() was called first.
     */
    metawriter_with_properties get_metawriter() const;
    /** Get metawriter properties.
     * This functions returns the default attributes of the
     * metawriter + symbolizer specific attributes.
     * \note This function is a helperfunction for class attribute_collector.
     */
    metawriter_properties const& get_metawriter_properties() const { return properties_complete_; }
    /** Get metawriter properties which only apply to this symbolizer.
     */
    metawriter_properties const& get_metawriter_properties_overrides() const { return properties_; }
    /** Get metawriter name. */
    std::string const& get_metawriter_name() const { return writer_name_; }
    void set_comp_op(composite_mode_e comp_op);
    composite_mode_e comp_op() const;
    void set_transform(transform_type const& );
    transform_type const& get_transform() const;
    std::string get_transform_string() const;
    void set_clip(bool clip);
    bool clip() const;
    void set_smooth(double smooth);
    double smooth() const;
private:
    metawriter_properties properties_;
    metawriter_properties properties_complete_;
    std::string writer_name_;
    metawriter_ptr writer_ptr_;
    composite_mode_e comp_op_;
    transform_type affine_transform_;
    bool clip_;
    double smooth_value_;
};


class MAPNIK_DECL symbolizer_with_image 
{
public:
    path_expression_ptr get_filename() const;
    void set_filename(path_expression_ptr filename);
    void set_opacity(float opacity);
    float get_opacity() const;
    void  set_image_transform(transform_type const& tr);
    transform_type const& get_image_transform() const;
    std::string get_image_transform_string() const;
protected:
    symbolizer_with_image(path_expression_ptr filename = path_expression_ptr());
    symbolizer_with_image(symbolizer_with_image const& rhs);
    path_expression_ptr image_filename_;
    float image_opacity_;
    transform_type image_transform_;
};
}

#endif // MAPNIK_SYMBOLIZER_HPP
