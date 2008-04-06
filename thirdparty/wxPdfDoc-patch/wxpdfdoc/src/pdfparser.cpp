///////////////////////////////////////////////////////////////////////////////
// Name:        pdfparser.cpp
// Purpose:     
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-10-15
// RCS-ID:      $$
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfparser.cpp Implementation of PDF parser

// For compilers that support precompilation, includes <wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// includes
#include <wx/uri.h>
#include <wx/url.h>

#include "wx/pdfdoc.h"
#include "wx/pdftemplate.h"
#include "wx/pdfobjects.h"
#include "wx/pdfparser.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxPdfXRef);

wxPdfXRefEntry::wxPdfXRefEntry()
{
  m_type = -1;
  m_ofs_idx = 0;
  m_gen_ref = 0;
}

wxPdfXRefEntry::~wxPdfXRefEntry()
{
}

void
wxPdfParser::ReserveXRef(size_t count)
{
  size_t currentCount = m_xref.GetCount();
  if (count > currentCount)
  {
    m_xref.Add(wxPdfXRefEntry(), count-currentCount);
  }
}

wxFileSystem* wxPdfParser::ms_fileSystem = NULL;

wxFileSystem*
wxPdfParser::GetFileSystem()
{
  if (ms_fileSystem == NULL)
  {
    static wxFileSystem fileSystem;
    //I dont know why this needs to be added manually, all the documents indicate that nothing needs to be done for local files.
    fileSystem.AddHandler(new wxLocalFSHandler());
    ms_fileSystem = &fileSystem;
  }
  return ms_fileSystem;
}

wxPdfParser::wxPdfParser(const wxString& filename, const wxString& password)
{
  m_objectQueue     = new wxPdfObjectQueue();
  m_objectQueueLast = m_objectQueue;
  m_objectMap       = new wxPdfObjectMap();
  m_objStmCache     = new wxPdfObjStmMap();
  m_tokens  = NULL;
  m_trailer = NULL;
  m_root    = NULL;
  m_useRawStream = false;
  m_cacheObjects = true;

  m_encrypted = false;
  m_decryptor = NULL;

  m_filename = filename;
  m_password = password;

  wxString fileURL = filename;
  wxURI uri(filename);
  if (!uri.HasScheme())
  {
    fileURL = wxFileSystem::FileNameToURL(filename);
  }
  m_pdfFile = wxPdfParser::GetFileSystem()->OpenFile(fileURL);
  if (m_pdfFile != NULL)
  {
    m_tokens = new wxPdfTokenizer(m_pdfFile->GetStream());
    m_initialized = ParseDocument();
  }
}

wxPdfParser::~wxPdfParser()
{
  wxPdfObjectQueue* entry = m_objectQueue;
  wxPdfObjectQueue* next; 
  while (entry != NULL)
  {
    wxPdfObject* object = entry->GetObject();
    if (object != NULL && object->IsIndirect())
    {
      delete object;
    }
    next = entry->GetNext(); 
    delete entry;
    entry = next;
  }
  delete m_objectMap;

  wxPdfObjStmMap::iterator objStm = m_objStmCache->begin();
  for (objStm = m_objStmCache->begin(); objStm != m_objStmCache->end(); objStm++)
  {
    if (objStm->second != NULL)
    {
      delete objStm->second;
    }
  }
  delete m_objStmCache;

  size_t j;
  for (j = 0; j < m_pages.GetCount(); j++)
  {
    wxPdfObject* obj = (wxPdfObject*) m_pages.Item(j);
    delete obj;
  }
  m_pages.Clear();

  if (m_trailer != NULL)
  {
    delete m_trailer;
  }
  if (m_root != NULL)
  {
    delete m_root;
  }

  delete m_tokens;
  if (m_pdfFile != NULL)
  {
    delete m_pdfFile;
  }

  if (m_decryptor != NULL)
  {
    delete m_decryptor;
  }
}

bool
wxPdfParser::IsOk()
{
  return (m_pdfFile != NULL && m_initialized);
}

void
wxPdfParser::AppendObject(int originalObjectId, int actualObjectId, wxPdfObject* obj)
{
  wxPdfObjectQueue* newEntry = new wxPdfObjectQueue(originalObjectId, actualObjectId, obj);
  m_objectQueueLast->SetNext(newEntry);
  m_objectQueueLast = newEntry;
  (*m_objectMap)[originalObjectId] = newEntry;
}

int
wxPdfParser::GetPageCount()
{
  return m_pages.GetCount();
}

bool
wxPdfParser::GetSourceInfo(wxPdfInfo& info)
{
  bool ok = false;
  wxPdfDictionary* infoDict = (wxPdfDictionary*) ResolveObject(m_trailer->Get(_T("/Info")));
  if (infoDict != NULL && infoDict->GetType() == OBJTYPE_DICTIONARY)
  {
    typedef void (wxPdfInfo::*InfoSetter) (const wxString& value);
    wxChar* entryList[] = { _T("/Title"),        _T("/Author"),   _T("/Subject"),
                            _T("/Keywords"),     _T("/Creator"), _T("/Producer"),
                            _T("/CreationDate"), _T("/ModDate"),
                            NULL }; //, "Trapped")
    InfoSetter entryFunc[] = { &wxPdfInfo::SetTitle,        &wxPdfInfo::SetAuthor,  &wxPdfInfo::SetSubject,
                               &wxPdfInfo::SetKeywords,     &wxPdfInfo::SetCreator, &wxPdfInfo::SetProducer,
                               &wxPdfInfo::SetCreationDate, &wxPdfInfo::SetModDate,
                               NULL };
    wxString value;
    size_t j;
    for (j = 0; entryList[j] != NULL; j++)
    {
      wxPdfString* entry = (wxPdfString*) infoDict->Get(entryList[j]);
      if (entry != NULL)
      {
        value = entry->GetValue();
#if wxUSE_UNICODE
        if ((value.Length() >= 2) && (value.GetChar(0) == 254) && (value.GetChar(1) == 255))
        {
          wxMBConvUTF16BE conv;
          size_t k;
          size_t len = value.Length()-2;
          char* mbstr = new char[len+2];
          for (k = 0; k < len; k++)
          {
            mbstr[k] = value.GetChar(k+2);
          }
          mbstr[len] = 0;
          mbstr[len+1] = 0;
          value = conv.cMB2WC(mbstr);
          delete [] mbstr;
        }
#endif
        (info.*entryFunc[j])(value);
      }
    }
    if (infoDict->IsIndirect())
    {
      delete infoDict;
    }
    ok = true;
  }
  return ok;
}
  
