/*****************************************************************************
 * 
 * This file is part of the wxPdfDoc modifications for mapnik
 *
 * Copyright (C) 2007 Ben Moores
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

//! \file pdf_renderer_layout.hpp
//! \brief Describes the paper page layout for the pdf map
//! 

#ifndef PDF_RENDERER_LAYOUT_H
#define PDF_RENDERER_LAYOUT_H

// WX
#include <wx/pdfdoc.h>

// STD
#include <vector>

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
#include <mapnik/envelope.hpp>


namespace mapnik {

  //! \brief Describes the layout of the page containing the map
  class MAPNIK_DECL pdf_renderer_layout {
    friend class pdf_renderer;
  public:
    //! \brief Structure describing overlay images
    typedef struct overlay_image_data {
      std::string file; //!< The image file
      double x;         //!< Top left corner x coordinate
      double y;         //!< Top left corner y coordinate
      double width;     //!< Width of the image (0 means scale proportional to height)
      double height;    //!< Height of the image (0 means scale proportional to width)
      double angle;     //!< Rotation angle (degrees)
    } overlay_image_data;

  public:
    //! \brief Constructor
    //!
    //! \param[in] units The units of all the measurements, e.g. "mm","pt","in"
    //! \param[in] width The width of the page (in pdf units)
    //! \param[in] height The height of the page (in pdf units)
    //! \param[in] portrait The orientation of the page, true for portrait, false for landscape
    pdf_renderer_layout(const std::string &units, const double width, const double height, const bool portrait = true);

    //! \brief Destructor
    ~pdf_renderer_layout();

    
    //! Get the page width
    //! \return The page width
    const double get_page_width(void) const {return page_area.width(); };

    //! Get the page height
    //! \return The page height
    const double get_page_height(void) const {return page_area.height(); };

    //! Get the page orientation
    //! \return The page orientation
    const bool get_page_portrait(void) const {return page_portrait; };

    //! Get the page units
    //! \return The page units
    const std::string &get_page_units(void) const {return page_units; };



    //----------------------------------------------------------------------------------------------------
    // Page layout

    //! \brief Set the font to use for overlays
    //!
    //! \param[in] name The name of the font
    void set_font(const std::string font) { font_name = font; };

    //! \brief set the page background colour
    //!
    //! This is separate to the map background colour in the Map class
    //!
    //! \param[in] colour The background colour
    void set_background_colour(const Color &colour) { page_background_colour = colour; };

    //! \brief Set the area of the page to render the map
    //!
    //! \param[in] x Top left corner x coordinate
    //! \param[in] y Top left corner y coordinate
    //! \param[in] width Width of the area
    //! \param[in] height Height of the area
    void set_map_area(const double x, const double y, const double width, const double height);

    //! \brief Set whether to draw a map border
    //!
    //! \param[in] drawBorder True for yes, false for no
    void set_map_area_border(const bool drawBorder) { map_area_border = drawBorder; };

    //! \brief Set the map border width
    //!
    //! \param[in] width Width of the border
    void set_map_area_border_width(const double width) { map_area_border_width = width; };

    //----------------------------------------------------------------------------------------------------
    // Overlay images

    //! \brief Add an overlay image
    //!
    //! Images added here will be overlaid on the map. You can add
    //!  png and pdf files.
    //!
    //! \param[in] image Image filename
    //! \param[in] x Top left corner x coordinate
    //! \param[in] y Top left corner y coordinate
    //! \param[in] width Width of the area
    //! \param[in] height Height of the area
    //! \param[in] angle Rotation angle (degrees)
    void add_overlay_image(const std::string &image, const double x, const double y, const double width, const double height, const double angle);

    //----------------------------------------------------------------------------------------------------
    // Grids

    //! \brief Enable/Disable drawing map grid
    //!
    //! \param[in] enable True to enable, false to disable
    void set_map_grid(const bool enable) { map_grid = enable; };

    //! \brief Enable/Disable drawing scales around edge of map
    //!
    //! \param[in] enable True to enable, false to disable
    void set_border_scales(const bool enable) { border_scales = enable; };

    //! \brief Sets the approximate spacing of the grid lines
    //!
    //! This will be tweaked to give nice round numbers to the grid lines.
    //!
    //! \param[in] spacing The approx spacing of the grid lines
    void set_map_grid_approx_spacing(const double spacing) { map_grid_approx_spacing = spacing; };

    //! \brief Set width of scale bars around edge of map
    //!
    //! \param[in] width The thickness of the bars
    void set_border_scale_width(const double width) { border_scale_width = width; };

    //! \brief Set thickness of lines around the grid edge
    //!
    //! \param[in] width The thickness of the lines
    void set_border_scale_linewidth(const double width) { border_scale_linewidth = width; };

    //! \brief Set the colour of the grid across the map
    //!
    //! \param[in] colour The colour of the grid (with alpha)
    void set_map_grid_colour(const Color &colour) { map_grid_colour = colour; };

    //! \brief Set the map grid line width
    //!
    //! \param[in] width The width of the map grid lines
    void set_map_grid_linewidth(const double width) { map_grid_linewidth = width; };

    //----------------------------------------------------------------------------------------------------
    // Scale bar

    //! \brief Enable/Disable the scale bar
    //!
    //! \param[in] enable True to draw the scale bar, false to disable
    void set_scale_bar(const bool enable) { scale_bar = enable; };

    //! \brief Set the scale bar size and location
    //!
    //! The width of the scale bar will be adjusted to make sense, but it will be
    //!  centered on the provided area.
    //!
    //! \param[in] area The area
    void set_scale_bar_area(const Envelope<double> area) { scale_bar_area = area; };

    //! \brief Sets the scale bar scale factor and label
    //!
    //! See scale_bar_factor and scale_bar_unit for details.
    //!
    //! \param[in] scale_factor The scale factor
    void set_scale_bar_factor(const double scale_factor, const std::string &unit) { scale_bar_factor = scale_factor; scale_bar_unit = unit; };

  private:
    //common parameters
    std::string font_name;              //!< The name of the font to use for text in the overlays

    //page layout
    const Envelope<double> page_area;   //!< Page area, specified in the constructors pdf document
    const std::string page_units;       //!< PDF units of measure
    bool page_portrait;                 //!< True if page is portrait, false for landscape
    Color page_background_colour;       //!< Background page colour (default white)

    Envelope<double> map_area;          //!< Area of the page that the map will occupy
    bool map_area_border;               //!< Whether to draw border around map
    double map_area_border_width;       //!< Thickness of map border (only drawn if no grid/border scales)

    //overlay images
    std::vector<overlay_image_data *> overlay_images; //!< List of overlay images
    typedef std::vector<overlay_image_data *>::iterator OIDitr; //!< overlay image vector iterator



    //grids
    bool map_grid;                      //!< True for draw grid across map
    bool border_scales;                 //!< True to draw edge scales, false otherwise
    double map_grid_approx_spacing;     //!< Approximate spacing between grid. This will be tweaked to give round coordinates
    double border_scale_width;          //!< Thickness of grid bars bordering map
    double border_scale_linewidth;      //!< Thickness of the lines around the grid edge (also used in the scale bar)
    Color map_grid_colour;              //!< Colour to draw the grid
    double map_grid_linewidth;          //!< Thickness of the map grid lines

    //scale bar
    bool scale_bar;                     //!< True to draw scale bar
    Envelope<double> scale_bar_area;    //!< Where to draw the scale bar. The width will be adjusted to give good units, but it will be centered on the given area.
    double scale_bar_factor;            //!< The scale factor to convert 1 projection unit into 1 'scale_bar_unit'.
    std::string scale_bar_unit;         //!< The units of the scale bar, e.g. 'meters' 'kilometers' 'miles'
  };

} //namespace mapnik


#endif //PDF_RENDERER_LAYOUT_H
