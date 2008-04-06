///////////////////////////////////////////////////////////////////////////////
// Name:        pdfcolor.cpp
// Purpose:     Implementation of wxPdfDocument color handling
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-01-27
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfcolor.cpp Implementation of the wxPdfDocument color handling

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/pdfdoc.h"

#include "pdfcolordata.inc"

wxColourDatabase* wxPdfColour::ms_colorDatabase = NULL;

wxColourDatabase*
wxPdfColour::GetColorDatabase()
{
  if (ms_colorDatabase == NULL)
  {
    if (wxTheColourDatabase != NULL)
    {
      ms_colorDatabase = wxTheColourDatabase;
    }
    else
    {
      static wxColourDatabase pdfColorDatabase;
      ms_colorDatabase = &pdfColorDatabase;
    }
    size_t n;
    for ( n = 0; n < WXSIZEOF(wxColourTable); n++ )
    {
      const wxColourDesc& cc = wxColourTable[n];
      ms_colorDatabase->AddColour(cc.name, wxColour(cc.r, cc.g, cc.b));
    }
  }
  return ms_colorDatabase;
}

wxPdfColour::wxPdfColour()
{
  m_type   = wxPDF_COLOURTYPE_UNKNOWN;
  m_prefix = wxEmptyString;
  m_color  = _T("0");
}

wxPdfColour::wxPdfColour(const unsigned char grayscale)
{
  SetColor(grayscale);
}

wxPdfColour::wxPdfColour(const wxColour& color)
{
  SetColor(color);
}

wxPdfColour::wxPdfColour(const unsigned char red, const unsigned char green, const unsigned char blue)
{
  SetColor(red, green, blue);
}

wxPdfColour::wxPdfColour(double cyan, double magenta, double yellow, double black)
{
  SetColor(cyan, magenta, yellow, black);
}

wxPdfColour::wxPdfColour(const wxPdfSpotColour& spot, double tint)
{
  SetColor(spot, tint);
}

wxPdfColour::wxPdfColour(const wxString& name)
{
  SetColor(name);
}

wxPdfColour::wxPdfColour(const wxPdfColour& color)
{
  m_type   = color.m_type;
  m_prefix = color.m_prefix;
  m_color  = color.m_color;
}

wxPdfColour::wxPdfColour(const wxString& color, bool WXUNUSED(internal))
{
  m_color = color;
}

wxPdfColour&
wxPdfColour::operator=(const wxPdfColour& color)
{
  m_type   = color.m_type;
  m_prefix = color.m_prefix;
  m_color  = color.m_color;
  return *this;
}

bool
wxPdfColour::Equals(const wxPdfColour& color) const
{
  return (m_type == color.m_type) && (m_prefix == color.m_prefix) && (m_color == color.m_color);
}

void
wxPdfColour::SetColor(const unsigned char grayscale)
{
  m_type   = wxPDF_COLOURTYPE_GRAY;
  m_prefix = wxEmptyString;
  m_color  = wxPdfDocument::Double2String(((double) grayscale)/255.,3);
}

void
wxPdfColour::SetColor(const wxColour& color)
{
  m_type   = wxPDF_COLOURTYPE_RGB;
  m_prefix = wxEmptyString;
  m_color  = wxPdfDocument::RGB2String(color);
}

void
wxPdfColour::SetColor(const unsigned char red, const unsigned char green, const unsigned char blue)
{
  SetColor(wxColour(red,green,blue));
}

void
wxPdfColour::SetColor(double cyan, double magenta, double yellow, double black)
{
  m_type   = wxPDF_COLOURTYPE_CMYK;
  m_prefix = wxEmptyString;
  m_color  = wxPdfDocument::Double2String(wxPdfDocument::ForceRange(cyan,    0., 100.)/100.,3) + _T(" ") +
             wxPdfDocument::Double2String(wxPdfDocument::ForceRange(magenta, 0., 100.)/100.,3) + _T(" ") +
             wxPdfDocument::Double2String(wxPdfDocument::ForceRange(yellow,  0., 100.)/100.,3) + _T(" ") +
             wxPdfDocument::Double2String(wxPdfDocument::ForceRange(black,   0., 100.)/100.,3);
}

