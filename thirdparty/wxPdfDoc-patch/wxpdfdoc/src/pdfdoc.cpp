///////////////////////////////////////////////////////////////////////////////
// Name:        pdfdoc.cpp
// Purpose:     Implementation of wxPdfDocument (public methods)
// Author:      Ulrich Telle
// Modified by:
// Created:     2005-08-04
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfdoc.cpp Implementation of the wxPdfDoc class

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/image.h>
#include <wx/paper.h>

#include "wx/pdfdoc.h"
#include "wx/pdfform.h"
#include "wx/pdfgraphics.h"
#include "wx/pdftemplate.h"

// ----------------------------------------------------------------------------
// wxPdfDocument: class representing a PDF document
// ----------------------------------------------------------------------------

wxPdfDocument::wxPdfDocument(int orientation, const wxString& unit, wxPaperSize format)
{
  double w, h;
  double unitscale;

  // Scale factor
  if (unit == _T("pt"))
  {
    unitscale = 1.;
  }
  else if (unit == _T("in"))
  {
    unitscale = 72.;
  }
  else if (unit == _T("cm"))
  {
    unitscale = 72. / 2.54;
  }
  else // if (unit == "mm") or unknown
  {
    unitscale = 72. / 25.4;
  }


  // get the page width/height and pass to other constructor

  wxPrintPaperDatabase* printPaperDatabase = new wxPrintPaperDatabase;
  printPaperDatabase->CreateDatabase();
  wxPrintPaperType* paperType = printPaperDatabase->FindPaperType(format);
  if (paperType == NULL)
  {
    paperType = printPaperDatabase->FindPaperType(wxPAPER_A4);
  }
  wxSize paperSize = paperType->GetSize();
  //get the width and height in user units. papersize is in tenths of a mm, convert to inches, then points, then to user units
  w = (paperSize.GetWidth() / 254.0) * 72.0 * unitscale;
  h = (paperSize.GetHeight() / 254.0) * 72.0 * unitscale;
  delete printPaperDatabase;

  wxPdfDocument(w, h, orientation, unit);
}

wxPdfDocument::wxPdfDocument(const double paperWidth, const double paperHeight, int orientation, const wxString& unit) {
  // Allocate arrays
  m_currentFont = NULL;

  m_page       = 0;
  m_n          = 2;
  m_offsets = new wxPdfOffsetHashMap();

  m_pages = new wxPdfPageHashMap();
  m_orientationChanges = new wxPdfBoolHashMap();

  m_state      = 0;

  InitializeCoreFonts();
  m_fonts       = new wxPdfFontHashMap();
  m_images      = new wxPdfImageHashMap();
  m_pageLinks   = new wxPdfPageLinksMap();
  m_links       = new wxPdfLinkHashMap();
  m_namedLinks  = new wxPdfNamedLinksMap();
  m_diffs       = new wxPdfDiffHashMap();
  m_extGStates  = new wxPdfExtGStateMap();
  m_extGSLookup = new wxPdfExtGSLookupMap();
  m_currentExtGState = 0;
  m_gradients   = new wxPdfGradientMap();
  m_annotations = new wxPdfAnnotationsMap();
  m_formAnnotations = new wxPdfFormAnnotsMap();
  m_formFields  = new wxPdfFormFieldsMap();
  m_radioGroups = new wxPdfRadioGroupMap();
  m_templates   = new wxPdfTemplatesMap();
  m_parsers     = new wxPdfParserMap();
  m_spotColors  = new wxPdfSpotColourMap();
  m_patterns    = new wxPdfPatternMap();

  m_outlineRoot     = -1;
  m_maxOutlineLevel = 0;

  m_inFooter   = false;
  m_lasth      = 0;
  m_fontFamily = _T("");
  m_fontStyle  = _T("");
  m_fontSizePt = 12;
  m_decoration = wxPDF_FONT_NORMAL;
  m_fontSubsetting = true;

  m_drawColor  = wxPdfColour();
  m_fillColor  = wxPdfColour();
  m_textColor  = wxPdfColour();
  m_colorFlag  = false;
  m_ws         = 0;

  // Scale factor
  if (unit == _T("pt"))
  {
    m_k = 1.;
  }
  else if (unit == _T("in"))
  {
    m_k = 72.;
  }
  else if (unit == _T("cm"))
  {
    m_k = 72. / 2.54;
  }
  else // if (unit == "mm") or unknown
  {
    m_k = 72. / 25.4;
  }

  // Initialize image scale factor
  m_imgscale = 1.;

  // Page format
  m_fw = paperWidth;
  m_fh = paperHeight;
  m_fwPt = m_fw * m_k;
  m_fhPt = m_fh * m_k;


  /*wxPrintPaperDatabase* printPaperDatabase = new wxPrintPaperDatabase;
  printPaperDatabase->CreateDatabase();
  wxPrintPaperType* paperType = printPaperDatabase->FindPaperType(format);
  if (paperType == NULL)
  {
    paperType = printPaperDatabase->FindPaperType(wxPAPER_A4);
  }
  wxSize paperSize = paperType->GetSize();
  m_fwPt = paperSize.GetWidth() / 254. * 72.;
  m_fhPt = paperSize.GetHeight() / 254. * 72.;
  delete printPaperDatabase;

  m_fw = m_fwPt / m_k;
  m_fh = m_fhPt / m_k;
*/

  // Page orientation
  if (orientation == wxLANDSCAPE)
  {
    m_defOrientation = wxLANDSCAPE;
    m_wPt = m_fhPt;
    m_hPt = m_fwPt;
  }
  else // orientation == wxPORTRAIT or unknown
  {
    m_defOrientation = wxPORTRAIT;
    m_wPt = m_fwPt;
    m_hPt = m_fhPt;
  }
  
  m_curOrientation = m_defOrientation;
  m_w = m_wPt / m_k;
  m_h = m_hPt / m_k;
  m_angle = 0;
  m_inTransform = 0;

  // Page margins (1 cm)
  double margin = 28.35 / m_k;
  SetMargins(margin, margin);
  
  // Interior cell margin (1 mm)
  m_cMargin = margin / 10;
  
  // Line width (0.2 mm)
  m_lineWidth = .567 / m_k;
  
  // Automatic page break
  SetAutoPageBreak(true, 2*margin);
  
  // Full width display mode
  SetDisplayMode(wxPDF_ZOOM_FULLWIDTH);
  m_zoomFactor = 100.;

  // Default viewer preferences
  m_viewerPrefs = 0;

  // Enable compression
  SetCompression(true);

  // Set default PDF version number
  m_PDFVersion = _T("1.3");
  m_importVersion = m_PDFVersion;

  m_encrypted = false;
  m_encryptor = NULL;

  m_javascript = wxEmptyString;

  m_inTemplate = false;
  m_templateId = 0;
  m_templatePrefix = _T("/TPL");

  m_currentParser = NULL;
  m_currentSource = wxEmptyString;

  SetFontPath();
  SetFont(_T("ZapfDingBats"), _T(""), 9);
  m_zapfdingbats = m_currentFont->GetIndex();
}



