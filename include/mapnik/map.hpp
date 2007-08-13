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

#include <mapnik/feature_type_style.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/layer.hpp>
#include <boost/optional/optional.hpp>

namespace mapnik
{
    class MAPNIK_DECL Map
    {	
        static const unsigned MIN_MAPSIZE=16;
        static const unsigned MAX_MAPSIZE=MIN_MAPSIZE<<10;
        unsigned width_;
        unsigned height_;
        std::string  srs_;
        boost::optional<Color> background_;
        std::map<std::string,feature_type_style> styles_;
        std::vector<Layer> layers_;
        Envelope<double> currentExtent_;
        
    public:
        typedef std::map<std::string,feature_type_style>::const_iterator const_style_iterator;
        typedef std::map<std::string,feature_type_style>::iterator style_iterator;
        
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
         *  @param The name of the style.
         */
        void remove_style(const std::string& name);

        /*! \brief Find a style.
         * @param name The name of the style.
         *  @return The style if found. If not found return the default map style.
         */
        feature_type_style const& find_style(std::string const& name) const;

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
        void set_background(const Color& c);
        
        /*! \brief Get the map background color 
         *  @return Background color as boost::optional
         *  object
         */
        boost::optional<Color> const& background() const;

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

        double scale() const;

        CoordTransform view_transform() const;

        featureset_ptr query_point(unsigned index, double x, double y) const;

        featureset_ptr query_map_point(unsigned index, double x, double y) const;
        ~Map();
    private:
        void fixAspectRatio();
    };
}

#endif //MAP_HPP