bool
wxPdfParser::ParseDocument()
{
  bool ok = false;
  m_fileSize = m_tokens->GetLength();
  m_pdfVersion = m_tokens->CheckPdfHeader();
  if (m_pdfVersion != wxEmptyString)
  {
    if (ParseXRef())
    {
      if (SetupDecryptor())
      {
        m_root = (wxPdfDictionary*) m_trailer->Get(_T("/Root"));
        m_root = (wxPdfDictionary*) ResolveObject(m_root);
        if (m_root != NULL)
        {
          wxPdfName* versionEntry = (wxPdfName*) ResolveObject(m_root->Get(_T("/Version")));
          if (versionEntry != NULL)
          {
            wxString version = versionEntry->GetName();
            version = version.Mid(1, 3);
            if (m_pdfVersion < version)
            {
              m_pdfVersion = version;
            }
            if (versionEntry->IsIndirect())
            {
              delete versionEntry;
            }
          }
          wxPdfDictionary* pages = (wxPdfDictionary*) ResolveObject(m_root->Get(_T("/Pages")));
          ok = ParsePageTree(pages);
          delete pages;
        }
      }
    }
  }
  return ok;
}

bool
wxPdfParser::SetupDecryptor()
{
  bool ok = true;
  wxPdfObject* encDic = m_trailer->Get(_T("/Encrypt"));
  if (encDic == NULL || encDic->GetType() == OBJTYPE_NULL)
  {
    return true;
  }
  wxPdfDictionary* enc = (wxPdfDictionary*) ResolveObject(encDic);
  wxPdfObject* obj;
  wxPdfArray* documentIDs = (wxPdfArray*) ResolveObject(m_trailer->Get(_T("/ID")));
  wxString documentID;
  if (documentIDs != NULL)
  {
    obj = (wxPdfObject*) documentIDs->Get(0);
    if (obj->GetType() == OBJTYPE_STRING)
    {
      documentID = ((wxPdfString*) obj)->GetValue();
    }
    if (documentIDs->IsIndirect())
    {
      delete documentIDs;
    }
  }

  wxString uValue = wxEmptyString;
  obj = enc->Get(_T("/U"));
  if (obj->GetType() == OBJTYPE_STRING)
  {
    uValue = ((wxPdfString*) obj)->GetValue();
    if (uValue.Length() != 32)
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Invalid length of U value."));
      ok = false;
    }
  }

  wxString oValue = wxEmptyString;
  obj = enc->Get(_T("/O"));
  if (obj->GetType() == OBJTYPE_STRING)
  {
    oValue = ((wxPdfString*) obj)->GetValue();
    if (oValue.Length() != 32)
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Invalid length of O value."));
      ok = false;
    }
  }

  int rValue = 0;
  obj = enc->Get(_T("/R"));
  if (obj->GetType() == OBJTYPE_NUMBER)
  {
    rValue = ((wxPdfNumber*) obj)->GetInt();
    if (rValue != 2 && rValue != 3)
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Unknown encryption type (%d)."), rValue);
      ok = false;
    }
  }
  else
  {
    wxLogError(_T("wxPdfParser::SetupDecryptor: Illegal R value."));
    ok = false;
  }

  int vValue = 0;
  obj = enc->Get(_T("/V"));
  if (obj != NULL && obj->GetType() == OBJTYPE_NUMBER)
  {
    vValue = ((wxPdfNumber*) obj)->GetInt();
    if (!((rValue == 2 && vValue == 1) || (rValue == 3 && vValue == 2)))
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Unsupported V value."));
      ok = false;
    }
  }
  else
  {
    wxLogError(_T("wxPdfParser::SetupDecryptor: Illegal V value."));
    ok = false;
  }

  int pValue = 0;
  obj = enc->Get(_T("/P"));
  if (obj->GetType() == OBJTYPE_NUMBER)
  {
    pValue = ((wxPdfNumber*) obj)->GetInt();
    // Check required permissions (Applications MUST respect the permission settings)
    if ((pValue & REQUIRED_PERMISSIONS) != REQUIRED_PERMISSIONS)
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Import of document not allowed due to missing permissions."));
      ok = false;
    }
  }
  else
  {
    wxLogError(_T("wxPdfParser::SetupDecryptor: Illegal P value."));
    ok = false;
  }

  int lengthValue = 40; // Default for revisison 2
  if (rValue == 3)
  {
    // Get the key length if revision is 3
    obj = enc->Get(_T("/Length"));
    if (obj->GetType() == OBJTYPE_NUMBER)
    {
      lengthValue = ((wxPdfNumber*) obj)->GetInt();
      if (lengthValue > 128 || lengthValue < 40 || lengthValue % 8 != 0)
      {
        wxLogError(_T("wxPdfParser::SetupDecryptor: Illegal Length value."));
        ok = false;
      }
    }
    else
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Illegal Length value."));
      ok = false;
    }
  }

  if (enc->IsIndirect())
  {
    delete enc;
  }

  if (ok)
  {
    m_encrypted = true;
    m_decryptor = new wxPdfEncrypt();
    if (!m_decryptor->Authenticate(documentID, m_password, uValue, oValue, pValue, lengthValue, rValue))
    {
      wxLogError(_T("wxPdfParser::SetupDecryptor: Bad password."));
      ok = false;
    }
  }

  return ok;
}

bool
wxPdfParser::ParsePageTree(wxPdfDictionary* pages)
{
  bool ok = false;
  // Get the kids dictionary
  wxPdfArray* kids = (wxPdfArray*) ResolveObject(pages->Get(_T("/Kids")));
  if (kids != NULL)
  {
    size_t nKids = kids->GetSize();
    size_t j;
    ok = true;
    for (j = 0; j < nKids; j++)
    {
      wxPdfDictionary* page = (wxPdfDictionary*) ResolveObject(kids->Get(j));
      wxPdfName* type = (wxPdfName*) page->Get(_T("/Type"));
      if (type->GetName() == _T("/Pages"))
      {
        // If one of the kids is an embedded
        // /Pages array, resolve it as well.
        ok = ok && ParsePageTree(page);
        delete page;
      }
      else
      {
        m_pages.Add(page);
      }
    }
    if (kids->IsIndirect())
    {
      delete kids;
    }
  }
  else
  {
    wxLogError(_("wxPdfParser::ParsePageTree: Cannot find /Kids in current /Page-Dictionary"));
  }
  return ok;
}