wxPdfDocument::~wxPdfDocument()
{
  delete m_coreFonts;

  wxPdfFontHashMap::iterator font = m_fonts->begin();
  for (font = m_fonts->begin(); font != m_fonts->end(); font++)
  {
    if (font->second != NULL)
    {
      delete font->second;
    }
  }
  delete m_fonts;

  wxPdfImageHashMap::iterator image = m_images->begin();
  for (image = m_images->begin(); image != m_images->end(); image++)
  {
    if (image->second != NULL)
    {
      delete image->second;
    }
  }
  delete m_images;

  wxPdfPageHashMap::iterator page = m_pages->begin();
  for (page = m_pages->begin(); page != m_pages->end(); page++)
  {
    if (page->second != NULL)
    {
      delete page->second;
    }
  }
  delete m_pages;

  wxPdfPageLinksMap::iterator pageLinks = m_pageLinks->begin();
  for (pageLinks = m_pageLinks->begin(); pageLinks != m_pageLinks->end(); pageLinks++)
  {
    if (pageLinks->second != NULL)
    {
      delete pageLinks->second;
    }
  }
  delete m_pageLinks;

  wxPdfLinkHashMap::iterator link = m_links->begin();
  for (link = m_links->begin(); link != m_links->end(); link++)
  {
    if (link->second != NULL)
    {
      delete link->second;
    }
  }
  delete m_links;

  delete m_namedLinks;

  size_t j;
  for (j = 0; j < m_outlines.GetCount(); j++)
  {
    wxPdfBookmark* bookmark = (wxPdfBookmark*) m_outlines[j];
    delete bookmark;
  }

  wxPdfDiffHashMap::iterator diff = m_diffs->begin();
  for (diff = m_diffs->begin(); diff != m_diffs->end(); diff++)
  {
    if (diff->second != NULL)
    {
      delete diff->second;
    }
  }
  delete m_diffs;

  wxPdfExtGStateMap::iterator extGState = m_extGStates->begin();
  for (extGState = m_extGStates->begin(); extGState != m_extGStates->end(); extGState++)
  {
    if (extGState->second != NULL)
    {
      delete extGState->second;
    }
  }
  delete m_extGStates;

  delete m_extGSLookup;

  wxPdfGradientMap::iterator gradient = m_gradients->begin();
  for (gradient = m_gradients->begin(); gradient != m_gradients->end(); gradient++)
  {
    if (gradient->second != NULL)
    {
      delete gradient->second;
    }
  }
  delete m_gradients;

  wxPdfAnnotationsMap::iterator annotation = m_annotations->begin();
  for (annotation = m_annotations->begin(); annotation != m_annotations->end(); annotation++)
  {
    if (annotation->second != NULL)
    {
      delete annotation->second;
    }
  }
  delete m_annotations;

  wxPdfFormAnnotsMap::iterator formAnnotation = m_formAnnotations->begin();
  for (formAnnotation = m_formAnnotations->begin(); formAnnotation != m_formAnnotations->end(); formAnnotation++)
  {
    if (formAnnotation->second != NULL)
    {
      delete formAnnotation->second;
    }
  }
  delete m_formAnnotations;

  wxPdfFormFieldsMap::iterator formField = m_formFields->begin();
  for (formField = m_formFields->begin(); formField != m_formFields->end(); formField++)
  {
    if (formField->second != NULL)
    {
      delete formField->second;
    }
  }
  delete m_formFields;

  wxPdfRadioGroupMap::iterator radioGroup = m_radioGroups->begin();
  for (radioGroup = m_radioGroups->begin(); radioGroup != m_radioGroups->end(); radioGroup++)
  {
    if (radioGroup->second != NULL)
    {
      delete radioGroup->second;
    }
  }
  delete m_radioGroups;

  wxPdfTemplatesMap::iterator templateIter = m_templates->begin();
  for (templateIter = m_templates->begin(); templateIter != m_templates->end(); templateIter++)
  {
    if (templateIter->second != NULL)
    {
      delete templateIter->second;
    }
  }
  delete m_templates;

  wxPdfParserMap::iterator parser = m_parsers->begin();
  for (parser = m_parsers->begin(); parser != m_parsers->end(); parser++)
  {
    if (parser->second != NULL)
    {
      delete parser->second;
    }
  }
  delete m_parsers;

  wxPdfSpotColourMap::iterator spotColor = m_spotColors->begin();
  for (spotColor = m_spotColors->begin(); spotColor != m_spotColors->end(); spotColor++)
  {
    if (spotColor->second != NULL)
    {
      delete spotColor->second;
    }
  }
  delete m_spotColors;

  wxPdfPatternMap::iterator pattern = m_patterns->begin();
  for(pattern = m_patterns->begin(); pattern != m_patterns->end(); pattern++) 
  {
    if(pattern->second != NULL)
    {
      delete pattern->second;
    }
  }
  delete m_patterns;

  delete m_orientationChanges;

  delete m_offsets;

  if (m_encryptor != NULL)
  {
    delete m_encryptor;
  }
}

// --- Public methods

void
wxPdfDocument::SetProtection(int permissions,
                             const wxString& userPassword,
                             const wxString& ownerPassword,
                             wxPdfEncryptionMethod encryptionMethod,
                             int keyLength)
{
  if (m_encryptor == NULL)
  {
    int revision = (keyLength > 0) ? 3 : 2;
    switch (encryptionMethod)
    {
      case wxPDF_ENCRYPTION_AESV2:
        revision = 4;
        if (m_PDFVersion < _T("1.6"))
        {
          m_PDFVersion = _T("1.6");
        }
        break;
      case wxPDF_ENCRYPTION_RC4V2:
        revision = 3;
        break;
      case wxPDF_ENCRYPTION_RC4V1:
      default:
        revision = 2;
        break;
    }
    m_encryptor = new wxPdfEncrypt(revision, keyLength);
    m_encrypted = true;
    int allowedFlags = wxPDF_PERMISSION_PRINT | wxPDF_PERMISSION_MODIFY |
                       wxPDF_PERMISSION_COPY  | wxPDF_PERMISSION_ANNOT;
    int protection = 192;
    protection += (permissions & allowedFlags);
    wxString ownerPswd = ownerPassword;
    if (ownerPswd.Length() == 0)
    {
      ownerPswd = wxPdfDocument::GetUniqueId(_T("wxPdfDoc"));
    }
    m_encryptor->GenerateEncryptionKey(userPassword, ownerPswd, protection);
  }
}

void
wxPdfDocument::SetImageScale(double scale)
{
  m_imgscale = scale;
}

double
wxPdfDocument::GetImageScale()
{
  return m_imgscale;
}

double
wxPdfDocument::GetPageWidth()
{
  return m_w;
}

double
wxPdfDocument::GetPageHeight()
{
  return m_h;
}

double
wxPdfDocument::GetBreakMargin()
{
  return m_bMargin;
}

double
wxPdfDocument::GetScaleFactor()
{
  return m_k;
}

void
wxPdfDocument::AliasNbPages(const wxString& alias)
{
  // Define an alias for total number of pages
  m_aliasNbPages = alias;
}

void
wxPdfDocument::Open()
{
  // Begin document
  m_state = 1;
}

