///////////////////////////////////////////////////////////////////////////////
// Name:        pdfgraphics.cpp
// Purpose:     Implementation of wxPdfDocument graphics primitives
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-01-27
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfgraphics.cpp Implementation of the wxPdfDocument graphics primitives

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/tokenzr.h>

#include "wx/pdfdoc.h"
#include "wx/pdfgraphics.h"

wxPdfExtGState::wxPdfExtGState(double lineAlpha, double fillAlpha, wxPdfBlendMode blendMode)
{
  m_lineAlpha = lineAlpha;
  m_fillAlpha = fillAlpha;
  m_blendMode = blendMode;
}

wxPdfExtGState::~wxPdfExtGState()
{
}

int
wxPdfDocument::SetAlpha(double lineAlpha, double fillAlpha, wxPdfBlendMode blendMode)
{
  int n = 0;

  // Force alpha into range 0 .. 1
  if (lineAlpha < 0) lineAlpha = 0;
  else if (lineAlpha > 1) lineAlpha = 1;
  if (fillAlpha < 0) fillAlpha = 0;
  else if (fillAlpha > 1) fillAlpha = 1;

  // Create state id for lookup map
  int id = ((int) blendMode) * 100000000 + (int) (lineAlpha * 1000) * 10000 + (int) (fillAlpha * 1000);

  // Lookup state
  wxPdfExtGSLookupMap::iterator extGState = (*m_extGSLookup).find(id);
  if (extGState == (*m_extGSLookup).end())
  {
    n = (*m_extGStates).size() + 1;
    (*m_extGStates)[n] = new wxPdfExtGState(lineAlpha, fillAlpha, blendMode);
    (*m_extGSLookup)[id] = n;
  }
  else
  {
    n = extGState->second;
  }

  if (n != m_currentExtGState)
  {
    SetAlphaState(n);
  }

  return n;
}

void
wxPdfDocument::SetAlphaState(int alphaState)
{
  if (alphaState > 0 && (size_t) alphaState <= (*m_extGStates).size())
  {
    OutAscii(wxString::Format(_T("/GS%d gs"), alphaState));
  }
}

// ----------------------------------------------------------------------------
// wxPdfLineStyle: class representing line style for drawing graphics
// ----------------------------------------------------------------------------

wxPdfLineStyle::wxPdfLineStyle(double width,
                               wxPdfLineCap cap, wxPdfLineJoin join,
                               const wxPdfArrayDouble& dash, double phase,
                               const wxPdfColour& color)
{
  m_isSet = (width > 0) || (cap >= 0) || (join >= 0) || (dash.GetCount() > 0);
  m_width = width;
  m_cap   = cap;
  m_join  = join;
  m_dash  = dash;
  m_phase = phase;
  m_color = color;
  m_miterLimit = 0;
}

wxPdfLineStyle::~wxPdfLineStyle()
{
}


wxPdfLineStyle::wxPdfLineStyle(const wxPdfLineStyle& lineStyle)
{
  m_isSet = lineStyle.m_isSet;
  m_width = lineStyle.m_width;
  m_cap   = lineStyle.m_cap;
  m_join  = lineStyle.m_join;
  m_dash  = lineStyle.m_dash;
  m_phase = lineStyle.m_phase;
  m_color = lineStyle.m_color;
  m_miterLimit = lineStyle.m_miterLimit;
}

wxPdfLineStyle&
wxPdfLineStyle::operator= (const wxPdfLineStyle& lineStyle)
{
  m_isSet = lineStyle.m_isSet;
  m_width = lineStyle.m_width;
  m_cap   = lineStyle.m_cap;
  m_join  = lineStyle.m_join;
  m_dash  = lineStyle.m_dash;
  m_phase = lineStyle.m_phase;
  m_color = lineStyle.m_color;
  m_miterLimit = lineStyle.m_miterLimit;
  return *this;
}

// --- Gradients

wxPdfGradient::wxPdfGradient(wxPdfGradientType type)
{
  m_type = type;
}

wxPdfGradient::~wxPdfGradient()
{
}

wxPdfAxialGradient::wxPdfAxialGradient(const wxPdfColour& color1, const wxPdfColour& color2, double x1, double y1, double x2, double y2, double intexp)
  : wxPdfGradient(wxPDF_GRADIENT_AXIAL)
{
  m_color1 = color1;
  m_color2 = color2;
  m_x1 = x1;
  m_y1 = y1;
  m_x2 = x2;
  m_y2 = y2;
  m_intexp = intexp;
}

wxPdfAxialGradient::~wxPdfAxialGradient()
{
}

wxPdfMidAxialGradient::wxPdfMidAxialGradient(const wxPdfColour& color1, const wxPdfColour& color2, double x1, double y1, double x2, double y2, double midpoint, double intexp)
  : wxPdfAxialGradient(color1, color2, x1, y1, x2, y2, intexp)
{
  m_type = wxPDF_GRADIENT_MIDAXIAL;
  m_midpoint = midpoint;
}

wxPdfMidAxialGradient::~wxPdfMidAxialGradient()
{
}

wxPdfRadialGradient::wxPdfRadialGradient(const wxPdfColour& color1, const wxPdfColour& color2,
                                         double x1, double y1, double r1,
                                         double x2, double y2, double r2, double intexp)
  : wxPdfAxialGradient(color1, color2, x1, y1, x2, y2, intexp)
{
  m_type = wxPDF_GRADIENT_RADIAL;
  m_r1 = r1;
  m_r2 = r2;
}

wxPdfRadialGradient::~wxPdfRadialGradient()
{
}

wxPdfCoonsPatch::wxPdfCoonsPatch(int edgeFlag, wxPdfColour colors[], double x[], double y[])
{
  m_edgeFlag = edgeFlag;
  size_t n = (edgeFlag == 0) ? 4 : 2;
  size_t j;
  for (j = 0; j < n; j++)
  {
    m_colors[j] = colors[j];
  }

  n = (edgeFlag == 0) ? 12 : 8;
  for (j = 0; j < n; j++)
  {
    m_x[j] = x[j];
    m_y[j] = y[j];
  }
}

wxPdfCoonsPatch::~wxPdfCoonsPatch()
{
}

wxPdfCoonsPatchMesh::wxPdfCoonsPatchMesh()
{
  m_ok = false;
  m_colorType = wxPDF_COLOURTYPE_UNKNOWN;
}

wxPdfCoonsPatchMesh::~wxPdfCoonsPatchMesh()
{
  size_t n = m_patches.size();
  if (n > 0)
  {
    size_t j;
    for (j = 0; j < n; j++)
    {
      delete ((wxPdfCoonsPatch*) m_patches[j]);
    }
  }
}

bool
wxPdfCoonsPatchMesh::AddPatch(int edgeFlag, wxPdfColour colors[], double x[], double y[])
{
  wxPdfColourType colorType = m_colorType;
  if (m_patches.size() == 0 && edgeFlag != 0) return false;
  int n = (edgeFlag == 0) ? 4 : 2;
  int j;
  for (j = 0; j < n; j++)
  {
    if (colorType == wxPDF_COLOURTYPE_UNKNOWN)
    {
      colorType = colors[j].GetColorType();
    }
    if (colors[j].GetColorType() != colorType) return false;
  }
  m_colorType = colorType;
  wxPdfCoonsPatch* patch = new wxPdfCoonsPatch(edgeFlag, colors, x, y);
  m_patches.Add(patch);
  m_ok = true;
  return true;
}

wxPdfCoonsPatchGradient::wxPdfCoonsPatchGradient(const wxPdfCoonsPatchMesh& mesh, double minCoord, double maxCoord)
  : wxPdfGradient(wxPDF_GRADIENT_COONS)
{
  int edgeFlag;
  double *x;
  double *y;
  const wxArrayPtrVoid* patches = mesh.GetPatches();
  size_t n = patches->size();
  size_t j, k, nc;
  unsigned char ch;
  int bpcd = 65535; //16 BitsPerCoordinate
  int coord;
  wxPdfColour *colors;

  m_colorType = mesh.GetColorType();
  // build the data stream
  for (j = 0;  j < n; j++)
  {
    wxPdfCoonsPatch* patch = (wxPdfCoonsPatch*) (*patches)[j];
    edgeFlag = patch->GetEdgeFlag();
    ch = edgeFlag;
    m_buffer.Write(&ch,1); //start with the edge flag as 8 bit
    x = patch->GetX();
    y = patch->GetY();
    nc = (edgeFlag == 0) ? 12 : 8;
    for (k = 0; k < nc; k++)
    {
      // each point as 16 bit
      coord = (int) (((x[k] - minCoord) / (maxCoord - minCoord)) * bpcd);
      if (coord < 0)    coord = 0;
      if (coord > bpcd) coord = bpcd;
      ch = (coord >> 8) & 0xFF;
      m_buffer.Write(&ch,1);
      ch = coord & 0xFF;
      m_buffer.Write(&ch,1);
      coord = (int) (((y[k] - minCoord) / (maxCoord - minCoord)) * bpcd);
      if (coord < 0)    coord = 0;
      if (coord > bpcd) coord = bpcd;
      ch = (coord >> 8) & 0xFF;
      m_buffer.Write(&ch,1);
      ch = coord & 0xFF;
      m_buffer.Write(&ch,1);
    }
    colors = patch->GetColors();
    nc = (edgeFlag == 0) ? 4 : 2;
    for (k = 0; k < nc; k++)
    {
      // each color component as 8 bit
      wxStringTokenizer tkz(colors[k].GetColorValue(), wxT(" "));
      while ( tkz.HasMoreTokens() )
      {
        ch = ((int) (wxPdfDocument::String2Double(tkz.GetNextToken()) * 255)) & 0xFF;
        m_buffer.Write(&ch,1);
      }
    }
  }
}

