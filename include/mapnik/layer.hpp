/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_LAYER_HPP
#define MAPNIK_LAYER_HPP

// mapnik
#include <mapnik/well_known_srs.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/image_compositing.hpp>

// stl
#include <vector>
#include <memory>

namespace mapnik {

class datasource;
using datasource_ptr = std::shared_ptr<datasource>;

/*!
 * @brief A Mapnik map layer.
 *
 * Create a layer with a named string and, optionally, an srs string either
 * with a Proj.4 epsg code ('epsg:<code>') or with a Proj.4 literal
 * ('+proj=<literal>'). If no srs is specified it will default to
 * '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'
 */
class MAPNIK_DECL layer
{
  public:
    layer(std::string const& name, std::string const& srs = MAPNIK_GEOGRAPHIC_PROJ);
    // copy
    layer(layer const& l);
    // move
    layer(layer&& l);
    layer& operator=(layer rhs);
    bool operator==(layer const& other) const;

    /*!
     * @brief Set the name of the layer.
     */
    void set_name(std::string const& name);

    /*!
     * @return the name of the layer.
     */

    std::string const& name() const;

    /*!
     * @brief Set the SRS of the layer.
     */
    void set_srs(std::string const& srs);

    /*!
     * @return the SRS of the layer.
     */
    std::string const& srs() const;

    /*!
     * @brief Add a new style to this layer.
     *
     * @param stylename The name of the style to add.
     */
    void add_style(std::string const& stylename);

    /*!
     * @return the styles list attached to this layer.
     */
    std::vector<std::string> const& styles() const;

    /*!
     * @return the styles list attached to this layer
     *         (non-const version).
     */
    std::vector<std::string>& styles();

    /*! \brief Add a child layer by copying it.
     *  @param l The layer to add.
     */
    void add_layer(layer const& l);

    /*! \brief Add a child layer by moving it.
     *  @param l The layer to add.
     */
    void add_layer(layer&& l);

    std::vector<layer> const& layers() const;

    /*!
     * @param minimum_scale_denom The minimum scale denominator
     */
    void set_minimum_scale_denominator(double minimum_scale_denom);

    /*!
     * @param maximum_scale_denom The maximum scale denominator
     */
    void set_maximum_scale_denominator(double maximum_scale_denom);

    /*!
     * @return the minimum zoom level of the layer.
     */
    double minimum_scale_denominator() const;

    /*!
     * @return the maximum zoom level of the layer.
     */
    double maximum_scale_denominator() const;

    /*!
     * @brief Set whether this layer is active and will be rendered.
     */
    void set_active(bool active);

    /*!
     * @return whether this layer is active and will be rendered.
     */
    bool active() const;

    /*!
     * @brief Set whether this layer is queryable.
     */
    void set_queryable(bool queryable);

    /*!
     * @return whether this layer is queryable or not.
     */
    bool queryable() const;

    /*!
     * @brief Get the visibility for a specific scale denominator.
     *
     * @param scale_denom Accepts an integer or float input.
     *
     * @return true if this layer's data is active and visible at a given scale denominator.
     *         Otherwise returns False.
     *         false if:
     *         scale_denominator >= minimum_scale_denominator - 1e-6
     *         or
     *         scale_denominator < maximum_scale_denominator + 1e-6
     */
    bool visible(double scale_denom) const;

    /*!
     * @param clear_cache Set whether this layer's labels are cached.
     */
    void set_clear_label_cache(bool clear_cache);

    /*!
     * @return whether this layer's labels are cached.
     */
    bool clear_label_cache() const;

    /*!
     * @param cache_features Set whether this layer's features should be cached if used by multiple styles.
     */
    void set_cache_features(bool cache_features);

    /*!
     * @return whether this layer's features will be cached if used by multiple styles
     */
    bool cache_features() const;

    /*!
     * @param column Set the field rendering of this layer is grouped by.
     */
    void set_group_by(std::string const& column);

    /*!
     * @return The field rendering of this layer is grouped by.
     */
    std::string const& group_by() const;

    /*!
     * @param column Set the field rendering of this layer is sorted by.
     */
    void set_sort_by(std::string const& column);

    /*!
     * @return optional field rendering of this layer is sorted by.
     */
    std::optional<std::string> const& sort_by() const;

    /*!
     * @brief Attach a datasource for this layer.
     *
     * @param ds The datasource to attach.
     */
    void set_datasource(datasource_ptr const& ds);

    /*!
     * @return the datasource attached to this layer.
     */
    datasource_ptr datasource() const;

    /*!
     * @return the geographic envelope/bounding box of the data in the layer.
     */
    box2d<double> envelope() const;

    // compositing
    void set_comp_op(composite_mode_e comp_op);
    std::optional<composite_mode_e> comp_op() const;
    void set_opacity(double opacity);
    double get_opacity() const;

    void set_maximum_extent(box2d<double> const& box);
    std::optional<box2d<double>> const& maximum_extent() const;
    void reset_maximum_extent();
    void set_buffer_size(int size);
    std::optional<int> const& buffer_size() const;
    void reset_buffer_size();
    ~layer();

  private:
    std::string name_;
    std::string srs_;
    double minimum_scale_denom_;
    double maximum_scale_denom_;
    bool active_;
    bool queryable_;
    bool clear_label_cache_;
    bool cache_features_;
    std::string group_by_;
    std::optional<std::string> sort_by_;
    std::vector<std::string> styles_;
    std::vector<layer> layers_;
    datasource_ptr ds_;
    std::optional<int> buffer_size_;
    std::optional<box2d<double>> maximum_extent_;
    std::optional<composite_mode_e> comp_op_;
    double opacity_;
};
} // namespace mapnik

#endif // MAPNIK_LAYER_HPP
