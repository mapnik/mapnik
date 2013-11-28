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

#ifndef MAPNIK_MAP_HPP
#define MAPNIK_MAP_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/config.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/datasource.hpp>  // for featureset_ptr
#include <mapnik/layer.hpp>
#include <mapnik/params.hpp>
#include <mapnik/well_known_srs.hpp>
#include <mapnik/image_compositing.hpp>

// boost
#include <boost/optional.hpp>

namespace mapnik
{

class feature_type_style;
class CoordTransform;

class MAPNIK_DECL Map
{
public:

    enum aspect_fix_mode
    {
        // grow the width or height of the specified geo bbox to fill the map size. default behaviour.
        GROW_BBOX,
        // grow the width or height of the map to accomodate the specified geo bbox.
        GROW_CANVAS,
        // shrink the width or height of the specified geo bbox to fill the map size.
        SHRINK_BBOX,
        // shrink the width or height of the map to accomodate the specified geo bbox.
        SHRINK_CANVAS,
        // adjust the width of the specified geo bbox, leave height and map size unchanged
        ADJUST_BBOX_WIDTH,
        // adjust the height of the specified geo bbox, leave width and map size unchanged
        ADJUST_BBOX_HEIGHT,
        // adjust the width of the map, leave height and geo bbox unchanged
        ADJUST_CANVAS_WIDTH,
        //adjust the height of the map, leave width and geo bbox unchanged
        ADJUST_CANVAS_HEIGHT,
        //
        aspect_fix_mode_MAX
    };

private:
    static const unsigned MIN_MAPSIZE=16;
    static const unsigned MAX_MAPSIZE=MIN_MAPSIZE<<10;
    unsigned width_;
    unsigned height_;
    std::string srs_;
    int buffer_size_;
    boost::optional<color> background_;
    boost::optional<std::string> background_image_;
    composite_mode_e background_image_comp_op_;
    float background_image_opacity_;
    std::map<std::string,feature_type_style> styles_;
    std::map<std::string,font_set> fontsets_;
    std::vector<layer> layers_;
    aspect_fix_mode aspectFixMode_;
    box2d<double> current_extent_;
    boost::optional<box2d<double> > maximum_extent_;
    std::string base_path_;
    parameters extra_params_;

public:

    typedef std::map<std::string,feature_type_style>::const_iterator const_style_iterator;
    typedef std::map<std::string,feature_type_style>::iterator style_iterator;
    typedef std::map<std::string,font_set>::const_iterator const_fontset_iterator;
    typedef std::map<std::string,font_set>::iterator fontset_iterator;

    /*! \brief Default constructor.
     *
     *  Creates a map with these parameters:
     *  - width = 400
     *  - height = 400
     *  - projection = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"
     */
    Map();

    /*! \brief Constructor
     *  @param width Initial map width.
     *  @param height Initial map height.
     *  @param srs Initial map projection.
     */
    Map(int width, int height, std::string const& srs=MAPNIK_LONGLAT_PROJ);

    /*! \brief Copy Constructor.
     *
     *  @param rhs Map to copy from.
     */
    Map(Map const& rhs);

    /*! \brief Assignment operator
     *
     *  TODO: to be documented
     *
     */
    Map& operator=(Map const& rhs);

    /*! \brief Get all styles
     * @return Const reference to styles
     */
    std::map<std::string,feature_type_style> const& styles() const;

    /*! \brief Get all styles
     * @return Non-constant reference to styles
     */
    std::map<std::string,feature_type_style> & styles();

    /*! \brief Get first iterator in styles.
     *  @return Constant style iterator.
     */
    const_style_iterator begin_styles() const;

    /*! \brief Get last iterator in styles.
     *  @return Constant style iterator.
     */
    const_style_iterator end_styles() const;

    /*! \brief Get first iterator in styles.
     *  @return Non-constant style iterator.
     */
    style_iterator begin_styles();

    /*! \brief Get last iterator in styles.
     *  @return Non-constant style iterator.
     */
    style_iterator end_styles();

    /*! \brief Insert a style in the map.
     *  @param name The name of the style.
     *  @param style The style to insert.
     *  @return true If success.
     *          false If no success.
     */
    bool insert_style(std::string const& name,feature_type_style const& style);

    /*! \brief Remove a style from the map.
     *  @param name The name of the style.
     */
    void remove_style(std::string const& name);

    /*! \brief Find a style.
     *  @param name The name of the style.
     *  @return The style if found. If not found return the default map style.
     */
    boost::optional<feature_type_style const&> find_style(std::string const& name) const;

    /*! \brief Insert a fontset into the map.
     *  @param name The name of the fontset.
     *  @param fontset The fontset to insert.
     *  @return true If success.
     *          false If failure.
     */
    bool insert_fontset(std::string const& name, font_set const& fontset);

    /*! \brief Find a fontset.
     *  @param name The name of the fontset.
     *  @return The fontset if found. If not found return the default map fontset.
     */
    boost::optional<font_set const&> find_fontset(std::string const& name) const;

    /*! \brief Get all fontsets
     * @return Const reference to fontsets
     */
    std::map<std::string,font_set> const& fontsets() const;

    /*! \brief Get all fontsets
     * @return Non-constant reference to fontsets
     */
    std::map<std::string,font_set> & fontsets();

    /*! \brief Get number of all layers.
     */
    size_t layer_count() const;

    /*! \brief Add a layer to the map.
     *  @param l The layer to add.
     */
    void add_layer(layer const& l);

    /*! \brief Get a layer.
     *  @param index layer number.
     *  @return Constant layer.
     */
    layer const& get_layer(size_t index) const;