void
wxPdfColour::SetColor(const wxString& name)
{
  if (name.Length() == 7 && name[0] == wxT('#'))
  {
    unsigned long r = 0, g = 0, b = 0;
    if (name.Mid(1,2).ToULong(&r,16) &&
        name.Mid(3,2).ToULong(&g,16) &&
        name.Mid(5,2).ToULong(&b,16))
    {
      SetColor((unsigned char) r, (unsigned char) g, (unsigned char) b);
    }
    else
    {
     SetColor(0);
    }
  }
  else
  {
    wxColourDatabase* colorDatabase = GetColorDatabase();
    wxColour color = colorDatabase->Find(name);
    if (color.Ok())
    {
      SetColor(color);
    }
    else
    {
      SetColor(0);
    }
  }
}

void
wxPdfColour::SetColor(const wxPdfSpotColour& spot, double tint)
{
  m_type   = wxPDF_COLOURTYPE_SPOT;
  m_prefix = wxString::Format(_T("/CS%d CS "), spot.GetIndex());
  m_color  = wxPdfDocument::Double2String(wxPdfDocument::ForceRange(tint, 0., 100.)/100.,3);
}

const wxString
wxPdfColour::GetColor(bool drawing) const
{
  wxString color = wxEmptyString;
  switch (m_type)
  {
    case wxPDF_COLOURTYPE_GRAY:
      color = m_color + wxString(_T(" g"));
      break;
    case wxPDF_COLOURTYPE_RGB:
      color = m_color + wxString(_T(" rg"));
      break;
    case wxPDF_COLOURTYPE_CMYK:
      color = m_color + wxString(_T(" k"));
      break;
    case wxPDF_COLOURTYPE_SPOT:
      color = m_prefix + m_color + wxString(_T(" scn"));
      break;
    default:
      color = wxString(_T("0 g"));
      break;
  }
  if (drawing)
    color.MakeUpper();
  else
    color.MakeLower();
  color.Replace(_T("/cs"), _T("/CS"));
  return color;
}

const wxString
wxPdfColour::GetColorValue() const
{
  return m_color;
}

wxPdfSpotColour::wxPdfSpotColour(int index, double cyan, double magenta, double yellow, double black)
  : m_objIndex(0), m_index(index), m_cyan(cyan), m_magenta(magenta), m_yellow(yellow), m_black(black)
{
}

wxPdfSpotColour::wxPdfSpotColour(const wxPdfSpotColour& color)
{
  m_objIndex = color.m_objIndex;
  m_index    = color.m_index;
  m_cyan     = color.m_cyan;
  m_magenta  = color.m_magenta;
  m_yellow   = color.m_yellow;
  m_black    = color.m_black;
}

wxPdfPattern::wxPdfPattern(const int index, const double width, const double height)
  : m_objIndex(0), m_index(index), m_imageWidth(width), m_imageHeight(height)
{
}

wxPdfPattern::wxPdfPattern(const wxPdfPattern& pattern)
{
  m_objIndex    = pattern.m_objIndex;
  m_index       = pattern.m_index; 
  m_imageWidth  = pattern.m_imageWidth;
  m_imageHeight = pattern.m_imageHeight;
  m_image       = pattern.m_image;
}

// ---

void
wxPdfDocument::AddPattern(const wxString& name, const wxImage& image, const wxString& imageName, const double width, const double height)
{
  wxPdfPatternMap::iterator pattern = (*m_patterns).find(name);
  wxPdfPattern *newPattern;
  if (pattern == (*m_patterns).end())
  {
    int i = (*m_patterns).size() + 1;
    newPattern = new wxPdfPattern(i, width, height);
    (*m_patterns)[name] = newPattern;


    //store the image
    wxImage tempImage = image.Copy();
    wxPdfImage* currentImage = NULL;
    wxPdfImageHashMap::iterator imageIter = (*m_images).find(imageName);
    int maskImage;
    if (imageIter == (*m_images).end())
    {
      if (tempImage.HasAlpha())
      {
        maskImage = ImageMask(imageName+wxString(_T(".mask")), tempImage);
        tempImage.ConvertAlphaToMask(0);

      }
      // First use of image, get info
      tempImage.SetMask(false);
      int i = (*m_images).size() + 1;
      currentImage = new wxPdfImage(this, i, imageName, tempImage);
      currentImage->Parse();
      currentImage->SetMaskImage(maskImage);
      (*m_images)[imageName] = currentImage;
    }

    imageIter = (*m_images).find(imageName);
    newPattern->SetImage( (*imageIter).second );

  }
}