wxPdfCoonsPatchGradient::~wxPdfCoonsPatchGradient()
{
}

// ---

wxPdfShape::wxPdfShape()
{
  m_subpath = -1;
  m_index = 0;
}

wxPdfShape::~wxPdfShape()
{
}

void
wxPdfShape::MoveTo(double x, double y)
{
  m_subpath = m_x.GetCount();
  m_types.Add(wxPDF_SEG_MOVETO);
  m_x.Add(x);
  m_y.Add(y);
}

void
wxPdfShape::LineTo(double x, double y)
{
  if (m_subpath >= 0)
  {
    m_types.Add(wxPDF_SEG_LINETO);
    m_x.Add(x);
    m_y.Add(y);
  }
  else
  {
    wxLogError(_T("wxPdfShape::LineTo: Invalid subpath."));
  }
}

void
wxPdfShape::CurveTo(double x1, double y1, double x2, double y2, double x3, double y3)
{
  if (m_subpath >= 0)
  {
    m_types.Add(wxPDF_SEG_CURVETO);
    m_x.Add(x1);
    m_y.Add(y1);
    m_x.Add(x2);
    m_y.Add(y2);
    m_x.Add(x3);
    m_y.Add(y3);
  }
  else
  {
    wxLogError(_T("wxPdfShape::LineTo: Invalid subpath."));
  }
}

void
wxPdfShape::ClosePath()
{
  if (m_subpath >= 0 && m_types.GetCount() > 0 && m_types.Last() != wxPDF_SEG_CLOSE)
  {
    m_types.Add(wxPDF_SEG_CLOSE);
    m_x.Add(m_x[m_subpath]);
    m_y.Add(m_y[m_subpath]);
    m_subpath = -1;
  }
}

wxPdfSegmentType
wxPdfShape::GetSegment(int iterType, int iterPoints, double coords[]) const
{
  wxPdfSegmentType segType = wxPDF_SEG_UNDEFINED;
  if (iterType >= 0 && (size_t) iterType < m_types.GetCount())
  {
    int pointCount = (m_types[iterType] == wxPDF_SEG_CURVETO) ? 2 : 0;
    if (iterPoints >= 0 && (size_t) (iterPoints + pointCount) < m_x.GetCount())
    {
      segType = (wxPdfSegmentType) m_types[iterType];
      switch (segType)
      {
        case wxPDF_SEG_CLOSE:
          coords[0] = m_x[iterPoints];
          coords[1] = m_y[iterPoints];
          break;

        case wxPDF_SEG_MOVETO:
        case wxPDF_SEG_LINETO:
          coords[0] = m_x[iterPoints];
          coords[1] = m_y[iterPoints];
          break;

        case wxPDF_SEG_CURVETO:
          coords[0] = m_x[iterPoints];
          coords[1] = m_y[iterPoints];
          iterPoints++;
          coords[2] = m_x[iterPoints];
          coords[3] = m_y[iterPoints];
          iterPoints++;
          coords[4] = m_x[iterPoints];
          coords[5] = m_y[iterPoints];
          break;
      }
    }
  }
  return segType;
}

wxPdfFlatPath::wxPdfFlatPath(const wxPdfShape* shape, double flatness, int limit)
{
  m_shape = shape;
  m_iterType = 0;
  m_iterPoints = 0;
  m_done = false;
  m_flatnessSq = flatness * flatness;
  m_recursionLimit = limit;

  m_stackMaxSize = 6 * m_recursionLimit + /* 6 + 2 */ 8;
  m_stack = new double[m_stackMaxSize];
  m_recLevel = new int[m_recursionLimit + 1];

  FetchSegment();
}

wxPdfFlatPath::~wxPdfFlatPath()
{
  delete [] m_stack;
  delete [] m_recLevel;
}

void
wxPdfFlatPath::InitIter()
{
  m_done       = false;
  m_iterType   = 0;
  m_iterPoints = 0;
  m_stackSize  = 0;
  FetchSegment();
}

  /**
   * Fetches the next segment from the source iterator.
   */
void
wxPdfFlatPath::FetchSegment()
{
  int sp;

  if ((size_t) m_iterType >= m_shape->GetSegmentCount())
  {
    m_done = true;
    return;
  }

  m_srcSegType = m_shape->GetSegment(m_iterType, m_iterPoints, m_scratch);
    
  switch (m_srcSegType)
  {
    case wxPDF_SEG_CLOSE:
      return;

    case wxPDF_SEG_MOVETO:
    case wxPDF_SEG_LINETO:
      m_srcPosX = m_scratch[0];
      m_srcPosY = m_scratch[1];
      return;

    case wxPDF_SEG_CURVETO:
      if (m_recursionLimit == 0)
      {
        m_srcPosX = m_scratch[4];
        m_srcPosY = m_scratch[5];
        m_stackSize = 0;
        return;
      }
      sp = 6 * m_recursionLimit;
      m_stackSize = 1;
      m_recLevel[0] = 0;
      m_stack[sp] = m_srcPosX;                  // P1.x
      m_stack[sp + 1] = m_srcPosY;              // P1.y
      m_stack[sp + 2] = m_scratch[0];           // C1.x
      m_stack[sp + 3] = m_scratch[1];           // C1.y
      m_stack[sp + 4] = m_scratch[2];           // C2.x
      m_stack[sp + 5] = m_scratch[3];           // C2.y
      m_srcPosX = m_stack[sp + 6] = m_scratch[4]; // P2.x
      m_srcPosY = m_stack[sp + 7] = m_scratch[5]; // P2.y
      SubdivideCubic();
      return;
  }
}

void
wxPdfFlatPath::Next()
{
  if (m_stackSize > 0)
  {
    --m_stackSize;
    if (m_stackSize > 0)
    {
      switch (m_srcSegType)
      {
        case wxPDF_SEG_CURVETO:
          SubdivideCubic();
          return;

        default:
          break;
          //throw new IllegalStateException();
      }
    }
  }

  if ((size_t) m_iterType < m_shape->GetSegmentCount())
  {
    switch (m_srcSegType)
    {
      case wxPDF_SEG_CLOSE:
      case wxPDF_SEG_MOVETO:
      case wxPDF_SEG_LINETO:
        m_iterPoints++;
        break;

      case wxPDF_SEG_CURVETO:
        m_iterPoints += 3;
        break;
    }
    m_iterType++;
  }

  FetchSegment();
}

int
wxPdfFlatPath::CurrentSegment(double coords[])
{
  //if (done)
    //throw new NoSuchElementException();

  switch (m_srcSegType)
  {
    case wxPDF_SEG_CLOSE:
      return m_srcSegType;

    case wxPDF_SEG_MOVETO:
    case wxPDF_SEG_LINETO:
      coords[0] = m_srcPosX;
      coords[1] = m_srcPosY;
      return m_srcSegType;

    case wxPDF_SEG_CURVETO:
      if (m_stackSize == 0)
      {
        coords[0] = m_srcPosX;
        coords[1] = m_srcPosY;
      }
      else
      {
        int sp = m_stackMaxSize - 6 * m_stackSize;
        coords[0] = m_stack[sp + 4];
        coords[1] = m_stack[sp + 5];
      }
      return wxPDF_SEG_LINETO;
  }

  //throw new IllegalStateException();
  return wxPDF_SEG_UNDEFINED;
}