void
wxPdfDocument::AddPage(int orientation)
{
  if (m_inTemplate)
  {
    wxLogError(_("wxPdfDocument::AddPage: Adding pages in templates is impossible. Current template ID is %d."), m_templateId);
    return;
  }

  // Start a new page
  if (m_state == 0)
  {
    Open();
  }
  wxString family = m_fontFamily;
  wxString style = m_fontStyle;
  if (m_decoration & wxPDF_FONT_UNDERLINE)
  {
    style += wxString(_T("U"));
  }
  if (m_decoration & wxPDF_FONT_OVERLINE)
  {
    style += wxString(_T("O"));
  }
  if (m_decoration & wxPDF_FONT_STRIKEOUT)
  {
    style += wxString(_T("S"));
  }
  double size = m_fontSizePt;
  double lw = m_lineWidth;
  wxPdfColour dc = m_drawColor;
  wxPdfColour fc = m_fillColor;
  wxPdfColour tc = m_textColor;
  bool cf = m_colorFlag;

  if (m_page > 0)
  {
    // Page footer
    m_inFooter = true;
    Footer();
    m_inFooter = false;
    // Close page
    EndPage();
  }

  // Start new page
  BeginPage(orientation);
  
  // Set line cap style to square
  Out("2 J");
  
  // Set line width
  m_lineWidth = lw;
  OutAscii(Double2String(lw*m_k,2)+wxString(_T(" w")));

  // Set font
  if (family.Length() > 0)
  {
    SetFont(family, style, size);
  }
  
  // Set colors
  m_drawColor = dc;
  if (dc != wxPdfColour(0))
  {
    OutAscii(dc.GetColor(true));
  }
  m_fillColor = fc;
  if (fc != wxPdfColour(0))
  {
    OutAscii(fc.GetColor(false));
  }
  m_textColor = tc;
  m_colorFlag = cf;

  // Page header
  Header();

  // Restore line width
  if (m_lineWidth != lw)
  {
    m_lineWidth = lw;
    OutAscii(Double2String(lw*m_k,2)+wxString(_T(" w")));
  }

  // Restore font
  if(family.Length() > 0)
  {
    SetFont(family, style, size);
  }
  
  // Restore colors
  if (m_drawColor != dc)
  {
    m_drawColor = dc;
    OutAscii(dc.GetColor(true));
  }
  if (m_fillColor != fc)
  {
    m_fillColor = fc;
    OutAscii(fc.GetColor(false));
  }
  m_textColor = tc;
  m_colorFlag = cf;
}

void
wxPdfDocument::SetLineWidth(double width)
{
  // Set line width
  m_lineWidth = width;
  if (m_page > 0)
  {
    OutAscii(Double2String(width*m_k,2)+ wxString(_T(" w")));
  }
}

double
wxPdfDocument::GetLineWidth()
{
  return m_lineWidth;
}

void
wxPdfDocument::SetFontPath(const wxString& fontPath)
{
  if (fontPath != wxEmptyString)
  {
    m_fontPath = fontPath;
  }
  else
  {
    wxString localFontPath;
    if (!wxGetEnv(_T("WXPDF_FONTPATH"), &localFontPath))
    {
      localFontPath = wxGetCwd();
      if (!wxEndsWithPathSeparator(localFontPath))
      {
        localFontPath += wxFILE_SEP_PATH;
      }
      localFontPath += _T("fonts");
    }
    m_fontPath = localFontPath;
  }
}

bool
wxPdfDocument::AddFont(const wxString& family, const wxString& style, const wxString& file)
{
  if (family.Length() == 0) return false;

  // Add a TrueType or Type1 font
  wxString lcFamily = family.Lower();
  wxString lcStyle = style.Lower();
  wxString ucStyle = style.Upper();

  wxString fileName = file;
  if (fileName.Length() == 0)
  {
    fileName = lcFamily + lcStyle + wxString(_T(".xml"));
    fileName.Replace(_T(" "),_T(""));
  }

  if (ucStyle == _T("IB"))
  {
    ucStyle = _T("BI");
  }

  // check if the font has been already added
  wxString fontkey = lcFamily + ucStyle;
  wxPdfFontHashMap::iterator font = (*m_fonts).find(fontkey);
  if (font != (*m_fonts).end())
  {
    // Font already loaded
    return true;
  }

  // Open font metrics XML file
  wxFileName fontFileName(fileName);
  fontFileName.MakeAbsolute(GetFontPath());
  //BM: wxFileSystem doesnt appear to have default handler for local files?, just let 
  // the wxXmlDocument load the file itself rather than generating a stream.
/*  wxFileSystem fs;
  wxFSFile* xmlFontMetrics = fs.OpenFile(fontFileName.GetFullPath());
  if (!xmlFontMetrics)
  {
    // Font metrics XML file not found
    wxLogDebug(_T("wxPdfDocument::AddFont: Font metrics file '%s' not found."), fileName.c_str());
    return false;
  }
*/
  // Load the XML file
  wxXmlDocument fontMetrics;
//  bool loaded = fontMetrics.Load(*xmlFontMetrics->GetStream());
//  delete xmlFontMetrics;
  bool loaded = fontMetrics.Load(fontFileName.GetFullPath());
  if (!loaded)
  {
    // Font metrics file loading failed
    wxLogDebug(_T("wxPdfDocument::AddFont: Loading of font metrics file '%s' failed."), fileName.c_str());
    return false;
  }
  if (!fontMetrics.IsOk() || fontMetrics.GetRoot()->GetName() != wxT("wxpdfdoc-font-metrics"))
  {
    // Not a font metrics file
    wxLogDebug(_T("wxPdfDocument::AddFont: Font metrics file '%s' invalid."), fileName.c_str());
    return false;
  }

  wxString fontType;
  wxXmlNode* root = fontMetrics.GetRoot();
  if (!root->GetPropVal(_T("type"), &fontType))
  {
    // Font type not specified
    wxLogDebug(_T("wxPdfDocument::AddFont: Font type not specified for font '%s'."), family.c_str());
    return false;
  }

  int i = (*m_fonts).size() + 1;
  wxPdfFont* addedFont = NULL;
  if (fontType == _T("TrueType"))
  {
    addedFont = new wxPdfFontTrueType(i);
  }
  else if (fontType == _T("Type1"))
  {
    addedFont = new wxPdfFontType1(i);
  }
#if wxUSE_UNICODE
  else if (fontType == _T("TrueTypeUnicode"))
  {
    addedFont = new wxPdfFontTrueTypeUnicode(i);
  }
  else if (fontType == _T("OpenTypeUnicode"))
  {
    addedFont = new wxPdfFontOpenTypeUnicode(i);
    if (m_PDFVersion < _T("1.6"))
    {
      m_PDFVersion = _T("1.6");
    }
  }
  else if (fontType == _T("Type0"))
  {
    addedFont = new wxPdfFontType0(i);
  }
#endif
  else
  {
    // Unknown font type
    wxLogDebug(_T("wxPdfDocument::AddFont: Unknown font type '%s'."), fontType.c_str());
    return false;
  }
  if (!addedFont->LoadFontMetrics(root))
  {
    delete addedFont;
    return false;
  }
  addedFont->SetFilePath(fontFileName.GetPath());
  (*m_fonts)[fontkey] = addedFont;

  if (addedFont->HasDiffs())
  {
    // Search existing encodings
    int d = 0;
    int nb = (*m_diffs).size();
    for (i = 1; i <= nb; i++)
    {
      if (*(*m_diffs)[i] == addedFont->GetDiffs())
      {
        d = i;
        break;
      }
    }
    if (d == 0)
    {
      d = nb + 1;
      (*m_diffs)[d] = new wxString(addedFont->GetDiffs());
    }
    addedFont->SetDiffIndex(d);
  }

  return true;
}