wxPdfObject*
wxPdfParser::GetPageResources(int pageno)
{
  wxPdfObject* resources = NULL;
  if (pageno >= 0 && pageno < GetPageCount())
  {
    resources = GetPageResources((wxPdfObject*) m_pages[pageno]);
  }
  return resources;
}

wxPdfObject*
wxPdfParser::GetPageResources(wxPdfObject* page)
{
  wxPdfObject* resources = NULL;
  wxPdfDictionary* dic = (wxPdfDictionary*) ResolveObject(page);

  // If the current object has a resources dictionary associated with it,
  // we use it. Otherwise, we move back to its parent object.
  wxPdfObject* resourceRef = ResolveObject(dic->Get(_T("/Resources")));
  if (resourceRef != NULL)
  {
    resources = ResolveObject(resourceRef);
  }
  else
  {
    wxPdfObject* parent = ResolveObject(dic->Get(_T("/Parent")));
    if (parent != NULL)
    {
      resources = GetPageResources(parent);
      delete parent;
    }
  }
  return resources;
}

void
wxPdfParser::GetContent(int pageno, wxArrayPtrVoid& contents)
{
  if (pageno >= 0 && pageno < GetPageCount())
  {
    wxPdfObject* content = ((wxPdfDictionary*) m_pages[pageno])->Get(_T("/Contents"));
    GetPageContent(content, contents);
  }
}
    
void
wxPdfParser::GetPageContent(wxPdfObject* contentRef, wxArrayPtrVoid& contents)
{
  int type = contentRef->GetType();
  if (type == OBJTYPE_INDIRECT)
  {
    wxPdfObject* content = ResolveObject(contentRef);
    if (content->GetType() == OBJTYPE_ARRAY)
    {
      GetPageContent(content, contents);
      delete content;
    }
    else
    {
      contents.Add(content);
    }
  }
  else if (type == OBJTYPE_ARRAY)
  {
    wxPdfArray* contentArray = (wxPdfArray*) contentRef;
    size_t n = contentArray->GetSize();
    size_t j;
    for (j = 0; j < n; j++)
    {
      GetPageContent(contentArray->Get(j), contents);
    }
  }
}

wxPdfArrayDouble*
wxPdfParser::GetPageMediaBox(int pageno)
{
  wxPdfArrayDouble* box = GetPageBox((wxPdfDictionary*) m_pages[pageno], _T("/MediaBox"));
  return box;
}

wxPdfArrayDouble*
wxPdfParser::GetPageCropBox(int pageno)
{
  wxPdfArrayDouble* box = GetPageBox((wxPdfDictionary*) m_pages[pageno], _T("/CropBox"));
  if (box == NULL)
  {
    box = GetPageBox((wxPdfDictionary*) m_pages[pageno], _T("/MediaBox"));
  }
  return box;
}

wxPdfArrayDouble*
wxPdfParser::GetPageBleedBox(int pageno)
{
  wxPdfArrayDouble* box = GetPageBox((wxPdfDictionary*) m_pages[pageno], _T("/BleedBox"));
  if (box == NULL)
  {
    box = GetPageCropBox(pageno);
  }
  return box;
}

wxPdfArrayDouble*
wxPdfParser::GetPageTrimBox(int pageno)
{
  wxPdfArrayDouble* box = GetPageBox((wxPdfDictionary*) m_pages[pageno], _T("/TrimBox"));
  if (box == NULL)
  {
    box = GetPageCropBox(pageno);
  }
  return box;
}

wxPdfArrayDouble*
wxPdfParser::GetPageArtBox(int pageno)
{
  wxPdfArrayDouble* box = GetPageBox((wxPdfDictionary*) m_pages[pageno], _T("/ArtBox"));
  if (box == NULL)
  {
    box = GetPageCropBox(pageno);
  }
  return box;
}

wxPdfArrayDouble*
wxPdfParser::GetPageBox(wxPdfDictionary* page, const wxString& boxIndex)
{
  wxPdfArrayDouble* pageBox = NULL;
  wxPdfArray* box = (wxPdfArray*) ResolveObject(page->Get(boxIndex));
  if (box == NULL)
  {
    wxPdfDictionary* parent = (wxPdfDictionary*) ResolveObject(page->Get(_T("/Parent")));
    if (parent != NULL)
    {
      pageBox = GetPageBox(parent, boxIndex);
      delete parent;
    }
  }
  else
  {
    pageBox = new wxPdfArrayDouble();
    size_t j;
    for (j = 0; j < box->GetSize(); j++)
    {
      wxPdfNumber* item = (wxPdfNumber*) box->Get(j);
      pageBox->Add(item->GetValue());
    }
  }
  return pageBox;
}

bool
wxPdfParser::ParseXRef()
{
  m_tokens->Seek(m_tokens->GetStartXRef());
  m_tokens->NextToken();
  if (m_tokens->GetStringValue() != _T("startxref"))
  {
    wxLogError(_("wxPdfParser::ParseXRef: 'startxref' not found."));
    return false;
  }
  m_tokens->NextToken();
  if (m_tokens->GetTokenType() != /*PRTokeniser.*/ TOKEN_NUMBER)
  {
    wxLogError(_("wxPdfParser::ParseXRef: 'startxref' is not followed by a number."));
    return false;
  }
  int startxref = m_tokens->GetIntValue();

  if (!ParseXRefStream(startxref, true))
  {
    m_xref.Clear();
    m_tokens->Seek(startxref);
    m_trailer = ParseXRefSection();
    wxPdfDictionary* trailer1 = m_trailer;
    wxPdfDictionary* trailer2 = NULL;
    while (trailer1 != NULL)
    {
      wxPdfNumber* prev = (wxPdfNumber*) trailer1->Get(_T("/Prev"));
      trailer2 = trailer1;
      if (prev != NULL)
      {
        m_tokens->Seek(prev->GetInt());
        trailer1 = ParseXRefSection();
      }
      else
      {
        trailer1 = NULL;
      }
      if (trailer2 != m_trailer)
      {
        delete trailer2;
      }
    }
  }
  return (m_trailer != NULL);
}