static void
SubdivideCubicCurve(double src[], int srcOff,
                    double left[], int leftOff,
                    double right[], int rightOff)
{
  // To understand this code, please have a look at the image
  // "CubicCurve2D-3.png" in the sub-directory "doc-files".
  double srcC1x;
  double srcC1y;
  double srcC2x;
  double srcC2y;
  double leftP1x;
  double leftP1y;
  double leftC1x;
  double leftC1y;
  double leftC2x;
  double leftC2y;
  double rightC1x;
  double rightC1y;
  double rightC2x;
  double rightC2y;
  double rightP2x;
  double rightP2y;
  double midx; // Mid = left.P2 = right.P1
  double midy; // Mid = left.P2 = right.P1

  leftP1x = src[srcOff];
  leftP1y = src[srcOff + 1];
  srcC1x = src[srcOff + 2];
  srcC1y = src[srcOff + 3];
  srcC2x = src[srcOff + 4];
  srcC2y = src[srcOff + 5];
  rightP2x = src[srcOff + 6];
  rightP2y = src[srcOff + 7];

  leftC1x = (leftP1x + srcC1x) / 2;
  leftC1y = (leftP1y + srcC1y) / 2;
  rightC2x = (rightP2x + srcC2x) / 2;
  rightC2y = (rightP2y + srcC2y) / 2;
  midx = (srcC1x + srcC2x) / 2;
  midy = (srcC1y + srcC2y) / 2;
  leftC2x = (leftC1x + midx) / 2;
  leftC2y = (leftC1y + midy) / 2;
  rightC1x = (midx + rightC2x) / 2;
  rightC1y = (midy + rightC2y) / 2;
  midx = (leftC2x + rightC1x) / 2;
  midy = (leftC2y + rightC1y) / 2;

  if (left != NULL)
  {
    left[leftOff] = leftP1x;
    left[leftOff + 1] = leftP1y;
    left[leftOff + 2] = leftC1x;
    left[leftOff + 3] = leftC1y;
    left[leftOff + 4] = leftC2x;
    left[leftOff + 5] = leftC2y;
    left[leftOff + 6] = midx;
    left[leftOff + 7] = midy;
  }

  if (right != NULL)
  {
    right[rightOff] = midx;
    right[rightOff + 1] = midy;
    right[rightOff + 2] = rightC1x;
    right[rightOff + 3] = rightC1y;
    right[rightOff + 4] = rightC2x;
    right[rightOff + 5] = rightC2y;
    right[rightOff + 6] = rightP2x;
    right[rightOff + 7] = rightP2y;
  }
}

static double
PointSegmentDistanceSq(double x1, double y1, double x2, double y2, double px, double py)
{
  double pd2 = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);

  double x, y;
  if (pd2 == 0)
  {
    // Points are coincident.
    x = x1;
    y = y2;
  }
  else
  {
    double u = ((px - x1) * (x2 - x1) + (py - y1) * (y2 - y1)) / pd2;

    if (u < 0)
    {
      // "Off the end"
      x = x1;
      y = y1;
    }
    else if (u > 1.0)
    {
      x = x2;
      y = y2;
    }
    else
    {
      x = x1 + u * (x2 - x1);
      y = y1 + u * (y2 - y1);
    }
  }

  return (x - px) * (x - px) + (y - py) * (y - py);
}

static double
GetFlatnessSq(double x1, double y1, double cx1, double cy1,
              double cx2, double cy2, double x2, double y2)
{

  double d1 = PointSegmentDistanceSq(x1, y1, x2, y2, cx1, cy1);
  double d2 = PointSegmentDistanceSq(x1, y1, x2, y2, cx2, cy2);
  return (d1 > d2) ? d1 : d2;
//  return Math.max(Line2D.ptSegDistSq(x1, y1, x2, y2, cx1, cy1),
//                  Line2D.ptSegDistSq(x1, y1, x2, y2, cx2, cy2));
}

static double
GetFlatnessSq(double coords[], int offset)
{
  return GetFlatnessSq(coords[offset+0], coords[offset+1], coords[offset+2],
                       coords[offset+3], coords[offset+4], coords[offset+5],
                       coords[offset+6], coords[offset+7]);
}

  /**
   * Repeatedly subdivides the cubic curve segment that is on top
   * of the stack. The iteration terminates when the recursion limit
   * has been reached, or when the resulting segment is flat enough.
   */
void
wxPdfFlatPath::SubdivideCubic()
{
  int sp;
  int level;

  sp = m_stackMaxSize - 6 * m_stackSize - 2;
  level = m_recLevel[m_stackSize - 1];
  while ((level < m_recursionLimit)
         && (GetFlatnessSq(m_stack, sp) >= m_flatnessSq))
  {
    m_recLevel[m_stackSize] = m_recLevel[m_stackSize - 1] = ++level;
      
    SubdivideCubicCurve(m_stack, sp, m_stack, sp - 6, m_stack, sp);
    ++m_stackSize;
    sp -= 6;
  }
}

double
wxPdfFlatPath::MeasurePathLength()
{
  double points[6];
  double moveX = 0, moveY = 0;
  double lastX = 0, lastY = 0;
  double thisX = 0, thisY = 0;
  int type = 0;
  double total = 0;

  // Save iterator state
  bool saveDone      = m_done;
  int saveIterType   = m_iterType;
  int saveIterPoints = m_iterPoints;
  int saveStackSize  = m_stackSize;

  InitIter();
  while (!IsDone())
  {
    type = CurrentSegment(points);
    switch( type )
    {
      case wxPDF_SEG_MOVETO:
        moveX = lastX = points[0];
        moveY = lastY = points[1];
        break;

      case wxPDF_SEG_CLOSE:
        points[0] = moveX;
        points[1] = moveY;
        // Fall into....

      case wxPDF_SEG_LINETO:
        thisX = points[0];
        thisY = points[1];
        double dx = thisX-lastX;
        double dy = thisY-lastY;
        total += sqrt(dx*dx + dy*dy);
        lastX = thisX;
        lastY = thisY;
        break;
    }
    Next();
  }

  // Restore iterator state
  m_done       = saveDone;
  m_iterType   = saveIterType;
  m_iterPoints = saveIterPoints;
  m_stackSize  = saveStackSize;
  FetchSegment();

  return total;
}

void
wxPdfDocument::ShapedText(const wxPdfShape& shape, const wxString& text, wxPdfShapedTextMode mode)
{
  bool stretchToFit = (mode == wxPDF_SHAPEDTEXTMODE_STRETCHTOFIT);
  bool repeat = (mode == wxPDF_SHAPEDTEXTMODE_REPEAT);
  double flatness = 0.25 / GetScaleFactor();
  wxPdfFlatPath it(&shape, flatness);
  double points[6];
  double moveX = 0, moveY = 0;
  double lastX = 0, lastY = 0;
  double thisX = 0, thisY = 0;
  int type = 0;
  bool first = false;
  double next = 0;
  int currentChar = 0;
  int length = text.Length();
  double height = GetFontSize() / GetScaleFactor();

  if ( length == 0 )
    return;

  double factor = stretchToFit ? it.MeasurePathLength() / GetStringWidth(text) : 1.0;
  double nextAdvance = 0;

  while (currentChar < length && !it.IsDone())
  {
    type = it.CurrentSegment(points);
    switch (type)
    {
      case wxPDF_SEG_MOVETO:
        moveX = lastX = points[0];
        moveY = lastY = points[1];
        SetXY(moveX, moveY);
        first = true;
        nextAdvance = GetStringWidth(text.Mid(currentChar,1)) * 0.5;
        next = nextAdvance;
        break;

      case wxPDF_SEG_CLOSE:
        points[0] = moveX;
        points[1] = moveY;
        // Fall into....

      case wxPDF_SEG_LINETO:
        thisX = points[0];
        thisY = points[1];
        double dx = thisX-lastX;
        double dy = thisY-lastY;
        double distance = sqrt(dx*dx + dy*dy);
        if (distance >= next)
        {
          double r = 1.0 / distance;
          double angle = atan2(-dy, dx) * 45. / atan(1.);
          while (currentChar < length && distance >= next)
          {
            wxString glyph = text.Mid(currentChar, 1);
            double x = lastX + next*dx*r;
            double y = lastY + next*dy*r;
            double advance = nextAdvance;
            nextAdvance = currentChar < length-1 ? GetStringWidth(text.Mid(currentChar+1,1)) * 0.5 : 
                                                   (repeat) ? GetStringWidth(text.Mid(0,1)) * 0.5 : 0;
            SetXY(x, y);
            StartTransform();
            Rotate(angle);
            SetXY(x-advance,y-height);
            Write(height, glyph);
            StopTransform();
            next += (advance+nextAdvance) * factor;
            currentChar++;
            if ( repeat )
            {
              currentChar %= length;
            }
          }
        }
        next -= distance;
        first = false;
        lastX = thisX;
        lastY = thisY;
        break;
    }
    it.Next();
  }
}

