///////////////////////////////////////////////////////////////////////////////
// Name:        pdfkernel.cpp
// Purpose:     Implementation of wxPdfDocument (internal methods)
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-01-27
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfkernel.cpp Implementation of the wxPdfDocument class (internal methods)

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/zstream.h>

#include "wx/pdfdoc.h"
#include "wx/pdfform.h"
#include "wx/pdfgraphics.h"
#include "wx/pdftemplate.h"

#include "pdffontdata.inc"

// ----------------------------------------------------------------------------
// wxPdfDocument: class representing a PDF document
// ----------------------------------------------------------------------------

void
wxPdfDocument::InitializeCoreFonts()
{
  m_coreFonts = new wxPdfCoreFontMap();
  int j;
  for (j = 0; wxCoreFontTable[j].id != wxEmptyString; j++)
  {
    (*m_coreFonts)[wxCoreFontTable[j].id] = j;
  }
}

bool
wxPdfDocument::SelectFont(const wxString& family, const wxString& style, double size, bool setFont)
{
  // Select a font; size given in points

  wxString ucStyle = style.Upper();
  wxString lcFamily = family.Lower();
  if (lcFamily.Length() == 0)
  {
    lcFamily = m_fontFamily;
  }
  if (lcFamily == _T("arial"))
  {
    lcFamily = _T("helvetica");
  }
  else if (lcFamily == _T("symbol") || lcFamily == _T("zapfdingbats"))
  {
    ucStyle = wxEmptyString;
  }
  m_decoration = wxPDF_FONT_NORMAL;
  if (ucStyle.Find(_T('U')) >= 0)
  {
    m_decoration |= wxPDF_FONT_UNDERLINE;
    ucStyle.Replace(_T("U"),_T(""));
  }
  if (ucStyle.Find(_T('O')) >= 0)
  {
    m_decoration |= wxPDF_FONT_OVERLINE;
    ucStyle.Replace(_T("O"),_T(""));
  }
  if (ucStyle.Find(_T('S')) >= 0)
  {
    m_decoration |= wxPDF_FONT_STRIKEOUT;
    ucStyle.Replace(_T("S"),_T(""));
  }
  if (ucStyle == _T("IB"))
  {
    ucStyle = _T("BI");
  }
  if (size == 0)
  {
    size = m_fontSizePt;
  }

  // Test if font is already selected
  if (m_fontFamily == lcFamily && m_fontStyle == ucStyle && m_fontSizePt == size && !m_inTemplate)
  {
    return true;
  }

  // Test if used for the first time
  wxPdfFont* currentFont = NULL;
  wxString fontkey = lcFamily + ucStyle;
  wxPdfFontHashMap::iterator font = (*m_fonts).find(fontkey);
  if (font == (*m_fonts).end())
  {
    // Check if one of the standard fonts
    wxPdfCoreFontMap::iterator coreFont = (*m_coreFonts).find(fontkey);
    if (coreFont != (*m_coreFonts).end())
    {
      int i = (*m_fonts).size() + 1;
      int j = coreFont->second;
      const wxCoreFontDesc& coreFontDesc = wxCoreFontTable[j];
      currentFont =
        new wxPdfFont(i, coreFontDesc.name, coreFontDesc.cwArray,
                      wxPdfFontDescription(coreFontDesc.ascent, coreFontDesc.descent,
                                           coreFontDesc.capHeight, coreFontDesc.flags,
                                           coreFontDesc.bbox, coreFontDesc.italicAngle,
                                           coreFontDesc.stemV, coreFontDesc.missingWidth,
                                           coreFontDesc.xHeight, coreFontDesc.underlinePosition,
                                           coreFontDesc.underlineThickness));
      (*m_fonts)[fontkey] = currentFont;
    }
    else
    {
      // Undefined font
      wxLogDebug(_T("wxPdfDocument::SetFont: Undefined font: '%s %s'."), family.c_str(), style.c_str());
      return false;
    }
  }
  else
  {
    currentFont = font->second;
  }

  // Select it
  m_fontFamily  = lcFamily;
  m_fontStyle   = ucStyle;
  m_fontSizePt  = size;
  m_fontSize    = size / m_k;
  m_currentFont = currentFont;
  if (setFont && m_page > 0)
  {
    OutAscii(wxString::Format(_T("BT /F%d "),m_currentFont->GetIndex()) +
             Double2String(m_fontSizePt,2) + wxString(_T(" Tf ET")));
  }
  if (m_inTemplate)
  {
    (*(m_currentTemplate->m_fonts))[fontkey] = currentFont;
  }
  return true;
}

void
wxPdfDocument::EndDoc()
{
  if(m_extGStates->size() > 0 && m_PDFVersion < _T("1.4"))
  {
    m_PDFVersion = _T("1.4");
  }
  if (m_importVersion > m_PDFVersion)
  {
    m_PDFVersion = m_importVersion;
  }

  PutHeader();
  PutPages();

  PutResources();
  
  // Info
  NewObj();
  Out("<<");
  PutInfo();
  Out(">>");
  Out("endobj");
  
  // Form fields
  PutFormFields();

  // Catalog
  NewObj();
  Out("<<");
  PutCatalog();
  Out(">>");
  Out("endobj");
  
  // Cross-Reference
  int o = m_buffer.TellO();
  Out("xref");
  OutAscii(wxString(_T("0 ")) + wxString::Format(_T("%d"),(m_n+1)));
  Out("0000000000 65535 f ");
  int i;
  for (i = 0; i < m_n; i++)
  {
    OutAscii(wxString::Format(_T("%010d 00000 n "),(*m_offsets)[i]));
  }
  
  // Trailer
  Out("trailer");
  Out("<<");
  PutTrailer();
  Out(">>");
  Out("startxref");
  OutAscii(wxString::Format(_T("%d"),o));
  Out("%%EOF");
  m_state = 3;
}

void
wxPdfDocument::BeginPage(int orientation)
{
  m_page++;
  (*m_pages)[m_page] = new wxMemoryOutputStream();
  m_state = 2;
  m_x = m_lMargin;
  m_y = m_tMargin;
  m_fontFamily = _T("");

  // Page orientation
  if (orientation < 0)
  {
    orientation = m_defOrientation;
  }
  else
  {
    if (orientation != m_defOrientation)
    {
      (*m_orientationChanges)[m_page] = true;
    }
  }
  if (orientation != m_curOrientation)
  {
    // Change orientation
    if (orientation == wxPORTRAIT)
    {
      m_wPt = m_fwPt;
      m_hPt = m_fhPt;
      m_w = m_fw;
      m_h = m_fh;
    }
    else
    {
      m_wPt = m_fhPt;
      m_hPt = m_fwPt;
      m_w = m_fh;
      m_h = m_fw;
    }
    m_pageBreakTrigger = m_h - m_bMargin;
    m_curOrientation = orientation;
  }
}

void
wxPdfDocument::EndPage()
{
  // End of page contents
  while (m_inTransform > 0)
  {
    StopTransform();
  }
  m_state = 1;
}

void
wxPdfDocument::PutHeader()
{
  OutAscii(wxString(_T("%PDF-")) + m_PDFVersion);
}

void
wxPdfDocument::PutTrailer()
{
  OutAscii(wxString(_T("/Size ")) + wxString::Format(_T("%d"),(m_n+1)));
  OutAscii(wxString(_T("/Root ")) + wxString::Format(_T("%d"),m_n) + wxString(_T(" 0 R")));
  OutAscii(wxString(_T("/Info ")) + wxString::Format(_T("%d"),(m_n-1)) + wxString(_T(" 0 R")));

  if (m_encrypted)
  {
    OutAscii(wxString::Format(_T("/Encrypt %d 0 R"), m_encObjId));
    Out("/ID [", false);
    m_encrypted = false;
    OutHexTextstring(m_encryptor->GetDocumentId(), false);
    OutHexTextstring(m_encryptor->GetDocumentId(), false);
    m_encrypted = true;
    Out("]");
  }
}

int
wxPdfDocument::GetNewObjId()
{
  m_n++;
  return m_n;
}

void
wxPdfDocument::NewObj(int objId)
{
  // Begin a new object
  int id = (objId > 0) ? objId : GetNewObjId();
  (*m_offsets)[id-1] = m_buffer.TellO();
  OutAscii(wxString::Format(_T("%d"),id) + wxString(_T(" 0 obj")));
}

void
wxPdfDocument::PutFormFields()
{
  wxPdfFormFieldsMap::iterator formField = m_formFields->begin();
  for (formField = m_formFields->begin(); formField != m_formFields->end(); formField++)
  {
    OutIndirectObject(formField->second);
  }
}

void
wxPdfDocument::PutInfo()
{
  Out("/Producer ",false); 
  OutTextstring(wxString(wxPDF_PRODUCER));
  if (m_title.Length() > 0)
  {
    Out("/Title ",false);
    OutTextstring(m_title);
  }
  if (m_subject.Length() > 0)
  {
    Out("/Subject ",false);
    OutTextstring(m_subject);
  }
  if (m_author.Length() > 0)
  {
    Out("/Author ",false);
    OutTextstring(m_author);
  }
  if (m_keywords.Length() > 0)
  {
    Out("/Keywords ",false);
    OutTextstring(m_keywords);
  }
  if (m_creator.Length() > 0)
  {
    Out("/Creator ",false);
    OutTextstring(m_creator);
  }
  wxDateTime now = wxDateTime::Now();
  Out("/CreationDate ",false);
  OutTextstring(wxString(_T("D:")+now.Format(_T("%Y%m%d%H%M%S"))));
}

