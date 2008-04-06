///////////////////////////////////////////////////////////////////////////////
// Name:        pdfimage.cpp
// Purpose:     Implementation of wxPdfImage classes
// Author:      Ulrich Telle
// Modified by:
// Created:     2005-08-11
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfimage.cpp Implementation of the wxPdfImage class

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/uri.h>
#include <wx/url.h>
#include <wx/gifdecod.h>
#include <wx/wfstream.h>
#include <wx/zstream.h>

#include "wx/pdfdoc.h"
#include "wx/pdfimage.h"

wxFileSystem* wxPdfImage::ms_fileSystem = NULL;

wxFileSystem*
wxPdfImage::GetFileSystem()
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

// ----------------------------------------------------------------------------
// wxPdfImage: class representing image objects
// ----------------------------------------------------------------------------

wxPdfImage::wxPdfImage(wxPdfDocument* document, int index, const wxString& filename, const wxString& type)
{
  m_document = document;
  m_index    = index;
  m_name     = filename;
  m_maskImage = 0;
  m_isFormObj = false;
  m_fromWxImage = false;
  m_validWxImage = false;

  m_width    = 0;
  m_height   = 0;
  m_cs       = _T("");
  m_bpc      = '\0';
  m_f        = _T("");
  m_parms    = _T("");

  m_palSize  = 0;
  m_pal      = NULL;
  m_trnsSize = 0;
  m_trns     = NULL;
  m_dataSize = 0;
  m_data     = NULL;

  wxString fileURL = m_name;
  wxURI uri(m_name);
  if (!uri.HasScheme())
  {
    fileURL = wxFileSystem::FileNameToURL(m_name);
  }
  m_imageFile = wxPdfImage::GetFileSystem()->OpenFile(fileURL);
  wxString mimeType = m_imageFile->GetMimeType();
  m_type = (mimeType != wxEmptyString) ? mimeType : type.Lower();
  m_imageStream = (m_imageFile != NULL) ? m_imageFile->GetStream() : NULL;
}

wxPdfImage::wxPdfImage(wxPdfDocument* document, int index, const wxString& name, const wxImage& image)
{
  m_document = document;
  m_index    = index;
  m_name     = name;
  m_maskImage = 0;
  m_isFormObj = false;
  m_fromWxImage = true;

  m_width    = 0;
  m_height   = 0;
  m_cs       = _T("");
  m_bpc      = '\0';
  m_f        = _T("");
  m_parms    = _T("");

  m_palSize  = 0;
  m_pal      = NULL;
  m_trnsSize = 0;
  m_trns     = NULL;
  m_dataSize = 0;
  m_data     = NULL;

  m_validWxImage = ConvertWxImage(image);

  m_imageFile = NULL;
  m_imageStream = NULL;
}

wxPdfImage::wxPdfImage(wxPdfDocument* document, int index, const wxString& name, wxInputStream& stream, const wxString& mimeType)
{
  m_document = document;
  m_index    = index;
  m_name     = name;
  m_maskImage = 0;
  m_isFormObj = false;
  m_fromWxImage = false;
  m_validWxImage = false;

  m_width    = 0;
  m_height   = 0;
  m_cs       = _T("");
  m_bpc      = '\0';
  m_f        = _T("");
  m_parms    = _T("");

  m_palSize  = 0;
  m_pal      = NULL;
  m_trnsSize = 0;
  m_trns     = NULL;
  m_dataSize = 0;
  m_data     = NULL;

  m_imageFile = NULL;
  m_type = mimeType;
  m_imageStream = &stream;
}

wxPdfImage::~wxPdfImage()
{
  if (m_pal  != NULL) delete [] m_pal;
  if (m_trns != NULL) delete [] m_trns;
  if (m_data != NULL) delete [] m_data;
}

bool
wxPdfImage::ConvertWxImage(const wxImage& image)
{
  bool isValid = false;
  if (wxImage::FindHandler(wxBITMAP_TYPE_PNG) == NULL)
  {
    wxImage::AddHandler(new wxPNGHandler());
  }
  wxMemoryOutputStream os;
  isValid = image.SaveFile(os, wxBITMAP_TYPE_PNG);
  if (isValid)
  {
    wxMemoryInputStream is(os);
    m_type = _T("png");
    isValid = ParsePNG(&is);
  }
  return isValid;
}