// ---

void
wxPdfDocument::Line(double x1, double y1, double x2, double y2)
{
  // Draw a line
  OutAscii(Double2String(x1*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y1)*m_k,2) + wxString(_T(" m ")) +
           Double2String(x2*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y2)*m_k,2) + wxString(_T(" l S")));
}

void
wxPdfDocument::Rect(double x, double y, double w, double h, int style)
{
  wxString op;
  // Draw a rectangle
  if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILL)
  {
    op = _T("f");
  }
  else if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILLDRAW)
  {
    op = _T("B");
  }
  else
  {
    op = _T("S");
  }
  OutAscii(Double2String(x*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" ")) +
           Double2String(w*m_k,2) + wxString(_T(" ")) +
           Double2String(-h*m_k,2) + wxString(_T(" re ")) + op);
}

void
wxPdfDocument::RoundedRect(double x, double y, double w, double h,
                           double r, int roundCorner, int style)
{
  if ((roundCorner & wxPDF_CORNER_ALL) == wxPDF_CORNER_NONE)
  {
    // Not rounded
    Rect(x, y, w, h, style);
  }
  else
  { 
    // Rounded
    wxString op;
    // Draw a rectangle
    if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILL)
    {
      op = _T("f");
    }
    else
    {
      if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILLDRAW)
      {
        op = _T("B");
      }
      else
      {
        op = _T("S");
      }
    }

    double myArc = 4. / 3. * (sqrt(2.) - 1.);

    OutPoint(x + r, y);
    double xc = x + w - r;
    double yc = y + r;
    OutLine(xc, y);

    if (roundCorner & wxPDF_CORNER_TOP_LEFT)
    {
      OutCurve(xc + (r * myArc), yc - r, xc + r, yc - (r * myArc), xc + r, yc);
    }
    else
    {
      OutLine(x + w, y);
    }

    xc = x + w - r ;
    yc = y + h - r;
    OutLine(x + w, yc);

    if (roundCorner & wxPDF_CORNER_TOP_RIGHT)
    {
      OutCurve(xc + r, yc + (r * myArc), xc + (r * myArc), yc + r, xc, yc + r);
    }
    else
    {
      OutLine(x + w, y + h);
    }

    xc = x + r;
    yc = y + h - r;
    OutLine(xc, y + h);

    if (roundCorner & wxPDF_CORNER_BOTTOM_LEFT)
    {
      OutCurve(xc - (r * myArc), yc + r, xc - r, yc + (r * myArc), xc - r, yc);
    }
    else
    {
      OutLine(x, y + h);
    }

    xc = x + r;
    yc = y + r;
    OutLine(x, yc);
    
    if (roundCorner & wxPDF_CORNER_BOTTOM_RIGHT)
    {
      OutCurve(xc - r, yc - (r * myArc), xc - (r * myArc), yc - r, xc, yc - r);
    }
    else
    {
      OutLine(x, y);
      OutLine(x + r, y);
    }
    OutAscii(op);
  }
}

void
wxPdfDocument::Curve(double x0, double y0, double x1, double y1, 
                     double x2, double y2, double x3, double y3,
                     int style)
{
  wxString op;
  // Draw a rectangle
  if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILL)
  {
    op = _T("f");
  }
  else
  {
    if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILLDRAW)
    {
      op = _T("B");
    }
    else
    {
      op = _T("S");
    }
  }

  OutPoint(x0, y0);
  OutCurve(x1, y1, x2, y2, x3, y3);
  OutAscii(op);
}

void
wxPdfDocument::Ellipse(double x0, double y0, double rx, double ry, 
                       double angle, double astart, double afinish,
                       int style, int nSeg)
{
  if (rx <= 0) return;

  wxString op;
  // Draw a rectangle
  if ((style & wxPDF_STYLE_MASK) == wxPDF_STYLE_FILL)
  {
    op = _T("f");
  }
  else
  {
    if ((style & wxPDF_STYLE_MASK) == wxPDF_STYLE_FILLDRAW)
    {
      op = _T("B");
    }
    else if ((style & wxPDF_STYLE_MASK) == wxPDF_STYLE_DRAWCLOSE)
    {
      op = _T("s"); // small 's' means closing the path as well
    }
    else
    {
      op = _T("S");
    }
  }

  if (ry <= 0)
  {
    ry = rx;
  }
  rx *= m_k;
  ry *= m_k;
  if (nSeg < 2)
  {
    nSeg = 2;
  }

  static double pi = 4. * atan(1.0);
  astart = pi * astart / 180.;
  afinish = pi * afinish / 180.;
  double totalAngle = afinish - astart;

  double dt = totalAngle / nSeg;
  double dtm = dt / 3;

  x0 *= m_k;
  y0 = (m_h - y0) * m_k;
  if (angle != 0)
  {
    double a = -(pi * angle / 180.);
    OutAscii(wxString(_T("q ")) + 
             Double2String(cos(a),2) + wxString(_T(" ")) +
             Double2String(-1 * sin(a),2) + wxString(_T(" ")) +
             Double2String(sin(a),2) + wxString(_T(" ")) +
             Double2String(cos(a),2) + wxString(_T(" ")) +
             Double2String(x0,2) + wxString(_T(" ")) +
             Double2String(y0,2) + wxString(_T(" cm")));
    x0 = 0;
    y0 = 0;
  }

  double t1, a0, b0, c0, d0, a1, b1, c1, d1;
  t1 = astart;
  a0 = x0 + (rx * cos(t1));
  b0 = y0 + (ry * sin(t1));
  c0 = -rx * sin(t1);
  d0 = ry * cos(t1);
  OutPoint(a0 / m_k, m_h - (b0 / m_k));
  int i;
  for (i = 1; i <= nSeg; i++)
  {
    // Draw this bit of the total curve
    t1 = (i * dt) + astart;
    a1 = x0 + (rx * cos(t1));
    b1 = y0 + (ry * sin(t1));
    c1 = -rx * sin(t1);
    d1 = ry * cos(t1);
    OutCurve((a0 + (c0 * dtm)) / m_k,
             m_h - ((b0 + (d0 * dtm)) / m_k),
             (a1 - (c1 * dtm)) / m_k,
             m_h - ((b1 - (d1 * dtm)) / m_k),
             a1 / m_k,
             m_h - (b1 / m_k));
    a0 = a1;
    b0 = b1;
    c0 = c1;
    d0 = d1;
  }
  OutAscii(op);
  if (angle !=0)
  {
    Out("Q");
  }
}

void
wxPdfDocument::Circle(double x0, double y0, double r, double astart, double afinish,
                      int style, int nSeg)
{
  Ellipse(x0, y0, r, 0, 0, astart, afinish, style, nSeg);
}