void
wxPdfDocument::PutCatalog()
{
  Out("/Type /Catalog");
  Out("/Pages 1 0 R");

  Out("/OCProperties <<");
  wxPdfOcgHashMap &ocgHashMap = m_pdfOc.GetOcgMap();
  wxPdfOcgHashMap::iterator ocgIter = ocgHashMap.begin();
  Out(" /OCGs [");
  for( ocgIter = ocgHashMap.begin(); ocgIter != ocgHashMap.end(); ocgIter++) {
    wxPdfOcg* ocg = ocgIter->second;
    OutAscii(wxString::Format(_T("%d 0 R"), ocg->GetObjectIndex()));
  }  
  Out("]");
  Out(" /D <<");
  Out(" /BaseState /ON ");
  Out(" /Intent /View ");
  Out(" /ON [");
  for( ocgIter = ocgHashMap.begin(); ocgIter != ocgHashMap.end(); ocgIter++) {
    wxPdfOcg* ocg = ocgIter->second;
    if( ocg->GetDefaultVisibilityState() == true) {
      OutAscii(wxString::Format(_T("%d 0 R"), ocg->GetObjectIndex()));
    }
  }
  Out(" ]");
  Out(" /OFF [");
  for( ocgIter = ocgHashMap.begin(); ocgIter != ocgHashMap.end(); ocgIter++) {
    wxPdfOcg* ocg = ocgIter->second;
    if( ocg->GetDefaultVisibilityState() == false) {
      OutAscii(wxString::Format(_T("%d 0 R"), ocg->GetObjectIndex()));
    }
  }
  Out(" ]");
  Out(" /Order [");
  for( ocgIter = ocgHashMap.begin(); ocgIter != ocgHashMap.end(); ocgIter++) {
    wxPdfOcg* ocg = ocgIter->second;
    OutAscii(wxString::Format(_T("%d 0 R"), ocg->GetObjectIndex()));
  }
  Out("]");

  Out(">>");
  Out(">>");

  if (m_zoomMode == wxPDF_ZOOM_FULLPAGE)
  {
    OutAscii(wxString::Format(_T("/OpenAction [%d 0 R /Fit]"), m_firstPageId));
  }
  else if (m_zoomMode == wxPDF_ZOOM_FULLWIDTH)
  {
    OutAscii(wxString::Format(_T("/OpenAction [%d 0 R /FitH null]"), m_firstPageId));
  }
  else if (m_zoomMode == wxPDF_ZOOM_REAL)
  {
    OutAscii(wxString::Format(_T("/OpenAction [%d 0 R /XYZ null null 1]"), m_firstPageId));
  }
  else if (m_zoomMode == wxPDF_ZOOM_FACTOR)
  {
    OutAscii(wxString::Format(_T("/OpenAction [%d 0 R /XYZ null null "), m_firstPageId) +
             Double2String(m_zoomFactor/100.,3) + wxString(_T("]")));
  }

  if (m_layoutMode == wxPDF_LAYOUT_SINGLE)
  {
    Out("/PageLayout /SinglePage");
  }
  else if (m_layoutMode == wxPDF_LAYOUT_CONTINUOUS)
  {
    Out("/PageLayout /OneColumn");
  }
  else if (m_layoutMode == wxPDF_LAYOUT_TWO)
  {
    Out("/PageLayout /TwoColumnLeft");
  }

  if(m_outlines.GetCount() > 0)
  {
    OutAscii(wxString::Format(_T("/Outlines %d 0 R"), m_outlineRoot));
    Out("/PageMode /UseOutlines");
  }

  
  if (m_viewerPrefs > 0)
  {
    Out("/ViewerPreferences <<");
    if (m_viewerPrefs & wxPDF_VIEWER_HIDETOOLBAR)
    {
      Out("/HideToolbar true");
    }
    if (m_viewerPrefs & wxPDF_VIEWER_HIDEMENUBAR)
    {
      Out("/HideMenubar true");
    }
    if (m_viewerPrefs & wxPDF_VIEWER_HIDEWINDOWUI)
    {
      Out("/HideWindowUI true");
    }
    if (m_viewerPrefs & wxPDF_VIEWER_FITWINDOW)
    {
      Out("/FitWindow true");
    }
    if (m_viewerPrefs & wxPDF_VIEWER_CENTERWINDOW)
    {
      Out("/CenterWindow true");
    }
    if (m_viewerPrefs & wxPDF_VIEWER_DISPLAYDOCTITLE)
    {
      Out("/DisplayDocTitle true");
    }
    Out(">>");
  }

  if (m_javascript.Length() > 0)
  {
    OutAscii(wxString::Format(_T("/Names <</JavaScript %d 0 R>>"), m_nJS));
  }

  if ((*m_formFields).size() > 0)
  {
    Out("/AcroForm <<");
    Out("/Fields [", false);
    wxPdfFormFieldsMap::iterator formField = m_formFields->begin();
    for (formField = m_formFields->begin(); formField != m_formFields->end(); formField++)
    {
      wxPdfIndirectObject* field = formField->second;
      OutAscii(wxString::Format(_T("%d %d R "), field->GetObjectId(), field->GetGenerationId()), false);
    }
    Out("]");
    Out("/DR 2 0 R");
//    Out("/DR [ 2 0 R << /Font << /ZaDb << /Type /Font /Subtype /Type1 /Name /ZaDb /BaseFont /ZapfDingbats >> >> >>]");
    Out("/NeedAppearances true");
    Out(">>");

     // << /DR << /Font << /ZaDb << /Type /Font /Subtype /Type1 /Name /ZaDb /BaseFont /ZapfDingbats >>
     //  /Helv 3 0 R >> >> /DA (/Helv 10 Tf 0 g )
     //  /NeedAppearances true >> 

  }
}

// --- Fast string search (KMP method) for page number alias replacement

static int*
makeFail(const char* target, int tlen)
{
  int t = 0;
  int s, m;
  m = tlen;
  int* f = new int[m+1];
  f[1] = 0;
  for (s = 1; s < m; s++)
  {
    while ((t > 0) && (target[s] != target[t]))
    {
      t = f[t];
    }
    if (target[t] == target[s])
    {
      t++;
      f[s+1] = t;
    }
    else
    {
      f[s+1] = 0;
    }
  }
  return f;
}

static int
findString(const char* src, int slen, const char* target, int tlen, int* f)
{
  int s = 0;
  int i;
  int m = tlen;
  for (i = 0; i < slen; i++)
  {
    while ( (s > 0) && (src[i] != target[s]))
    {
      s = f[s];
    }
    if (src[i] == target[s]) s++;
    if (s == m) return (i-m+1);
  }
  return slen;
}

void
wxPdfDocument::ReplaceNbPagesAlias()
{
  int lenAsc = m_aliasNbPages.Length();
#if wxUSE_UNICODE
  wxCharBuffer wcb(m_aliasNbPages.ToAscii());
  const char* nbAsc = (const char*) wcb;
#else
  const char* nbAsc = m_aliasNbPages.c_str();
#endif
  int* fAsc = makeFail(nbAsc,lenAsc);

#if wxUSE_UNICODE
  wxMBConvUTF16BE conv;
  int lenUni = conv.WC2MB(NULL, m_aliasNbPages, 0);
  char* nbUni = new char[lenUni+3];
  lenUni = conv.WC2MB(nbUni, m_aliasNbPages, lenUni+3);
  int* fUni = makeFail(nbUni,lenUni);
#endif

  wxString pg = wxString::Format(_T("%d"),m_page);
  int lenPgAsc = pg.Length();
#if wxUSE_UNICODE
  wxCharBuffer wpg(pg.ToAscii());
  const char* pgAsc = (const char*) wpg;
  int lenPgUni = conv.WC2MB(NULL, pg, 0);
  char* pgUni = new char[lenPgUni+3];
  lenPgUni = conv.WC2MB(pgUni, pg, lenPgUni+3);
#else
  const char* pgAsc = pg.c_str();
#endif

  int n;
  for (n = 1; n <= m_page; n++)
  {
    wxMemoryOutputStream* p = new wxMemoryOutputStream();
    wxMemoryInputStream inPage(*((*m_pages)[n]));
    int len = inPage.GetSize();
    char* buffer = new char[len];
    char* pBuf = buffer;
    inPage.Read(buffer,len);
    int pAsc = findString(buffer,len,nbAsc,lenAsc,fAsc);
#if wxUSE_UNICODE
    int pUni = findString(buffer,len,nbUni,lenUni,fUni);
    while (pAsc < len || pUni < len)
    {
      if (pAsc < len && pAsc < pUni)
      {
        if (pAsc > 0)
        {
          p->Write(pBuf,pAsc);
        }
        p->Write(pgAsc,lenPgAsc);
        pBuf += pAsc + lenAsc;
        len  -= (pAsc + lenAsc);
        pUni -= (pAsc + lenAsc);
        pAsc = findString(pBuf,len,nbAsc,lenAsc,fAsc);
      }
      else if (pUni < len && pUni < pAsc)
      {
        if (pUni > 0)
        {
          p->Write(pBuf,pUni);
        }
        p->Write(pgUni,lenPgUni);
        pBuf += pUni + lenUni;
        len  -= (pUni + lenUni);
        pAsc -= (pUni + lenUni);
        pUni = findString(pBuf,len,nbUni,lenUni,fUni);
      }
    }
#else
    while (pAsc < len)
    {
      if (pAsc > 0)
      {
        p->Write(pBuf,pAsc);
      }
      p->Write(pgAsc,lenPgAsc);
      pBuf += pAsc + lenAsc;
      len  -= (pAsc + lenAsc);
      pAsc = findString(pBuf,len,nbAsc,lenAsc,fAsc);
    }
#endif
    if (len > 0)
    {
      p->Write(pBuf,len);
    }
    delete [] buffer;
    delete (*m_pages)[n];
    (*m_pages)[n] = p;
  }

#if wxUSE_UNICODE
  delete [] pgUni;
  delete [] fUni;
  delete [] nbUni;
#endif
  delete [] fAsc;
}