#if wxUSE_UNICODE

bool
wxPdfDocument::AddFontCJK(const wxString& family)
{
  wxString lcFamily = family.Lower();
  wxString fontFile = lcFamily + wxString(_T(".xml"));
  wxString fontkey = lcFamily;
  wxString fontName;
  bool valid;

  wxPdfFontHashMap::iterator font = (*m_fonts).find(fontkey);
  if (font != (*m_fonts).end())
  {
    return true;
  }

  valid = AddFont(family, _T(""), fontFile);
  if (valid)
  {
    // Add all available styles (bold, italic and bold-italic)
    // For all styles the same font metric file is used, therefore
    // the font name has to be changed afterwards to reflect the
    // style.
    AddFont(family, _T("B"), fontFile);
    fontkey = lcFamily + wxString(_T("B"));
    font = (*m_fonts).find(fontkey);
    fontName = font->second->GetName();
    fontName += wxString(_T(",Bold"));
    font->second->SetName(fontName);

    AddFont(family, _T("I"), fontFile);
    fontkey = lcFamily + wxString(_T("I"));
    font = (*m_fonts).find(fontkey);
    fontName = font->second->GetName();
    fontName += wxString(_T(",Italic"));
    font->second->SetName(fontName);
    
    AddFont(family, _T("BI"), fontFile);
    fontkey = lcFamily + wxString(_T("BI"));
    font = (*m_fonts).find(fontkey);
    fontName = font->second->GetName();
    fontName += wxString(_T(",BoldItalic"));
    font->second->SetName(fontName);
  }
  return valid;
}

#endif // wxUSE_UNICODE

bool
wxPdfDocument::SetFont(const wxString& family, const wxString& style, double size)
{
  return SelectFont(family, style, size, true);
}

void
wxPdfDocument::SetFontSize(double size)
{
  // Set font size in points
  if (m_fontSizePt == size)
  {
    return;
  }
  m_fontSizePt = size;
  m_fontSize = size / m_k;
  if ( m_page > 0)
  {
    OutAscii(wxString::Format(_T("BT /F%d "),m_currentFont->GetIndex()) +
             Double2String(m_fontSizePt,2) + wxString(_T(" Tf ET")));
  }
}

const wxPdfFontDescription&
wxPdfDocument::GetFontDescription() const
{
  return m_currentFont->GetDesc();
}

const wxString
wxPdfDocument::GetFontFamily()
{
  return m_fontFamily;
}

const wxString
wxPdfDocument::GetFontStyle()
{
  wxString style = m_fontStyle;
  if (m_decoration & wxPDF_FONT_UNDERLINE)
  {
    style += wxString(_T("U"));
  }
  if (m_decoration & wxPDF_FONT_OVERLINE)
  {
    style += wxString(_T("O"));
  }
  if (m_decoration & wxPDF_FONT_STRIKEOUT)
  {
    style += wxString(_T("S"));
  }
  return style;
}

double
wxPdfDocument::GetFontSize()
{
  return m_fontSizePt;
}

double
wxPdfDocument::GetStringWidth(const wxString& s)
{
  double w = 0;
  if (m_currentFont != 0)
  {
    w = m_currentFont->GetStringWidth(s) * m_fontSize;
  }
  return w;
}

void
wxPdfDocument::Text(double x, double y, const wxString& txt, const int mode)
{
  // Output a string
  if (m_colorFlag)
  {
    Out("q ", false);
    OutAscii(m_textColor.GetColor(false), false);
    Out(" ", false);
  }
  OutAscii(wxString(_T("BT ")) +
           Double2String(x*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" Td ")), false);
  if(mode<3)
  {
    OutAscii(Double2String(mode) + wxString(_T(" Tr")), false);
  }

  OutAscii(wxString(_T(" (")), false);
  TextEscape(txt,false);
  Out(") Tj ET", false);

  if ((m_decoration & wxPDF_FONT_DECORATION) && txt.Length() > 0)
  {
    Out(" ", false);
    OutAscii(DoDecoration(x, y, txt), false);
  }

  if (m_colorFlag)
  {
    Out(" Q", false);
  }
  Out("\n", false);
}

void
wxPdfDocument::RotatedText(double x, double y, const wxString& txt, double angle, const int mode)
{
  // Text rotated around its origin
  StartTransform();
  Rotate(angle, x, y);
  Text(x, y, txt, mode);
  StopTransform();
}

bool
wxPdfDocument::AcceptPageBreak()
{
  // Accept automatic page break or not
  return m_autoPageBreak;
}

void
wxPdfDocument::Cell(double w, double h, const wxString& txt, int border, int ln, int align, int fill, const wxPdfLink& link)
{
  // Output a cell
  double x, y;
  double k = m_k;
  if (m_y + h > m_pageBreakTrigger && !m_inFooter && AcceptPageBreak())
  {
    // Automatic page break
    x = m_x;
    double ws = m_ws;
    if (ws > 0)
    {
      m_ws = 0;
      Out("0 Tw");
    }
    AddPage(m_curOrientation);
    m_x = x;
    if (ws > 0)
    {
      m_ws = ws;
      OutAscii(Double2String(ws*k,3)+wxString(_T(" Tw")));
    }
  }
  if ( w == 0)
  {
    w = m_w - m_rMargin - m_x;
  }
  wxString s = wxEmptyString;
  if (fill == 1 || border == wxPDF_BORDER_FRAME)
  {
    s = Double2String(m_x*k,2) + wxString(_T(" ")) +
        Double2String((m_h-m_y)*k,2) + wxString(_T(" ")) +
        Double2String(w*k,2) + wxString(_T(" ")) +
        Double2String(-h*k,2);
    if (fill == 1)
    {
      if (border == wxPDF_BORDER_FRAME)
      {
        s += wxString(_T(" re B "));
      }
      else
      {
        s += wxString(_T(" re f "));
      }
    }
    else
    {
      s += wxString(_T(" re S "));
    }
  }
  if (border != wxPDF_BORDER_NONE && border != wxPDF_BORDER_FRAME)
  {
    x = m_x;
    y = m_y;
    if (border & wxPDF_BORDER_LEFT)
    {
      s += Double2String(x*k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*k,2) + wxString(_T(" m ")) +
           Double2String(x*k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+h))*k,2) + wxString(_T(" l S "));
    }
    if (border & wxPDF_BORDER_TOP)
    {
      s += Double2String(x*k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*k,2) + wxString(_T(" m ")) +
           Double2String((x+w)*k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*k,2) + wxString(_T(" l S "));
    }
    if (border & wxPDF_BORDER_RIGHT)
    {
      s += Double2String((x+w)*k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*k,2) + wxString(_T(" m ")) +
           Double2String((x+w)*k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+h))*k,2) + wxString(_T(" l S "));
    }
    if (border & wxPDF_BORDER_BOTTOM)
    {
      s += Double2String(x*k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+h))*k,2) + wxString(_T(" m ")) +
           Double2String((x+w)*k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+h))*k,2) + wxString(_T(" l S "));
    }
  }
  if (s.Length() > 0)
  {
    bool newline = txt.Length() == 0;
    OutAscii(s, newline);
    s = _T("");
  }
  
  if (txt.Length() > 0)
  {
    double width = GetStringWidth(txt);
    double dx;
    if (align == wxPDF_ALIGN_RIGHT)
    {
      dx = w - m_cMargin - width;
    }
    else if (align == wxPDF_ALIGN_CENTER)
    {
      dx = (w - width) / 2;
    }
    else
    {
      dx = m_cMargin;
    }
    if (m_colorFlag)
    {
      s += wxString(_T("q ")) + m_textColor.GetColor(false) + wxString(_T(" "));
    }
    s += wxString(_T("BT ")) +
         Double2String((m_x+dx)*k,2) + wxString(_T(" ")) +
         Double2String((m_h-(m_y+.5*h+.3*m_fontSize))*k,2) + wxString(_T(" Td ("));
    OutAscii(s,false);
    TextEscape(txt,false);
    s = _T(") Tj ET");
    if (m_currentFont != 0)
    {
      m_currentFont->UpdateUsedChars(txt);
    }

    if (m_decoration & wxPDF_FONT_DECORATION)
    {
      s += wxString(_T(" ")) + DoDecoration(m_x+dx,m_y+.5*h+.3*m_fontSize,txt);
    }
    if (m_colorFlag)
    {
      s += wxString(_T(" Q"));
    }
    if (link.IsValid())
    {
      Link(m_x+dx,m_y+.5*h-.5*m_fontSize,width,m_fontSize,link);
    }
    OutAscii(s);
  }
  m_lasth = h;
  if (ln > 0)
  {
    // Go to next line
    m_y += h;
    if ( ln == 1)
    {
      m_x = m_lMargin;
    }
  }
  else
  {
    m_x += w;
  }
}