void
wxPdfDocument::Sector(double xc, double yc, double r, double astart, double afinish,
                      int style, bool clockwise, double origin)
{
  static double pi = 4. * atan(1.);
  static double pi2 = 0.5 * pi;
  double d;
  if (clockwise)
  {
    d = afinish;
    afinish = origin - astart;
    astart = origin - d;
  }
  else
  {
    afinish += origin;
    astart += origin;
  }
  astart = fmod(astart, 360.) + 360;
  afinish = fmod(afinish, 360.) + 360;
  if (astart > afinish)
  {
    afinish += 360;
  }
  afinish = afinish / 180. * pi;
  astart = astart / 180. * pi;
  d = afinish - astart;
  if (d == 0)
  {
    d = 2 * pi;
  }
  
  wxString op;
  if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILL)
  {
    op = _T("f");
  }
  else
  {
    if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILLDRAW)
    {
      op = _T("b");
    }
    else
    {
      op = _T("s");
    }
  }

  double myArc;
  if (sin(d/2) != 0.0)
  {
    myArc = 4./3. * (1.-cos(d/2))/sin(d/2) * r;
  }
  else
  {
    myArc = 0.0;
  }
  // first put the center
  OutPoint(xc,yc);
  // put the first point
  OutLine(xc+r*cos(astart),yc-r*sin(astart));
  // draw the arc
  if (d < pi2)
  {
    OutCurve(xc+r*cos(astart)+myArc*cos(pi2+astart),
             yc-r*sin(astart)-myArc*sin(pi2+astart),
             xc+r*cos(afinish)+myArc*cos(afinish-pi2),
             yc-r*sin(afinish)-myArc*sin(afinish-pi2),
             xc+r*cos(afinish),
             yc-r*sin(afinish));
  }
  else
  {
    afinish = astart + d/4;
    myArc = 4./3. * (1.-cos(d/8))/sin(d/8) * r;
    OutCurve(xc+r*cos(astart)+myArc*cos(pi2+astart),
             yc-r*sin(astart)-myArc*sin(pi2+astart),
             xc+r*cos(afinish)+myArc*cos(afinish-pi2),
             yc-r*sin(afinish)-myArc*sin(afinish-pi2),
             xc+r*cos(afinish),
             yc-r*sin(afinish));
    astart = afinish;
    afinish = astart + d/4;
    OutCurve(xc+r*cos(astart)+myArc*cos(pi2+astart),
             yc-r*sin(astart)-myArc*sin(pi2+astart),
             xc+r*cos(afinish)+myArc*cos(afinish-pi2),
             yc-r*sin(afinish)-myArc*sin(afinish-pi2),
             xc+r*cos(afinish),
             yc-r*sin(afinish));
    astart = afinish;
    afinish = astart + d/4;
    OutCurve(xc+r*cos(astart)+myArc*cos(pi2+astart),
             yc-r*sin(astart)-myArc*sin(pi2+astart),
             xc+r*cos(afinish)+myArc*cos(afinish-pi2),
             yc-r*sin(afinish)-myArc*sin(afinish-pi2),
             xc+r*cos(afinish),
             yc-r*sin(afinish));
    astart = afinish;
    afinish = astart + d/4;
    OutCurve(xc+r*cos(astart)+myArc*cos(pi2+astart),
             yc-r*sin(astart)-myArc*sin(pi2+astart),
             xc+r*cos(afinish)+myArc*cos(afinish-pi2),
             yc-r*sin(afinish)-myArc*sin(afinish-pi2),
             xc+r*cos(afinish),
             yc-r*sin(afinish));
  }
  // terminate drawing
  OutAscii(op);
}

void
wxPdfDocument::Polygon(const wxPdfArrayDouble& x, const wxPdfArrayDouble& y, int style)
{
  int np = (x.GetCount() < y.GetCount()) ? x.GetCount() : y.GetCount();

  wxString op;
  if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILL)
  {
    op = _T("f");
  }
  else
  {
    if ((style & wxPDF_STYLE_FILLDRAW) == wxPDF_STYLE_FILLDRAW)
    {
      op = _T("B");
    }
    else
    {
      op = _T("S");
    }
  }

  OutPoint(x[0], y[0]);
  int i;
  for (i = 1; i < np; i++)
  {
    OutLine(x[i], y[i]);
  }
  OutLine(x[0], y[0]);
  OutAscii(op);
}

void
wxPdfDocument::RegularPolygon(double x0, double y0, double r, int ns, double angle, bool circle, int style, 
                              int circleStyle, const wxPdfLineStyle& circleLineStyle, const wxPdfColour& circleFillColor)
{
  if (ns < 3)
  {
    ns = 3;
  }
  if (circle)
  {
    wxPdfLineStyle saveStyle = GetLineStyle();
    SetLineStyle(circleLineStyle);
    wxPdfColour saveColor = GetFillColor();
    SetFillColor(circleFillColor);
    Circle(x0, y0, r, 0, 360, circleStyle);
    SetLineStyle(saveStyle);
    SetFillColor(saveColor);
  }
  static double pi = 4. * atan(1.);
  double a;
  wxPdfArrayDouble x, y;
  int i;
  for (i = 0; i < ns; i++)
  {
    a = (angle + (i * 360 / ns)) / 180. * pi;
    x.Add(x0 + (r * sin(a)));
    y.Add(y0 + (r * cos(a)));
  }
  Polygon(x, y, style);
}


void
wxPdfDocument::StarPolygon(double x0, double y0, double r, int nv, int ng, double angle, bool circle, int style, 
                           int circleStyle, const wxPdfLineStyle& circleLineStyle, const wxPdfColour& circleFillColor)
{
  if (nv < 2)
  {
    nv = 2;
  }
  if (circle)
  {
    wxPdfLineStyle saveStyle = GetLineStyle();
    SetLineStyle(circleLineStyle);
    wxPdfColour saveColor = GetFillColor();
    SetFillColor(circleFillColor);
    Circle(x0, y0, r, 0, 360, circleStyle);
    SetLineStyle(saveStyle);
    SetFillColor(saveColor);
  }
  wxArrayInt visited;
  visited.SetCount(nv);
  int i;
  for (i = 0; i < nv; i++)
  {
    visited[i] = 0;
  }
  static double pi = 4. * atan(1.);
  double a;
  wxPdfArrayDouble x, y;
  i = 0;
  do
  {
    visited[i] = 1;
    a = (angle + (i * 360 / nv)) / 180. * pi;
    x.Add(x0 + (r * sin(a)));
    y.Add(y0 + (r * cos(a)));
    i = (i + ng) % nv;
  }
  while (visited[i] == 0);
  Polygon(x, y, style);
}

void
wxPdfDocument::Shape(const wxPdfShape& shape, int style, bool alternativeFillRule)
{
  wxString op;
  if ((style & wxPDF_STYLE_MASK) == wxPDF_STYLE_FILL)
  {
    op = _T("f");
  }
  else
  {
    if ((style & wxPDF_STYLE_MASK) == wxPDF_STYLE_FILLDRAW)
    {
      op = _T("B");
    }
    else if ((style & wxPDF_STYLE_MASK) == (wxPDF_STYLE_DRAWCLOSE | wxPDF_STYLE_FILL))
    {
      op = _T("b"); // small 'b' means closing the path as well
    }
    else if ((style & wxPDF_STYLE_MASK) == wxPDF_STYLE_DRAWCLOSE)
    {
      op = _T("s"); // small 's' means closing the path as well
    }
    else
    {
      op = _T("S");
    }
  }

  //if alternative fill rule requested, add the modifier to the command if its f, b or B
  if (alternativeFillRule)
  {
    if(op == _T("f") || op == _T("b") || op == _T("B")) 
    {
      op += _T("*");
    }
  }

  Out("q");

  double scratch[6];
  int iterType;
  int iterPoints = 0;
  int segCount = shape.GetSegmentCount();
  for (iterType = 0; iterType < segCount; iterType++)
  {
    int segType = shape.GetSegment(iterType, iterPoints, scratch);
    switch (segType)
    {
      case wxPDF_SEG_CLOSE:
        Out("h");
        iterPoints++;
        break;
      case wxPDF_SEG_MOVETO:
        OutPoint(scratch[0], scratch[1]);
        iterPoints++;
        break;
      case wxPDF_SEG_LINETO:
        OutLine(scratch[0], scratch[1]);
        iterPoints++;
        break;
      case wxPDF_SEG_CURVETO:
        OutCurve(scratch[0], scratch[1], scratch[2], scratch[3],scratch[4], scratch[5]);
        iterPoints += 3;
        break;
    }
  }
  OutAscii(op);
  Out("Q");

//  ClosePath(style);
}

void
wxPdfDocument::ClippingText(double x, double y, const wxString& txt, bool outline)
{
  wxString op = outline ? _T("5") : _T("7");
  OutAscii(wxString(_T("q BT ")) +
           Double2String(x*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" Td ")) +
           op + wxString(_T(" Tr (")),false);
  TextEscape(txt,false);
  Out(") Tj 0 Tr ET");
}

void
wxPdfDocument::ClippingRect(double x, double y, double w, double h, bool outline)
{
  wxString op = outline ? _T("S") : _T("n");
  OutAscii(wxString(_T("q ")) +
           Double2String(x*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" ")) +
           Double2String(w*m_k,2) + wxString(_T(" ")) +
           Double2String(-h*m_k,2) + wxString(_T(" re W ")) + op);
}

void
wxPdfDocument::ClippingEllipse(double x, double y, double rx, double ry, bool outline)
{
  wxString op = outline ? _T("S") : _T("n");
  if (ry <= 0)
  {
    ry = rx;
  }
  double lx = 4./3. * (sqrt(2.)-1.) * rx;
  double ly = 4./3. * (sqrt(2.)-1.) * ry;

  OutAscii(wxString(_T("q ")) +
           Double2String((x+rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" m ")) +
           Double2String((x+rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y-ly))*m_k,2) + wxString(_T(" ")) +
           Double2String((x+lx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y-ry))*m_k,2) + wxString(_T(" ")) +
           Double2String(x*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y-ry))*m_k,2) + wxString(_T(" c")));

  OutAscii(Double2String((x-lx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y-ry))*m_k,2) + wxString(_T(" ")) +
           Double2String((x-rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y-ly))*m_k,2) + wxString(_T(" ")) +
           Double2String((x-rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" c")));

  OutAscii(Double2String((x-rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+ly))*m_k,2) + wxString(_T(" ")) +
           Double2String((x-lx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+ry))*m_k,2) + wxString(_T(" ")) +
           Double2String(x*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+ry))*m_k,2) + wxString(_T(" c")));

  OutAscii(Double2String((x+lx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+ry))*m_k,2) + wxString(_T(" ")) +
           Double2String((x+rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-(y+ly))*m_k,2) + wxString(_T(" ")) +
           Double2String((x+rx)*m_k,2) + wxString(_T(" ")) +
           Double2String((m_h-y)*m_k,2) + wxString(_T(" c W ")) + op);
}