void
wxPdfDocument::PutPages()
{
  double wPt, hPt;
  int nb = m_page;
  int n;
  int nbannot = 0;
  int firstTextAnnotation = m_n;

  //Text annotations
  for (n = 1; n <= nb; n++)
  {
    wxPdfAnnotationsMap::iterator pageAnnots = (*m_annotations).find(n);
    if (pageAnnots != (*m_annotations).end())
    {
      // Links
      wxArrayPtrVoid* pageAnnotsArray = pageAnnots->second;

      int pageAnnotsCount = pageAnnotsArray->GetCount();
      int j;
      for (j = 0; j < pageAnnotsCount; j++)
      {
        wxPdfAnnotation* annotation = (wxPdfAnnotation*) (*pageAnnotsArray)[j];
        NewObj();
        double x = annotation->GetX();
        double y = annotation->GetY();
        Out("<</Type /Annot /Subtype /Text /Rect [", false);
        wxString rect = Double2String(x,2) + wxString(_T(" ")) +
                        Double2String(y,2) + wxString(_T(" ")) +
                        Double2String(x,2) + wxString(_T(" ")) +
                        Double2String(y,2);
        OutAscii(rect,false);
        Out("] /Contents ", false);
        OutTextstring(annotation->GetText(), false);
        Out(">>");
        Out("endobj");
      }
    }
  }

  if (m_aliasNbPages.Length() > 0)
  {
    // Replace number of pages
    ReplaceNbPagesAlias();
  }

  if (m_defOrientation == wxPORTRAIT)
  {
    wPt = m_fwPt;
    hPt = m_fhPt;
  }
  else
  {
    wPt = m_fhPt;
    hPt = m_fwPt;
  }
  wxString filter = (m_compress) ? _T("/Filter /FlateDecode ") : _T("");

  m_firstPageId = m_n + 1;
  for (n = 1; n <= nb; n++)
  {
    // Page
    NewObj();
    Out("<</Type /Page");
    Out("/Parent 1 0 R");

    wxPdfBoolHashMap::iterator oChange = (*m_orientationChanges).find(n);
    if (oChange != (*m_orientationChanges).end())
    {
      OutAscii(wxString(_T("/MediaBox [0 0 ")) +
               Double2String(hPt,2) + wxString(_T(" ")) +
               Double2String(wPt,2) + wxString(_T("]")));
    }

    Out("/Resources 2 0 R");

    Out("/Annots [",false);
    wxPdfPageLinksMap::iterator pageLinks = (*m_pageLinks).find(n);
    if (pageLinks != (*m_pageLinks).end())
    {
      // Links
      wxArrayPtrVoid* pageLinkArray = pageLinks->second;
      int pageLinkCount = pageLinkArray->GetCount();
      int j;
      for (j = 0; j < pageLinkCount; j++)
      {
        wxPdfPageLink* pl = (wxPdfPageLink*) (*pageLinkArray)[j];
        wxString rect = Double2String(pl->GetX(),2) + wxString(_T(" ")) +
                        Double2String(pl->GetY(),2) + wxString(_T(" ")) +
                        Double2String(pl->GetX()+pl->GetWidth(),2) + wxString(_T(" ")) +
                        Double2String(pl->GetY()-pl->GetHeight(),2);
        Out("<</Type /Annot /Subtype /Link /Rect [",false);
        OutAscii(rect,false);
        Out("] /Border [0 0 0] ",false);
        if (!pl->IsLinkRef())
        {
          Out("/A <</S /URI /URI ",false);
          OutAsciiTextstring(pl->GetLinkURL(),false);
          Out(">>>>",false);
        }
        else
        {
          wxPdfLink* link = (*m_links)[pl->GetLinkRef()];
          wxPdfBoolHashMap::iterator oChange = (*m_orientationChanges).find(link->GetPage());
          double h = (oChange != (*m_orientationChanges).end()) ? wPt : hPt;
          OutAscii(wxString::Format(_T("/Dest [%d 0 R /XYZ 0 "),m_firstPageId+2*(link->GetPage()-1)) +
                   Double2String(h-link->GetPosition()*m_k,2) + 
                   wxString(_T(" null]>>")),false);
        }
        delete pl;
        (*pageLinkArray)[j] = NULL;
      }
    }
    wxPdfAnnotationsMap::iterator pageAnnots = (*m_annotations).find(n);
    if (pageAnnots != (*m_annotations).end())
    {
      //Text annotations
      wxArrayPtrVoid* pageAnnotsArray = pageAnnots->second;
      int pageAnnotsCount = pageAnnotsArray->GetCount();
      int j;
      for (j = 0; j < pageAnnotsCount; j++)
      {
        nbannot++;
        OutAscii(wxString::Format(_T("%d 0 R "), firstTextAnnotation+nbannot), false);
        delete ((wxPdfAnnotation*) (*pageAnnotsArray)[j]);
        (*pageAnnotsArray)[j] = NULL;
      }
    }
    wxPdfFormAnnotsMap::iterator formAnnots = (*m_formAnnotations).find(n);
    if (formAnnots != (*m_formAnnotations).end())
    {
      // Form annotations
      wxArrayPtrVoid* formAnnotsArray = formAnnots->second;
      int formAnnotsCount = formAnnotsArray->GetCount();
      int j;
      for (j = 0; j < formAnnotsCount; j++)
      {
        wxPdfIndirectObject* object = static_cast<wxPdfIndirectObject*>((*formAnnotsArray)[j]);
        OutAscii(wxString::Format(_T("%d %d R "), object->GetObjectId(), object->GetGenerationId()), false);
//        delete ((wxPdfAnnotation*) (*formAnnotsArray)[j]);
//        (*formAnnotsArray)[j] = NULL;
      }
    }
    Out("]");

    OutAscii(wxString::Format(_T("/Contents %d 0 R>>"), m_n+1));
    Out("endobj");
    
    // Page content
    wxMemoryOutputStream* p;
    if (m_compress)
    {
      p = new wxMemoryOutputStream();
      wxZlibOutputStream q(*p);
      wxMemoryInputStream tmp(*((*m_pages)[n]));
      q.Write(tmp);
    }
    else
    {
      p = (*m_pages)[n];
    }

    NewObj();
    OutAscii(wxString(_T("<<")) + filter + wxString(_T("/Length ")) + 
             wxString::Format(_T("%ld"), CalculateStreamLength(p->TellO())) + wxString(_T(">>")));
    PutStream(*p);
    Out("endobj");
    if (m_compress)
    {
      delete p;
    }
  }
  // Pages root
  (*m_offsets)[0] = m_buffer.TellO();
  Out("1 0 obj");
  Out("<</Type /Pages");
  wxString kids = _T("/Kids [");
  int i;
  for (i = 0; i < nb; i++)
  {
    kids += wxString::Format(_T("%d"),(m_firstPageId+2*i)) + wxString(_T(" 0 R "));
  }
  OutAscii(kids + wxString(_T("]")));
  OutAscii(wxString(_T("/Count ")) + wxString::Format(_T("%d"),nb));
  OutAscii(wxString(_T("/MediaBox [0 0 ")) +
           Double2String(wPt,2) + wxString(_T(" ")) +
           Double2String(hPt,2) + wxString(_T("]")));
  Out(">>");
  Out("endobj");
}

void
wxPdfDocument::PutExtGStates()
{
  static wxChar* bms[] = {
    _T("/Normal"),     _T("/Multiply"),   _T("/Screen"),    _T("/Overlay"),    _T("/Darken"),
    _T("/Lighten"),    _T("/ColorDodge"), _T("/ColorBurn"), _T("/HardLight"),  _T("/SoftLight"),
    _T("/Difference"), _T("/Exclusion"),  _T("/Hue"),       _T("/Saturation"), _T("/Color"),
    _T("Luminosity")
  };
  wxPdfExtGStateMap::iterator extGState;
  for (extGState = m_extGStates->begin(); extGState != m_extGStates->end(); extGState++)
  {
    NewObj();
    extGState->second->SetObjIndex(m_n);
    Out("<</Type /ExtGState");
    OutAscii(wxString(_T("/ca ")) + Double2String(extGState->second->GetFillAlpha(), 3));
    OutAscii(wxString(_T("/CA ")) + Double2String(extGState->second->GetLineAlpha(), 3));
    OutAscii(wxString(_T("/bm ")) + wxString(bms[extGState->second->GetBlendMode()]));
    Out(">>");
    Out("endobj");
  }
}