    /*! \brief Get a layer.
     *  @param index layer number.
     *  @return Non-constant layer.
     */
    layer& get_layer(size_t index);

    /*! \brief Remove a layer.
     *  @param index layer number.
     */
    void remove_layer(size_t index);

    /*! \brief Get all layers.
     *  @return Constant layers.
     */
    std::vector<layer> const& layers() const;

    /*! \brief Get all layers.
     *  @return Non-constant layers.
     */
    std::vector<layer> & layers();

    /*! \brief Remove all layers and styles from the map.
     */
    void remove_all();

    /*! \brief Get map width.
     */
    unsigned width() const;

    /*! \brief Get map height.
     */
    unsigned height() const;

    /*! \brief Set map width.
     */
    void set_width(unsigned width);

    /*! \brief Set map height.
     */
    void set_height(unsigned height);

    /*! \brief Resize the map.
     */
    void resize(unsigned width,unsigned height);

    /*! \brief Get the map projection.
     *  @return Map projection.
     */
    std::string const& srs() const;

    /*! \brief Set the map projection.
     *  @param srs Map projection.
     */
    void set_srs(std::string const& srs);

    /*! \brief Set the map background color.
     *  @param c Background color.
     */
    void set_background(color const& c);

    /*! \brief Get the map background color
     *  @return Background color as boost::optional
     *  object
     */
    boost::optional<color> const& background() const;

    /*! \brief Set the map background image filename.
     *  @param image_filename Background image filename.
     */
    void set_background_image(std::string const& image_filename);

    /*! \brief Get the map background image
     *  @return Background image path as std::string
     *  object
     */
    boost::optional<std::string> const& background_image() const;

    /*! \brief Set the compositing operation uses to blend the background image into the background color.
     *  @param comp_op compositing operation.
     */
    void set_background_image_comp_op(composite_mode_e comp_op);

    /*! \brief Get the map background image compositing operation
     *  @return Background image compositing operation as composite_mode_e
     *  object
     */
    composite_mode_e background_image_comp_op() const;

    /*! \brief Set the map background image opacity.
     *  @param opacity Background image opacity.
     */
    void set_background_image_opacity(float opacity);

    /*! \brief Get the map background image opacity
     *  @return opacity value as float
     */
    float background_image_opacity() const;

    /*! \brief Set buffer size
     *  @param buffer_size Buffer size in pixels.
     */
    void set_buffer_size(int buffer_size);

    /*! \brief Get the map buffer size
     *  @return Buffer size as int
     */
    int buffer_size() const;

    /*! \brief Set the map maximum extent.
     *  @param box The bounding box for the maximum extent.
     */
    void set_maximum_extent(box2d<double> const& box);

    /*! \brief Get the map maximum extent as box2d<double>
     */
    boost::optional<box2d<double> > const& maximum_extent() const;

    void reset_maximum_extent();

    /*! \brief Get the map base path where paths should be relative to.
     */
    std::string const& base_path() const;

    /*! \brief Set the map base path where paths should be relative to.
     *  @param base Map base_path.
     */
    void set_base_path(std::string const& base);

    /*! \brief Zoom the map at the current position.
     *  @param factor The factor how much the map is zoomed in or out.
     */
    void zoom(double factor);

    /*! \brief Zoom the map to a bounding box.
     *
     *  Aspect is handled automatic if not fitting to width/height.
     *  @param box The bounding box where to zoom.
     */
    void zoom_to_box(box2d<double> const& box);

    /*! \brief Zoom the map to show all data.
     */
    void zoom_all();

    void pan(int x,int y);

    void pan_and_zoom(int x,int y,double zoom);

    /*! \brief Get current bounding box.
     *  @return The current bounding box.
     */
    box2d<double> const& get_current_extent() const;

    /*! \brief Get current buffered bounding box.
     *  @return The current buffered bounding box.
     */
    box2d<double> get_buffered_extent() const;

    /*!
     * @return The Map Scale.
     */
    double scale() const;

    double scale_denominator() const;

    CoordTransform view_transform() const;

    /*!
     * @brief Query a Map layer (by layer index) for features
     *
     * Intersecting the given x,y location in the coordinates
     * of map projection.
     *
     * @param index The index of the layer to query from.
     * @param x The x coordinate where to query.
     * @param y The y coordinate where to query.
     * @return A Mapnik Featureset if successful otherwise will return nullptr.
     */
    featureset_ptr query_point(unsigned index, double x, double y) const;

    /*!
     * @brief Query a Map layer (by layer index) for features
     *
     * Intersecting the given x,y location in the coordinates
     * of the pixmap or map surface.
     *
     * @param index The index of the layer to query from.
     * @param x The x coordinate where to query.
     * @param y The y coordinate where to query.
     * @return A Mapnik Featureset if successful otherwise will return nullptr.
     */
    featureset_ptr query_map_point(unsigned index, double x, double y) const;

    ~Map();

    inline void set_aspect_fix_mode(aspect_fix_mode afm) { aspectFixMode_ = afm; }
    inline aspect_fix_mode get_aspect_fix_mode() const { return aspectFixMode_; }

    /*!
     * @brief Get extra, arbitrary Parameters attached to the Map
     */
    parameters const& get_extra_parameters() const;

    /*!
     * @brief Get non-const extra, arbitrary Parameters attached to the Map
     */
    parameters& get_extra_parameters();

    /*!
     * @brief Set extra, arbitary Parameters of the Map
     */
    void set_extra_parameters(parameters& params);

private:
    void fixAspectRatio();
};

DEFINE_ENUM(aspect_fix_mode_e,Map::aspect_fix_mode);
}

#endif // MAPNIK_MAP_HPP