void
wxPdfDocument::ClippingPolygon(const wxPdfArrayDouble& x, const wxPdfArrayDouble& y, bool outline)
{
  int np = (x.GetCount() < y.GetCount()) ? x.GetCount() : y.GetCount();

  wxString op = outline ? _T("S") : _T("n");

  Out("q");
  OutPoint(x[0], y[0]);
  int i;
  for (i = 1; i < np; i++)
  {
    OutLine(x[i], y[i]);
  }
  OutLine(x[0], y[0]);
  OutAscii(wxString(_T("h W ")) + op);
}

void
wxPdfDocument::ClippingPath()
{
  Out("q");
}

void
wxPdfDocument::MoveTo(double x, double y)
{
  OutPoint(x, y);
}

void
wxPdfDocument::LineTo(double x, double y)
{
  OutLine(x, y);
}

void
wxPdfDocument::CurveTo(double x1, double y1, double x2, double y2, double x3, double y3)
{
  OutCurve(x1, y1, x2, y2, x3, y3);
}

void
wxPdfDocument::ClosePath(int style)
{
  wxString op;
  switch (style)
  {
    case wxPDF_STYLE_DRAW:     op = _T("S"); break;
    case wxPDF_STYLE_FILL:     op = _T("F"); break;
    case wxPDF_STYLE_FILLDRAW: op = _T("B"); break;
    default:                   op = _T("n"); break;
  }
  OutAscii(wxString(_T("h W ")) + op);
}

void
wxPdfDocument::ClippingPath(const wxPdfShape& shape, int style)
{
  ClippingPath();
  double scratch[6];
  int iterType;
  int iterPoints = 0;
  int segCount = shape.GetSegmentCount();
  for (iterType = 0; iterType < segCount; iterType++)
  {
    int segType = shape.GetSegment(iterType, iterPoints, scratch);
    switch (segType)
    {
      case wxPDF_SEG_CLOSE:
        iterPoints++;
        break;
      case wxPDF_SEG_MOVETO:
        MoveTo(scratch[0], scratch[1]);
        iterPoints++;
        break;
      case wxPDF_SEG_LINETO:
        LineTo(scratch[0], scratch[1]);
        iterPoints++;
        break;
      case wxPDF_SEG_CURVETO:
        CurveTo(scratch[0], scratch[1], scratch[2], scratch[3],scratch[4], scratch[5]);
        iterPoints += 3;
        break;
    }
  }
  ClosePath(style);
}

void
wxPdfDocument::UnsetClipping()
{
  Out("Q");
}

void
wxPdfDocument::ClippedCell(double w, double h, const wxString& txt,
                           int border, int ln, int align, int fill, const wxPdfLink& link)
{
  if ((border != wxPDF_BORDER_NONE) || (fill != 0) || (m_y+h > m_pageBreakTrigger))
  {
    Cell(w, h, _T(""), border, 0, wxPDF_ALIGN_LEFT, fill);
    m_x -= w;
  }
  ClippingRect(m_x, m_y, w, h);
  Cell(w, h, txt, wxPDF_BORDER_NONE, ln, align, 0, link);
  UnsetClipping();
}

void
wxPdfDocument::SetLineStyle(const wxPdfLineStyle& linestyle)
{
  m_lineStyle = linestyle;
  if (linestyle.GetWidth() >= 0)
  {
    double width_prev = m_lineWidth;
    SetLineWidth(linestyle.GetWidth());
    m_lineWidth = width_prev;
  }
  switch (linestyle.GetLineCap())
  {
    case wxPDF_LINECAP_BUTT:
    case wxPDF_LINECAP_ROUND:
    case wxPDF_LINECAP_SQUARE:
      OutAscii(wxString::Format(_T("%d  J"), linestyle.GetLineCap()));
      break;
    default:
      break;
  }
  switch (linestyle.GetLineJoin())
  {
    case wxPDF_LINEJOIN_MITER:
    case wxPDF_LINEJOIN_ROUND:
    case wxPDF_LINEJOIN_BEVEL:
      OutAscii(wxString::Format(_T("%d  j"), linestyle.GetLineJoin()));
      break;
    default:
      break;
  }

  if ( (linestyle.GetLineJoin() == wxPDF_LINEJOIN_MITER) && (linestyle.GetMiterLimit() != 0.0) )
  {
    OutAscii(wxString::Format(_T("%f M"), linestyle.GetMiterLimit()));
  }

  const wxPdfArrayDouble& dash = linestyle.GetDash();
  if (&dash != NULL)
  {
    wxString dashString = _T("");
    size_t j;
    for (j = 0; j < dash.GetCount(); j++)
    {
      if (j > 0)
      {
        dashString += wxString(_T(" "));
      }
      dashString += Double2String(dash[j],2);
    }
    double phase = linestyle.GetPhase();
    if (phase < 0 || dashString.Length() == 0)
    {
      phase = 0;
    }
    OutAscii(wxString(_T("[")) + dashString + wxString(_T("] ")) +
             Double2String(phase,2) + wxString(_T(" d")));
  }
  SetDrawColor(linestyle.GetColour());
}

const wxPdfLineStyle&
wxPdfDocument::GetLineStyle()
{
  return m_lineStyle;
}

void
wxPdfDocument::StartTransform()
{
  //save the current graphic state
  m_inTransform++;
  Out("q");
}

bool
wxPdfDocument::ScaleX(double sx, double x, double y)
{
  return Scale(sx, 100, x, y);
}

bool
wxPdfDocument::ScaleY(double sy, double x, double y)
{
  return Scale(100, sy, x, y);
}

bool
wxPdfDocument::ScaleXY(double s, double x, double y)
{
  return Scale(s, s, x, y);
}

bool
wxPdfDocument::Scale(double sx, double sy, double x, double y)
{
  if (x < 0)
  {
    x = m_x;
  }
  if (y < 0)
  {
    y = m_y;
  }
  if (sx == 0 || sy == 0)
  {
    wxLogError(_T("wxPdfDocument::Scale: Please use values unequal to zero for Scaling."));
    return false;
  }
  y = (m_h - y) * m_k;
  x *= m_k;
  //calculate elements of transformation matrix
  sx /= 100;
  sy /= 100;
  double tm[6];
  tm[0] = sx;
  tm[1] = 0;
  tm[2] = 0;
  tm[3] = sy;
  tm[4] = x * (1 - sx);
  tm[5] = y * (1 - sy);
  //scale the coordinate system
  if (m_inTransform == 0)
  {
    StartTransform();
  }
  Transform(tm);
  return true;
}

void
wxPdfDocument::MirrorH(double x)
{
  Scale(-100, 100, x);
}

void
wxPdfDocument::MirrorV(double y)
{
  Scale(100, -100, -1, y);
}

void
wxPdfDocument::TranslateX(double tx)
{
  Translate(tx, 0);
}

void
wxPdfDocument::TranslateY(double ty)
{
  Translate(0, ty);
}

void
wxPdfDocument::Translate(double tx, double ty)
{
  if (m_inTransform == 0)
  {
    StartTransform();
  }
  // calculate elements of transformation matrix
  double tm[6];
  tm[0] = 1;
  tm[1] = 0;
  tm[2] = 0;
  tm[3] = 1;
  tm[4] = tx;
  tm[5] = -ty;
  // translate the coordinate system
  Transform(tm);
}

void
wxPdfDocument::Rotate(double angle, double x, double y)
{
  if (m_inTransform == 0)
  {
    StartTransform();
  }
  if (x < 0)
  {
    x = m_x;
  }
  if (y < 0)
  {
    y = m_y;
  }
  y = (m_h - y) * m_k;
  x *= m_k;
  // calculate elements of transformation matrix
  double tm[6];
  angle *= (atan(1.) / 45.);
  tm[0] = cos(angle);
  tm[1] = sin(angle);
  tm[2] = -tm[1];
  tm[3] = tm[0];
  tm[4] = x + tm[1] * y - tm[0] * x;
  tm[5] = y - tm[0] * y - tm[1] * x;
  //rotate the coordinate system around ($x,$y)
  Transform(tm);
}