bool
wxPdfImage::Parse()
{
  // Check whether this image originated from an wxImage and is valid
  if (m_fromWxImage) return m_validWxImage;

  bool isValid = false;

  if (m_imageStream)
  {
    if (m_type == _T("image/png") || m_type == _T("png"))
    {
      isValid = ParsePNG(m_imageStream);
    }
    else if (m_type == _T("image/jpeg") || m_type == _T("jpeg") || m_type == _T("jpg"))
    {
      isValid = ParseJPG(m_imageStream);
    }
    else if (m_type == _T("image/gif") || m_type == _T("gif"))
    {
      isValid = ParseGIF(m_imageStream);
    }
    else
    {
      if (m_type == _T("image/wmf") || m_type == _T("wmf") || m_name.Right(2) == _T(".wmf"))
      {
        m_isFormObj = true;
        isValid = ParseWMF(m_imageStream);
      }
    }
    if (m_imageFile != NULL)
    {
      delete m_imageFile;
      m_imageFile = NULL;
    }
  }
  return isValid;
}

// --- Parse PNG image file ---

bool
wxPdfImage::ParsePNG(wxInputStream* imageStream)
{
  bool isValid = false;

  // Check signature
  char buffer[8];
  imageStream->Read(buffer,8);
  if (strncmp(buffer,"\x89PNG\x0D\x0A\x1A\x0A",8) != 0)
  {
    // Not a PNG file
    wxLogDebug(_T("wxPdfImage::ParsePNG: '%s' not a PNG file."), m_name.c_str());
    return false;
  }

  // Read header chunk
  imageStream->Read(buffer,4);
  imageStream->Read(buffer,4);
  if (strncmp(buffer,"IHDR",4) != 0)
  {
    // Incorrect PNG file
    wxLogDebug(_T("wxPdfImage::ParsePNG: Incorrect PNG file '%s'."), m_name.c_str());
    return false;
  }

  int w = ReadIntBE(imageStream);
  int h = ReadIntBE(imageStream);

  imageStream->Read(buffer,1);
  char bpc = buffer[0];
  if (bpc > 8)
  {
    // 16-bit depth not supported
    wxLogDebug(_T("wxPdfImage::ParsePNG: 16-bit depth not supported: '%s'."), m_name.c_str());
    return false;
  }

  wxString colspace = wxEmptyString;
  imageStream->Read(buffer,1);
  char ct = buffer[0];
  if (ct == 0)
  {
    colspace = _T("DeviceGray");
  }
  else if (ct == 2)
  {
    colspace = _T("DeviceRGB");
  }
  else if (ct == 3)
  {
    colspace = _T("Indexed");
  }
  else
  {
    // Alpha channel not supported
    wxLogDebug(_T("wxPdfImage::ParsePNG: Alpha channel not supported: '%s'."), m_name.c_str());
    return false;
  }
  
  imageStream->Read(buffer,3);
  if (buffer[0] != 0)
  {
    // Unknown compression method
    wxLogDebug(_T("wxPdfImage::ParsePNG: Unknown compression method: '%s'."), m_name.c_str());
    return false;
  }
  if (buffer[1] != 0)
  {
    // Unknown filter method
    wxLogDebug(_T("wxPdfImage::ParsePNG: Unknown filter method: '%s'."), m_name.c_str());
    return false;
  }
  if (buffer[2] != 0)
  {
    // Interlacing not supported
    wxLogDebug(_T("wxPdfImage::ParsePNG: Interlacing not supported: '%s'."), m_name.c_str());
    return false;
  }

  imageStream->Read(buffer,4);
  m_parms = wxString::Format(_T("/DecodeParms <</Predictor 15 /Colors %d /BitsPerComponent %d /Columns %d>>"), (ct==2 ? 3 : 1), bpc, w);

  // Scan chunks looking for palette, transparency and image data
  m_palSize  = 0;
  m_pal      = NULL;
  m_trnsSize = 0;
  m_trns     = NULL;
  m_dataSize = 0;
  m_data     = NULL;
  int n;
  do
  {
    n = ReadIntBE(imageStream);
    imageStream->Read(buffer,4);
    if (strncmp(buffer,"PLTE",4) == 0)
    {
      // Read palette
      m_palSize = n;
      m_pal = new char[n];
      imageStream->Read(m_pal,n);
      imageStream->Read(buffer,4);
    }
    else if (strncmp(buffer,"tRNS",4) == 0)
    {
      // Read transparency info
      char* t = new char[n];
      imageStream->Read(t,n);
      if (ct == 0)
      {
        m_trnsSize = 1;
        m_trns = new char[1];
        m_trns[0] = t[1];
      }
      else if (ct == 2)
      {
        m_trnsSize = 3;
        m_trns = new char[3];
        m_trns[0] = t[1];
        m_trns[1] = t[3];
        m_trns[2] = t[5];
      }
      else
      {
        int pos;
        for (pos = 0; (pos < n) && (t[pos] != 0); pos++);
        if (pos < n)
        {
          m_trnsSize = 1;
          m_trns = new char[1];
          m_trns[0] = pos;
        }
      }
      imageStream->Read(buffer,4);
      delete [] t;
    }
    else if (strncmp(buffer,"IDAT",4) == 0)
    {
      // Read image data block
      int prevSize = m_dataSize;
      char* prevData = m_data;
      m_dataSize += n;
      m_data = new char[m_dataSize];
      if (prevSize > 0)
      {
        memcpy(m_data, prevData, prevSize);
        delete [] prevData;
      }
      imageStream->Read(m_data+prevSize,n);
      imageStream->Read(buffer,4);
    }
    else if (strncmp(buffer,"IEND",4) == 0)
    {
      break;
    }
    else
    {
      char* temp = new char[n];
      imageStream->Read(temp,n);
      delete [] temp;
      imageStream->Read(buffer,4);
    }
  }
  while (n);

  if (colspace == _T("Indexed") && m_pal == NULL)
  {
    if (m_pal  != NULL) delete [] m_pal;
    if (m_trns != NULL) delete [] m_trns;
    if (m_data != NULL) delete [] m_data;
    // Missing palette
    wxLogDebug(_T("wxPdfImage::ParsePNG: Missing palette: '%s'."), m_name.c_str());
    return false;
  }

  m_width  = w;
  m_height = h;
  m_cs     = colspace;
  m_bpc    = bpc;
  m_f      = _T("FlateDecode");

  isValid = true;
  return isValid;
}