int
wxPdfDocument::MultiCell(double w, double h, const wxString& txt, int border, int align, int fill, int maxline)
{
  // Output text with automatic or explicit line breaks
  if (w == 0)
  {
    w = m_w - m_rMargin - m_x;
  }

  double wmax = (w - 2 * m_cMargin);
  wxString s = txt;
  s.Replace(_T("\r"),_T("")); // remove carriage returns
  int nb = s.Length();
  if (nb > 0 && s[nb-1] == _T('\n'))
  {
    nb--;
  }

  int b = wxPDF_BORDER_NONE;
  int b2 = wxPDF_BORDER_NONE;
  if (border != wxPDF_BORDER_NONE)
  {
    if (border == wxPDF_BORDER_FRAME)
    {
      b = wxPDF_BORDER_LEFT | wxPDF_BORDER_RIGHT | wxPDF_BORDER_TOP;
      b2 = wxPDF_BORDER_LEFT | wxPDF_BORDER_RIGHT;
    }
    else
    {
      b2 = wxPDF_BORDER_NONE;
      if (border & wxPDF_BORDER_LEFT)
      {
        b2 = b2 | wxPDF_BORDER_LEFT;
      }
      if (border & wxPDF_BORDER_RIGHT)
      {
        b2 = b2 | wxPDF_BORDER_RIGHT;
      }
      b = (border & wxPDF_BORDER_TOP) ? b2 | wxPDF_BORDER_TOP : b2;
    }
  }
  int sep = -1;
  int i = 0;
  int j = 0;
  double len = 0;
  double ls = 0;
  int ns = 0;
  int nl = 1;
  wxChar c;
  while (i < nb)
  {
    // Get next character
    c = s[i];
    if (c == _T('\n'))
    {
      // Explicit line break
      if (m_ws > 0)
      {
        m_ws = 0;
        Out("0 Tw");
      }
      Cell(w,h,s.SubString(j,i-1),b,2,align,fill);
      i++;
      sep = -1;
      j = i;
      len = 0;
      ns = 0;
      nl++;
      if (border != wxPDF_BORDER_NONE && nl == 2)
      {
        b = b2;
      }
      if (maxline > 0 && nl > maxline)
      {
        return j;
      }
      continue;
    }
    if (c == _T(' '))
    {
      sep = i;
      ls = len;
      ns++;
    }
    len = GetStringWidth(s.SubString(j, i));

    if (len > wmax)
    {
      // Automatic line break
      if (sep == -1)
      {
        if (i == j)
        {
          i++;
        }
        if (m_ws > 0)
        {
          m_ws=0;
          Out("0 Tw");
        }
        Cell(w,h,s.SubString(j,i-1),b,2,align,fill);
      }
      else
      {
        if (align == wxPDF_ALIGN_JUSTIFY)
        {
          m_ws = (ns > 1) ? (wmax - ls)/(ns-1) : 0;
          OutAscii(Double2String(m_ws*m_k,3)+wxString(_T(" Tw")));
        }
        Cell(w,h,s.SubString(j,sep-1),b,2,align,fill);
        i = sep + 1;
      }
      sep = -1;
      j = i;
      len = 0;
      ns = 0;
      nl++;
      if (border != wxPDF_BORDER_NONE && nl == 2)
      {
        b = b2;
      }
      if (maxline > 0 && nl > maxline)
      {
        return j;
      }
    }
    else
    {
      i++;
    }
  }
  // Last chunk
  if (m_ws > 0)
  {
    m_ws = 0;
    Out("0 Tw");
  }
  if ((border != wxPDF_BORDER_NONE) && (border & wxPDF_BORDER_BOTTOM))
  {
    b = b | wxPDF_BORDER_BOTTOM;
  }
  Cell(w,h,s.SubString(j,i-1),b,2,align,fill);
  m_x = m_lMargin;
  return i;
}

int
wxPdfDocument::LineCount(double w, const wxString& txt)
{
  // Output text with automatic or explicit line breaks
  if (w == 0)
  {
    w = m_w - m_rMargin - m_x;
  }

  double wmax = (w - 2 * m_cMargin);
  wxString s = txt;
  s.Replace(_T("\r"),_T("")); // remove carriage returns
  int nb = s.Length();
  if (nb > 0 && s[nb-1] == _T('\n'))
  {
    nb--;
  }

  int sep = -1;
  int i = 0;
  int j = 0;
  double len = 0;
  int nl = 1;
  wxChar c;
  while (i < nb)
  {
    // Get next character
    c = s[i];
    if (c == _T('\n'))
    {
      // Explicit line break
      i++;
      sep = -1;
      j = i;
      len = 0;
      nl++;
      continue;
    }
    if (c == _T(' '))
    {
      sep = i;
    }
    len = GetStringWidth(s.SubString(j, i));

    if (len > wmax)
    {
      // Automatic line break
      if (sep == -1)
      {
        if (i == j)
        {
          i++;
        }
      }
      else
      {
        i = sep + 1;
      }
      sep = -1;
      j = i;
      len = 0;
      nl++;
    }
    else
    {
      i++;
    }
  }
  return nl;
}

int
wxPdfDocument::TextBox(double w, double h, const wxString& txt,
                       int halign, int valign, int border, int fill)
{
  double xi = m_x;
  double yi = m_y;
  
  double hrow  = m_fontSize;
  int textrows = LineCount(w, txt);
  int maxrows  = (int) floor(h / hrow);
  int rows     = (textrows < maxrows) ? textrows : maxrows;

  double dy = 0;
  if (valign == wxPDF_ALIGN_MIDDLE)
  {
    dy = (h - rows * hrow) / 2;
  }
  else if (valign == wxPDF_ALIGN_BOTTOM)
  {
    dy = h - rows * hrow;
  }

  SetY(yi+dy);
  SetX(xi);
  int trail = MultiCell(w, hrow, txt, 0, halign, fill, rows);

  if (border == wxPDF_BORDER_FRAME)
  {
    Rect(xi, yi, w, h);
  }
  else
  {
    if (border & wxPDF_BORDER_LEFT)   Line(xi,yi,xi,yi+h);
    if (border & wxPDF_BORDER_RIGHT)  Line(xi+w,yi,xi+w,yi+h);
    if (border & wxPDF_BORDER_TOP)    Line(xi,yi,xi+w,yi);
    if (border & wxPDF_BORDER_BOTTOM) Line(xi,yi+h,xi+w,yi+h);
  }

  return trail;
}