bool
wxPdfDocument::SkewX(double xAngle, double x, double y)
{
  return Skew(xAngle, 0, x, y);
}

bool
wxPdfDocument::SkewY(double yAngle, double x, double y)
{
  return Skew(0, yAngle, x, y);
}

bool
wxPdfDocument::Skew(double xAngle, double yAngle, double x, double y)
{
  if (x < 0)
  {
    x = m_x;
  }
  if (y < 0)
  {
    y = m_y;
  }
  if (xAngle <= -90 || xAngle >= 90 || yAngle <= -90 || yAngle >= 90)
  {
    wxLogError(_T("wxPdfDocument::Skew: Please use values between -90 and 90 degree for skewing."));
    return false;
  }
  x *= m_k;
  y = (m_h - y) * m_k;
  //calculate elements of transformation matrix
  double tm[6];
  xAngle *= (atan(1.) / 45.);
  yAngle *= (atan(1.) / 45.);
  tm[0] = 1;
  tm[1] = tan(yAngle);
  tm[2] = tan(xAngle);
  tm[3] = 1;
  tm[4] = -tm[2] * y;
  tm[5] = -tm[1] * x;
  //skew the coordinate system
  if (m_inTransform == 0)
  {
    StartTransform();
  }
  Transform(tm);
  return true;
}

void
wxPdfDocument::StopTransform()
{
  //restore previous graphic state
  if (m_inTransform > 0)
  {
    m_inTransform--;
    Out("Q");

    //hack to make next font change work, see https://sourceforge.net/tracker/?func=detail&atid=462816&aid=1861202&group_id=51305
    m_fontFamily = wxEmptyString;
    //hack to make colour change work, related to same bug as above
    m_fillColor = wxPdfColour(1,2,3);
  }
}

static bool
ColorSpaceOk(const wxPdfColour& col1, const wxPdfColour& col2)
{
  return (col1.GetColorType() != wxPDF_COLOURTYPE_SPOT &&
          col1.GetColorType() == col2.GetColorType());
}

int
wxPdfDocument::LinearGradient(const wxPdfColour& col1, const wxPdfColour& col2,
                              wxPdfLinearGradientType gradientType)
{
  static double h[] = { 0, 0, 1, 0 };
  static double v[] = { 0, 0, 0, 1 };
  wxPdfGradient* gradient;

  int n = 0;
  if (ColorSpaceOk(col1, col2))
  {
    switch (gradientType)
    {
      case wxPDF_LINEAR_GRADIENT_REFLECTION_TOP:
        gradient = new wxPdfMidAxialGradient(col1, col2, v[0], v[1], v[2], v[3], 0.67, 0.7);
        break;
      case wxPDF_LINEAR_GRADIENT_REFLECTION_BOTTOM:
        gradient = new wxPdfMidAxialGradient(col1, col2, v[0], v[1], v[2], v[3], 0.33, 0.7);
        break;
      case wxPDF_LINEAR_GRADIENT_REFLECTION_LEFT:
        gradient = new wxPdfMidAxialGradient(col1, col2, h[0], h[1], h[2], h[3], 0.33, 0.7);
        break;
      case wxPDF_LINEAR_GRADIENT_REFLECTION_RIGHT:
        gradient = new wxPdfMidAxialGradient(col1, col2, h[0], h[1], h[2], h[3], 0.67, 0.7);
        break;
      case wxPDF_LINEAR_GRADIENT_MIDVERTICAL:
        gradient = new wxPdfMidAxialGradient(col1, col2, v[0], v[1], v[2], v[3], 0.5, 1);
        break;
      case wxPDF_LINEAR_GRADIENT_MIDHORIZONTAL:
        gradient = new wxPdfMidAxialGradient(col1, col2, h[0], h[1], h[2], h[3], 0.5, 1);
        break;
      case wxPDF_LINEAR_GRADIENT_VERTICAL:
        gradient = new wxPdfAxialGradient(col1, col2, v[0], v[1], v[2], v[3], 1);
        break;
      case wxPDF_LINEAR_GRADIENT_HORIZONTAL:
      default:
        gradient = new wxPdfAxialGradient(col1, col2, h[0], h[1], h[2], h[3], 1);
        break;
    }
    n = (*m_gradients).size()+1;
    (*m_gradients)[n] = gradient;
  }
  else
  {
    wxLogError(_("wxPdfDocument::LinearGradient: Color spaces do not match."));
  }
  return n;
}

int
wxPdfDocument::AxialGradient(const wxPdfColour& col1, const wxPdfColour& col2,
                             double x1, double y1, double x2, double y2,
                             double intexp)
{
  int n = 0;
  if (ColorSpaceOk(col1, col2))
  {
    n = (*m_gradients).size()+1;
    (*m_gradients)[n] = new wxPdfAxialGradient(col1, col2, x1, y1, x2, y2, intexp);
  }
  else
  {
    wxLogError(_("wxPdfDocument::LinearGradient: Color spaces do not match."));
  }
  return n;
}

int
wxPdfDocument::MidAxialGradient(const wxPdfColour& col1, const wxPdfColour& col2,
                               double x1, double y1, double x2, double y2,
                               double midpoint, double intexp)
{
  int n = 0;
  if (ColorSpaceOk(col1, col2))
  {
    n = (*m_gradients).size()+1;
    (*m_gradients)[n] = new wxPdfMidAxialGradient(col1, col2, x1, y1, x2, y2, midpoint, intexp);
  }
  else
  {
    wxLogError(_("wxPdfDocument::LinearGradient: Color spaces do not match."));
  }
  return n;
}

int
wxPdfDocument::RadialGradient(const wxPdfColour& col1, const wxPdfColour& col2,
                              double x1, double y1, double r1,
                              double x2, double y2, double r2, double intexp)
{
  int n = 0;
  if (ColorSpaceOk(col1, col2))
  {
    n = (*m_gradients).size()+1;
    (*m_gradients)[n] = new wxPdfRadialGradient(col1, col2, x1, y1, r1, x2, y2, r2, intexp);
  }
  else
  {
    wxLogError(_("wxPdfDocument::RadialGradient: Color spaces do not match."));
  }
  return n;
}

int
wxPdfDocument::CoonsPatchGradient(const wxPdfCoonsPatchMesh& mesh, double minCoord, double maxCoord)
{
  int n = 0;
  if (mesh.Ok())
  {
    n = (*m_gradients).size()+1;
    (*m_gradients)[n] = new wxPdfCoonsPatchGradient(mesh, minCoord, maxCoord);
  }
  else
  {
    wxLogError(_("wxPdfDocument::CoonsPatchGradient: Mesh is invalid."));
  }
  return n;
}