//--- Parse JPEG image file

// some defines for the different JPEG block types

#define M_SOF0  0xC0     // Start Of Frame N
#define M_SOF1  0xC1     // N indicates which compression process
#define M_SOF2  0xC2     // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5     // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8
#define M_EOI   0xD9     // End Of Image (end of datastream)
#define M_SOS   0xDA     // Start Of Scan (begins compressed data)
#define M_COM   0xFE     // COMment

#define M_PSEUDO 0xFFD8  // pseudo marker for start of image(byte 0)

bool
wxPdfImage::ParseJPG(wxInputStream* imageStream)
{
  bool isValid = false;
  wxString colspace = _T("");

  m_palSize  = 0;
  m_pal      = NULL;
  m_trnsSize = 0;
  m_trns     = NULL;
  m_dataSize = 0;
  m_data     = NULL;

  unsigned char buffer[3];
  imageStream->Read(buffer,3);
  if (strncmp((const char*) buffer,"\xff\xd8\xff",3) != 0)
  {
    // Not a JPEG file
    wxLogDebug(_T("wxPdfImage::ParseJPG: '%s' not a JPEG file."), m_name.c_str());
    return false;
  }

  // Extract info from a JPEG file
  unsigned int   marker = M_PSEUDO;
  unsigned short length, ffRead = 1;
  unsigned char  bits     = 0;
  unsigned short height   = 0;
  unsigned short width    = 0;
  unsigned char  channels = 0;


  bool ready = false;
  int lastMarker;
  int commentCorrection;
  int a;
  while (!ready)
  {
    lastMarker = marker;
    commentCorrection = 1;
    a = 0;

    // get marker byte, swallowing possible padding
    if (lastMarker == M_COM && commentCorrection)
    {
      // some software does not count the length bytes of COM section
      // one company doing so is very much envolved in JPEG... so we accept too
      // by the way: some of those companies changed their code now...
      commentCorrection = 2;
    }
    else
    {
      lastMarker = 0;
      commentCorrection = 0;
    }
    if (ffRead)
    {
      a = 1; // already read 0xff in filetype detection
    }
    do
    {
      imageStream->Read(buffer,1);
      if (imageStream->Eof())
      {
        marker = M_EOI; // we hit EOF
        break;
      }
      marker = buffer[0];
      if (lastMarker == M_COM && commentCorrection > 0)
      {
        if (marker != 0xFF)
        {
          marker = 0xff;
          commentCorrection--;
        }
        else
        {
          lastMarker = M_PSEUDO; // stop skipping non 0xff for M_COM
        }
      }
      if (++a > 10)
      {
        // who knows the maxim amount of 0xff? though 7
        // but found other implementations
        marker = M_EOI;
        break;
      }
    } 
    while (marker == 0xff);

    if (a < 2)
    {
      marker = M_EOI; // at least one 0xff is needed before marker code
    }
    if (lastMarker == M_COM && commentCorrection)
    {
      marker = M_EOI; // ah illegal: char after COM section not 0xFF
    }

    ffRead = 0;
    switch (marker)
    {
      case M_SOF0:
      case M_SOF1:
      case M_SOF2:
      case M_SOF3:
      case M_SOF5:
      case M_SOF6:
      case M_SOF7:
      case M_SOF9:
      case M_SOF10:
      case M_SOF11:
      case M_SOF13:
      case M_SOF14:
      case M_SOF15:
        // handle SOFn block
        length = ReadUShortBE(imageStream);
        imageStream->Read(&bits,1);
        height = ReadUShortBE(imageStream);
        width  = ReadUShortBE(imageStream);
        imageStream->Read(&channels,1);
        isValid = true;
        ready = true;
        break;

      case M_SOS:
      case M_EOI:
        isValid = false;
        ready = true;

      default:
        {
          // anything else isn't interesting
          off_t pos = (unsigned int) ReadUShortBE(imageStream);
          pos = pos-2;
          if (pos)
          {
            imageStream->SeekI(pos, wxFromCurrent);
          }
        }
        break;
    }
  }

  if (isValid)
  {
    if (channels == 3)
    {
      colspace = _T("DeviceRGB");
    }
    else if(channels == 4)
    {
      colspace = _T("DeviceCMYK");
    }
    else
    {
      colspace = _T("DeviceGray");
    }
    m_bpc = bits;

    //Read whole file
    imageStream->SeekI(0);
    m_dataSize = imageStream->GetLength();
    m_data = new char[m_dataSize];
    imageStream->Read(m_data,m_dataSize);

    m_width  = width;
    m_height = height;
    m_cs     = colspace;
    m_bpc    = bits;
    m_f      = _T("DCTDecode");
  }

  return isValid;
}