void
wxPdfDocument::Write(double h, const wxString& txt, const wxPdfLink& link)
{
  WriteCell(h, txt, wxPDF_BORDER_NONE, 0, link);
}

void
wxPdfDocument::WriteCell(double h, const wxString& txt, int border, int fill, const wxPdfLink& link)
{
  // Output text in flowing mode
  wxString s = txt;
  s.Replace(_T("\r"),_T("")); // remove carriage returns
  int nb = s.Length();

  // handle single space character
  if ((nb == 1) && s[0] == _T(' '))
  {
    m_x += GetStringWidth(s);
    return;
  }

  double saveCellMargin = GetCellMargin();
  SetCellMargin(0);

  double w = m_w - m_rMargin - m_x;
  double wmax = (w - 2 * m_cMargin) + wxPDF_EPSILON;

  int sep = -1;
  int i = 0;
  int j = 0;
  double len=0;
  int nl = 1;
  wxChar c;
  while (i < nb)
  {
    // Get next character
    c = s[i];
    if (c == _T('\n'))
    {
      // Explicit line break
      Cell(w, h, s.SubString(j,i-1), border, 2, wxPDF_ALIGN_LEFT, fill, link);
      i++;
      sep = -1;
      j = i;
      len = 0;
      if (nl == 1)
      {
        m_x = m_lMargin;
        w = m_w - m_rMargin - m_x;
        wmax = (w - 2 * m_cMargin);
      }
      nl++;
      continue;
    }
    if (c == _T(' '))
    {
      sep = i;
    }
    len = GetStringWidth(s.SubString(j, i));
    if (len > wmax)
    {
      // Automatic line break
      if (sep == -1)
      {
        if (m_x > m_lMargin)
        {
          // Move to next line
          m_x = m_lMargin;
          m_y += h;
          w = m_w - m_rMargin -m_x;
          wmax = (w - 2 * m_cMargin);
          i++;
          nl++;
          continue;
        }
        if (i == j)
        {
          i++;
        }
        Cell(w, h,s.SubString(j, i-1), border, 2, wxPDF_ALIGN_LEFT, fill, link);
      }
      else
      {
        Cell(w, h, s.SubString(j, sep-1), border, 2, wxPDF_ALIGN_LEFT, fill, link);
        i = sep + 1;
      }
      sep = -1;
      j = i;
      len = 0;
      if (nl == 1)
      {
        m_x = m_lMargin;
        w = m_w - m_rMargin - m_x;
        wmax = (w - 2 * m_cMargin);
      }
      nl++;
    }
    else
    {
      i++;
    }
  }
  // Last chunk
  if (i != j)
  {
    Cell(len, h, s.SubString(j,i-1), border, 0, wxPDF_ALIGN_LEFT, fill, link);
  }

  // Following statement was in PHP code, but seems to be in error.
  // m_x += GetStringWidth(s.SubString(j, i-1));
  SetCellMargin(saveCellMargin);
}

bool
wxPdfDocument::Image(const wxString& file, double x, double y, double w, double h,
                     const wxString& type, const wxPdfLink& link, int maskImage)
{
  wxPdfImage* currentImage = NULL;
  // Put an image on the page
  wxPdfImageHashMap::iterator image = (*m_images).find(file);
  if (image == (*m_images).end())
  {
    // First use of image, get info
    int i = (*m_images).size() + 1;
    currentImage = new wxPdfImage(this, i, file, type);
    if (!currentImage->Parse())
    {
      bool isValid = false;
      delete currentImage;

      if (wxImage::FindHandler(wxBITMAP_TYPE_PNG) == NULL)
      {
        wxImage::AddHandler(new wxPNGHandler());
      }
      wxImage tempImage;
      tempImage.LoadFile(file);
      if (tempImage.Ok())
      {
        isValid = Image(file, tempImage, x, y, w, h, link, maskImage);
      }
      return isValid;
    }
    if (maskImage > 0)
    {
      currentImage->SetMaskImage(maskImage);
    }
    (*m_images)[file] = currentImage;
  }
  else
  {
    currentImage = image->second;
    if (maskImage > 0 && currentImage->GetMaskImage() != maskImage)
    {
      currentImage->SetMaskImage(maskImage);
    }
  }
  OutImage(currentImage, x, y, w, h, link);
  return true;
}

bool
wxPdfDocument::Image(const wxString& name, const wxImage& img, double x, double y, double w, double h,
                     const wxPdfLink& link, int maskImage)
{
  bool isValid = false;
  if (img.Ok())
  {
    wxImage tempImage = img.Copy();
    wxPdfImage* currentImage = NULL;
    // Put an image on the page
    wxPdfImageHashMap::iterator image = (*m_images).find(name);
    if (image == (*m_images).end())
    {
      if (tempImage.HasAlpha())
      {
        if (maskImage <= 0)
        {
          maskImage = ImageMask(name+wxString(_T(".mask")), tempImage);
        }
        if(!tempImage.ConvertAlphaToMask(0))
        {
          return false;
        }
      }
      // First use of image, get info
      tempImage.SetMask(false);
      int i = (*m_images).size() + 1;
      currentImage = new wxPdfImage(this, i, name, tempImage);
      if (!currentImage->Parse())
      {
        delete currentImage;
        return false;
      }
      if (maskImage > 0)
      {
        currentImage->SetMaskImage(maskImage);
      }
      (*m_images)[name] = currentImage;
    }
    else
    {
      currentImage = image->second;
      if (maskImage > 0 && currentImage->GetMaskImage() != maskImage)
      {
        currentImage->SetMaskImage(maskImage);
      }
    }
    OutImage(currentImage, x, y, w, h, link);
    isValid = true;
  }
  return isValid;
}

bool
wxPdfDocument::Image(const wxString& name, wxInputStream& stream,
                     const wxString& mimeType,
                     double x, double y, double w, double h,
                     const wxPdfLink& link, int maskImage)
{
  bool isValid = false;
  wxPdfImage* currentImage = NULL;
  // Put an image on the page
  wxPdfImageHashMap::iterator image = (*m_images).find(name);
  if (image == (*m_images).end())
  {
    // First use of image, get info
    int i = (*m_images).size() + 1;
    currentImage = new wxPdfImage(this, i, name, stream, mimeType);
    if (!currentImage->Parse())
    {
      delete currentImage;
      if (wxImage::FindHandler(wxBITMAP_TYPE_PNG) == NULL)
      {
        wxImage::AddHandler(new wxPNGHandler());
      }
      wxImage tempImage;
      tempImage.LoadFile(stream, mimeType);
      if (tempImage.Ok())
      {
        isValid = Image(name, tempImage, x, y, w, h, link, maskImage);
      }
      return isValid;

    }
    if (maskImage > 0)
    {
      currentImage->SetMaskImage(maskImage);
    }
    (*m_images)[name] = currentImage;
  }
  else
  {
    currentImage = image->second;
    if (maskImage > 0 && currentImage->GetMaskImage() != maskImage)
    {
      currentImage->SetMaskImage(maskImage);
    }
  }
  OutImage(currentImage, x, y, w, h, link);
  isValid = true;
  return isValid;
}

