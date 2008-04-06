///////////////////////////////////////////////////////////////////////////////
// Name:        pdfoc.cpp
// Purpose:     Implementation of pdfoc.h
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfoc.cpp Implementation of pdfoc.h

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/pdfoc.h"

// ----------------------------------------------------------------------------
// wxPdfOc: class representing the optional content in a document
// ----------------------------------------------------------------------------

wxPdfOc::wxPdfOc()
{
  m_nextOcgId = 0;
}

wxPdfOc::~wxPdfOc()
{
  //need to delete all the OCGs
  wxPdfOcgHashMap::iterator ocg = m_ocgs.begin();
  while(m_ocgs.size() != 0) {
    ocg = m_ocgs.begin();
    delete ocg->second;
    m_ocgs.erase(ocg);
  }
}

void wxPdfOc::AddOcg(wxPdfOcg *ocg)
{
  //set its index
  ocg->SetOcgIndex(m_nextOcgId); 
  m_nextOcgId++;
  ocg->SetObjectIndex(0); //not set yet

  //store
  m_ocgs[ocg->GetOcgIndex()] = ocg;
}


// ----------------------------------------------------------------------------
// wxPdfOcg: class representing an optional content group
// ----------------------------------------------------------------------------

wxPdfOcg::wxPdfOcg(const wxString &name) 
: m_name(name)
{
  m_intent = wxPDF_OCG_INTENT_VIEW;
  m_index = 0;
  m_objIndex = 0;
  m_defaultState = true;
}


wxPdfOcg::~wxPdfOcg()
{
}

wxString& wxPdfOcg::GetIntentString(void)
{
  m_intentStr = _T("[ ");
  if(m_intent & wxPDF_OCG_INTENT_VIEW) {
    m_intentStr += _T("/View ");
  }
  if(m_intent & wxPDF_OCG_INTENT_DESIGN) {
    m_intentStr += _T("/Design ");
  }
  m_intentStr += _T("]");

  return m_intentStr;
}


// ----------------------------------------------------------------------------
// wxPdfOcmd: class representing an optional membership dictionary
// ----------------------------------------------------------------------------

/*wxPdfOcmd::wxPdfOcmd()
{

}

wxPdfOcmd::~wxPdfOcmd()
{

}

void wxPdfOcmd::SetVisibilityPolicy(const VisibilityPolicy policy)
{
  m_policy = policy;

  switch(policy) {
  case AllOn:
    m_strPolicy = wxPdfName(_T("AllOn"));
    break;
  case AnyOn:
    m_strPolicy = wxPdfName(_T("AnyOn"));
    break;
  case AnyOff:
    m_strPolicy = wxPdfName(_T("AnyOff"));
    break;
  case AllOff:
    m_strPolicy = wxPdfName(_T("AllOff"));
    break;
  }
}
*/