// --- Parse GIF image file ---

bool
wxPdfImage::ParseGIF(wxInputStream* imageStream)
{
  bool isValid = false;

  m_palSize  = 0;
  m_pal      = NULL;
  m_trnsSize = 0;
  m_trns     = NULL;
  m_dataSize = 0;
  m_data     = NULL;

#if wxCHECK_VERSION(2,7,1)
  wxGIFDecoder gif;
  if (!gif.CanRead(*imageStream))
  {
    wxLogDebug(_T("wxPdfImage::ParseGIF: '%s' not a GIF file."), m_name.c_str());
    return false;
  }

  if (gif.LoadGIF(*imageStream) != wxGIF_OK)
  {
    wxLogDebug(_T("wxPdfImage::ParseGIF: Invalid GIF file '%s'."), m_name.c_str());
    return false;
  }

  isValid = true;
  wxSize gifSize = gif.GetFrameSize(0);
  m_width = gifSize.GetWidth();
  m_height = gifSize.GetHeight();
  m_cs = _T("Indexed");
  m_bpc    = 8;

  m_palSize  = 768;
  m_pal = new char[m_palSize];
  memcpy(m_pal,gif.GetPalette(0),m_palSize);

  int trns = gif.GetTransparentColourIndex(0);
  if (trns != -1)
  {
    m_trnsSize = 3;
    m_trns     = new char[3];
    m_trns[0] = m_pal[3*trns + 0];
    m_trns[1] = m_pal[3*trns + 1];
    m_trns[2] = m_pal[3*trns + 2];
  }
  
  m_dataSize = m_width * m_height;
  if (m_document->m_compress)
  {
    m_f = _T("FlateDecode");
    wxMemoryOutputStream* p = new wxMemoryOutputStream();
    wxZlibOutputStream q(*p);
    q.Write(gif.GetData(0),m_dataSize);
    q.Close();
    m_dataSize = p->TellO();
    m_data = new char[m_dataSize];
    p->CopyTo(m_data,m_dataSize);
    delete p;
  }
  else
  {
    m_f = _T("");
    m_data = new char[m_dataSize];
    memcpy(m_data,gif.GetData(0),m_dataSize);
  }
#else
  wxGIFDecoder gif(imageStream);
  if (!gif.CanRead())
  {
    wxLogDebug(_T("wxPdfImage::ParseGIF: '%s' not a GIF file."), m_name.c_str());
    return false;
  }

  if (gif.ReadGIF() != wxGIF_OK)
  {
    wxLogDebug(_T("wxPdfImage::ParseGIF: Invalid GIF file '%s'."), m_name.c_str());
    return false;
  }

  isValid = true;
  m_width = gif.GetWidth();
  m_height = gif.GetHeight();
  m_cs = _T("Indexed");
  m_bpc    = 8;

  m_palSize  = 768;
  m_pal = new char[m_palSize];
  memcpy(m_pal,gif.GetPalette(),m_palSize);

  int trns = gif.GetTransparentColour();
  if (trns != -1)
  {
    m_trnsSize = 3;
    m_trns     = new char[3];
    m_trns[0] = m_pal[3*trns + 0];
    m_trns[1] = m_pal[3*trns + 1];
    m_trns[2] = m_pal[3*trns + 2];
  }
  
  m_dataSize = m_width * m_height;
  if (m_document->m_compress)
  {
    m_f = _T("FlateDecode");
    wxMemoryOutputStream* p = new wxMemoryOutputStream();
    wxZlibOutputStream q(*p);
    q.Write(gif.GetData(),m_dataSize);
    q.Close();
    m_dataSize = p->TellO();
    m_data = new char[m_dataSize];
    p->CopyTo(m_data,m_dataSize);
    delete p;
  }
  else
  {
    m_f = _T("");
    m_data = new char[m_dataSize];
    memcpy(m_data,gif.GetData(),m_dataSize);
  }
#endif
  return isValid;
}