wxPdfDictionary*
wxPdfParser::ParseXRefSection()
{
  m_tokens->NextValidToken();
  if (m_tokens->GetStringValue() != _T("xref"))
  {
    wxLogError(_("wxPdfParser::ParseXRefSection: xref subsection not found."));
    return NULL;
  }
  int start = 0;
  int end = 0;
  int pos = 0;
  int gen = 0;
  while (true)
  {
    m_tokens->NextValidToken();
    if (m_tokens->GetStringValue() == _T("trailer"))
      break;
    if (m_tokens->GetTokenType() != TOKEN_NUMBER)
    {
      wxLogError(_("wxPdfParser::ParseXRefSection: Object number of the first object in this xref subsection not found."));
      return NULL;
    }
    start = m_tokens->GetIntValue();
    m_tokens->NextValidToken();
    if (m_tokens->GetTokenType() != TOKEN_NUMBER)
    {
      wxLogError(_("wxPdfParser::ParseXRefSection: Number of entries in this xref subsection not found."));
      return NULL;
    }
    end = m_tokens->GetIntValue() + start;
    if (start == 1)
    { // fix incorrect start number
      int back = m_tokens->Tell();
      m_tokens->NextValidToken();
      pos = m_tokens->GetIntValue();
      m_tokens->NextValidToken();
      gen = m_tokens->GetIntValue();
      if (pos == 0 && gen == 65535)
      {
        --start;
        --end;
      }
      m_tokens->Seek(back);
    }
    ReserveXRef(end);

    int k;
    for (k = start; k < end; ++k)
    {
      wxPdfXRefEntry& xrefEntry = m_xref[k];
      m_tokens->NextValidToken();
      pos = m_tokens->GetIntValue();
      m_tokens->NextValidToken();
      gen = m_tokens->GetIntValue();
      m_tokens->NextValidToken();
      if (m_tokens->GetStringValue() == _T("n"))
      {
        if (xrefEntry.m_ofs_idx == 0 && xrefEntry.m_gen_ref == 0)
        {
          // TODO: if (pos == 0)
          //   wxLogError(_T("File position 0 cross-reference entry in this xref subsection"));
          xrefEntry.m_ofs_idx = pos;
          xrefEntry.m_gen_ref = gen;
          xrefEntry.m_type = 1;
        }
      }
      else if (m_tokens->GetStringValue() == _T("f"))
      {
        if (xrefEntry.m_ofs_idx == 0 && xrefEntry.m_gen_ref == 0)
        {
          xrefEntry.m_ofs_idx = -1;
          xrefEntry.m_gen_ref = 0;
          xrefEntry.m_type = 0;
        }
      }
      else
      {
        wxLogError(_("wxPdfParser:ReadXRefSection: Invalid cross-reference entry in this xref subsection."));
        return NULL;
      }
    }
  }
  wxPdfDictionary* trailer = (wxPdfDictionary*) ParseObject();
  wxPdfNumber* xrefSize = (wxPdfNumber*) trailer->Get(_T("/Size"));
  ReserveXRef(xrefSize->GetInt());

  wxPdfObject* xrs = trailer->Get(_T("/XRefStm"));
  if (xrs != NULL && xrs->GetType() == OBJTYPE_NUMBER)
  {
    int loc = ((wxPdfNumber*) xrs)->GetInt();
    ParseXRefStream(loc, false);
  }
  return trailer;
}

bool
wxPdfParser::ParseXRefStream(int ptr, bool setTrailer)
{
  int idx, k;

  m_tokens->Seek(ptr);
  int streamRef = 0;
  if (!m_tokens->NextToken())
  {
    return false;
  }
  if (m_tokens->GetTokenType() != TOKEN_NUMBER)
  {
    return false;
  }
  streamRef = m_tokens->GetIntValue();
  if (!m_tokens->NextToken() || m_tokens->GetTokenType() != TOKEN_NUMBER)
  {
    return false;
  }
  if (!m_tokens->NextToken() || m_tokens->GetStringValue() != _T("obj"))
  {
    return false;
  }
  wxPdfObject* object = ParseObject();
  wxPdfStream* stm = NULL;
  if (object->GetType() == OBJTYPE_STREAM)
  {
    stm = (wxPdfStream*) object;
    if (((wxPdfName*) stm->Get(_T("/Type")))->GetName() != _T("/XRef"))
    {
      delete object;
      return false;
    }
  }
  int size = ((wxPdfNumber*) stm->Get(_T("/Size")))->GetInt();
  bool indexAllocated = false;
  wxPdfArray* index;
  wxPdfObject* obj = stm->Get(_T("/Index"));
  if (obj == NULL)
  {
    indexAllocated = true;
    index = new wxPdfArray();
    index->Add(0);
    index->Add(size);
  }
  else
  {
    index = (wxPdfArray*) obj;
  }
  wxPdfArray* w = (wxPdfArray*) stm->Get(_T("/W"));
  int prev = -1;
  obj = stm->Get(_T("/Prev"));
  if (obj != NULL)
  {
    prev = ((wxPdfNumber* )obj)->GetInt();
  }
  // Each xref pair is a position
  // type 0 -> -1, 0
  // type 1 -> offset, 0
  // type 2 -> index, obj num
  ReserveXRef(size);

  GetStreamBytes(stm);
  wxMemoryOutputStream* streamBuffer = stm->GetBuffer();
  wxMemoryInputStream streamBytes(*streamBuffer);
  size_t inLength = streamBytes.GetSize();
  char* buffer = new char[inLength];
  streamBytes.Read(buffer, inLength);

  int bptr = 0;
  int wc[3];
  for (k = 0; k < 3; ++k)
  {
    wc[k] = ((wxPdfNumber*) (w->Get(k)))->GetInt();
  }
  for (idx = 0; (size_t) idx < index->GetSize(); idx += 2)
  {
    int start = ((wxPdfNumber*) (index->Get(idx)))->GetInt();
    int length = ((wxPdfNumber*) (index->Get(idx + 1)))->GetInt();
    ReserveXRef(start+length);
    while (length-- > 0)
    {
      wxPdfXRefEntry& xrefEntry = m_xref[start];
      int type = 1;
      if (wc[0] > 0)
      {
        type = 0;
        for (k = 0; k < wc[0]; ++k)
        {
          type = (type << 8) + (buffer[bptr++] & 0xff);
        }
      }
      int field2 = 0;
      for (k = 0; k < wc[1]; ++k)
      {
        field2 = (field2 << 8) + (buffer[bptr++] & 0xff);
      }
      int field3 = 0;
      for (k = 0; k < wc[2]; ++k)
      {
        field3 = (field3 << 8) + (buffer[bptr++] & 0xff);
      }
      if (xrefEntry.m_ofs_idx == 0 && xrefEntry.m_gen_ref == 0)
      {
        switch (type)
        {
          case 0:
            xrefEntry.m_type = 0;
            xrefEntry.m_ofs_idx = -1;
            xrefEntry.m_gen_ref = 0;
            break;
          case 1:
            xrefEntry.m_type = 1;
            xrefEntry.m_ofs_idx = field2;
            xrefEntry.m_gen_ref = field3;
            break;
          case 2:
            xrefEntry.m_type = 2;
            xrefEntry.m_ofs_idx = field3;
            xrefEntry.m_gen_ref = field2;
            break;
        }
      }
      start++;
    }
  }
  delete [] buffer;
  if ((size_t) streamRef < m_xref.GetCount())
  {
    m_xref[streamRef].m_ofs_idx = -1;
  }
  if (indexAllocated)
  {
    delete index;
  }

  // Set the first xref stream dictionary as the trailer dictionary
  if (setTrailer && m_trailer == NULL)
  {

    m_trailer = stm->GetDictionary();
    stm->SetDictionary(NULL);
  }
  delete stm;

  if (prev == -1)
  {
    return true;
  }
  return ParseXRefStream(prev, false);
}

