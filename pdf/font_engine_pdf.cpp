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

#if ENABLE_PDF

// pdf
#include <pdf/font_engine_pdf.hpp>

namespace mapnik {

  pdf_text_renderer::pdf_text_renderer(wxPdfDocument &_pdf)
      : fill(0,0,0), 
        halo_fill(255,255,255),
        halo_radius(0),
        default_font_name(_T("arial"))
  {
    pdf = &_pdf;
    font_description = NULL;

    //set the other defaults
    set_size(10);
    set_font(default_font_name);

  }


  void pdf_text_renderer::set_external_font_options(const std::wstring& externalFontPath, const bool enableSubsetting) {
    pdf->SetFontPath(externalFontPath);
    pdf->SetFontSubsetting(enableSubsetting);
  }


  bool pdf_text_renderer::add_external_font(const std::wstring& fontName,  const std::wstring& fontXMLFile) {
    return pdf->AddFont(fontName, _T(""), fontXMLFile);
  }



  bool pdf_text_renderer::set_font(const std::string& font) {
    std::wstring wFont;

    //convert to wide name
    std::string::const_iterator itr;
    for(itr = font.begin();itr!=font.end();itr++) {
      wFont.push_back(*itr);
    }

    return set_font(wFont);
  }

  bool pdf_text_renderer::set_font(const std::wstring& font) {

    //try to set font
    if(pdf->SetFont(font, _T(""), pdf->GetFontSize()))
    {
      font_name = font;

      font_description = &pdf->GetFontDescription();

      return true;
    }

    //if we get here, give up and use the default which always exists (internal pdf font)
    std::wclog << __FILE__ << ":" << __LINE__ << ": failed to set font: " << font << ". Using default: " << default_font_name << "\n";
    pdf->SetFont(default_font_name, _T(""), pdf->GetFontSize());
    font_description = &pdf->GetFontDescription();

    return false;
  }


  void pdf_text_renderer::set_size(const double size) {
    font_size = size;
    pdf->SetFontSize(font_size * pdf->GetScaleFactor());
  }


  void pdf_text_renderer::set_fill(mapnik::Color const& _fill)
  {
    fill = _fill;
  }


  void pdf_text_renderer::set_halo_fill(mapnik::Color const& _halo)
  {
    halo_fill = _halo;
  }


  void pdf_text_renderer::set_halo_radius( const double radius )
  {
    halo_radius = radius;
  }


  pdf_text_renderer::char_dimension_t pdf_text_renderer::character_dimensions(const unsigned c)
  {
    std::wstring str;
    str.push_back(c);

    char_dimension_t dim;
    dim.first = pdf->GetStringWidth(str); //gets it in pdf units, not points. no conversion needed.
    dim.second = (font_description->GetCapHeight() - font_description->GetDescent()) * font_size / 1000.0;      // The cap-height and descent are scaled by 1000, and are relative to a font size of 1.

    return dim;
  }


  void pdf_text_renderer::get_string_info(string_info *info)
  {
    double width = 0;
    double height = 0;

    const UnicodeString &text = info->get_string();

    for(int i=0;i<text.length();i++)
    {
      unsigned int c = text[i];
      char_dimension_t char_dim = character_dimensions(c);

      info->add_info(c, char_dim.first, char_dim.second);

      width += char_dim.first;
      height = char_dim.second > height ? char_dim.second : height;
    }
    info->set_dimensions(width, height);
  }

  void pdf_text_renderer::prepare_render(void)
  {
    pdf->SetTextColor(fill.red(), fill.green(), fill.blue());
    wxPdfLineStyle lineStyle;
    lineStyle.SetWidth(halo_radius);
    lineStyle.SetColour(wxPdfColour(halo_fill.red(), halo_fill.green(), halo_fill.blue()));
    lineStyle.SetLineJoin(wxPDF_LINEJOIN_ROUND);
    lineStyle.SetLineCap(wxPDF_LINECAP_ROUND);
    pdf->SetLineStyle(lineStyle);

  }



  void pdf_text_renderer::render(text_path *path, double x0, double y0)
  {
    int c;
    double x, y, angle;
    std::wstring character;

    //render the halos if they're used
    if(halo_radius > 0) {
      for (int i = 0; i < path->num_nodes(); i++) 
      {
        path->vertex(&c, &x, &y, &angle);

        character.clear();
        character.push_back(c);

        //note that angle has to be converted from radians to degrees, and y direction is opposite to what you expect
        pdf->RotatedText(x0 + x, y0 - y, character, (180.0/M_PI)*angle, 1);  //stroke, no fill
      }
    }

    //rewind the iterator
    path->itr_ = 0; 

    //render the text
    for (int i = 0; i < path->num_nodes(); i++) 
    {
      path->vertex(&c, &x, &y, &angle);

      character.clear();
      character.push_back(c);

      //note that angle has to be converted from radians to degrees, and y direction is opposite to what you expect
      pdf->RotatedText(x0 + x, y0 - y, character, (180.0/M_PI)*angle, 0);  //fill, no stroke
    }
  }

}

#endif //ENABLE_PDF