void
wxPdfDocument::AddSpotColor(const wxString& name, double cyan, double magenta, double yellow, double black)
{
  wxPdfSpotColourMap::iterator spotColor = (*m_spotColors).find(name);
  if (spotColor == (*m_spotColors).end())
  {
    int i = (*m_spotColors).size() + 1;
    (*m_spotColors)[name] = new wxPdfSpotColour(i, cyan, magenta, yellow, black);
  }
}
 
void
wxPdfDocument::SetDrawColorSpace(const int space)
{
  switch(space) {
  case 1:
    OutAscii(_T("/Pattern CS"));
    break;
  case 0:
  default:
    OutAscii(_T("/DeviceRGB CS"));
    break;
  }
}

void
wxPdfDocument::SetFillColorSpace(const int space)
{
  switch(space) {
  case 1:
    OutAscii(_T("/Pattern cs"));
    break;
  case 0:
  default:
    OutAscii(_T("/DeviceRGB cs"));
    break;
  }
}

void
wxPdfDocument::SetDrawColor(const wxColour& color)
{
  wxPdfColour tempColor(color);
  m_drawColor = tempColor;
  if (m_page > 0)
  {
    OutAscii(m_drawColor.GetColor(true));
  }
}

void
wxPdfDocument::SetDrawColor(const unsigned char grayscale)
{
  wxPdfColour tempColor(grayscale);
  m_drawColor = tempColor;
  if (m_page > 0)
  {
    OutAscii(m_drawColor.GetColor(true));
  }
}

void
wxPdfDocument::SetDrawColor(const unsigned char red, const unsigned char green, const unsigned char blue)
{
  SetDrawColor(wxColour(red, green, blue));
}

void
wxPdfDocument::SetDrawColor(double cyan, double magenta, double yellow, double black)
{
  SetDrawColor(wxPdfColour(cyan, magenta, yellow, black));
}

void
wxPdfDocument::SetDrawColor(const wxPdfColour& color)
{
  m_drawColor = color;
  if (m_page > 0)
  {
    OutAscii(m_drawColor.GetColor(true));
  }
}

void
wxPdfDocument::SetDrawColor(const wxString& name, double tint)
{
  wxPdfSpotColourMap::iterator spotColor = (*m_spotColors).find(name);
  if (spotColor != (*m_spotColors).end())
  {
    wxPdfColour tempColor(*(spotColor->second), tint);
    m_drawColor = tempColor;
    if (m_page > 0)
    {
      OutAscii(m_drawColor.GetColor(true));
    }
  }
  else
  {
    wxLogError(_("SetDrawColor: Undefined spot color: ") + name);
  }
}

void
wxPdfDocument::SetDrawPattern(const wxString& name) {
  wxPdfPatternMap::iterator patternIter = (*m_patterns).find(name);
  wxPdfPattern *pattern;
  if( patternIter != (*m_patterns).end()) 
  {
    pattern = patternIter->second;
    OutAscii(wxString::Format(_T("/P%d SCN"), pattern->GetIndex()));
  }
  else {
    wxLogError(_("SetFillPattern: Undefined pattern: ") + name);
  }
}

const wxPdfColour
wxPdfDocument::GetDrawColor()
{
  return wxPdfColour(m_drawColor);
}

void
wxPdfDocument::SetFillColor(const wxColour& color)
{
  wxPdfColour tempColor(color);
  m_fillColor = tempColor;
  m_colorFlag = (m_fillColor != m_textColor);
  if (m_page > 0)
  {
    OutAscii(m_fillColor.GetColor(false));
  }
}