/* draw a marker at a raw point-based coordinate */
void
wxPdfDocument::Marker(double x, double y, wxPdfMarker markerType, double size)
{
  double saveLineWidth = m_lineWidth;
  double halfsize = size * 0.5;
  static double b = 4. / 3.;

  Out("q");
  switch (markerType) 
  {
    case wxPDF_MARKER_CIRCLE:
      SetLineWidth(size * 0.15);
      OutPoint(x - halfsize, y);
      OutCurve(x - halfsize, y + b * halfsize, x + halfsize, y + b * halfsize, x + halfsize, y);
      OutCurve(x + halfsize, y - b * halfsize, x - halfsize, y - b * halfsize, x - halfsize, y);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_TRIANGLE_UP:
      SetLineWidth(size * 0.15);
      OutPoint(x, y - size * 0.6667);
      OutLineRelative(-size / 1.7321, size);
      OutLineRelative(1.1546 * size, 0.0);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_TRIANGLE_DOWN:
      SetLineWidth(size * 0.15);
      OutPoint(x, y + size * 0.6667);
      OutLineRelative(-size / 1.7321, -size);
      OutLineRelative(1.1546 * size, 0.0);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_TRIANGLE_LEFT:
      SetLineWidth(size * 0.15);
      OutPoint(x - size * 0.6667, y);
      OutLineRelative(size, -size / 1.7321);
      OutLineRelative(0.0, 1.1546 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_TRIANGLE_RIGHT:
      SetLineWidth(size * 0.15);
      OutPoint(x + size * 0.6667, y);
      OutLineRelative(-size, -size / 1.7321);
      OutLineRelative(0.0, 1.1546 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_DIAMOND:
      SetLineWidth(size * 0.15);
      size *= 0.9;
      OutPoint( x, y+size/1.38);
      OutLineRelative( 0.546 * size, -size / 1.38);
      OutLineRelative(-0.546 * size, -size / 1.38);
      OutLineRelative(-0.546 * size,  size / 1.38);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_SQUARE:
      SetLineWidth(size * 0.15);
      Rect(x - halfsize, y - halfsize, size, size, wxPDF_STYLE_FILLDRAW);
      Out("B");
      break;
    case wxPDF_MARKER_STAR:
      size *= 1.2;
      halfsize = 0.5 * size;
      SetLineWidth(size * 0.09);
      OutPoint(x, y + size * 0.5);
      OutLine(x + 0.112255 * size, y + 0.15451 * size);
      OutLine(x + 0.47552  * size, y + 0.15451 * size);
      OutLine(x + 0.181635 * size, y - 0.05902 * size);
      OutLine(x + 0.29389  * size, y - 0.40451 * size);
      OutLine(x, y - 0.19098 * size);
      OutLine(x - 0.29389  * size, y - 0.40451 * size);
      OutLine(x - 0.181635 * size, y - 0.05902 * size);
      OutLine(x - 0.47552  * size, y + 0.15451 * size);
      OutLine(x - 0.112255 * size, y + 0.15451 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_STAR4:
      size *= 1.2;
      halfsize = 0.5 * size;
      SetLineWidth(size * 0.09);
      OutPoint(x, y + size * 0.5);
      OutLine(x + 0.125 * size, y + 0.125 * size);
      OutLine(x + size * 0.5, y);
      OutLine(x + 0.125 * size, y - 0.125 * size);
      OutLine(x, y - size * 0.5);
      OutLine(x - 0.125 * size, y - 0.125 * size);
      OutLine(x - size * 0.5, y);
      OutLine(x - 0.125 * size, y + 0.125 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_PLUS:
      size *= 1.2;
      halfsize = 0.5 * size;
      SetLineWidth(size * 0.1);
      OutPoint(x + 0.125 * size, y + size * 0.5);
      OutLine(x + 0.125 * size, y + 0.125 * size);
      OutLine(x + size * 0.5, y + 0.125 * size);
      OutLine(x + size * 0.5, y - 0.125 * size);
      OutLine(x + 0.125 * size, y - 0.125 * size);
      OutLine(x + 0.125 * size, y - size * 0.5);
      OutLine(x - 0.125 * size, y - size * 0.5);
      OutLine(x - 0.125 * size, y - 0.125 * size);
      OutLine(x - size * 0.5, y - 0.125 * size);
      OutLine(x - size * 0.5, y + 0.125 * size);
      OutLine(x - 0.125 * size, y + 0.125 * size);
      OutLine(x - 0.125 * size, y + size * 0.5);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_CROSS:
      size *= 1.2;
      halfsize = 0.5 * size;
      SetLineWidth(size * 0.1);
      OutPoint(x, y + 0.176777 * size);
      OutLine(x + 0.265165 * size, y + 0.441941 * size);
      OutLine(x + 0.441941 * size, y + 0.265165 * size);
      OutLine(x + 0.176777 * size, y);
      OutLine(x + 0.441941 * size, y - 0.265165 * size);
      OutLine(x + 0.265165 * size, y - 0.441941 * size);
      OutLine(x, y - 0.176777 * size);
      OutLine(x - 0.265165 * size, y - 0.441941 * size);
      OutLine(x - 0.441941 * size, y - 0.265165 * size);
      OutLine(x - 0.176777 * size, y);
      OutLine(x - 0.441941 * size, y + 0.265165 * size);
      OutLine(x - 0.265165 * size, y + 0.441941 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_PENTAGON_UP:
      SetLineWidth(size * 0.15);
      OutPoint(x + 0.5257 * size, y - size * 0.1708);
      OutLineRelative(-0.5257 * size, -0.382  * size);
      OutLineRelative(-0.5257 * size, 0.382  * size);
      OutLineRelative(0.2008 * size, 0.6181 * size);
      OutLineRelative(0.6499 * size,  0.0);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_PENTAGON_DOWN:
      SetLineWidth(size * 0.15);
      OutPoint(x - 0.5257 * size, y + size * 0.1708);
      OutLineRelative( 0.5257 * size,  0.382  * size);
      OutLineRelative( 0.5257 * size, -0.382  * size);
      OutLineRelative(-0.2008 * size, -0.6181 * size);
      OutLineRelative(-0.6499 * size,  0.0);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_PENTAGON_LEFT:
      SetLineWidth(size * 0.15);
      OutPoint(x - size * 0.1708, y + 0.5257 * size);
      OutLineRelative(-0.382  * size, -0.5257 * size);
      OutLineRelative( 0.382  * size, -0.5257 * size);
      OutLineRelative( 0.6181 * size,  0.2008 * size);
      OutLineRelative( 0.0,            0.6499 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_PENTAGON_RIGHT:
      SetLineWidth(size * 0.15);
      OutPoint(x + size * 0.1708, y - 0.5257 * size);
      OutLineRelative( 0.382  * size,  0.5257 * size);
      OutLineRelative(-0.382  * size,  0.5257 * size);
      OutLineRelative(-0.6181 * size, -0.2008 * size);
      OutLineRelative( 0.0,           -0.6499 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_BOWTIE_HORIZONTAL:
      SetLineWidth(size * 0.13);
      OutPoint(x - 0.5 * size, y - 0.5 * size);
      OutLine(x + 0.5 * size, y + 0.5 * size);
      OutLine(x + 0.5 * size, y - 0.5 * size);
      OutLine(x - 0.5 * size, y + 0.5 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_BOWTIE_VERTICAL:
      SetLineWidth(size * 0.13);
      OutPoint(x - 0.5 * size, y - 0.5 * size);
      OutLine(x + 0.5 * size, y + 0.5 * size);
      OutLine(x - 0.5 * size, y + 0.5 * size);
      OutLine(x + 0.5 * size, y - 0.5 * size);
      Out("h");
      Out("B");
      break;
    case wxPDF_MARKER_ASTERISK:
      size *= 1.05;
      SetLineWidth(size * 0.15);
      OutPoint( x, y + size * 0.5);
      OutLineRelative(0.0, -size);
      OutPoint( x + 0.433 * size, y + 0.25 * size);
      OutLine(x - 0.433 * size, y - 0.25 * size);
      OutPoint(x + 0.433 * size, y - 0.25 * size);
      OutLine(x - 0.433 * size, y + 0.25 * size);
      Out("S");
      break;
    case wxPDF_MARKER_SUN:
      SetLineWidth(size * 0.15);
      halfsize = size * 0.25;
      OutPoint(x - halfsize, y);
      OutCurve(x - halfsize, y + b * halfsize, x + halfsize, y + b * halfsize, x + halfsize, y);
      OutCurve(x + halfsize, y - b * halfsize, x - halfsize, y - b * halfsize, x - halfsize, y);
      Out("h");
      OutPoint(x + size * 0.5, y);
      OutLine(x + size * 0.25, y);
      OutPoint(x - size * 0.5, y);
      OutLine(x - size * 0.25, y);
      OutPoint(x, y - size * 0.5);
      OutLine(x, y - size * 0.25);
      OutPoint(x, y + size * 0.5);
      OutLine(x, y + size * 0.25);
      Out("B");
      break;

    default:
      break;
  }
  Out("Q");
  m_x = x;
  m_y = y;
  SetLineWidth(saveLineWidth);
}

void
wxPdfDocument::Arrow(double x1, double y1, double x2, double y2, double linewidth, double height, double width)
{
  double saveLineWidth = m_lineWidth;
  double dx = x2 - x1;
  double dy = y2 - y1;
  double dz = sqrt (dx*dx+dy*dy);
  double sina = dy / dz;
  double cosa = dx / dz;
  double x3 = x2 - cosa * height + sina * width;
  double y3 = y2 - sina * height - cosa * width;
  double x4 = x2 - cosa * height - sina * width;
  double y4 = y2 - sina * height + cosa * width;

  SetLineWidth(0.2);

  //Draw a arrow head
  OutAscii(Double2String( x2*m_k,2) + wxString(_T(" ")) +
           Double2String( (m_h-y2)*m_k,2) + wxString(_T(" m ")) +
           Double2String( x3*m_k,2) + wxString(_T(" ")) +
           Double2String( (m_h-y3)*m_k,2) + wxString(_T(" l ")) +
           Double2String( x4*m_k,2) + wxString(_T(" ")) +
           Double2String( (m_h-y4)*m_k,2) + wxString(_T(" l b")));

  SetLineWidth(linewidth);
  Line(x1+cosa*linewidth, y1+sina*linewidth, x2-cosa*height, y2-sina*height);
  SetLineWidth(saveLineWidth);
}