int
wxPdfDocument::ImageMask(const wxString& file, const wxString& type)
{
  int n = 0;
  wxPdfImage* currentImage = NULL;
  // Put an image on the page
  wxPdfImageHashMap::iterator image = (*m_images).find(file);
  if (image == (*m_images).end())
  {
    // First use of image, get info
    n = (*m_images).size() + 1;
    currentImage = new wxPdfImage(this, n, file, type);
    if (!currentImage->Parse())
    {
      delete currentImage;
      return 0;
    }
    // Check whether this is a gray scale image (must be)
    if (currentImage->GetColorSpace() != _T("DeviceGray"))
    {
      delete currentImage;
      return 0;
    }
    (*m_images)[file] = currentImage;
  }
  else
  {
    currentImage = image->second;
    n = currentImage->GetIndex();
  }
  if (m_PDFVersion < _T("1.4"))
  {
    m_PDFVersion = _T("1.4");
  }
  return n;
}

int
wxPdfDocument::ImageMask(const wxString& name, const wxImage& img)
{
  int n = 0;
  if (img.Ok())
  {
    wxPdfImage* currentImage = NULL;
    // Put an image on the page
    wxPdfImageHashMap::iterator image = (*m_images).find(name);
    if (image == (*m_images).end())
    {
      wxImage tempImage;
      if (img.HasAlpha())
      {
        int x, y;
        int w = img.GetWidth();
        int h = img.GetHeight();
        tempImage = wxImage(w, h);
        unsigned char alpha;
        for (x = 0; x < w; x++)
        {
          for (y = 0; y < h; y++)
          {
            alpha = img.GetAlpha(x, y);
            tempImage.SetRGB(x, y, alpha, alpha, alpha);
          }
        }
        tempImage.SetOption(wxIMAGE_OPTION_PNG_FORMAT, wxPNG_TYPE_GREY_RED);
      }
      else
      {
        tempImage = img.Copy();
        tempImage.SetOption(wxIMAGE_OPTION_PNG_FORMAT, wxPNG_TYPE_GREY);
      }
      tempImage.SetMask(false);
      // First use of image, get info
      n = (*m_images).size() + 1;
      currentImage = new wxPdfImage(this, n, name, tempImage);
      if (!currentImage->Parse())
      {
        delete currentImage;
        return 0;
      }
      (*m_images)[name] = currentImage;
    }
    else
    {
      currentImage = image->second;
      n = currentImage->GetIndex();
    }
    if (m_PDFVersion < _T("1.4"))
    {
      m_PDFVersion = _T("1.4");
    }
  }
  return n;
}

int
wxPdfDocument::ImageMask(const wxString& name, wxInputStream& stream, const wxString& mimeType)
{
  int n = 0;
  wxPdfImage* currentImage = NULL;
  // Put an image on the page
  wxPdfImageHashMap::iterator image = (*m_images).find(name);
  if (image == (*m_images).end())
  {
    // First use of image, get info
    n = (*m_images).size() + 1;
    currentImage = new wxPdfImage(this, n, name, stream, mimeType);
    if (!currentImage->Parse())
    {
      delete currentImage;
      return 0;
    }
    // Check whether this is a gray scale image (must be)
    if (currentImage->GetColorSpace() != _T("DeviceGray"))
    {
      delete currentImage;
      return 0;
    }
    (*m_images)[name] = currentImage;
  }
  else
  {
    currentImage = image->second;
    n = currentImage->GetIndex();
  }
  if (m_PDFVersion < _T("1.4"))
  {
    m_PDFVersion = _T("1.4");
  }
  return n;
}

void
wxPdfDocument::RotatedImage(const wxString& file, double x, double y, double w, double h,
                            double angle, const wxString& type, const wxPdfLink& link, int maskImage)
{
  // Image rotated around its upper-left corner
  StartTransform();
  Rotate(angle, x, y);
  Image(file, x, y, w, h, type, link, maskImage);
  StopTransform();
}

void
wxPdfDocument::Ln(double h)
{
  // Line feed; default value is last cell height
  m_x = m_lMargin;
  if (h < 0)
  {
    m_y += m_lasth;
  }
  else
  {
    m_y += h;
  }
}

void
wxPdfDocument::SaveAsFile(const wxString& name)
{
  wxString fileName = name;
  // Finish document if necessary
  if (m_state < 3)
  {
    Close();
  }
  // Normalize parameters
  if(fileName.Length() == 0)
  {
    fileName = _T("doc.pdf");
  }
  // Save to local file
  wxFileOutputStream outfile(fileName);
  wxMemoryInputStream tmp(m_buffer);
  outfile.Write(tmp);
  outfile.Close();
}

const wxMemoryOutputStream&
wxPdfDocument::CloseAndGetBuffer()
{
  if (m_state < 3)
  {
    Close();
  }
  
  return m_buffer;
}

void
wxPdfDocument::SetViewerPreferences(int preferences)
{
  m_viewerPrefs = (preferences > 0) ? preferences : 0;
  if (((m_viewerPrefs & wxPDF_VIEWER_DISPLAYDOCTITLE) != 0) && (m_PDFVersion < _T("1.4")))
  {
    m_PDFVersion = _T("1.4");
  }
}

void
wxPdfDocument::SetTitle(const wxString& title)
{
  // Title of document
  m_title = title;
}

void
wxPdfDocument::SetSubject(const wxString& subject)
{
  // Subject of document
  m_subject = subject;
}

void
wxPdfDocument::SetAuthor(const wxString& author)
{
  // Author of document
  m_author = author;
}

void
wxPdfDocument::SetKeywords(const wxString& keywords)
{
  // Keywords of document
  m_keywords = keywords;
}

void
wxPdfDocument::SetCreator(const wxString& creator)
{
  // Creator of document
  m_creator = creator;
}

void
wxPdfDocument::SetMargins(double left, double top, double right)
{
  // Set left, top and right margins
  m_lMargin = left;
  m_tMargin = top;
  if (right == -1)
  {
    right = left;
  }
  m_rMargin = right;
}

void
wxPdfDocument::SetLeftMargin(double margin)
{
  // Set left margin
  m_lMargin = margin;
  if (m_page > 0 && m_x < margin)
  {
    m_x = margin;
  }
}

double
wxPdfDocument::GetLeftMargin()
{
  return m_lMargin;
}

void
wxPdfDocument::SetTopMargin(double margin)
{
  // Set top margin
  m_tMargin = margin;
}

double
wxPdfDocument::GetTopMargin()
{
  return m_tMargin;
}

void
wxPdfDocument::SetRightMargin(double margin)
{
  // Set right margin
  m_rMargin = margin;
}

double
wxPdfDocument::GetRightMargin()
{
  return m_rMargin;
}

void
wxPdfDocument::SetCellMargin(double margin)
{
  // Set cell margin
  m_cMargin = margin;
}

double
wxPdfDocument::GetCellMargin()
{
  return m_cMargin;
}

void
wxPdfDocument::SetLineHeight(double height)
{
  m_lasth = height;
}

