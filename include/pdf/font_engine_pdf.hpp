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

#ifndef FONT_ENGINE_PDF_HPP
#define FONT_ENGINE_PDF_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text_path.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/mutex.hpp>

// stl
#include <string>
#include <vector>
#include <map>
#include <iostream>

// WX
#include <wx/pdfdoc.h>

namespace mapnik
{

  class pdf_text_renderer : private boost::noncopyable
  {
  private:
    //! \brief Records width and height of a character
    typedef std::pair<double,double> char_dimension_t;

  public:
    //! \brief Constructor
    //!
    //! \param[in] _pdf The PDF document
    pdf_text_renderer(wxPdfDocument &_pdf);


    //! \brief Sets the options for using external fonts
    //!
    //! \param[in] externalFontPath Path to the fonts, and the generated xml files.
    //! \param[in] enableSubsetting Enables/Disables font subsetting to reduce file size.
    void set_external_font_options(const std::wstring& externalFontPath, const bool enableSubsetting);


    //! \brief Embeds a new font
    //!
    //! \param[in] fontName The name to use for the font internally, does not need to be related
    //!             to the real font name.
    //! \param[in] fontXMLFile The xml file generated with the wxpdfdoc makefont utility
    //! \return true if successfull, false otherwise
    bool add_external_font(const std::wstring& fontName,  const std::wstring& fontXMLFile);


    //! \brief Set the font to use
    //!
    //! The font must either be one of the pdf internal fonts (arial, courier, times, symbol, zapfDingbats)
    //!  or have been loaded with add_external_font(...).
    //!
    //! \param[in] fontThe name of the font
    //! \return True if successfully set, false otherwise
    bool set_font(const std::string& font);


    //! \brief Set the font to use
    //!
    //! The font must either be one of the pdf internal fonts (arial, courier, times, symbol, zapfDingbats)
    //!  or have been loaded with add_external_font(...).
    //!
    //! \param[in] fontThe name of the font
    //! \return True if successfully set, false otherwise
    bool set_font(const std::wstring& font);


    //! \brief Set the font size
    //!
    //! \param[in] size The size in pdf units
    void set_size(const double size);


    //! \brief Set the font colour
    //!
    //! \param[in] _fill The colour
    void set_fill(mapnik::Color const& _fill);


    //! \brief Set the halo colour
    //!
    //! \param[in] _halo The colour
    void set_halo_fill(mapnik::Color const& _halo);


    //! \brief Set the halo radius
    //!
    //! \param[in] radius The radius in pdf units
    void set_halo_radius( const double radius );


    //! \brief Get the dimensions of a character in mm
    //!
    //! <b>Note:</b> The height of the character is taken to be the
    //!  height of a capital letter plus the descent height of the font. This
    //!  should give reasonable results.
    //!
    //! \param[in] c The character
    //! \return The characters dimensions.
    pdf_text_renderer::char_dimension_t character_dimensions(const unsigned c);


    //! \brief Calculates the dimensions of all the characters and the whole string
    //!
    //! \param[in] info The string_info to populate
    void get_string_info(string_info *info);


    //! \brief Prepares to render a whole lot of text
    //!
    //! This sets the line style and colour in case we are rendering
    //!  text halos.
    void prepare_render(void);


    //! \brief Renders a text path
    //!
    //! This renders the text path calculated by the placement_finder.
    //!
    //! \param[in] path The path to render
    //! \param[in] x0 x offset
    //! \param[in] y0 y offset
    void render(text_path *path, double x0, double y0);

    
  private:
    wxPdfDocument       *pdf;               //!< The PDF Document
    const std::wstring  default_font_name;  //!< The default font. This must be an internal pdf font (arial, courier, times, symbol, zapfDingbats)
    std::wstring        font_name;          //!< Name of the font
    double              font_size;          //!< Font size in pdf units
    const wxPdfFontDescription *font_description; //!< Internal information about a font
    mapnik::Color       fill;               //!< Fill colour
    mapnik::Color       halo_fill;          //!< Halo colour
    double              halo_radius;        //!< Halo radius in pdf units
  };

}

#endif //FONT_ENGINE_PDF_HPP