void
wxPdfDocument::SetFillColor(const unsigned char grayscale)
{
  wxPdfColour tempColor(grayscale);
  m_fillColor = tempColor;
  m_colorFlag = (m_fillColor != m_textColor);
  if (m_page > 0)
  {
    OutAscii(m_fillColor.GetColor(false));
  }
}

void
wxPdfDocument::SetFillColor(const wxPdfColour& color)
{
  m_fillColor = color;
  m_colorFlag = (m_fillColor != m_textColor);
  if (m_page > 0)
  {
    OutAscii(m_fillColor.GetColor(false));
  }
}

void
wxPdfDocument::SetFillColor(const unsigned char red, const unsigned char green, const unsigned char blue)
{
  SetFillColor(wxColour(red, green, blue));
}

void
wxPdfDocument::SetFillColor(double cyan, double magenta, double yellow, double black)
{
  SetFillColor(wxPdfColour(cyan, magenta, yellow, black));
}

void
wxPdfDocument::SetFillColor(const wxString& name, double tint)
{
  wxPdfSpotColourMap::iterator spotColor = (*m_spotColors).find(name);
  if (spotColor != (*m_spotColors).end())
  {
    wxPdfColour tempColor(*(spotColor->second), tint);
    m_fillColor = tempColor;
    m_colorFlag = (m_fillColor != m_textColor);
    if (m_page > 0)
    {
      OutAscii(m_fillColor.GetColor(false));
    }
  }
  else
  {
    wxLogError(_("SetFillColor: Undefined spot color: ") + name);
  }
}

void
wxPdfDocument::SetFillPattern(const wxString& name) {
  wxPdfPatternMap::iterator patternIter = (*m_patterns).find(name);
  wxPdfPattern *pattern;
  if( patternIter != (*m_patterns).end()) 
  {
    pattern = patternIter->second;
    OutAscii(wxString::Format(_T("/P%d scn"), pattern->GetIndex()));
  }
  else {
    wxLogError(_("SetFillPattern: Undefined pattern: ") + name);
  }
}


const wxPdfColour
wxPdfDocument::GetFillColor()
{
  return wxPdfColour(m_fillColor);
}

void
wxPdfDocument::SetTextColor(const wxColour& color)
{
  wxPdfColour tempColor(color);
  m_textColor = tempColor;
  m_colorFlag = (m_fillColor != m_textColor);
}

void
wxPdfDocument::SetTextColor(const unsigned char grayscale)
{
  wxPdfColour tempColor(grayscale);
  m_textColor = tempColor;
  m_colorFlag = (m_fillColor != m_textColor);
}

void
wxPdfDocument::SetTextColor(const wxPdfColour& color)
{
  m_textColor = color;
  m_colorFlag = (m_fillColor != m_textColor);
}

void
wxPdfDocument::SetTextColor(const unsigned char red, const unsigned char green, const unsigned char blue)
{
  SetTextColor(wxColour(red, green, blue));
}

void
wxPdfDocument::SetTextColor(double cyan, double magenta, double yellow, double black)
{
  SetTextColor(wxPdfColour(cyan, magenta, yellow, black));
}

void
wxPdfDocument::SetTextColor(const wxString& name, double tint)
{
  wxPdfSpotColourMap::iterator spotColor = (*m_spotColors).find(name);
  if (spotColor != (*m_spotColors).end())
  {
    wxPdfColour tempColor(*(spotColor->second), tint);
    m_textColor = tempColor;
    m_colorFlag = (m_fillColor != m_textColor);
  }
  else
  {
    wxLogError(_("SetTextColor: Undefined spot color: ") + name);
  }
}
 
const wxPdfColour
wxPdfDocument::GetTextColor()
{
  return wxPdfColour(m_textColor);
}

bool operator==(const wxPdfColour& a, const wxPdfColour& b)
{
  return a.Equals(b);
}

bool operator!=(const wxPdfColour& a, const wxPdfColour& b)
{
  return !a.Equals(b);
}
