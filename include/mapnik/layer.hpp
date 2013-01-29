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

#ifndef MAPNIK_LAYER_HPP
#define MAPNIK_LAYER_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/well_known_srs.hpp>

// stl
#include <vector>

namespace mapnik
{
/*!
 * @brief A Mapnik map layer.
 *
 * Create a layer with a named string and, optionally, an srs string either
 * with a Proj.4 epsg code ('+init=epsg:<code>') or with a Proj.4 literal
 * ('+proj=<literal>'). If no srs is specified it will default to
 * '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'
 */
class MAPNIK_DECL layer
{
public:
    layer(std::string const& name,
          std::string const& srs=MAPNIK_LONGLAT_PROJ);

    layer(layer const& l);
    layer& operator=(layer const& rhs);
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

    /*!
     * @param min_zoom The minimum zoom level to set
     */
    void set_min_zoom(double min_zoom);

    /*!
     * @param max_zoom The maximum zoom level to set
     */
    void set_max_zoom(double max_zoom);

    /*!
     * @return the minimum zoom level of the layer.
     */
    double min_zoom() const;

    /*!
     * @return the maximum zoom level of the layer.
     */
    double max_zoom() const;

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
     * @brief Get the visability for a specific scale.
     *
     * @param scale Accepts an integer or float input.
     *
     * @return true if this layer's data is active and visible at a given scale.
     *         Otherwise returns False.
     *         false if:
     *         scale >= minzoom - 1e-6
     *         or
     *         scale < maxzoom + 1e-6
     */
    bool visible(double scale) const;

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
    void set_group_by(std::string column);

    /*!
     * @return The field rendering of this layer is grouped by.
     */
    std::string const& group_by() const;

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

    void set_maximum_extent(box2d<double> const& box);
    boost::optional<box2d<double> > const&  maximum_extent() const;
    void reset_maximum_extent();
    void set_buffer_size(int size);
    boost::optional<int> const& buffer_size() const;
    void reset_buffer_size();
    ~layer();
private:
    void swap(layer& other);

    std::string name_;
    std::string srs_;

    double min_zoom_;
    double max_zoom_;
    bool active_;
    bool queryable_;
    bool clear_label_cache_;
    bool cache_features_;
    std::string group_by_;
    std::vector<std::string> styles_;
    datasource_ptr ds_;
    boost::optional<int> buffer_size_;
    boost::optional<box2d<double> > maximum_extent_;
};
}

#endif // MAPNIK_LAYER_HPP
