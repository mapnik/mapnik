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
//$Id: map.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef MAP_HPP
#define MAP_HPP

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

// mapnik
#include <mapnik/enumeration.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>

// boost
#include <boost/optional/optional.hpp>

namespace mapnik
{
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
        std::string  srs_;
        int buffer_size_;
        boost::optional<color> background_;
        std::map<std::string,feature_type_style> styles_;
        std::map<std::string,FontSet> fontsets_;
        std::vector<Layer> layers_;
        aspect_fix_mode aspectFixMode_;
        Envelope<double> currentExtent_;
        
    public:

        typedef std::map<std::string,feature_type_style>::const_iterator const_style_iterator;
        typedef std::map<std::string,feature_type_style>::iterator style_iterator;
        typedef std::map<std::string,FontSet>::const_iterator const_fontset_iterator;
        typedef std::map<std::string,FontSet>::iterator fontset_iterator;
        
        /*! \brief Default constructor.
         *
         *  Creates a map with these parameters:
         *  - width = 400
         *  - height = 400
         *  - projection = "+proj=latlong +datum=WGS84"
         */
        Map();

        /*! \brief Constructor
         *  @param width Initial map width.
         *  @param height Initial map height.
         *  @param srs Initial map projection.
         */
        Map(int width, int height, std::string const& srs="+proj=latlong +datum=WGS84");

        /*! \brief Copy Constructur.
         *
         *  @param rhs Map to copy from.
         */
        Map(const Map& rhs);

        /*! \brief Assignment operator
         *
         *  TODO: to be documented
         *  
         */
        Map& operator=(const Map& rhs);
        
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
         *  @return false If no success.
         */
        bool insert_style(std::string const& name,feature_type_style const& style);

        /*! \brief Remove a style from the map.
         *  @param name The name of the style.
         */
        void remove_style(const std::string& name);

        /*! \brief Find a style.
         *  @param name The name of the style.
         *  @return The style if found. If not found return the default map style.
         */
        boost::optional<feature_type_style const&> find_style(std::string const& name) const;
        
        /*! \brief Insert a fontset into the map.
         *  @param name The name of the fontset.
         *  @param style The fontset to insert.
         *  @return true If success.
         *  @return false If failure.
         */
        bool insert_fontset(std::string const& name, FontSet const& fontset);
       
        /*! \brief Find a fontset.
         *  @param name The name of the fontset.
         *  @return The fontset if found. If not found return the default map fontset.
         */
        FontSet const& find_fontset(std::string const& name) const;

        /*! \brief Get all fontsets
         * @return Const reference to fontsets
         */
        std::map<std::string,FontSet> const& fontsets() const;

        /*! \brief Get all fontsets
         * @return Non-constant reference to fontsets
         */
        std::map<std::string,FontSet> & fontsets();

        /*! \brief Get number of all layers.
         */
        size_t layerCount() const;

        /*! \brief Add a layer to the map.
         *  @param l The layer to add.
         */
        void addLayer(const Layer& l);

        /*! \brief Get a layer.
         *  @param index Layer number.
         *  @return Constant layer.
         */
        const Layer& getLayer(size_t index) const;

        /*! \brief Get a layer.
         *  @param index Layer number.
         *  @return Non-constant layer.
         */
        Layer& getLayer(size_t index);
        
        /*! \brief Remove a layer.
         *  @param index Layer number.
         */
        void removeLayer(size_t index);

        /*! \brief Get all layers.
         *  @return Constant layers.
         */
        std::vector<Layer> const& layers() const;

        /*! \brief Get all layers.
         *  @return Non-constant layers.
         */
        std::vector<Layer> & layers();

        /*! \brief Remove all layers and styles from the map.
         */
        void remove_all();

        /*! \brief Get map width.
         */
        unsigned getWidth() const;

        /*! \brief Get map height.
         */
        unsigned getHeight() const;

        /*! \brief Set map width.
         */
        void setWidth(unsigned width);

        /*! \brief Set map height.
         */
        void setHeight(unsigned height);

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
        void set_background(const color& c);

        /*! \brief Get the map background color 
         *  @return Background color as boost::optional
         *  object
         */
        boost::optional<color> const& background() const;

        /*! \brief Set buffer size 
         *  @param buffer_size Buffer size in pixels.
         */
        
        void set_buffer_size(int buffer_size);
        
       /*! \brief Get the map buffer size 
         *  @return Buffer size as int
         */
        int buffer_size() const;
        
        /*! \brief Zoom the map at the current position.
         *  @param factor The factor how much the map is zoomed in or out.
         */
        void zoom(double factor);

        /*! \brief Zoom the map to a bounding box. 
         *
         *  Aspect is handled automatic if not fitting to width/height.
         *  @param box The bounding box where to zoom.
         */
        void zoomToBox(const Envelope<double>& box);

        /*! \brief Zoom the map to show all data.
         */
        void zoom_all();

        void pan(int x,int y);

        void pan_and_zoom(int x,int y,double zoom);

        /*! \brief Get current bounding box.
         *  @return The current bounding box.
         */
        const Envelope<double>& getCurrentExtent() const;

        /*! \brief Get current buffered bounding box.
         *  @return The current buffered bounding box.
         */
        Envelope<double> get_buffered_extent() const;
        
        /*!
         * @return The Map Scale.
         */
        double scale() const;
        
        double scale_denominator() const;

        CoordTransform view_transform() const;
        
        /*!
         * @brief Query a Map Layer (by layer index) for features
         *
         * Intersecting the given x,y location in the coordinates
         * of map projection.
         *
         * @param index The index of the layer to query from.
         * @param x The x coordinate where to query.
         * @param y The y coordinate where to query.
         * @return A Mapnik Featureset if successful otherwise will return NULL.
         */
        featureset_ptr query_point(unsigned index, double x, double y) const;

        /*!
         * @brief Query a Map Layer (by layer index) for features
         *
         * Intersecting the given x,y location in the coordinates
         * of the pixmap or map surface.
         *
         * @param index The index of the layer to query from.
         * @param x The x coordinate where to query.
         * @param y The y coordinate where to query.
         * @return A Mapnik Featureset if successful otherwise will return NULL.
         */
        featureset_ptr query_map_point(unsigned index, double x, double y) const;
        
        ~Map();

        inline void setAspectFixMode(aspect_fix_mode afm) { aspectFixMode_ = afm; }
        inline aspect_fix_mode getAspectFixMode() const { return aspectFixMode_; }

    private:
        void fixAspectRatio();
    };
   
   DEFINE_ENUM(aspect_fix_mode_e,Map::aspect_fix_mode);
}

#endif //MAP_HPP