wxPdfDictionary*
wxPdfParser::ParseDictionary()
{
  wxPdfDictionary* dic = new wxPdfDictionary();
  while (true)
  {
    m_tokens->NextValidToken();
    if (m_tokens->GetTokenType() == TOKEN_END_DICTIONARY)
      break;
    if (m_tokens->GetTokenType() != TOKEN_NAME)
    {
      wxLogError(_("wxPdfParser::ParseDictionary: Dictionary key is not a name."));
      break;
    }
    wxPdfName* name = new wxPdfName(m_tokens->GetStringValue());
    wxPdfObject* obj = ParseObject();
    int type = obj->GetType();
    if (-type == TOKEN_END_DICTIONARY)
    {
      wxLogError(_("wxPdfParser::ParseDictionary: Unexpected '>>'."));
      delete obj;
      delete name;
      break;
    }
    if (-type == TOKEN_END_ARRAY)
    {
      wxLogError(_("wxPdfParser::ParseDictionary: Unexpected ']'."));
      delete obj;
      delete name;
      break;
    }
    dic->Put(name, obj);
    delete name;
  }
  return dic;
}

wxPdfArray*
wxPdfParser::ParseArray()
{
  wxPdfArray* array = new wxPdfArray();
  while (true)
  {
    wxPdfObject* obj = ParseObject();
    int type = obj->GetType();
    if (-type == TOKEN_END_ARRAY)
    {
      delete obj;
      break;
    }
    if (-type == TOKEN_END_DICTIONARY)
    {
      wxLogError(_("wxPdfParser::ParseArray: Unexpected '>>'."));
      delete obj;
      break;
    }
    array->Add(obj);
  }
  return array;
}

wxPdfObject*
wxPdfParser::ParseObject()
{
  wxPdfObject* obj;
  m_tokens->NextValidToken();
  int type = m_tokens->GetTokenType();
  switch (type)
  {
    case TOKEN_START_DICTIONARY:
      {
        wxPdfDictionary* dic = ParseDictionary();
        int pos = m_tokens->Tell();
        // be careful in the trailer. May not be a "next" token.
        if (m_tokens->NextToken() && m_tokens->GetStringValue() == _T("stream"))
        {
          int ch = m_tokens->ReadChar();
          if (ch != '\n')
            ch = m_tokens->ReadChar();
          if (ch != '\n')
            m_tokens->BackOnePosition(ch);
          wxPdfStream* stream = new wxPdfStream(m_tokens->Tell());
          stream->SetDictionary(dic);
          obj = stream;
        }
        else
        {
          m_tokens->Seek(pos);
          obj = dic;
        }
      }
      break;
    
    case TOKEN_START_ARRAY:
      {
        obj = ParseArray();
      }
      break;

    case TOKEN_NUMBER:
      {
        obj = new wxPdfNumber(m_tokens->GetStringValue());
      }
      break;

    case TOKEN_STRING:
      {
        wxString token = m_tokens->GetStringValue();
        // Decrypt if necessary
        if (m_encrypted)
        {
          m_decryptor->Encrypt(m_objNum, m_objGen, token);
        }

        wxPdfString* strObj = new wxPdfString(token);
        strObj->SetIsHexString(m_tokens->IsHexString());
        obj = strObj;
      }
      break;
    
    case TOKEN_NAME:
      {
        obj = new wxPdfName(m_tokens->GetStringValue());
      }
      break;
    
    case TOKEN_REFERENCE:
      {
        int num = m_tokens->GetReference();
        obj = new wxPdfIndirectReference(num, m_tokens->GetGeneration());
      }
      break;
    
    case TOKEN_BOOLEAN:
      {
        obj = new wxPdfBoolean((m_tokens->GetStringValue() == _T("true")));
      }
      break;

    case TOKEN_NULL:
      {
        obj = new wxPdfNull();
      }
      break;

    default:
      {
        wxString token = m_tokens->GetStringValue();
        obj = new wxPdfLiteral(-type, m_tokens->GetStringValue());
      }
      break;
  }
  return obj;
}

wxPdfObject*
wxPdfParser::ResolveObject(wxPdfObject* obj)
{
  if (obj != NULL && obj->GetType() == OBJTYPE_INDIRECT)
  {
    wxPdfIndirectReference* ref = (wxPdfIndirectReference*)obj;
    int idx = ref->GetNumber();
    obj = ParseSpecificObject(idx);
    obj->SetIndirect(true);
  }
  return obj;
}

wxPdfObject*
wxPdfParser::ParseSpecificObject(int idx)
{
  wxPdfObject* obj = NULL;
  if ((size_t)(idx) >= m_xref.GetCount())
  {
    return NULL;
  }
  obj = ParseDirectObject(idx);
  return obj;
}