// --- Parse WMF image file ---

/// Class representing GDI objects while parsing WMF files. (For internal use only)
class GdiObject
{
public:
  char           type;
  short          style;
  unsigned char  r;
  unsigned char  g;
  unsigned char  b;
  unsigned char  a;
  unsigned short hatch;
  double         width;
};

static void
AddGdiObject(wxArrayPtrVoid& gdiObjects, void* obj)
{
  // find next available slot
  size_t idx;
  size_t n = gdiObjects.GetCount();
  for (idx = 0; idx < n; idx++)
  {
    if (gdiObjects[idx] == NULL) break;
  }
  if (idx < n)
  {
    gdiObjects[idx] = obj;
  }
  else
  {
    gdiObjects.Add(obj);
  }
}

bool
wxPdfImage::ParseWMF(wxInputStream* imageStream)
{
  bool isValid = false;
  char buffer[64];

  wxArrayPtrVoid gdiObjects;

  // check for Aldus placeable metafile header
  unsigned int key = ReadIntLE(imageStream);
  int headSize = 18 - 4; // WMF header minus four bytes already read
  if (key == 0x9AC6CDD7)
  {
    headSize += 22; // Aldus header
  }

  // strip headers
  imageStream->Read(buffer, headSize);

  // define some state variables
  short polyFillMode = 0;
  bool nullPen = false;
  bool nullBrush = false;
  bool endRecord = false;

  wxString data = wxEmptyString;
  wxString op;
  // read the records
  unsigned int size;
  unsigned short func;
  unsigned short idx;
  short wo[2];
  short we[2];
  short dashArray[8];
  size_t lenDashArray;
  size_t i;
  short j, k, px, py;
  GdiObject* obj = NULL;
  while (!imageStream->Eof() && !endRecord)
  {
    // size of record given in WORDs (= 2 bytes)
    size = ReadUIntLE(imageStream);
    // func is number of GDI function
    func = ReadUShortLE(imageStream);

    // parameters are read and processed
    // as necessary by the case statement below.
    // NB. parameters to GDI functions are stored in reverse order
    // however structures are not reversed,
    // e.g. POINT { int x, int y } where x=3000 (0x0BB8) and y=-1200 (0xFB50)
    // is stored as B8 0B 50 FB

    // process each record.
    // function numbers are defined in wingdi.h
    switch (func)
    {
      case 0x020b:  // SetWindowOrg
        // do not allow window origin to be changed
        // after drawing has begun
        if (data.Length() == 0)
        {
          wo[1] = ReadShortLE(imageStream);
          wo[0] = ReadShortLE(imageStream);
        }
        break;

      case 0x020c:  // SetWindowExt
        // do not allow window extent to be changed
        // after drawing has begun
        if (data.Length() == 0)
        {
          we[1] = ReadShortLE(imageStream);
          we[0] = ReadShortLE(imageStream);
        }
        break;

      case 0x02fc:  // CreateBrushIndirect
        {
          GdiObject* brush = new GdiObject();
          brush->style = ReadShortLE(imageStream);
          imageStream->Read(&brush->r, 1);
          imageStream->Read(&brush->g, 1);
          imageStream->Read(&brush->b, 1);
          imageStream->Read(&brush->a, 1);
          brush->hatch = ReadUShortLE(imageStream);
          brush->type = 'B';
          AddGdiObject(gdiObjects, brush);
        }
        break;

      case 0x02fa:  // CreatePenIndirect
        {
          GdiObject* pen = new GdiObject();
          pen->style = ReadShortLE(imageStream);
          short width = ReadShortLE(imageStream);
          /* short dummy = */ ReadShortLE(imageStream);
          imageStream->Read(&pen->r, 1);
          imageStream->Read(&pen->g, 1);
          imageStream->Read(&pen->b, 1);
          imageStream->Read(&pen->a, 1);

          // convert width from twips to user unit
          pen->width = width / (20 * m_document->m_k);
          pen->type = 'P';
          AddGdiObject(gdiObjects, pen);
        }
        break;

      // MUST create other GDI objects even if we don't handle them
      // otherwise object numbering will get out of sequence
      case 0x06fe: // CreateBitmap
      case 0x02fd: // CreateBitmapIndirect
      case 0x00f8: // CreateBrush
      case 0x02fb: // CreateFontIndirect
      case 0x00f7: // CreatePalette
      case 0x01f9: // CreatePatternBrush
      case 0x06ff: // CreateRegion
      case 0x0142: // DibCreatePatternBrush
        {
          GdiObject* dummy = new GdiObject();
          dummy->type = 'D';
          AddGdiObject(gdiObjects, dummy);
        }
        break;

      case 0x0106:  // SetPolyFillMode
        polyFillMode = ReadShortLE(imageStream);
        break;

      case 0x01f0:  // DeleteObject
        {
          idx = ReadUShortLE(imageStream);
          delete ((GdiObject*) gdiObjects[idx]);
          gdiObjects[idx] = NULL;
        }
        break;

      case 0x012d:  // SelectObject
        {
          idx = ReadUShortLE(imageStream);
          obj = (GdiObject*) gdiObjects[idx];

          switch (obj->type)
          {
            case 'B':
              nullBrush = false;

              if (obj->style == 1) // BS_NULL, BS_HOLLOW
              {
                nullBrush = true;
              }
              else
              {
                data += wxPdfDocument::Double2String(obj->r/255.,3) + wxString(_T(" "));
                data += wxPdfDocument::Double2String(obj->g/255.,3) + wxString(_T(" "));
                data += wxPdfDocument::Double2String(obj->b/255.,3) + wxString(_T(" rg\n"));
              }
              break;

            case 'P':
              nullPen = false;
              lenDashArray = 0;

              // dash parameters are my own - feel free to change them
              switch (obj->style)
              {
                case 0: // PS_SOLID
                  break;
                case 1: // PS_DASH
                  dashArray[0] = 3;
                  dashArray[1] = 1;
                  lenDashArray = 2;
                  break;
                case 2: // PS_DOT
                  dashArray[0] = 0;
                  dashArray[1] = 5;
                  dashArray[2] = 0;
                  dashArray[3] = 5;
                  lenDashArray = 4;
                  break;
                case 3: // PS_DASHDOT
                  dashArray[0] = 2;
                  dashArray[1] = 1;
                  dashArray[2] = 0;
                  dashArray[3] = 5;
                  dashArray[4] = 1;
                  lenDashArray = 5;
                  break;
                case 4: // PS_DASHDOTDOT
                  dashArray[0] = 2;
                  dashArray[1] = 1;
                  dashArray[2] = 0;
                  dashArray[3] = 5;
                  dashArray[4] = 1;
                  dashArray[5] = 0;
                  dashArray[6] = 5;
                  dashArray[7] = 1;
                  lenDashArray = 8;
                  break;
                case 5: // PS_NULL
                  nullPen = true;
                  break;
              }

              if (!nullPen)
              {
                data += wxPdfDocument::Double2String(obj->r/255.,3) + wxString(_T(" "));
                data += wxPdfDocument::Double2String(obj->g/255.,3) + wxString(_T(" "));
                data += wxPdfDocument::Double2String(obj->b/255.,3) + wxString(_T(" RG\n"));

                data += wxPdfDocument::Double2String(obj->width*m_document->m_k,2) + wxString(_T(" w\n"));
              }

              if (lenDashArray > 0)
              {
                wxString s = _T("[");
                for (i = 0; i < lenDashArray; i++)
                {
                  s += wxPdfDocument::Double2String(dashArray[i] * m_document->m_k,4);
                  if (i != lenDashArray-1)
                  {
                    s += _T(" ");
                  }
                }
                s += _T("] 0 d\n");
                data += s;
              }
              break;
          }
        }
        break;

      case 0x0325: // Polyline
      case 0x0324: // Polygon
        {
          short* coords = new short[size-3];
          for (i = 0; i < size-3; i++)
          {
            coords[i] = ReadShortLE(imageStream);
          }
          short numpoints = coords[0];

          for (k = numpoints; k > 0; k--)
          {
            px = coords[2*k-1];
            py = coords[2*k];

            if (k < numpoints)
            {
              data += wxString::Format(_T("%d %d l\n"), px, py);
            }
            else
            {
              data += wxString::Format(_T("%d %d m\n"), px, py);
            }
          }

          if (func == 0x0325)
          {
            op = _T("s");
          }
          else if (func == 0x0324)
          {
            if (nullPen)
            {
              if (nullBrush)
              {
                op = _T("n");  // no op
              }
              else
              {
                op = _T("f");  // fill
              }
            }
            else
            {
              if (nullBrush)
              {
                op = _T("s");  // stroke
              }
              else
              {
                op = _T("b");  // stroke and fill
              }
            }

            if (polyFillMode == 1 && (op == _T("b") || op == _T("f")))
            {
              op += _T("*");  // use even-odd fill rule
            }
          }
          data += op + _T("\n");
          delete [] coords;
        }
        break;

      case 0x0538: // PolyPolygon
        {
          short* coords = new short[size-3];
          for (i = 0; i < size-3; i++)
          {
            coords[i] = ReadShortLE(imageStream);
          }
          short numpolygons = coords[0];

          short adjustment = numpolygons;

          for (j = 1; j <= numpolygons; j++)
          {
            short numpoints = coords[j + 1];

            for (k = numpoints; k > 0; k--)
            {
              px = coords[2*k-1 + adjustment];
              py = coords[2*k   + adjustment];

              if (k == numpoints)
              {
                data += wxString::Format(_T("%d %d m\n"), px, py);
              }
              else
              {
                data += wxString::Format(_T("%d %d m\n"), px, py);
              }
            }

            adjustment += numpoints * 2;
          }

          if (nullPen)
          {
            if (nullBrush)
            {
              op = _T("n");  // no op
            }
            else
            {
              op = _T("f");  // fill
            }
          }
          else
          {
            if (nullBrush)
            {
              op = _T("s");  // stroke
            }
            else
            {
              op = _T("b");  // stroke and fill
            }
          }

          if (polyFillMode == 1 && (op == _T("b") || op == _T("f"))) 
          {
            op += _T("*");  // use even-odd fill rule
          }

          data += op + _T("\n");
          delete [] coords;
        }
        break;

      case 0x0000:
        endRecord = true;
        isValid = true;
        break;
      default:
        if (size > 3)
        {
          imageStream->SeekI(2*(size-3), wxFromCurrent);
        }
        break;
    }
  }

  for (i = 0; i < gdiObjects.GetCount(); i++)
  {
    if (gdiObjects[i] != NULL)
    {
      delete ((GdiObject*) gdiObjects[i]);
    }
  }
  m_x = wo[0];
  m_y = wo[1];
  m_width = we[0];
  m_height = we[1];

  wxCharBuffer wcb(data.ToAscii());
  m_dataSize = data.Length();
  m_data = new char[m_dataSize];
  memcpy(m_data,(const char*) wcb,m_dataSize);
  return isValid;
}