void
wxPdfDocument::PutShaders()
{
  wxPdfGradientMap::iterator gradient;
  for (gradient = m_gradients->begin(); gradient != m_gradients->end(); gradient++)
  {
    wxPdfGradientType type = gradient->second->GetType();
    switch (type)
    {
      case wxPDF_GRADIENT_AXIAL:
      case wxPDF_GRADIENT_MIDAXIAL:
      case wxPDF_GRADIENT_RADIAL:
      {
        wxPdfColour color1 = ((wxPdfAxialGradient*)(gradient->second))->GetColor1();
        wxPdfColour color2 = ((wxPdfAxialGradient*)(gradient->second))->GetColor2();
        double intexp      = ((wxPdfAxialGradient*)(gradient->second))->GetIntExp();

        NewObj();
        Out("<<");
        Out("/FunctionType 2");
        Out("/Domain [0.0 1.0]");
        Out("/C0 [", false);
        OutAscii(color1.GetColorValue(), false);
        Out("]");
        Out("/C1 [", false);
        OutAscii(color2.GetColorValue(), false);
        Out("]");
        OutAscii(wxString(_T("/N ")) + Double2String(intexp,2));
        Out(">>");
        Out("endobj");
        int f1 = m_n;

        if (type == wxPDF_GRADIENT_MIDAXIAL)
        {
          double midpoint = ((wxPdfMidAxialGradient*)(gradient->second))->GetMidPoint();
          NewObj();
          Out("<<");
          Out("/FunctionType 3");
          Out("/Domain [0.0 1.0]");
          OutAscii(wxString::Format(_T("/Functions [%d 0 R %d 0 R]"), f1, f1));
          OutAscii(wxString(_T("/Bounds [")) + Double2String(midpoint,3) + wxString(_T("]")));
          Out("/Encode [0.0 1.0 1.0 0.0]");
          Out(">>");
          Out("endobj");
          f1 = m_n;
        }

        NewObj();
        Out("<<");
        OutAscii(wxString::Format(_T("/ShadingType %d"), ((type == wxPDF_GRADIENT_RADIAL) ? 3 : 2)));
        switch (color1.GetColorType())
        {
          case wxPDF_COLOURTYPE_GRAY:
            Out("/ColorSpace /DeviceGray");
            break;
          case wxPDF_COLOURTYPE_RGB:
            Out("/ColorSpace /DeviceRGB");
            break;
          case wxPDF_COLOURTYPE_CMYK:
            Out("/ColorSpace /DeviceCMYK");
            break;
        }
        if (type == wxPDF_GRADIENT_AXIAL ||
            type == wxPDF_GRADIENT_MIDAXIAL)
        {
          wxPdfAxialGradient* grad = (wxPdfAxialGradient*) (gradient->second);
          OutAscii(wxString(_T("/Coords [")) +
                   Double2String(grad->GetX1(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetY1(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetX2(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetY2(),3) + wxString(_T("]")));
          OutAscii(wxString::Format(_T("/Function %d 0 R"), f1));
          Out("/Extend [true true] ");
        }
        else
        {
          wxPdfRadialGradient* grad = (wxPdfRadialGradient*) (gradient->second);
          OutAscii(wxString(_T("/Coords [")) +
                   Double2String(grad->GetX1(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetY1(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetR1(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetX2(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetY2(),3) + wxString(_T(" ")) +
                   Double2String(grad->GetR2(),3) + wxString(_T("]")));
          OutAscii(wxString::Format(_T("/Function %d 0 R"), f1));
          Out("/Extend [true true] ");
        }
        Out(">>");
        Out("endobj");
        gradient->second->SetObjIndex(m_n);
        break;
      }
      case wxPDF_GRADIENT_COONS:
      {
        wxPdfCoonsPatchGradient* grad = (wxPdfCoonsPatchGradient*) (gradient->second);
        NewObj();
        Out("<<");
        Out("/ShadingType 6");
        switch (grad->GetColorType())
        {
          case wxPDF_COLOURTYPE_GRAY:
            Out("/ColorSpace /DeviceGray");
            break;
          case wxPDF_COLOURTYPE_RGB:
            Out("/ColorSpace /DeviceRGB");
            break;
          case wxPDF_COLOURTYPE_CMYK:
            Out("/ColorSpace /DeviceCMYK");
            break;
        }
        Out("/BitsPerCoordinate 16");
        Out("/BitsPerComponent 8");
        Out("/Decode[0 1 0 1 0 1 0 1 0 1]");
        Out("/BitsPerFlag 8");
        wxMemoryOutputStream* p = grad->GetBuffer();
        OutAscii(wxString::Format(_T("/Length %ld"), CalculateStreamLength(p->TellO())));
        Out(">>");
        PutStream(*p);
        Out("endobj");
        gradient->second->SetObjIndex(m_n);
      }
      default:
        break;
    }
  }
}

void
wxPdfDocument::PutFonts()
{
  int nf = m_n;

  int nb = (*m_diffs).size();
  int i;
  for (i = 1; i <= nb; i++)
  {
    // Encodings
    NewObj();
    Out("<</Type /Encoding /BaseEncoding /WinAnsiEncoding /Differences [", false);
    OutAscii(*(*m_diffs)[i], false);
    Out("]>>");
    Out("endobj");
  }

  wxString type;
  wxPdfFontHashMap::iterator fontIter = m_fonts->begin();
  for (fontIter = m_fonts->begin(); fontIter != m_fonts->end(); fontIter++)
  {
    wxPdfFont* font = fontIter->second;
    if (font->HasFile())
    {
      type = font->GetType();
      // Font file embedding
      NewObj();
      font->SetFileIndex(m_n);

      wxString strFontFileName = font->GetFontFile();
      wxFileName fontFileName(strFontFileName);
      fontFileName.MakeAbsolute(font->GetFilePath());

//      wxFileSystem fs;
//      wxFSFile* fontFile = fs.OpenFile(fontFileName.GetFullPath());
      if (fontFileName.FileExists())
      {
        wxFileInputStream* fontStream = new wxFileInputStream(fontFileName.GetFullPath());
        int fontLen   = fontStream->GetSize();
        int fontSize1 = font->GetSize1();
        wxMemoryOutputStream* p = new wxMemoryOutputStream();
        
        bool compressed = strFontFileName.Right(2) == _T(".z");
        if (!compressed && font->HasSize2())
        {
          unsigned char first = (unsigned char) fontStream->Peek();
          if (first == 128)
          {
            unsigned char* buffer = new unsigned char[fontLen];
            fontStream->Read(buffer, fontLen);
            if (buffer[6+font->GetSize1()] == 128)
            {
              // Strip first and second binary header
              fontLen -= 12;
              p->Write(&buffer[6], font->GetSize1());
              p->Write(&buffer[12 + font->GetSize1()], fontLen - font->GetSize1());
            }
            else
            {
              // Strip first binary header
              fontLen -= 6;
              p->Write(&buffer[6], fontLen);
            }
            delete [] buffer;
          }
          else
          {
            p->Write(*fontStream);
          }
        }
        else
        {
          if (m_fontSubsetting && font->SupportsSubset())
          {
            font->SetSubset(true);
            fontSize1 = font->CreateSubset(fontStream, p);
            compressed = true;
          }
          else
          {
            p->Write(*fontStream);
          }
        }
        fontLen = CalculateStreamLength(p->TellO());
        OutAscii(wxString::Format(_T("<</Length %d"), fontLen));
        if (compressed)
        {
          Out("/Filter /FlateDecode");
        }
        if (type == _T("OpenTypeUnicode"))
        {
          Out("/Subtype /CIDFontType0C");
        }
        else
        {
          OutAscii(wxString::Format(_T("/Length1 %d"), fontSize1));
          if (font->HasSize2())
          {
            OutAscii(wxString::Format(_T("/Length2 %d /Length3 0"), font->GetSize2()));
          }
        }
        Out(">>");
        PutStream(*p);
        Out("endobj");

        delete p;
//        delete fontFile;
        delete fontStream;
      }
    }
  }
  
  fontIter = m_fonts->begin();
  for (fontIter = m_fonts->begin(); fontIter != m_fonts->end(); fontIter++)
  {
    // Font objects
    wxPdfFont* font = fontIter->second;
    font->SetObjIndex(m_n+1);
    wxString type = font->GetType();
    wxString name = font->GetName();
    if (type == _T("core"))
    {
      // Standard font
      NewObj();
      Out("<</Type /Font");
      OutAscii(wxString(_T("/BaseFont /"))+name);
      Out("/Subtype /Type1");
      if (name != _T("Symbol") && name != _T("ZapfDingbats"))
      {
        Out("/Encoding /WinAnsiEncoding");
      }
      Out(">>");
      Out("endobj");
    }
    else if (type == _T("Type1") || type == _T("TrueType"))
    {
      // Additional Type1 or TrueType font
      NewObj();
      Out("<</Type /Font");
      OutAscii(wxString(_T("/BaseFont /")) + name);
      OutAscii(wxString(_T("/Subtype /")) + type);
      Out("/FirstChar 32 /LastChar 255");
      OutAscii(wxString::Format(_T("/Widths %d  0 R"), m_n+1));
      OutAscii(wxString::Format(_T("/FontDescriptor %d 0 R"), m_n+2));
      if (font->GetEncoding() != _T(""))
      {
        if (font->HasDiffs())
        {
          OutAscii(wxString::Format(_T("/Encoding %d 0 R"), (nf+font->GetDiffIndex())));
        }
        else
        {
          Out("/Encoding /WinAnsiEncoding");
        }
      }
      Out(">>");
      Out("endobj");

      // Widths
      NewObj();
      wxString s = font->GetWidthsAsString();
      OutAscii(s);
      Out("endobj");

      // Descriptor
      const wxPdfFontDescription& fd = font->GetDesc();
      NewObj();
      Out("<</Type /FontDescriptor");
      OutAscii(wxString(_T("/FontName /")) + name);
      OutAscii(wxString::Format(_T("/Ascent %d"), fd.GetAscent()));
      OutAscii(wxString::Format(_T("/Descent %d"), fd.GetDescent()));
      OutAscii(wxString::Format(_T("/CapHeight %d"), fd.GetCapHeight()));
      OutAscii(wxString::Format(_T("/Flags %d"), fd.GetFlags()));
      OutAscii(wxString(_T("/FontBBox")) + fd.GetFontBBox());
      OutAscii(wxString::Format(_T("/ItalicAngle %d"), fd.GetItalicAngle()));
      OutAscii(wxString::Format(_T("/StemV %d"), fd.GetStemV()));
      OutAscii(wxString::Format(_T("/MissingWidth %d"), fd.GetMissingWidth()));
      if (font->HasFile())
      {
        if (type == _T("Type1"))
        {
          OutAscii(wxString::Format(_T("/FontFile %d 0 R"), font->GetFileIndex()));
        }
        else
        {
          OutAscii(wxString::Format(_T("/FontFile2 %d 0 R"), font->GetFileIndex()));
        }
      }
      Out(">>");
      Out("endobj");
    }
    else if (type == _T("TrueTypeUnicode") || type == _T("OpenTypeUnicode"))
    {
      // Type0 Font
      // A composite font composed of other fonts, organized hierarchically
      NewObj();
      Out("<</Type /Font");
      Out("/Subtype /Type0");
      OutAscii(wxString(_T("/BaseFont /")) + name);
      // The horizontal identity mapping for 2-byte CIDs; may be used with
      // CIDFonts using any Registry, Ordering, and Supplement values.
      Out("/Encoding /Identity-H");
      OutAscii(wxString::Format(_T("/DescendantFonts [%d 0 R]"), (m_n + 1)));
      if (type == _T("OpenTypeUnicode"))
      {
        OutAscii(wxString::Format(_T("/ToUnicode %d 0 R"), (m_n + 4)));
      }
      Out(">>");
      Out("endobj");
      
      // CIDFontType
      NewObj();
      Out("<</Type /Font");
      if (type == _T("TrueTypeUnicode"))
      {
        // A CIDFont whose glyph descriptions are based on TrueType font technology
        Out("/Subtype /CIDFontType2");
      }
      else
      {
        // A CIDFont whose glyph descriptions are based on OpenType font technology
        Out("/Subtype /CIDFontType0");
      }
      OutAscii(wxString(_T("/BaseFont /")) + name);
      OutAscii(wxString::Format(_T("/CIDSystemInfo %d 0 R"), (m_n + 1))); 
      OutAscii(wxString::Format(_T("/FontDescriptor %d 0 R"), (m_n + 2)));

      const wxPdfFontDescription& fd = font->GetDesc();
      if (fd.GetMissingWidth() > 0)
      {
        // The default width for glyphs in the CIDFont MissingWidth
        OutAscii(wxString::Format(_T("/DW %d"), fd.GetMissingWidth()));
      }
      
      OutAscii(wxString(_T("/W ")) + font->GetWidthsAsString()); // A description of the widths for the glyphs in the CIDFont
      if (type == _T("TrueTypeUnicode"))
      {
        OutAscii(wxString::Format(_T("/CIDToGIDMap %d 0 R"), (m_n + 3)));
      }

      Out(">>");
      Out("endobj");
      
      // CIDSystemInfo dictionary
      // A dictionary containing entries that define the character collectionof the CIDFont.
      NewObj();
      // A string identifying an issuer of character collections
      Out("<</Registry ", false);
      OutAsciiTextstring(wxString(_T("Adobe")));
      // A string that uniquely names a character collection issued by a specific registry
      Out("/Ordering ", false);
      if (type == _T("TrueTypeUnicode"))
      {
        OutAsciiTextstring(wxString(_T("UCS")));
      }
      else
      {
        OutAsciiTextstring(wxString(_T("Identity")));
//        OutAsciiTextstring(wxString(_T("UCS")));
      }
      // The supplement number of the character collection.
      Out("/Supplement 0");
      Out(">>");
      Out("endobj");
      
      // Font descriptor
      // A font descriptor describing the CIDFonts default metrics other than its glyph widths
      NewObj();
      Out("<</Type /FontDescriptor");
      OutAscii(wxString(_T("/FontName /")) + name);
      wxString s = wxEmptyString;
      OutAscii(wxString::Format(_T("/Ascent %d"), fd.GetAscent()));
      OutAscii(wxString::Format(_T("/Descent %d"), fd.GetDescent()));
      OutAscii(wxString::Format(_T("/CapHeight %d"), fd.GetCapHeight()));
      OutAscii(wxString::Format(_T("/Flags %d"), fd.GetFlags()));
      OutAscii(wxString(_T("/FontBBox")) + fd.GetFontBBox());
      OutAscii(wxString::Format(_T("/ItalicAngle %d"), fd.GetItalicAngle()));
      OutAscii(wxString::Format(_T("/StemV %d"), fd.GetStemV()));
      OutAscii(wxString::Format(_T("/MissingWidth %d"), fd.GetMissingWidth()));

      if (font->HasFile())
      {
        if (type == _T("TrueTypeUnicode"))
        {
          // A stream containing a TrueType font program
          OutAscii(wxString::Format(_T("/FontFile2 %d 0 R"), font->GetFileIndex()));
        }
        else
        {
          // A stream containing a CFF font program
          OutAscii(wxString::Format(_T("/FontFile3 %d 0 R"), font->GetFileIndex()));
        }
      }
      Out(">>");
      Out("endobj");

      // Embed CIDToGIDMap
      // A specification of the mapping from CIDs to glyph indices
      NewObj();
      wxString strCtgFileName = font->GetCtgFile();
      wxFileName ctgFileName(strCtgFileName);
      ctgFileName.MakeAbsolute(font->GetFilePath());

//      wxFileSystem fs;
//      wxFSFile* ctgFile = fs.OpenFile(ctgFileName.GetFullPath());
      if (ctgFileName.FileExists())
      {
        wxMemoryOutputStream* p = new wxMemoryOutputStream();
        wxFileInputStream* ctgStream = new wxFileInputStream(ctgFileName.GetFullPath());
        int ctgLen = CalculateStreamLength(ctgStream->GetSize());
        OutAscii(wxString::Format(_T("<</Length %d"), ctgLen));
        // check file extension
        bool compressed = strCtgFileName.Right(2) == _T(".z");
        if (compressed)
        {
          // Decompresses data encoded using the public-domain zlib/deflate compression
          // method, reproducing the original text or binary data
          Out("/Filter /FlateDecode");
        }
        Out(">>");
        p->Write(*ctgStream);
        PutStream(*p);
        delete p;
//        delete ctgFile;
        delete ctgStream;
      }
      else
      {
        // TODO : file not found, should be checked already when adding font!
        wxLogDebug(_T("wxPdfDocument::PutFonts: Font file '%s' not found."), strCtgFileName.c_str());
      }
      Out("endobj");
    }
    else if (type == _T("Type0"))
    {
      // Type0
      NewObj();
      Out("<</Type /Font");
      Out("/Subtype /Type0");
      OutAscii(wxString(_T("/BaseFont /")) + name + wxString(_T("-")) + font->GetCMap());
      OutAscii(wxString(_T("/Encoding /")) + font->GetCMap());
      OutAscii(wxString::Format(_T("/DescendantFonts [%d 0 R]"), (m_n+1)));
      Out(">>");
      Out("endobj");

      // CIDFont
      NewObj();
      Out("<</Type /Font");
      Out("/Subtype /CIDFontType0");
      OutAscii(wxString(_T("/BaseFont /")) + name);
      Out("/CIDSystemInfo <</Registry ", false);
      OutAsciiTextstring(_T("Adobe"), false);
      Out("/Ordering ", false);
      OutAsciiTextstring(font->GetOrdering(), false);
      OutAscii(wxString(_T(" /Supplement ")) + font->GetSupplement() + wxString(_T(">>")));
      OutAscii(wxString::Format(_T("/FontDescriptor %d 0 R"), (m_n+1)));

      // Widths
      // A description of the widths for the glyphs in the CIDFont
      OutAscii(wxString(_T("/W ")) + font->GetWidthsAsString());
      Out(">>");
      Out("endobj");

      // Font descriptor
      const wxPdfFontDescription& fd = font->GetDesc();
      NewObj();
      Out("<</Type /FontDescriptor");
      OutAscii(wxString(_T("/FontName /")) + name);
      OutAscii(wxString::Format(_T("/Ascent %d"), fd.GetAscent()));
      OutAscii(wxString::Format(_T("/Descent %d"), fd.GetDescent()));
      OutAscii(wxString::Format(_T("/CapHeight %d"), fd.GetCapHeight()));
      OutAscii(wxString::Format(_T("/Flags %d"), fd.GetFlags()));
      OutAscii(wxString(_T("/FontBBox")) + fd.GetFontBBox());
      OutAscii(wxString::Format(_T("/ItalicAngle %d"), fd.GetItalicAngle()));
      OutAscii(wxString::Format(_T("/StemV %d"), fd.GetStemV()));
      Out(">>");
      Out("endobj");
    }
  }
}

void
wxPdfDocument::PutImages()
{
  wxString filter = (m_compress) ? _T("/Filter /FlateDecode ") : _T("");
  int iter;
  for (iter = 0; iter < 2; iter++)
  {
    // We need two passes to resolve dependencies
    wxPdfImageHashMap::iterator image = m_images->begin();
    for (image = m_images->begin(); image != m_images->end(); image++)
    {
      // Image objects
      wxPdfImage* currentImage = image->second;

      if (currentImage->GetMaskImage() > 0)
      {
        // On first pass skip images depending on a mask
        if (iter == 0) continue;
      }
      else
      {
        // On second pass skip images already processed
        if (iter != 0) continue;
      }

      NewObj();
      currentImage->SetObjIndex(m_n);
      Out("<</Type /XObject");
      if (currentImage->IsFormObject())
      {
        Out("/Subtype /Form");
        OutAscii(wxString::Format(_T("/BBox [%d %d %d %d]"),
                   currentImage->GetX(), currentImage->GetY(),
                   currentImage->GetWidth()+currentImage->GetX(),
                   currentImage->GetHeight() + currentImage->GetY()));
        if (m_compress)
        {
          Out("/Filter /FlateDecode");
        }
        int dataLen = currentImage->GetDataSize();
        wxMemoryOutputStream* p = new wxMemoryOutputStream();
        if (m_compress)
        {
          wxZlibOutputStream q(*p);
          q.Write(currentImage->GetData(),currentImage->GetDataSize());
        }
        else
        {
          p->Write(currentImage->GetData(),currentImage->GetDataSize());
        }
        dataLen = CalculateStreamLength(p->TellO());
        OutAscii(wxString::Format(_T("/Length %d>>"),dataLen));
        PutStream(*p);

        Out("endobj");
        delete p;
      }
      else
      {
        Out("/Subtype /Image");
        OutAscii(wxString::Format(_T("/Width %d"),currentImage->GetWidth()));
        OutAscii(wxString::Format(_T("/Height %d"),currentImage->GetHeight()));

        int maskImage = currentImage->GetMaskImage();
        if (maskImage > 0)
        {
          int maskObjId = 0;
          wxPdfImageHashMap::iterator img = m_images->begin();
          while (maskObjId == 0 && img != m_images->end())
          {
            if (img->second->GetIndex() == maskImage)
            {
              maskObjId = img->second->GetObjIndex();
            }
            img++;
          }
          if (maskObjId > 0)
          {
            OutAscii(wxString::Format(_T("/SMask %d 0 R"), maskObjId));
          }
        }

        if (currentImage->GetColorSpace() == _T("Indexed"))
        {
          int palLen = currentImage->GetPaletteSize() / 3 - 1;
          OutAscii(wxString::Format(_T("/ColorSpace [/Indexed /DeviceRGB %d %d 0 R]"),
                   palLen,(m_n+1)));
        }
        else
        {
          OutAscii(wxString(_T("/ColorSpace /")) + currentImage->GetColorSpace());
          if (currentImage->GetColorSpace() == _T("DeviceCMYK"))
          {
            Out("/Decode [1 0 1 0 1 0 1 0]");
          }
        }
        OutAscii(wxString::Format(_T("/BitsPerComponent %d"),currentImage->GetBitsPerComponent()));
        wxString f = currentImage->GetF();
        if (f.Length() > 0)
        {
          OutAscii(wxString(_T("/Filter /")) + f);
        }
        wxString parms = currentImage->GetParms();
        if (parms.Length() > 0)
        {
          OutAscii(parms);
        }
        int trnsSize = currentImage->GetTransparencySize();
        unsigned char* trnsData = (unsigned char*) currentImage->GetTransparency();
        if (trnsSize > 0)
        {
          wxString trns = _T("");;
          int i;
          for (i = 0; i < trnsSize; i++)
          {
            int trnsValue = trnsData[i];
            trns += wxString::Format(_T("%d %d "), trnsValue, trnsValue);
          }
          OutAscii(wxString(_T("/Mask [")) + trns + wxString(_T("]")));
        }

        OutAscii(wxString::Format(_T("/Length %d>>"), CalculateStreamLength(currentImage->GetDataSize())));

        wxMemoryOutputStream* p = new wxMemoryOutputStream();
        p->Write(currentImage->GetData(),currentImage->GetDataSize());
        PutStream(*p);
        delete p;
        Out("endobj");

        // Palette
        if (currentImage->GetColorSpace() == _T("Indexed"))
        {
          NewObj();
          int palLen = currentImage->GetPaletteSize();
          p = new wxMemoryOutputStream();
          if (m_compress)
          {
            wxZlibOutputStream q(*p);
            q.Write(currentImage->GetPalette(),currentImage->GetPaletteSize());
          }
          else
          {
            p->Write(currentImage->GetPalette(),currentImage->GetPaletteSize());
          }
          palLen = CalculateStreamLength(p->TellO());
          OutAscii(wxString(_T("<<")) + filter + wxString::Format(_T("/Length %d>>"), palLen));
          PutStream(*p);
          Out("endobj");
          delete p;
        }
      }
    }
  }
}

void
wxPdfDocument::PutTemplates()
{
  wxString filter = (m_compress) ? _T("/Filter /FlateDecode ") : _T("");
  wxPdfTemplatesMap::iterator templateIter = m_templates->begin();
  for (templateIter = m_templates->begin(); templateIter != m_templates->end(); templateIter++)
  {
    // Image objects
    wxPdfTemplate* currentTemplate = templateIter->second;
    NewObj();
    currentTemplate->SetObjIndex(m_n);

    OutAscii(wxString(_T("<<")) + filter + wxString(_T("/Type /XObject")));
    Out("/Subtype /Form");
    Out("/FormType 1");

    OutAscii(wxString(_T("/BBox [")) +
             Double2String(currentTemplate->GetX()*m_k,2) + wxString(_T(" ")) +
             Double2String(currentTemplate->GetY()*m_k,2) + wxString(_T(" ")) +
             Double2String((currentTemplate->GetX()+currentTemplate->GetWidth())*m_k,2) + wxString(_T(" ")) +
             Double2String((currentTemplate->GetY()+currentTemplate->GetHeight())*m_k,2) + wxString(_T("]")));

    Out("/Resources ");
    if (currentTemplate->GetResources() != NULL)
    {
      m_currentParser = currentTemplate->GetParser();
      WriteObjectValue(currentTemplate->GetResources());
    }
    else
    {
      Out("<</ProcSet [/PDF /Text /ImageB /ImageC /ImageI]");
      // Font references
      if (currentTemplate->m_fonts->size() > 0)
      {
        Out("/Font <<");
        wxPdfFontHashMap::iterator font = currentTemplate->m_fonts->begin();
        for (font = currentTemplate->m_fonts->begin(); font != currentTemplate->m_fonts->end(); font++)
        {
          OutAscii(wxString::Format(_T("/F%d %d 0 R"), font->second->GetIndex(), font->second->GetObjIndex()));
        }
        Out(">>");
      }
      // Image and template references
      if (currentTemplate->m_images->size() > 0 ||
          currentTemplate->m_templates->size() > 0)
      {
        Out("/XObject <<");
        wxPdfImageHashMap::iterator image = currentTemplate->m_images->begin();
        for (image = currentTemplate->m_images->begin(); image != currentTemplate->m_images->end(); image++)
        {
          wxPdfImage* currentImage = image->second;
          OutAscii(wxString::Format(_T("/I%d %d 0 R"), currentImage->GetIndex(), currentImage->GetObjIndex()));
        }
        wxPdfTemplatesMap::iterator templateIter = currentTemplate->m_templates->begin();
        for (templateIter = currentTemplate->m_templates->begin(); templateIter != currentTemplate->m_templates->end(); templateIter++)
        {
          wxPdfTemplate* tpl = templateIter->second;
          OutAscii(m_templatePrefix + wxString::Format(_T("%d %d 0 R"), tpl->GetIndex(), tpl->GetObjIndex()));
        }
        Out(">>");
      }
      Out(">>");
    }
    
    // Template data
    wxMemoryOutputStream* p;
    if (m_compress)
    {
      p = new wxMemoryOutputStream();
      wxZlibOutputStream q(*p);
      wxMemoryInputStream tmp(currentTemplate->m_buffer);
      q.Write(tmp);
    }
    else
    {
      p = &(currentTemplate->m_buffer);
    }

    OutAscii(wxString::Format(_T("/Length %ld >>"), CalculateStreamLength(p->TellO())));
    PutStream(*p);
    Out("endobj");
    if (m_compress)
    {
      delete p;
    }
  }
}

void
wxPdfDocument::PutImportedObjects()
{
  wxPdfParserMap::iterator parser = m_parsers->begin();
  for (parser = m_parsers->begin(); parser != m_parsers->end(); parser++)
  {
    m_currentParser = parser->second;
    if (m_currentParser != NULL)
    {
      m_currentParser->SetUseRawStream(true);
      wxPdfObjectQueue* entry = m_currentParser->GetObjectQueue();
      while ((entry = entry->GetNext()) != NULL)
      {
        wxPdfObject* resolvedObject = m_currentParser->ResolveObject(entry->GetObject());
        NewObj(entry->GetActualObjectId());
        WriteObjectValue(resolvedObject);
        Out("endobj");
        entry->SetObject(resolvedObject);
      }
    }
  }
}

void
wxPdfDocument::PutXObjectDict()
{
  wxPdfImageHashMap::iterator image = m_images->begin();
  for (image = m_images->begin(); image != m_images->end(); image++)
  {
    wxPdfImage* currentImage = image->second;
    OutAscii(wxString::Format(_T("/I%d %d 0 R"), currentImage->GetIndex(), currentImage->GetObjIndex()));
  }
  wxPdfTemplatesMap::iterator templateIter = m_templates->begin();
  for (templateIter = m_templates->begin(); templateIter != m_templates->end(); templateIter++)
  {
    wxPdfTemplate* tpl = templateIter->second;
    OutAscii(m_templatePrefix + wxString::Format(_T("%d %d 0 R"), tpl->GetIndex(), tpl->GetObjIndex()));
  }
}

void
wxPdfDocument::PutResourceDict()
{
  Out("/ProcSet [/PDF /Text /ImageB /ImageC /ImageI]");
  Out("/Font <<");
  wxPdfFontHashMap::iterator font = m_fonts->begin();

  for (font = m_fonts->begin(); font != m_fonts->end(); font++)
  {
    OutAscii(wxString::Format(_T("/F%d %d 0 R"), font->second->GetIndex(), font->second->GetObjIndex()));
  }
  Out(">>");
  Out("/XObject <<");
  PutXObjectDict();
  Out(">>");

  Out("/ExtGState <<");
  wxPdfExtGStateMap::iterator extGState;
  for (extGState = m_extGStates->begin(); extGState != m_extGStates->end(); extGState++)
  {
    OutAscii(wxString::Format(_T("/GS%ld %d 0 R"), extGState->first, extGState->second->GetObjIndex()));
  }
  Out(">>");

  Out("/Shading <<");
  wxPdfGradientMap::iterator gradient;
  for (gradient = m_gradients->begin(); gradient != m_gradients->end(); gradient++)
  {
    //foreach(m_gradients as $id=>$grad)
    OutAscii(wxString::Format(_T("/Sh%ld %d 0 R"), gradient->first, gradient->second->GetObjIndex()));
  }
  Out(">>");
  Out("/ColorSpace <<");
  wxPdfSpotColourMap::iterator spotColor;
  for (spotColor = m_spotColors->begin(); spotColor != m_spotColors->end(); spotColor++)
  {
    OutAscii(wxString::Format(_T("/CS%d %d 0 R"), spotColor->second->GetIndex(), spotColor->second->GetObjIndex()));
  }
  Out(">>");
  Out("/Pattern <<");
  wxPdfPatternMap::iterator pattern;
  for (pattern = m_patterns->begin(); pattern != m_patterns->end(); pattern++)
  {
    OutAscii(wxString::Format(_T("/P%d %d 0 R"), pattern->second->GetIndex(), pattern->second->GetObjIndex()));
  }
  Out(">>");
  wxPdfOcgHashMap &ocgHashMap = m_pdfOc.GetOcgMap();
  wxPdfOcgHashMap::iterator ocgIter = ocgHashMap.begin();
  Out("/Properties <<");
  for( ocgIter = ocgHashMap.begin(); ocgIter != ocgHashMap.end(); ocgIter++) {
    wxPdfOcg* ocg = ocgIter->second;
    OutAscii(wxString::Format(_T("/OC%d %d 0 R"), ocg->GetOcgIndex(), ocg->GetObjectIndex()));
  }  
  Out(">>");
}

void
wxPdfDocument::PutBookmarks()
{
  int nb = m_outlines.GetCount();
  if (nb == 0)
  {
    return;
  }

  int i;
  int parent;
  wxArrayInt lru;
  lru.SetCount(m_maxOutlineLevel+1);
  int level = 0;
  for (i = 0; i < nb; i++)
  {
    wxPdfBookmark* bookmark = (wxPdfBookmark*) m_outlines[i];
    int currentLevel = bookmark->GetLevel();
    if (currentLevel > 0)
    {
      parent = lru[currentLevel-1];
      // Set parent and last pointers
      bookmark->SetParent(parent);
      wxPdfBookmark* parentBookmark = (wxPdfBookmark*) m_outlines[parent];
      parentBookmark->SetLast(i);
      if (currentLevel > level)
      {
        // Level increasing: set first pointer
        parentBookmark->SetFirst(i);
      }
    }
    else
    {
      bookmark->SetParent(nb);
    }
    if (currentLevel <= level && i > 0)
    {
      // Set prev and next pointers
      int prev = lru[currentLevel];
      wxPdfBookmark* prevBookmark = (wxPdfBookmark*) m_outlines[prev];
      prevBookmark->SetNext(i);
      bookmark->SetPrev(prev);
    }
    lru[currentLevel] = i;
    level = currentLevel;
  }

  // Outline items
  int n = m_n + 1;
  for (i = 0; i < nb; i++)
  {
    wxPdfBookmark* bookmark = (wxPdfBookmark*) m_outlines[i];
    NewObj();
    Out("<</Title ", false);
    OutTextstring(bookmark->GetText());
    OutAscii(wxString::Format(_T("/Parent %d 0 R"), (n+bookmark->GetParent())));
    if (bookmark->GetPrev() >= 0)
    {
      OutAscii(wxString::Format(_T("/Prev %d 0 R"), (n+bookmark->GetPrev())));
    }
    if (bookmark->GetNext() >= 0)
    {
      OutAscii(wxString::Format(_T("/Next %d 0 R"), (n+bookmark->GetNext())));
    }
    if (bookmark->GetFirst() >= 0)
    {
      OutAscii(wxString::Format(_T("/First %d 0 R"), (n+bookmark->GetFirst())));
    }
    if(bookmark->GetLast() >= 0)
    {
      OutAscii(wxString::Format(_T("/Last %d 0 R"), (n+bookmark->GetLast())));
    }
    OutAscii(wxString::Format(_T("/Dest [%d 0 R /XYZ 0 "), (m_firstPageId+2*(bookmark->GetPage()-1))) +
             Double2String((m_h-bookmark->GetY())*m_k,2) + wxString(_T(" null]")));
    Out("/Count 0>>");
    Out("endobj");
  }
  // Outline root
  NewObj();
  m_outlineRoot = m_n;
  OutAscii(wxString::Format(_T("<</Type /Outlines /First %d 0 R"), n));
  OutAscii(wxString::Format(_T("/Last %d 0 R>>"), (n+lru[0])));
  Out("endobj");
}

void
wxPdfDocument::PutEncryption()
{
  Out("/Filter /Standard");
  switch (m_encryptor->GetRevision())
  {
    case 4:
      {
        Out("/V 4");
        Out("/R 4");
        Out("/Length 128");
        Out("/CF <</StdCF <</CFM /AESV2 /Length 16 /AuthEvent /DocOpen>>>>");
        Out("/StrF /StdCF");
        Out("/StmF /StdCF");
      }
      break;
    case 3:
      {
        Out("/V 2");
        Out("/R 3");
        OutAscii(wxString::Format(_T("/Length %d"), m_encryptor->GetKeyLength()));
      }
      break;
    case 2:
    default:
      {
        Out("/V 1");
        Out("/R 2");
      }
      break;
  }
  Out("/O (",false);
  OutEscape((char*) m_encryptor->GetOValue(),32);
  Out(")");
  Out("/U (",false);
  OutEscape((char*) m_encryptor->GetUValue(),32);
  Out(")");
  OutAscii(wxString::Format(_T("/P %d"), m_encryptor->GetPValue()));
}

void
wxPdfDocument::PutSpotColors()
{
  wxPdfSpotColourMap::iterator spotIter = (*m_spotColors).begin();
  for (spotIter = (*m_spotColors).begin(); spotIter != (*m_spotColors).end(); spotIter++)
  {
    wxPdfSpotColour* spotColor = spotIter->second;
    NewObj();
    wxString spotColorName = spotIter->first;
    spotColorName.Replace(_T(" "),_T("#20"));
    Out("[/Separation /", false);
    OutAscii(spotColorName);
    Out("/DeviceCMYK <<");
    Out("/Range [0 1 0 1 0 1 0 1] /C0 [0 0 0 0] ");
    OutAscii(_T("/C1 [") +
             Double2String(ForceRange(spotColor->GetCyan(),    0., 100.)/100., 4) + _T(" ") +
             Double2String(ForceRange(spotColor->GetMagenta(), 0., 100.)/100., 4) + _T(" ") +
             Double2String(ForceRange(spotColor->GetYellow(),  0., 100.)/100., 4) + _T(" ") +
             Double2String(ForceRange(spotColor->GetBlack(),   0., 100.)/100., 4) + _T("] "));
    Out("/FunctionType 2 /Domain [0 1] /N 1>>]");
    Out("endobj");
    spotColor->SetObjIndex(m_n);
  }
}
 
void
wxPdfDocument::PutPatterns()
{
  wxPdfPatternMap::iterator patternIter = (*m_patterns).begin();
  for (patternIter = (*m_patterns).begin(); patternIter != (*m_patterns).end(); patternIter++)
  {
    wxPdfPattern* pattern = patternIter->second;
    NewObj();
    pattern->SetObjIndex(m_n);
    Out("<<");
    Out("/Type /Pattern");
    Out("/PatternType 1");
    Out("/PaintType 1");
    Out("/TilingType 1");
    OutAscii(wxString::Format(_T("/BBox [0 0 %d %d]"), pattern->GetImageWidth(), pattern->GetImageHeight()));
    OutAscii(wxString::Format(_T("/XStep %d"), pattern->GetImageWidth()));
    OutAscii(wxString::Format(_T("/YStep %d"), pattern->GetImageHeight()));
    wxPdfImage *image = pattern->GetImage();
    OutAscii(wxString::Format(_T("/Resources << /XObject << /I%d %d 0 R >> >>"), image->GetIndex(), image->GetObjIndex()));
    Out("/Matrix [ 1 0 0 1 0 0 ]");

    wxString sdata = wxString::Format(_T("q %d 0 0 %d 0 0 cm /I%d Do Q"), pattern->GetImageWidth(), pattern->GetImageHeight(), image->GetIndex());
    wxMemoryOutputStream *p = new wxMemoryOutputStream;
    p->Write(sdata.ToAscii(), sdata.Length());
    OutAscii(wxString(_T("/Length ")) + wxString::Format(_T("%ld"), CalculateStreamLength(p->TellO())) );
    Out(">>");
    PutStream(*p);
    delete p;
    Out("endobj");
  }
}

void
wxPdfDocument::PutOc()
{
  wxPdfOcgHashMap &ocgHashMap = m_pdfOc.GetOcgMap();
  wxPdfOcgHashMap::iterator ocgIter = ocgHashMap.begin();
  for( ocgIter = ocgHashMap.begin(); ocgIter != ocgHashMap.end(); ocgIter++) {
    wxPdfOcg* ocg = ocgIter->second;
    NewObj();
    ocg->SetObjectIndex(m_n);
    Out("<<");
    Out("/Type /OCG");
    OutAscii(wxString::Format(_T("/Name (%s)"), ocg->GetName()));
    OutAscii(_T("/Intent ") + ocg->GetIntentString());
    Out(">>");
    Out("endobj");
  }
}

void
wxPdfDocument::PutJavaScript()
{
  if (m_javascript.Length() > 0)
  {
    NewObj();
    m_nJS = m_n;
    Out("<<");
    Out("/Names [", false);
    OutAsciiTextstring(wxString(_T("EmbeddedJS")), false);
    OutAscii(wxString::Format(_T(" %d 0 R ]"), m_n+1));
    Out(">>");
    Out("endobj");
    NewObj();
    Out("<<");
    Out("/S /JavaScript");
    Out("/JS ", false);
    // TODO: Write Javascript object as stream
    OutTextstring(m_javascript);
    Out(">>");
    Out("endobj");
  }
}

void
wxPdfDocument::PutResources()
{
  PutExtGStates();
  PutShaders();
  PutFonts();
  PutImages();
  PutTemplates();
  PutImportedObjects();
  PutSpotColors();
  PutPatterns();
  PutOc();

  // Resource dictionary
  (*m_offsets)[2-1] = m_buffer.TellO();
  Out("2 0 obj");
  Out("<<");
  PutResourceDict();
  Out(">>");
  Out("endobj");

  PutBookmarks();
  PutJavaScript();

  if (m_encrypted)
  {
    NewObj();
    m_encObjId = m_n;
    Out("<<");
    PutEncryption();
    Out(">>");
    Out("endobj");
  }

}

wxString
wxPdfDocument::DoDecoration(double x, double y, const wxString& txt)
{
  // Decorate text
  int top  = m_currentFont->GetBBoxTopPosition();
  int up   = m_currentFont->GetUnderlinePosition();
  int ut   = m_currentFont->GetUnderlineThickness();
  double w = GetStringWidth(txt) + m_ws * txt.Freq(_T(' '));
  wxString decoration = _T("");
  if (m_decoration & wxPDF_FONT_UNDERLINE)
  {
    decoration = decoration + _T(" ") +
      Double2String(x * m_k,2) + wxString(_T(" ")) +
      Double2String((m_h - (y - up/1000.*m_fontSize)) * m_k,2) + wxString(_T(" ")) +
      Double2String(w * m_k,2) + wxString(_T(" ")) +
      Double2String(-ut/1000.*m_fontSizePt,2) + wxString(_T(" re f"));
  }
  if (m_decoration & wxPDF_FONT_OVERLINE)
  {
    up = (int) (top * 0.9);
    decoration = decoration + _T(" ") +
      Double2String(x * m_k,2) + wxString(_T(" ")) +
      Double2String((m_h - (y - up/1000.*m_fontSize)) * m_k,2) + wxString(_T(" ")) +
      Double2String(w * m_k,2) + wxString(_T(" ")) +
      Double2String(-ut/1000.*m_fontSizePt,2) + wxString(_T(" re f"));
  }
  if (m_decoration & wxPDF_FONT_STRIKEOUT)
  {
    up = (int) (top * 0.26);
    decoration = decoration + _T(" ") +
      Double2String(x * m_k,2) + wxString(_T(" ")) +
      Double2String((m_h - (y - up/1000.*m_fontSize)) * m_k,2) + wxString(_T(" ")) +
      Double2String(w * m_k,2) + wxString(_T(" ")) +
      Double2String(-ut/1000.*m_fontSizePt,2) + wxString(_T(" re f"));
  }
  return decoration;
}

void
wxPdfDocument::TextEscape(const wxString& s, bool newline)
{
#if wxUSE_UNICODE
  wxString t = m_currentFont->ConvertCID2GID(s);
  wxMBConv* conv = m_currentFont->GetEncodingConv();
  int len = conv->WC2MB(NULL, t, 0);
  char* mbstr = new char[len+3];
  len = conv->WC2MB(mbstr, t, len+3);
#else
  int len = s.Length();;
  char* mbstr = new char[len+1];
  strcpy(mbstr,s.c_str());
#endif

  OutEscape(mbstr,len);
  if (newline)
  {
    Out("\n",false);
  }
  delete [] mbstr;
}

int
wxPdfDocument::CalculateStreamLength(int length)
{
  int realLength = length;
  if (m_encrypted)
  {
    realLength = m_encryptor->CalculateStreamLength(length);
  }
  return realLength;
}

int
wxPdfDocument::CalculateStreamOffset()
{
  int offset = 0;
  if (m_encrypted)
  {
    offset = m_encryptor->CalculateStreamOffset();
  }
  return offset;
}

void
wxPdfDocument::PutStream(wxMemoryOutputStream& s)
{
  Out("stream");
  if (s.GetLength() != 0)
  {
    if (m_encrypted)
    {
      wxMemoryInputStream instream(s);
      int len = instream.GetSize();
      int lenbuf = CalculateStreamLength(len);
      int ofs = CalculateStreamOffset();
      char* buffer = new char[lenbuf];
      instream.Read(&buffer[ofs],len);
      m_encryptor->Encrypt(m_n, 0, (unsigned char*) buffer, len);
      Out(buffer, lenbuf);
      delete [] buffer;
    }
    else
    {
      wxMemoryInputStream tmp(s);
      if(m_state==2)
      {
        if (!m_inTemplate)
        {
          (*m_pages)[m_page]->Write(tmp);
          (*m_pages)[m_page]->Write("\n",1);
        }
        else
        { 
          m_currentTemplate->m_buffer.Write(tmp);
          m_currentTemplate->m_buffer.Write("\n",1);
        }
      }
      else
      {
        m_buffer.Write(tmp);
        m_buffer.Write("\n",1);
      }
    }
  }
  Out("endstream");
}

void
wxPdfDocument::OutEscape(const char* s, int len)
{
  int j;
  for (j = 0; j < len; j++)
  {
    switch (s[j])
    {
      case '\b':
        Out("\\b",false);
        break;
      case '\f':
        Out("\\f",false);
        break;
      case '\n':
        Out("\\n",false);
        break;
      case '\r':
        Out("\\r",false);
        break;
      case '\t':
        Out("\\t",false);
        break;
      case '\\':
      case '(':
      case ')':
        Out("\\",false);
      default:
        Out(&s[j],1,false);
        break;
    }
  }
}

void
wxPdfDocument::OutTextstring(const wxString& s, bool newline)
{
  // Format a text string
  int ofs = CalculateStreamOffset();
#if wxUSE_UNICODE
  wxMBConvUTF16BE conv;
  int len = conv.WC2MB(NULL, s, 0);
  int lenbuf = CalculateStreamLength(len+2);
  char* mbstr = new char[lenbuf+3];
  mbstr[ofs+0] = '\xfe';
  mbstr[ofs+1] = '\xff';
  len = 2 + conv.WC2MB(&mbstr[ofs+2], s, len+3);
#else
  int len = s.Length();;
  int lenbuf = CalculateStreamLength(len);
  char* mbstr = new char[lenbuf+1];
  strcpy(&mbstr[ofs], s.c_str());
#endif

  if (m_encrypted)
  {
    m_encryptor->Encrypt(m_n, 0, (unsigned char*) mbstr, len);
  }
  Out("(",false);
  OutEscape(mbstr,lenbuf);
  Out(")",newline);
  delete [] mbstr;
}

void
wxPdfDocument::OutRawTextstring(const wxString& s, bool newline)
{
  // Format a text string
  int ofs = CalculateStreamOffset();
  int len = s.Length();;
  int lenbuf = CalculateStreamLength(len);
  char* mbstr = new char[lenbuf+1];
#if wxUSE_UNICODE
  int j;
  for (j = 0; j < len; j++)
  {
    mbstr[ofs+j] = s[j];
  }
  mbstr[ofs+len] = 0;
#else
  strcpy(&mbstr[ofs],s.c_str());
#endif

  if (m_encrypted)
  {
    m_encryptor->Encrypt(m_n, 0, (unsigned char*) mbstr, len);
  }
  Out("(",false);
  OutEscape(mbstr,lenbuf);
  Out(")",newline);
  delete [] mbstr;
}

void
wxPdfDocument::OutHexTextstring(const wxString& s, bool newline)
{
  static char hexDigits[17] = "0123456789ABCDEF";
  // Format a text string
  int j;
  int ofs = CalculateStreamOffset();
  int len = s.Length();;
  int lenbuf = CalculateStreamLength(len);
  char* mbstr = new char[lenbuf+1];
#if wxUSE_UNICODE
  for (j = 0; j < len; j++)
  {
    mbstr[ofs+j] = s[j];
  }
  mbstr[ofs+len] = 0;
#else
  strcpy(&mbstr[ofs],s.c_str());
#endif

  if (m_encrypted)
  {
    m_encryptor->Encrypt(m_n, 0, (unsigned char*) mbstr, len);
  }

  char hexDigit;
  Out("<",false);
  for (j = 0; j < lenbuf; ++j)
  {
    hexDigit = hexDigits[(mbstr[j] >> 4) & 0x0f];
    Out(&hexDigit, 1, false);
    hexDigit = hexDigits[mbstr[j] & 0x0f];
    Out(&hexDigit, 1, false);
  }
  Out(">",newline);
  delete [] mbstr;
}

void
wxPdfDocument::OutAsciiTextstring(const wxString& s, bool newline)
{
  // Format an ASCII text string
  int ofs = CalculateStreamOffset();
  int len = s.Length();;
  int lenbuf = CalculateStreamLength(len);
  char* mbstr = new char[lenbuf+1];
  strcpy(&mbstr[ofs],s.ToAscii());

  if (m_encrypted)
  {
    m_encryptor->Encrypt(m_n, 0, (unsigned char*) mbstr, len);
  }
  Out("(",false);
  OutEscape(mbstr,lenbuf);
  Out(")",newline);
  delete [] mbstr;
}

void
wxPdfDocument::OutAscii(const wxString& s, bool newline)
{
  // Add a line of ASCII text to the document
  Out((const char*) s.ToAscii(),newline);
}

void
wxPdfDocument::Out(const char* s, bool newline)
{
  int len = strlen(s);
  Out(s,len,newline);
}

void
wxPdfDocument::Out(const char* s, int len, bool newline)
{
  if(m_state==2)
  {
    if (!m_inTemplate)
    {
      (*m_pages)[m_page]->Write(s,len);
      if (newline)
      {
        (*m_pages)[m_page]->Write("\n",1);
      }
    }
    else
    {
      m_currentTemplate->m_buffer.Write(s,len);
      if (newline)
      {
        m_currentTemplate->m_buffer.Write("\n",1);
      }
    }
  }
  else
  {
    m_buffer.Write(s,len);
    if (newline)
    {
      m_buffer.Write("\n",1);
    }
  }
}

void
wxPdfDocument::OutPoint(double x, double y)
{
  OutAscii(Double2String(x * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - y) * m_k,2) + wxString(_T(" m")));
  m_x = x;
  m_y = y;
}

void
wxPdfDocument::OutPointRelative(double dx, double dy)
{
  m_x += dx;
  m_y += dy;
  OutAscii(Double2String(m_x * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - m_y) * m_k,2) + wxString(_T(" m")));
}

void
wxPdfDocument::OutLine(double x, double y)
{
  // Draws a line from last draw point
  OutAscii(Double2String(x * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - y) * m_k,2) + wxString(_T(" l")));
  m_x = x;
  m_y = y;
}

void
wxPdfDocument::OutLineRelative(double dx, double dy)
{
  m_x += dx;
  m_y += dy;
  // Draws a line from last draw point
  OutAscii(Double2String(m_x * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - m_y) * m_k,2) + wxString(_T(" l")));
}

void
wxPdfDocument::OutCurve(double x1, double y1, double x2, double y2, double x3, double y3)
{
  // Draws a Bézier curve from last draw point
  OutAscii(Double2String(x1 * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - y1) * m_k,2) + wxString(_T(" ")) +
           Double2String(x2 * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - y2) * m_k,2) + wxString(_T(" ")) +
           Double2String(x3 * m_k,2) + wxString(_T(" ")) +
           Double2String((m_h - y3) * m_k,2) + wxString(_T(" c")));
  m_x = x3;
  m_y = y3;
}

void
wxPdfDocument::OutImage(wxPdfImage* currentImage,
                        double x, double y, double w, double h, const wxPdfLink& link)
{
  // Automatic width and height calculation if needed
  if (w == 0 && h == 0)
  {
    // Put image at 72 dpi, apply scale factor
    if (currentImage->IsFormObject())
    {
      w = currentImage->GetWidth() / (20 * m_imgscale * m_k);
      h = currentImage->GetHeight() / (20 * m_imgscale * m_k);
    }
    else
    {
      w = currentImage->GetWidth() / (m_imgscale * m_k);
      h = currentImage->GetHeight() / (m_imgscale * m_k);
    }
  }
  if (w == 0)
  {
    w = (h * currentImage->GetWidth()) / currentImage->GetHeight();
  }
  if (h == 0)
  {
    h = (w * currentImage->GetHeight()) / currentImage->GetWidth();
  }

  double sw, sh, sx, sy;
  if (currentImage->IsFormObject())
  {
    sw = w * m_k / currentImage->GetWidth();
    sh = -h * m_k / currentImage->GetHeight();
    sx = x * m_k - sw * currentImage->GetX();
    sy = ((m_h - y) * m_k) - sh * currentImage->GetY();
  }
  else
  {
    sw = w * m_k;
    sh = h * m_k;
    sx = x * m_k;
    sy = (m_h-(y+h))*m_k;
  }
  OutAscii(wxString(_T("q ")) +
           Double2String(sw,2) + wxString(_T(" 0 0 ")) +
           Double2String(sh,2) + wxString(_T(" ")) +
           Double2String(sx,2) + wxString(_T(" ")) +
           Double2String(sy,2) + 
           wxString::Format(_T(" cm /I%d Do Q"),currentImage->GetIndex()));

  if (link.IsValid())
  {
    Link(x,y,w,h,link);
  }

  // set right-bottom corner coordinates
  m_img_rb_x = x + w;
  m_img_rb_y = y + h;

  // 
  if (m_inTemplate)
  {
    (*(m_currentTemplate->m_images))[currentImage->GetName()] = currentImage;
  }
}

bool wxPdfDocument::ms_seeded = false;
int  wxPdfDocument::ms_s1     = 0;
int  wxPdfDocument::ms_s2     = 0;

#define MODMULT(a, b, c, m, s) q = s / a; s = b * (s - a * q) - c * q; if (s < 0) s += m

wxString
wxPdfDocument::GetUniqueId(const wxString& prefix)
{
  wxString uid = (prefix.Length() <= 114) ? prefix : prefix.Left(114);

  wxDateTime ts;
  ts.SetToCurrent();

  int q;
  int z;
  if (!ms_seeded)
  {
    ms_seeded = true;
    ms_s1 = ts.GetSecond() ^ (~ts.GetMillisecond());
    if (ms_s1 == 0) ms_s1 = 1;
    ms_s2 = wxGetProcessId();
  }
  MODMULT(53668, 40014, 12211, 2147483563L, ms_s1);
  MODMULT(52774, 40692,  3791, 2147483399L, ms_s2);

  z = ms_s1 - ms_s2;
  if (z < 1)
  {
    z += 2147483562;
  }

  uid += wxString::Format(_T("%08x%05x"), ts.GetSecond(), ts.GetMillisecond());
  uid += Double2String(z * 4.656613e-9,8);

  return uid;
}

void
wxPdfDocument::Transform(double tm[6])
{
  OutAscii(Double2String( tm[0],3) + wxString(_T(" ")) +
           Double2String( tm[1],3) + wxString(_T(" ")) +
           Double2String( tm[2],3) + wxString(_T(" ")) +
           Double2String( tm[3],3) + wxString(_T(" ")) +
           Double2String( tm[4],3) + wxString(_T(" ")) +
           Double2String( tm[5],3) + wxString(_T(" cm")));
}

void
wxPdfDocument::SetFillGradient(double x, double y, double w, double h, int gradient)
{
  if (gradient > 0 && (size_t) gradient <= (*m_gradients).size())
  {
    ClippingRect(x, y, w, h, false);
    //set up transformation matrix for gradient
    double tm[6];
    tm[0] = w * m_k;
    tm[1] = 0;
    tm[2] = 0;
    tm[3] = h * m_k;
    tm[4] = x * m_k;
    tm[5] = (m_h - (y+h)) * m_k;
    Transform(tm);
    // paint the gradient
    OutAscii(wxString::Format(_T("/Sh%d sh"), gradient));
    // restore previous Graphic State
    Out("Q");
  }
  else
  {
    wxLogError(_("wxPdfDocument::SetFillGradient: Gradient Id out of range."));
  }
}