wxPdfObject*
wxPdfParser::ParseDirectObject(int k)
{
  int objIndex = 0;
  int objStreamIndex = 0;
  bool isCached = false;
  wxPdfObject* obj = NULL;

  // Check for free object
  if (m_xref[k].m_type == 0)
  {
    return NULL;
  }
  int pos = m_xref[k].m_ofs_idx;
  if (m_xref[k].m_type == 2)
  {
    objIndex = m_xref[k].m_gen_ref;
    wxPdfObjStmMap::iterator objStm = m_objStmCache->find(objIndex);
    if (objStm != m_objStmCache->end())
    {
      obj = objStm->second;
      isCached = true;
    }
    else
    {
      objStreamIndex = m_xref[k].m_gen_ref;
      pos = m_xref[objStreamIndex].m_ofs_idx;
    }
  }
  if (!isCached)
  {
    m_tokens->Seek(pos);
    m_tokens->NextValidToken();
    if (m_tokens->GetTokenType() != TOKEN_NUMBER)
    {
      wxLogError(_T("wxPdfParser::ParseSingleObject: Invalid object number."));
      return NULL;
    }
    m_objNum = m_tokens->GetIntValue();
    m_tokens->NextValidToken();
    if (m_tokens->GetTokenType() != TOKEN_NUMBER)
    {
      wxLogError(_T("wxPdfParser::ParseSingleObject: Invalid generation number."));
      return NULL;
    }
    m_objGen = m_tokens->GetIntValue();
    m_tokens->NextValidToken();
    if (m_tokens->GetStringValue() != _T("obj"))
    {
      wxLogError(_T("wxPdfParser::ParseSingleObject: Token 'obj' expected."));
      return NULL;
    }
    obj = ParseObject();
  }

  // TODO: Check for valid 'endstream'

  if (m_xref[k].m_type == 2)
  {
    m_objNum = k;
    m_objGen = 0;
    wxPdfStream* objStream = (wxPdfStream*) obj;
    obj = ParseObjectStream((wxPdfStream*) obj, m_xref[k].m_ofs_idx);
    if (m_cacheObjects)
    {
      if (!isCached)
      {
        (*m_objStmCache)[objIndex] = objStream;
      }
    }
    else
    {
      delete objStream;
    }
  }

  if (obj != NULL)
  {
    obj->SetObjNum(m_objNum, m_objGen);
  }
  if (obj->GetType() == OBJTYPE_STREAM)
  {
    GetStreamBytes((wxPdfStream*) obj);
  }
  return obj;
}

wxPdfObject*
wxPdfParser::ParseObjectStream(wxPdfStream* objStm, int idx)
{
  wxPdfObject* obj = NULL;

  wxPdfNumber* firstNumber = (wxPdfNumber*) ResolveObject(objStm->Get(_T("/First")));
  int first = firstNumber->GetInt();
  if (objStm->GetBuffer() == NULL)
  {
    bool saveUseRawStream = m_useRawStream;
    m_useRawStream = false;
    GetStreamBytes(objStm);
    m_useRawStream = saveUseRawStream;
  }

  bool saveEncrypted = m_encrypted;
  m_encrypted = false;
  wxPdfTokenizer* saveTokens = m_tokens;
  wxMemoryInputStream objStream(*(objStm->GetBuffer()));
  m_tokens = new wxPdfTokenizer(&objStream);

  int address = 0;
  bool ok = true;
  if (!objStm->HasObjOffsets())
  {
    // Read object offsets
    wxArrayInt* objOffsets = objStm->GetObjOffsets();
    int objCount = idx + 1;
    if (m_cacheObjects)
    {
      wxPdfNumber* objCountNumber = (wxPdfNumber*) ResolveObject(objStm->Get(_T("/N")));
      objCount = objCountNumber->GetInt();
    }
    int offset;
    int k;
    for (k = 0; k < objCount; ++k)
    {
      ok = m_tokens->NextToken();
      if (!ok)
        break;
      if (m_tokens->GetTokenType() != TOKEN_NUMBER)
      {
        ok = false;
        break;
      }
      ok = m_tokens->NextToken();
      if (!ok)
        break;
      if (m_tokens->GetTokenType() != TOKEN_NUMBER)
      {
        ok = false;
        break;
      }
      offset = m_tokens->GetIntValue() + first;
      if (m_cacheObjects)
      {
        objOffsets->Add(offset);
      }
      if (k == idx)
      {
        address = offset;
      }
    }
    if (ok)
    {
      objStm->SetHasObjOffsets(m_cacheObjects);
    }
  }
  else
  {
    address = objStm->GetObjOffset(idx);
    ok = (address > 0);
  }
  if (ok)
  {
    m_tokens->Seek(address);
    obj = ParseObject();
  }
  else
  {
    wxLogError(_T("wxPdfParser::ParseOneObjStm: Error reading ObjStm."));
  }

  delete m_tokens;
  m_tokens = saveTokens;
  m_encrypted = saveEncrypted;

  return obj;
}

void
wxPdfParser::GetStreamBytes(wxPdfStream* stream)
{
  GetStreamBytesRaw(stream);

  // Do not decode the content of resource object streams
  if (m_useRawStream) return;

  // Check whether the stream buffer is empty
  wxMemoryOutputStream* osIn = stream->GetBuffer();
  if (osIn->GetLength() == 0) return;

  size_t j;
  wxArrayPtrVoid filters;
  wxPdfObject* filter = ResolveObject(stream->Get(_T("/Filter")));
  if (filter != NULL)
  {
    int type = filter->GetType();
    if (type == OBJTYPE_NAME)
    {
      filters.Add(filter);
    }
    else if (type == OBJTYPE_ARRAY)
    {
      wxPdfArray* filterArray = (wxPdfArray*) filter;
      size_t size = filterArray->GetSize();
      for (j = 0; j < size; j++)
      {
        filters.Add(filterArray->Get(j));
      }
    }

    // Read decode parameters if available
    wxArrayPtrVoid dp;
    wxPdfObject* dpo = ResolveObject(stream->Get(_T("/DecodeParms")));
    if (dpo == NULL || (dpo->GetType() != OBJTYPE_DICTIONARY && dpo->GetType() != OBJTYPE_ARRAY))
    {
      dpo = ResolveObject(stream->Get(_T("/DP")));
    }
    if (dpo != NULL)
    {
      if (dpo->GetType() == OBJTYPE_DICTIONARY)
      {
        dp.Add(dpo);
      }
      else if (dpo->GetType() == OBJTYPE_ARRAY)
      {
        wxPdfArray* dpArray = (wxPdfArray*) dpo;
        size_t size = dpArray->GetSize();
        for (j = 0; j < size; j++)
        {
          dp.Add(dpArray->Get(j));
        }
      }
    }

    wxPdfObject* dicParam = NULL;
    wxMemoryOutputStream* osOut = NULL;
    for (j = 0; j < filters.GetCount(); j++)
    {
      osIn = stream->GetBuffer();
      wxPdfName* name = (wxPdfName*) filters[j];
      if (name->GetName() == _T("/FlateDecode") || name->GetName() == _T("/Fl"))
      {
        osOut = FlateDecode(osIn);
        if (j < dp.GetCount())
        {
          wxMemoryOutputStream* osIn2 = osOut;
          dicParam = (wxPdfObject*) dp[j];
          osOut = DecodePredictor(osIn2, dicParam);
          if (osOut != osIn2)
          {
            delete osIn2;
          }
        }
      }
      else if(name->GetName() == _T("/ASCIIHexDecode") || name->GetName() == _T("/AHx"))
      {
        osOut = ASCIIHexDecode(osIn);
      }
      else if(name->GetName() == _T("/ASCII85Decode") || name->GetName() == _T("/A85"))
      {
        osOut = ASCII85Decode(osIn);
      }
      else if(name->GetName() == _T("/LZWDecode"))
      {
        osOut = LZWDecode(osIn);
        if (j < dp.GetCount())
        {
          wxMemoryOutputStream* osIn2 = osOut;
          dicParam = (wxPdfObject*) dp[j];
          osOut = DecodePredictor(osIn2, dicParam);
          if (osOut != osIn2)
          {
            delete osIn2;
          }
        }
      }
      else
      {
        wxLogError(wxString(_T("wxPdfParser::GetStreamBytes: Filter '")) +
                   name->GetName() + wxString(_T("' not supported")));
      }
      if (osOut != NULL)
      {
        stream->SetBuffer(osOut);
        if (osIn != osOut)
        {
          delete osIn;
        }
      }
    }
  }
}