int
wxPdfImage::ReadIntBE(wxInputStream* imageStream)
{
  // Read a 4-byte integer from file (big endian)
  int i32;
  imageStream->Read(&i32, 4);
  return wxINT32_SWAP_ON_LE(i32);
}

int
wxPdfImage::ReadIntLE(wxInputStream* imageStream)
{
  // Read a 4-byte integer from file (little endian)
  int i32;
  imageStream->Read(&i32, 4);
  return wxINT32_SWAP_ON_BE(i32);
}

unsigned int
wxPdfImage::ReadUIntBE(wxInputStream* imageStream)
{
  // Read a unsigned 4-byte integer from file (big endian)
  unsigned int i32;
  imageStream->Read(&i32, 4);
  return wxUINT32_SWAP_ON_LE(i32);
}

unsigned int
wxPdfImage::ReadUIntLE(wxInputStream* imageStream)
{
  // Read a unsigned 4-byte integer from file (little endian)
  unsigned int i32;
  imageStream->Read(&i32, 4);
  return wxUINT32_SWAP_ON_BE(i32);
}

short
wxPdfImage::ReadShortBE(wxInputStream* imageStream)
{
  // Read a 2-byte integer from file (big endian)
  short i16;
  imageStream->Read(&i16, 2);
  return wxINT16_SWAP_ON_LE(i16);
}

short
wxPdfImage::ReadShortLE(wxInputStream* imageStream)
{
  // Read a 2-byte integer from file (little endian)
  short i16;
  imageStream->Read(&i16, 2);
  return wxINT16_SWAP_ON_BE(i16);
}

unsigned short
wxPdfImage::ReadUShortBE(wxInputStream* imageStream)
{
  // Read a unsigned 2-byte integer from file (big endian)
  unsigned short i16;
  imageStream->Read(&i16, 2);
  return wxUINT16_SWAP_ON_LE(i16);
}

unsigned short
wxPdfImage::ReadUShortLE(wxInputStream* imageStream)
{
  // Read a unsigned 2-byte integer from file (little endian)
  unsigned short i16;
  imageStream->Read(&i16, 2);
  return wxUINT16_SWAP_ON_BE(i16);
}