double
wxPdfDocument::GetLineHeight()
{
  return m_lasth;
}

void
wxPdfDocument::SetAutoPageBreak(bool autoPageBreak, double margin)
{
  // Set auto page break mode and triggering margin
  m_autoPageBreak = autoPageBreak;
  m_bMargin = margin;
  m_pageBreakTrigger = m_h - margin;
}

void
wxPdfDocument::SetDisplayMode(wxPdfZoom zoom, wxPdfLayout layout, double zoomFactor)
{
  // Set display mode in viewer
  switch (zoom)
  {
    case wxPDF_ZOOM_FULLPAGE:
    case wxPDF_ZOOM_FULLWIDTH:
    case wxPDF_ZOOM_REAL:
    case wxPDF_ZOOM_DEFAULT:
      m_zoomMode = zoom;
      break;
    case wxPDF_ZOOM_FACTOR:
      m_zoomMode = zoom;
      m_zoomFactor = (zoomFactor > 0) ? zoomFactor : 100.;
      break;
    default:
      m_zoomMode = wxPDF_ZOOM_FULLWIDTH;
      break;
  }

  switch (layout)
  {
    case wxPDF_LAYOUT_SINGLE:
    case wxPDF_LAYOUT_TWO:
    case wxPDF_LAYOUT_DEFAULT:
    case wxPDF_LAYOUT_CONTINUOUS:
      m_layoutMode = layout;
      break;
    default:
      m_layoutMode = wxPDF_LAYOUT_CONTINUOUS;
      break;
  }
}

void
wxPdfDocument::Close()
{
  // Terminate document
  if (m_state == 3)
  {
    return;
  }
  if (m_page == 0)
  {
    AddPage();
  }
  
  // Page footer
  m_inFooter = true;
  Footer();
  m_inFooter = false;

  // Close page
  EndPage();

  // Close document
  EndDoc();
}

void
wxPdfDocument::Header()
{
  // To be implemented in your own inherited class
}

void
wxPdfDocument::Footer()
{
  // To be implemented in your own inherited class
}

bool
wxPdfDocument::IsInFooter()
{
  return m_inFooter;
}

int
wxPdfDocument::PageNo()
{
  // Get current page number
  return m_page;
}

double
wxPdfDocument::GetX()
{
  // Get x position
  return m_x;
}

void
wxPdfDocument::SetX(double x)
{
  // Set x position
  if ( x >= 0.0)
  {
    m_x = x;
  }
  else
  {
    m_x = m_w + x;
  }
}

double
wxPdfDocument::GetY()
{
  // Get y position
  return m_y;
}

void
wxPdfDocument::SetY(double y)
{
  // Set y position and reset x
  m_x = m_lMargin;
  if ( y >= 0)
  {
    m_y = y;
  }
  else
  {
    m_y = m_h + y;
  }
}

void
wxPdfDocument::SetXY(double x, double y)
{
  // Set x and y positions
  SetY(y);
  SetX(x);
}

void
wxPdfDocument::SetCompression(bool compress)
{
  m_compress = compress;
}

void
wxPdfDocument::AppendJavascript(const wxString& javascript)
{
  m_javascript += javascript;
}

// --- Static methods

wxString
wxPdfDocument::RGB2String(const wxColour& color)
{
  double r = color.Red();
  double g = color.Green();
  double b = color.Blue();
  wxString rgb = Double2String(r/255.,3) + _T(" ") + 
                 Double2String(g/255.,3) + _T(" ") + 
                 Double2String(b/255.,3);
  return rgb;
}

wxString
wxPdfDocument::Double2String(double value, int precision)
{
  wxString number;
  if (precision < 0)
  {
    precision = 0;
  }
  else if (precision > 16)
  {
    precision = 16;
  }

  // Use absolute value locally
  double localValue = fabs(value);
  double localFraction = (localValue - floor(localValue)) +(5. * pow(10.0, -precision-1));
  if (localFraction >= 1)
  {
    localValue += 1.0;
    localFraction -= 1.0;
  }
  localFraction *= pow(10.0, precision);

  if (value < 0)
  {
    number += wxString(_T("-"));
  }

  number += wxString::Format(_T("%.0lf"), floor(localValue));

  // generate fraction, padding with zero if necessary.
  if (precision > 0)
  {
    number += wxString(_T("."));
    wxString fraction = wxString::Format(_T("%.0lf"), floor(localFraction));
    if (fraction.Length() < ((size_t) precision))
    {
      number += wxString(_T('0'), precision-fraction.Length());
    }
    number += fraction;
  }

  return number;
}

double
wxPdfDocument::String2Double(const wxString& str)
{
  wxString value = str.Strip(wxString::both);
  double result = 0;
  double sign = 1;
  int scale = 0;
  int exponent = 0;
  int expsign = 1;
  int j = 0;
  int jMax = value.Length();
  if (jMax > 0)
  {
    if (value[j] == wxT('+'))
    {
      j++;
    }
    else if (value[j] == wxT('-'))
    {
      sign = -1;
      j++;
    }
    while (j < jMax && wxIsdigit(value[j]))
    {
      result = result*10 + (value[j] - wxT('0'));
      j++;
    }
    if (j < jMax && value[j] == wxT('.'))
    {
      j++;
      while (j < jMax && wxIsdigit(value[j]))
      {
        result = result*10 + (value[j] - wxT('0'));
        scale++;
        j++;
      }
    }
    if (j < jMax && (value[j] == wxT('E') || value[j] == wxT('e')))
    {
      j++;
      if (value[j] == wxT('+'))
      {
        j++;
      }
      else if (value[j] == wxT('-'))
      {
        expsign = -1;
        j++;
      }
      while (j < jMax && wxIsdigit(value[j]))
      {
        exponent = exponent*10 + (value[j] - wxT('0'));
        j++;
      }
      exponent *= expsign;
    }
    result = sign * result * pow(10.0, exponent-scale);
  }
  return result;
}

wxString
wxPdfDocument::Convert2Roman(int value)
{
  wxString result = wxEmptyString;

  if (value > 0 && value < 4000)
  {
    static wxString romans = _T("MDCLXVI");
    int pos = 6;  // Point to LAST character in 'romans'
    int currentDigit;

    while (value > 0)
    {
      currentDigit = value % 10;   
      if (currentDigit == 4 || currentDigit == 9)
      {
        result.Prepend(romans.Mid(pos  - currentDigit / 4, 1));
        result.Prepend(romans.Mid(pos, 1));
      }
      else
      {
        int x = currentDigit % 5;
        while (x-- > 0)
        {
          result.Prepend(romans.Mid(pos, 1));
        }
        if (currentDigit >= 5)
        {
          result.Prepend(romans.Mid(pos - 1, 1));
        }
      }
      value /= 10;
      pos -= 2;
    }
  }
  else
  {
    result = _T("???");
  }
  return result;
}

double
wxPdfDocument::ForceRange(double value, double minValue, double maxValue)
{
  if (value < minValue) 
  {
    value = minValue;
  }
  else if (value > maxValue)
  {
    value = maxValue;
  }
  return value;
}

void
wxPdfDocument::EnterOcg(const wxPdfOcg *ocg)
{
  OutAscii(wxString::Format(_T("/OC /OC%d BDC"), ocg->GetOcgIndex()));
}

void
wxPdfDocument::ExitOcg(void)
{
  OutAscii(_T("EMC"));
}