void
wxPdfParser::GetStreamBytesRaw(wxPdfStream* stream)
{
  wxPdfNumber* streamLength = (wxPdfNumber*) ResolveObject(stream->Get(_T("/Length")));
  size_t size = streamLength->GetInt();
  m_tokens->Seek(stream->GetOffset());
  wxMemoryOutputStream* memoryBuffer = NULL;
  wxMemoryOutputStream* streamBuffer = m_tokens->ReadBuffer(size);

  if (m_encrypted && size > 0)
  {
    wxMemoryInputStream inData(*streamBuffer);
    delete streamBuffer;
    memoryBuffer = new wxMemoryOutputStream();
    unsigned char* buffer = new unsigned char[size];
    inData.Read(buffer, size);
    if (inData.LastRead() == size)
    {
      m_decryptor->Encrypt(m_objNum, m_objGen, buffer, size);
      memoryBuffer->Write(buffer, size);
    }
    delete [] buffer;
    memoryBuffer->Close();
  }
  else
  {
    memoryBuffer = streamBuffer;
  }

  stream->SetBuffer(memoryBuffer);
  if (streamLength->IsIndirect())
  {
    delete streamLength;
  }
}

// --- Tokenizer

wxPdfTokenizer::wxPdfTokenizer(wxInputStream* inputStream)
{
  m_inputStream = inputStream;
}

wxPdfTokenizer::~wxPdfTokenizer()
{
}

off_t
wxPdfTokenizer::Seek(off_t pos)
{
  return m_inputStream->SeekI(pos);
}

off_t
wxPdfTokenizer::Tell()
{
  return m_inputStream->TellI();
}

void
wxPdfTokenizer::BackOnePosition(int ch)
{
  if (ch != -1)
  {
    off_t pos = Tell();
    if (pos > 0) pos--;
    Seek(pos);
  }
}

off_t
wxPdfTokenizer::GetLength()
{
  return m_inputStream->GetLength();
}

int
wxPdfTokenizer::ReadChar()
{
  int readChar;
  char ch = m_inputStream->GetC();
  readChar = (m_inputStream->LastRead() > 0) ? (unsigned char) ch : -1;
  return readChar;
}

wxMemoryOutputStream*
wxPdfTokenizer::ReadBuffer(size_t size)
{
  wxMemoryOutputStream* memoryBuffer = new wxMemoryOutputStream();
  if (size > 0)
  {
    char* buffer = new char[size];
    m_inputStream->Read(buffer, size);
    if (m_inputStream->LastRead() == size)
    {
      memoryBuffer->Write(buffer, size);
    }
    delete [] buffer;
  }
  memoryBuffer->Close();
  return memoryBuffer;
}
    
off_t
wxPdfTokenizer::GetStartXRef()
{
  off_t size = GetLength();
  if (size > 1024) size = 1024;
  off_t pos = GetLength() - size;
  m_inputStream->SeekI(pos);
  wxString str = ReadString(1024);
  int idx = str.rfind(wxString(_T("startxref")));
  if (idx < 0)
  {
    wxLogError(_("wxPdfTokenizer::GetStartXRef: PDF startxref not found."));
  }
  return pos + idx;
}

wxString
wxPdfTokenizer::CheckPdfHeader()
{
  wxString version = wxEmptyString;
  m_inputStream->SeekI(0);
  wxString str = ReadString(1024);
  int idx = str.Find(_T("%PDF-1."));
  if (idx >= 0)
  {
    m_inputStream->SeekI(idx);
    version = str.Mid(idx + 5, 3);
  }
  else
  {
    m_inputStream->SeekI(0);
    wxLogError(_("wxPdfTokenizer::GetStartXref: PDF header signature not found."));
  }
  return version;
}

wxString
wxPdfTokenizer::ReadString(int size)
{
  wxString buf;
  int ch;
  while (size > 0)
  {
    size--;
    ch = ReadChar();
    if (ch == -1)
      break;
    buf += ch;
  }
  return buf;
}

