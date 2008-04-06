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

//! \file pdf_renderer.hpp
//! \brief PDF renderer for mapnik data
//! 

#ifndef PDF_RENDERER_H
#define PDF_RENDERER_H

// pdf renderer
#include "font_engine_pdf.hpp"
namespace mapnik { class pdf_renderer_layout; }


// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/map.hpp>
#include <mapnik/image_data.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// WX
#include <wx/pdfdoc.h>
class wxImage;

namespace mapnik {

  //! \brief round a number to nearest integer
  //!
  //! \param[in] x number to round
  //! \return number rounded to nearest integer
  #define round(x) ( (x)>=0 ? (long)((x)+0.5) : (long)((x)-0.5) )

  //class prototypes
  class pdf_point_symbolizer;
  class pdf_line_pattern_symbolizer;
  class pdf_polygon_pattern_symbolizer;
  class pdf_shield_symbolizer;


  //! \brief Renderer for PDFs
  class MAPNIK_DECL pdf_renderer 
  : public feature_style_processor<pdf_renderer>, private boost::noncopyable
  {
  public:
    //! \brief Constructor
    //!
    //! \param[in] m The map data
    //! \param[in] _pdf The PDF document to populate
    //! \param[in] _page_layout Describes the layout of the map page
    //! \param[in] offset_x Number of pdf units to offset the map data in x direction
    //! \param[in] offset_y Number of pdf units to offset the map data in y direction
    pdf_renderer(Map const& m, wxPdfDocument &_pdf, const pdf_renderer_layout &_page_layout, const double offset_x=0.0, const double offset_y=0.0);


    //! \brief Sets the options for using external fonts
    //!
    //! See http://wxcode.sourceforge.net/docs/wxpdfdoc/makefont.html for how
    //!  to generate the required xml files for external fonts.
    //!
    //! For example:
    //! - ttf2ufm -a trebuc.ttf trebuc
    //! - makefont -a trebuc.afm -f trebuc.ttf -e cp1252
    //!
    //! \param[in] externalFontPath Path to the fonts, and the generated xml files.
    //! \param[in] enableSubsetting Enables/Disables font subsetting to reduce file size.
    void set_external_font_options(const std::wstring& externalFontPath, const bool enableSubsetting);


    //! \brief Embeds a new font
    //!
    //! See set_external_font_options for how to generate the xml file.
    //!
    //! \param[in] fontName The name to use for the font internally, does not need to be related
    //!             to the real font name.
    //! \param[in] fontXMLFile The xml file generated with the wxpdfdoc makefont utility
    //! \return true if successfull, false otherwise
    bool add_external_font(const std::wstring& fontName,  const std::wstring& fontXMLFile);


    //! \brief Called at the start of map processing
    //!
    //! \param[in] map The map data
    void start_map_processing(Map const& map);

    //! \brief Called at the end of map processing
    //!
    //! \param[in] map The map data
    void end_map_processing(Map const& map);

    //! \brief Called before each layer is processed
    //!
    //! \param[in] lay The layer data
    void start_layer_processing(Layer const& lay);

    //! \brief Called after each layer has been proecssed
    //!
    //! \param[in] lay The layer data
    void end_layer_processing(Layer const& lay);

    //! \brief Process point symbolizer
    void process(point_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);
    
    //! \brief Process pdf point symbolizer
    void process(pdf_point_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process line symbolizer
    void process(line_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process line pattern symbolizer
    void process(line_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process pdf line pattern symbolizer
    void process(pdf_line_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process polygon symbolizer
    void process(polygon_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process polygon pattern symbolizer
    void process(polygon_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process pdf polygon pattern symbolizer
    void process(pdf_polygon_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process raster symbolizer
    void process(raster_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process shield symbolizer
    void process(shield_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process pdf shield symbolizer
    void process(pdf_shield_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process text symbolizer
    void process(text_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process building symbolizer
    void process(building_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);

    //! \brief Process marker symbolizer
    //!
    //! <b> This is not currently supported by the PDF renderer</b>
    void process(markers_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans);
    

    //! \brief convert string to wstring
    //!
    //! \param[in] str string to convert
    //! \return the wide string
    static std::wstring convert_string_to_wstring(const std::string str);

  private:
    //! \brief Creates a wxImage from an ImageData32 image
    //!
    //! This performs all the pixel format conversions. dst is 
    //!  created by the function, and must be deleted by the
    //!  caller when no longer required.
    //!
    //! \param[in] src The source image
    //! \param[in] dst Pointer to the wx image pointer that will be created and returned
    void create_wximage_from_imagedata(boost::shared_ptr<ImageData32> src, wxImage **dst);      

    //! \brief Creates a wxImage from an ImageData32 image
    //!
    //! This performs all the pixel format conversions. dst is 
    //!  created by the function, and must be deleted by the
    //!  caller when no longer required.
    //!
    //! \param[in] src The source image
    //! \param[in] dst Pointer to the wx image pointer that will be created and returned
    void create_wximage_from_imagedata(const ImageData32* src, wxImage **dst);      
    
    //! \brief Generates a pdf shape from a transformed path
    //!
    //! This takes a path (wrapped with a coordinate transform) and converts
    //!  it into a pdf shape.
    //!
    //! \param[in] transformed_path The transformed path
    //! \param[in] shape The shape to fill
    void create_shape_from_path(const coord_transform2<CoordTransform,geometry2d> &transformed_path, wxPdfShape &shape);

    //! \brief Get a unique image name
    //!
    //! This uses unnamed_image_count to generate a unique name
    //!
    //! \return The name
    std::wstring generate_unique_image_name(void);

  
    //! \brief Do all the overlay rendering
    //!
    //! This parses the page_layout to render all the additional
    //!  content.
    void render_overlays(void);

    //! \brief Render overlay images
    void render_overlay_images(void);

    //! \brief Render the grid
    void render_overlay_grid(void);

    //! \brief Render the scale bar
    void render_overlay_scale_bar(void);


  private:
      Map const& map;                       //!< The map being rendered

      CoordTransform coord_trans;           //!< Converter for moving between map cartesian coordinates and pdf coordinates
      pdf_text_renderer text_renderer;      //!< Utility for rendering pdf text
      label_collision_detector4 detector;   //!< Collision detector

      wxPdfDocument *pdf;                   //!< The pdf document being filled
      const pdf_renderer_layout &page_layout; //!< The layout of the map page
      double line_miter_limit;              //!< Line miter limit
      unsigned int unnamed_image_count;     //!< Counter used for generating unique name for unnamed images (e.g. rasters)
  };
}




#endif //PDF_RENDERER_H