bool
wxPdfTokenizer::NextToken()
{
  static wxString buffer;
  buffer = wxEmptyString;
  m_stringValue = wxEmptyString;
  int ch = 0;
  do
  {
    ch = ReadChar();
  }
  while (ch != -1 && IsWhitespace(ch));

  if (ch == -1)
    return false;

  switch (ch)
  {
    case '[':
      m_type = TOKEN_START_ARRAY;
      break;
    case ']':
      m_type = TOKEN_END_ARRAY;
      break;
    case '/':
    {
      m_type = TOKEN_NAME;
      buffer += ch;
      while (true)
      {
        ch = ReadChar();
        if (IsDelimiterOrWhitespace(ch))
          break;
        buffer += ch;
      }
      BackOnePosition(ch);
      break;
    }
    case '>':
      ch = ReadChar();
      if (ch != '>')
      {
        wxLogError(_("wxPdfTokenizer::NextToken: '>' not expected."));
        return false;
      }
      m_type = TOKEN_END_DICTIONARY;
      break;
    case '<':
    {
      int v1 = ReadChar();
      if (v1 == '<')
      {
        m_type = TOKEN_START_DICTIONARY;
        break;
      }
      m_type = TOKEN_STRING;
      m_hexString = true;
      int v2 = 0;
      while (true)
      {
        while (IsWhitespace(v1))
        {
          v1 = ReadChar();
        }
        if (v1 == '>')
          break;
        v1 = GetHex(v1);
        if (v1 < 0)
          break;
        v2 = ReadChar();
        while (IsWhitespace(v2))
        {
          v2 = ReadChar();
        }
        if (v2 == '>')
        {
          ch = v1 << 4;
          buffer += ch;
          break;
        }
        v2 = GetHex(v2);
        if (v2 < 0)
          break;
        ch = (v1 << 4) + v2;
        buffer += ch;
        v1 = ReadChar();
      }
      if (v1 < 0 || v2 < 0)
      {
        wxLogError(_("wxPdfTokenizer::NextToken: Error reading string."));
        return false;
      }
      break;
    }
    case '%':
      m_type = TOKEN_COMMENT;
      do
      {
        ch = ReadChar();
      }
      while (ch != -1 && ch != '\r' && ch != '\n');
      break;
    case '(':
    {
      m_type = TOKEN_STRING;
      m_hexString = false;
      int nesting = 0;
      while (true)
      {
        ch = ReadChar();
        if (ch == -1)
          break;
        if (ch == '(')
        {
          ++nesting;
        }
        else if (ch == ')')
        {
          --nesting;
        }
        else if (ch == '\\')
        {
          bool lineBreak = false;
          ch = ReadChar();
          switch (ch)
          {
            case 'n':
              ch = '\n';
              break;
            case 'r':
              ch = '\r';
              break;
            case 't':
              ch = '\t';
              break;
            case 'b':
              ch = '\b';
              break;
            case 'f':
              ch = '\f';
              break;
            case '(':
            case ')':
            case '\\':
              break;
            case '\r':
              lineBreak = true;
              ch = ReadChar();
              if (ch != '\n')
                BackOnePosition(ch);
              break;
            case '\n':
              lineBreak = true;
              break;
            default:
            {
              if (ch < '0' || ch > '7')
              {
                break;
              }
              int octal = ch - '0';
              ch = ReadChar();
              if (ch < '0' || ch > '7')
              {
                BackOnePosition(ch);
                ch = octal;
                break;
              }
              octal = (octal << 3) + ch - '0';
              ch = ReadChar();
              if (ch < '0' || ch > '7')
              {
                BackOnePosition(ch);
                ch = octal;
                break;
              }
              octal = (octal << 3) + ch - '0';
              ch = octal & 0xff;
              break;
            }
          }
          if (lineBreak)
            continue;
          if (ch < 0)
            break;
        }
        else if (ch == '\r')
        {
          ch = ReadChar();
          if (ch < 0)
            break;
          if (ch != '\n')
          {
            BackOnePosition(ch);
            ch = '\n';
          }
        }
        if (nesting == -1)
          break;
        buffer += ch;
      }
      if (ch == -1)
      {
        wxLogError(_("wxPdfTokenizer::NextToken: Error reading string."));
        return false;
      }
      break;
    }
    default:
    {
      if (ch == '-' || ch == '+' || ch == '.' || (ch >= '0' && ch <= '9'))
      {
        m_type = TOKEN_NUMBER;
        do
        {
          buffer += ch;
          ch = ReadChar();
        }
        while (ch != -1 && ((ch >= '0' && ch <= '9') || ch == '.'));
      }
      else
      {
        m_type = TOKEN_OTHER;
        do
        {
          buffer += ch;
          ch = ReadChar();
        }
        while (!IsDelimiterOrWhitespace(ch));
      }
      BackOnePosition(ch);
      break;
    }
  }
  if (buffer != wxEmptyString)
  {
    m_stringValue.Append(buffer);
    if (m_type == TOKEN_OTHER && (m_stringValue == _T("true") || m_stringValue == _T("false")))
    {
      m_type = TOKEN_BOOLEAN;
    }
  }
  return true;
}

void
wxPdfTokenizer::NextValidToken()
{
  int level = 0;
  wxString n1 = wxEmptyString;
  wxString n2 = wxEmptyString;
  int ptr = 0;
  while (NextToken())
  {
    if (m_type == TOKEN_COMMENT)
      continue;
    switch (level)
    {
      case 0:
      {
        if (m_type != TOKEN_NUMBER)
          return;
        ptr = Tell();
        n1 = m_stringValue;
        ++level;
        break;
      }
      case 1:
      {
        if (m_type != TOKEN_NUMBER) {
          Seek(ptr);
          m_type = TOKEN_NUMBER;
          m_stringValue = n1;
          return;
        }
        n2 = m_stringValue;
        ++level;
        break;
      }
      default:
      {
        if (m_type != TOKEN_OTHER || m_stringValue != _T("R"))
        {
          Seek(ptr);
          m_type = TOKEN_NUMBER;
          m_stringValue = n1;
          return;
        }
        m_type = TOKEN_REFERENCE;
        long value;
        n1.ToLong(&value);
        m_reference = value;
        n2.ToLong(&value);
        m_generation = value;
        return;
      }
    }
  }
  // throwError("Unexpected end of file");
}

int
wxPdfTokenizer::GetTokenType()
{
  return m_type;
}
    
wxString
wxPdfTokenizer::GetStringValue()
{
  return m_stringValue;
}
    
int
wxPdfTokenizer::GetIntValue()
{
  long value;
  m_stringValue.ToLong(&value);
  return value;
}

int
wxPdfTokenizer::GetReference()
{
  return m_reference;
}
    
int
wxPdfTokenizer::GetGeneration()
{
  return m_generation;
}
    
bool
wxPdfTokenizer::IsWhitespace(int ch)
{
  return (ch == 0 || ch == 9 || ch == 10 || ch == 12 || ch == 13 || ch == 32);
}

bool
wxPdfTokenizer::IsDelimiter(int ch)
{
  return (ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '[' || ch == ']' || ch == '/' || ch == '%');
}

bool
wxPdfTokenizer::IsDelimiterOrWhitespace(int ch)
{
  return IsWhitespace(ch) || IsDelimiter(ch) || (ch == -1);
}
    
int
wxPdfTokenizer::GetHex(int v)
{
  if (v >= '0' && v <= '9')
    return v - '0';
  if (v >= 'A' && v <= 'F')
    return v - 'A' + 10;
  if (v >= 'a' && v <= 'f')
    return v - 'a' + 10;
  return -1;
}
