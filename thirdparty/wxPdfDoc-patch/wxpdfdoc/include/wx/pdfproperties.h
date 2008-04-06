///////////////////////////////////////////////////////////////////////////////
// Name:        pdfproperties.h
// Purpose:     
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-07-13
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfproperties.h Interface of the several wxPdfDocument property classes

#ifndef _PDFPROPERTIES_H_
#define _PDFPROPERTIES_H_

// wxWidgets headers
#include "wx/pdfdocdef.h"

/// Border options
#define wxPDF_BORDER_NONE    0x0000
#define wxPDF_BORDER_LEFT    0x0001
#define wxPDF_BORDER_RIGHT   0x0002
#define wxPDF_BORDER_TOP     0x0004
#define wxPDF_BORDER_BOTTOM  0x0008
#define wxPDF_BORDER_FRAME   0x000F

/// Corner options
#define wxPDF_CORNER_NONE          0x0000
#define wxPDF_CORNER_TOP_LEFT      0x0001
#define wxPDF_CORNER_TOP_RIGHT     0x0002
#define wxPDF_CORNER_BOTTOM_LEFT   0x0004
#define wxPDF_CORNER_BOTTOM_RIGHT  0x0008
#define wxPDF_CORNER_ALL           0x000F

/// Style options
#define wxPDF_STYLE_NOOP      0x0000
#define wxPDF_STYLE_DRAW      0x0001
#define wxPDF_STYLE_FILL      0x0002
#define wxPDF_STYLE_FILLDRAW  0x0003
#define wxPDF_STYLE_DRAWCLOSE 0x0004
#define wxPDF_STYLE_MASK      0x0007

/// Font decoration options
#define wxPDF_FONT_NORMAL     0x0000
#define wxPDF_FONT_UNDERLINE  0x0001
#define wxPDF_FONT_OVERLINE   0x0002
#define wxPDF_FONT_STRIKEOUT  0x0004
#define wxPDF_FONT_DECORATION 0x0007 // Mask all possible decorations

/// Permission options
#define wxPDF_PERMISSION_NONE   0x0000  ///< Allow nothing
#define wxPDF_PERMISSION_PRINT  0x0004  ///< Allow printing
#define wxPDF_PERMISSION_MODIFY 0x0008  ///< Allow modifying
#define wxPDF_PERMISSION_COPY   0x0010  ///< Allow text copying
#define wxPDF_PERMISSION_ANNOT  0x0020  ///< Allow annotations
#define wxPDF_PERMISSION_ALL    0x003C  ///< Allow anything

/// Encryption methods
enum wxPdfEncryptionMethod
{
  wxPDF_ENCRYPTION_RC4V1,
  wxPDF_ENCRYPTION_RC4V2,
  wxPDF_ENCRYPTION_AESV2
};

/// Color types
enum wxPdfColourType
{
  wxPDF_COLOURTYPE_UNKNOWN,
  wxPDF_COLOURTYPE_GRAY,
  wxPDF_COLOURTYPE_RGB,
  wxPDF_COLOURTYPE_CMYK,
  wxPDF_COLOURTYPE_SPOT
};

/// Form field border styles
enum wxPdfBorderStyle
{
  wxPDF_BORDER_SOLID,
  wxPDF_BORDER_DASHED,
  wxPDF_BORDER_BEVELED,
  wxPDF_BORDER_INSET,
  wxPDF_BORDER_UNDERLINE
};

/// Alignment options
enum wxPdfAlignment
{
  wxPDF_ALIGN_LEFT,
  wxPDF_ALIGN_CENTER,
  wxPDF_ALIGN_RIGHT,
  wxPDF_ALIGN_JUSTIFY,
  wxPDF_ALIGN_TOP    = wxPDF_ALIGN_LEFT,
  wxPDF_ALIGN_MIDDLE = wxPDF_ALIGN_CENTER,
  wxPDF_ALIGN_BOTTOM = wxPDF_ALIGN_RIGHT
};

/// Zoom options
enum wxPdfZoom
{
  wxPDF_ZOOM_FULLPAGE,
  wxPDF_ZOOM_FULLWIDTH,
  wxPDF_ZOOM_REAL,
  wxPDF_ZOOM_DEFAULT,
  wxPDF_ZOOM_FACTOR
};

/// Layout options
enum wxPdfLayout
{
  wxPDF_LAYOUT_CONTINUOUS,
  wxPDF_LAYOUT_SINGLE,
  wxPDF_LAYOUT_TWO,
  wxPDF_LAYOUT_DEFAULT
};

/// Viewer preferences
#define wxPDF_VIEWER_HIDETOOLBAR     0x0001
#define wxPDF_VIEWER_HIDEMENUBAR     0x0002
#define wxPDF_VIEWER_HIDEWINDOWUI    0x0004
#define wxPDF_VIEWER_FITWINDOW       0x0008
#define wxPDF_VIEWER_CENTERWINDOW    0x0010
#define wxPDF_VIEWER_DISPLAYDOCTITLE 0x0020

/// Line Cap options
enum wxPdfLineCap
{
  wxPDF_LINECAP_NONE   = -1,
  wxPDF_LINECAP_BUTT   = 0,
  wxPDF_LINECAP_ROUND  = 1,
  wxPDF_LINECAP_SQUARE = 2
};

/// Line join options
enum wxPdfLineJoin
{
  wxPDF_LINEJOIN_NONE  = -1,
  wxPDF_LINEJOIN_MITER = 0,
  wxPDF_LINEJOIN_ROUND = 1,
  wxPDF_LINEJOIN_BEVEL = 2
};

/// Marker symbols
enum wxPdfMarker
{
  wxPDF_MARKER_CIRCLE,
  wxPDF_MARKER_SQUARE,
  wxPDF_MARKER_TRIANGLE_UP,
  wxPDF_MARKER_TRIANGLE_DOWN,
  wxPDF_MARKER_TRIANGLE_LEFT,
  wxPDF_MARKER_TRIANGLE_RIGHT,
  wxPDF_MARKER_DIAMOND,
  wxPDF_MARKER_PENTAGON_UP,
  wxPDF_MARKER_PENTAGON_DOWN,
  wxPDF_MARKER_PENTAGON_LEFT,
  wxPDF_MARKER_PENTAGON_RIGHT,
  wxPDF_MARKER_STAR,
  wxPDF_MARKER_STAR4,
  wxPDF_MARKER_PLUS,
  wxPDF_MARKER_CROSS,
  wxPDF_MARKER_SUN,
  wxPDF_MARKER_BOWTIE_HORIZONTAL,
  wxPDF_MARKER_BOWTIE_VERTICAL,
  wxPDF_MARKER_ASTERISK,
  wxPDF_MARKER_LAST  // Marks the last available marker symbol; do not use!
};

/// Linear gradient types
enum wxPdfLinearGradientType
{
  wxPDF_LINEAR_GRADIENT_HORIZONTAL,
  wxPDF_LINEAR_GRADIENT_VERTICAL,
  wxPDF_LINEAR_GRADIENT_MIDHORIZONTAL,
  wxPDF_LINEAR_GRADIENT_MIDVERTICAL,
  wxPDF_LINEAR_GRADIENT_REFLECTION_LEFT,
  wxPDF_LINEAR_GRADIENT_REFLECTION_RIGHT,
  wxPDF_LINEAR_GRADIENT_REFLECTION_TOP,
  wxPDF_LINEAR_GRADIENT_REFLECTION_BOTTOM
};

enum wxPdfBlendMode
{
  wxPDF_BLENDMODE_NORMAL,
  wxPDF_BLENDMODE_MULTIPLY, 
  wxPDF_BLENDMODE_SCREEN, 
  wxPDF_BLENDMODE_OVERLAY, 
  wxPDF_BLENDMODE_DARKEN, 
  wxPDF_BLENDMODE_LIGHTEN, 
  wxPDF_BLENDMODE_COLORDODGE, 
  wxPDF_BLENDMODE_COLORBURN,
  wxPDF_BLENDMODE_HARDLIGHT,
  wxPDF_BLENDMODE_SOFTLIGHT,
  wxPDF_BLENDMODE_DIFFERENCE, 
  wxPDF_BLENDMODE_EXCLUSION, 
  wxPDF_BLENDMODE_HUE, 
  wxPDF_BLENDMODE_SATURATION, 
  wxPDF_BLENDMODE_COLOR, 
  wxPDF_BLENDMODE_LUMINOSITY
};

enum wxPdfShapedTextMode
{
  wxPDF_SHAPEDTEXTMODE_ONETIME,
  wxPDF_SHAPEDTEXTMODE_STRETCHTOFIT,
  wxPDF_SHAPEDTEXTMODE_REPEAT
};

/// Class representing a PDF document information dictionary.
class WXDLLIMPEXP_PDFDOC wxPdfInfo
{
public:
  wxPdfInfo() {}
  virtual~wxPdfInfo() {}

  void SetTitle(const wxString& title) { m_title = title; }
  void SetAuthor(const wxString& author) { m_author = author; }
  void SetSubject(const wxString& subject) { m_subject = subject; }
  void SetKeywords(const wxString& keywords) { m_keywords = keywords; }
  void SetCreator(const wxString& creator) { m_creator = creator; }
  void SetProducer(const wxString& producer) { m_producer = producer; }
  void SetCreationDate(const wxString& creationDate) { m_creationDate = creationDate; }
  void SetModDate(const wxString& modDate) { m_modDate = modDate; }

  const wxString GetTitle() const { return m_title; }
  const wxString GetAuthor() const { return m_author; }
  const wxString GetSubject() const { return m_subject; }
  const wxString GetKeywords() const { return m_keywords; }
  const wxString GetCreator() const { return m_creator; }
  const wxString GetProducer() const { return m_producer; }
  const wxString GetCreationDate() const { return m_creationDate; }
  const wxString GetModDate() const { return m_modDate; }

private:
  wxString m_title;        ///< The document’s title.
  wxString m_author;       ///< The name of the person who created the document.
  wxString m_subject;      ///< The subject of the document.
  wxString m_keywords;     ///< Keywords associated with the document.
  wxString m_creator;      ///< The name of the application that created the original document.
  wxString m_producer;     ///< The name of the application that produced the document.
  wxString m_creationDate; ///< The date and time the document was created.
  wxString m_modDate;      ///< The date and time the document was modified.
};

/// Class representing internal or external links.
class WXDLLIMPEXP_PDFDOC wxPdfLink
{
public:
  /// Constructor for internal link
  /**
  * Use this constructor to create an \b internal link reference.
  * \see wxPdfDocument::Link(), wxPdfDocument::Write(), wxPdfDocument::Cell(), wxPdfDocument::ClippedCell(), wxPdfDocument::Image(), wxPdfDocument::RotatedImage()
  */
  wxPdfLink(int linkRef);

  /// Constructor for external link
  /**
  * Use this constructor to create an \b external link reference.
  * \see wxPdfDocument::Link(), wxPdfDocument::Write(), wxPdfDocument::Cell(), wxPdfDocument::ClippedCell(), wxPdfDocument::Image(), wxPdfDocument::RotatedImage()
  */
  wxPdfLink(const wxString& linkURL);

  /// Copy constructor
  wxPdfLink(const wxPdfLink& pdfLink);

  /// Destructor
  virtual ~wxPdfLink();

  /// Check whether this instance is a valid link reference
  bool  IsValid() const { return m_isValid; }

  /// Check whether this instance is an internal reference
  bool  IsLinkRef() const { return m_isRef; }

  /// Get the internal link reference
  int   GetLinkRef() const { return m_linkRef; }

  /// Get the external link reference
  const wxString GetLinkURL() const { return m_linkURL; }

  /// Set page number and position on page
  void   SetLink(int page, double ypos) { m_page = page; m_ypos = ypos; }

  /// Get the page this link refers to
  int    GetPage() { return m_page; }

  /// Get the page position this link refers to
  double GetPosition() { return m_ypos; }

private:
  bool     m_isValid;   ///< Flag whether this instance is valid
  bool     m_isRef;     ///< Flag whether this is an internal link reference
  int      m_linkRef;   ///< Internal link reference
  wxString m_linkURL;   ///< External link reference
  int      m_page;      ///< Page number this link refers to
  double   m_ypos;      ///< Position on page this link refers to
};

/// Class representing the sensitive area of links referring to a page. (For internal use only)
class WXDLLIMPEXP_PDFDOC wxPdfPageLink : public wxPdfLink
{
public:
  /// Constructor
  wxPdfPageLink(double x, double y, double w, double h, const wxPdfLink& pdfLink);

  /// Destructor
  virtual ~wxPdfPageLink();

  /// Get the X offset
  double GetX() { return m_x; }

  /// Get the Y offset
  double GetY() { return m_y; }

  /// Get the width
  double GetWidth() { return m_w; }

  /// Get the height
  double GetHeight() { return m_h; }

private:
  double m_x;   ///< X offset of sensitive area
  double m_y;   ///< Y offset of sensitive area
  double m_w;   ///< Width of sensitive area
  double m_h;   ///< Height of sensitive area
};

/// Class representing text annotations.
class WXDLLIMPEXP_PDFDOC wxPdfAnnotation
{
public:
  /// Constructor for text annotation
  /**
  * Use this constructor to create a text annotation.
  * \param x X offset of the annotation
  * \param y Y offset of the annotation
  * \param text annotation text
  */
  wxPdfAnnotation(double x, double y, const wxString& text);

  /// Copy constructor
  wxPdfAnnotation(const wxPdfAnnotation& annotation);

  /// Destructor
  virtual ~wxPdfAnnotation() {}

  /// Get the X offset of the annotation
  double GetX() const { return m_x; }

  /// Get the Y offset of the annotation
  double GetY() const { return m_y; }

  /// Get the text of the annotation
  wxString GetText() const { return m_text; }

private:
  double   m_x;     ///< X offset of the annotation
  double   m_y;     ///< Y offset of the annotation
  wxString m_text;  ///< Annotation text
};

/// Class representing bookmarks for defining the document's outline. (For internal use only)
class WXDLLIMPEXP_PDFDOC wxPdfBookmark
{
public:
  /// Constructor
  wxPdfBookmark(const wxString& txt, int level, double y, int page);

  /// Destructor
  virtual ~wxPdfBookmark();

  /// Get the bookmark text
  wxString GetText() { return m_text; }

  /// Get the associated level
  int GetLevel() { return m_level; }

  /// Get the Y offset of the bookmark
  double GetY() { return m_y; }

  /// Get the page number of the bookmark
  int GetPage() { return m_page; }

  /// Set the parent of the bookmark
  void SetParent(int parent) { m_parent = parent; }

  /// Get the parent of the bookmark
  int GetParent() { return m_parent; }

  /// Set previous bookmark
  void SetPrev(int prev) { m_prev = prev; }

  /// Get previous bookmark
  int GetPrev() { return m_prev; }

  /// Set next bookmark
  void SetNext(int next) { m_next = next; }

  /// Get next bookmark
  int GetNext() { return m_next; }

  /// Set first bookmark
  void SetFirst(int first) { m_first = first; }

  /// Get first bookmark
  int GetFirst() { return m_first; }

  /// Set last bookmark
  void SetLast(int last) { m_last = last; }

  /// Get last bookmark
  int GetLast() { return m_last; }

private:
  wxString m_text;    ///< Text of bookmark
  int      m_level;   ///< Associated level
  double   m_y;       ///< Y offset
  int      m_page;    ///< Page number
  int      m_parent;  ///< Parent bookmark
  int      m_prev;    ///< Previous bookmark
  int      m_next;    ///< Next bookmark
  int      m_first;   ///< First bookmark
  int      m_last;    ///< Last bookmark
};

/// Class representing spot colors.
class WXDLLIMPEXP_PDFDOC wxPdfSpotColour
{
public:
  /// Constructor for spot color
  wxPdfSpotColour(int index, double cyan, double magenta, double yellow, double black);

  /// Copy constructor
  wxPdfSpotColour(const wxPdfSpotColour& color);

  /// Set object index
  void SetObjIndex(int index) { m_objIndex = index; }

  /// Get object index
  int  GetObjIndex() const { return m_objIndex; }

  /// Get spot color index
  int    GetIndex() const { return m_index; }

  /// Get cyan level
  double GetCyan() const { return m_cyan; }

  /// Get magenta level
  double GetMagenta() const { return m_magenta; }

  /// Get yellow level
  double GetYellow() const { return m_yellow; }

  /// Get black level
  double GetBlack() const { return m_black; }

private:
  int    m_objIndex;   ///< object index
  int    m_index;      ///< color index
  double m_cyan;       ///< cyan level
  double m_magenta;    ///< magenta level
  double m_yellow;     ///< yellow level
  double m_black;      ///< black level
};


/// Class representing patterns.
// Implementation in pdfcolor.cpp
class WXDLLIMPEXP_PDFDOC wxPdfPattern
{
public:
  /// Constructor for pattern
  /**
  * \param index The pattern index
  * \param width The image width
  * \param height The image height
  */
  wxPdfPattern(const int index, const double width, const double height);

  /// Copy constructor
  wxPdfPattern(const wxPdfPattern& pattern);

  /// Set object index
  void SetObjIndex(int index) { m_objIndex = index; };

  /// Get object index
  int GetObjIndex() const { return m_objIndex; };

  /// Get pattern index
  int GetIndex() const { return m_index; };

  /// Set image
  void SetImage(wxPdfImage *image) { m_image = image; };

  /// Get image
  wxPdfImage *GetImage() const {return m_image; };

  /// Get image width
  int GetImageWidth() const {return m_imageWidth; };

  /// Get image height
  int GetImageHeight() const {return m_imageHeight; };

private:
  int    m_objIndex;   ///< object index
  int    m_index;      ///< pattern index
  wxPdfImage *m_image; ///< image
  double m_imageWidth; ///< image width
  double m_imageHeight;///< image height
};


/// Class representing wxPdfDocument colors.
class WXDLLIMPEXP_PDFDOC wxPdfColour
{
public:
  /// Default constructor
  /**
  * Constructs a color object with an undefined color
  */
  wxPdfColour();

  /// Constructor for grayscale color
  /**
  * Defines a grayscale color
  * \param grayscale indicates the gray level. Value between 0 and 255
  */
  wxPdfColour(const unsigned char grayscale);
  
  /// Constructor for wxColour color
  /**
  * Defines a wxColour color.
  * \param color defines a wxColour color composed of a red, green and blue component
  */
  wxPdfColour(const wxColour& color);

  /// Constructor for RGB color
  /**
  * Defines a RGB color.
  * \param red indicates the red level. Value between 0 and 255
  * \param green indicates the green level. Value between 0 and 255
  * \param blue indicates the blue level. Value between 0 and 255
  */
  wxPdfColour(const unsigned char red, const unsigned char green, const unsigned char blue);
  
  /// Constructor for CMYK color
  /**
  * Defines a CMYK color.
  * \param cyan indicates the cyan level. Value between 0 and 100
  * \param magenta indicates the magenta level. Value between 0 and 100
  * \param yellow indicates the yellow level. Value between 0 and 100
  * \param black indicates the black level. Value between 0 and 100
  */
  wxPdfColour(double cyan, double magenta, double yellow, double black);
  
  /// Constructor for named RGB color
  /**
  * Defines a named RGB color.
  * \param name is the name of the requested color. Use of HTML notation <b><tt>\#rrggbb</tt></b> as color name is also supported.
  */
  wxPdfColour(const wxString& name);
  
  /// Constructor for named RGB color
  /**
  * Defines a spot color.
  * \param spotColor is the spot color to be used
  * \param tint indicates the tint level. Value between 0 and 100. Default: 100.
  */
  wxPdfColour(const wxPdfSpotColour& spotColor, double tint);

  /// Copy constructor
  wxPdfColour(const wxPdfColour& color);

  /// Assignment operator
  wxPdfColour& operator=(const wxPdfColour& color);

  /// Set grayscale color
  /**
  * \param grayscale indicates the gray level. Value between 0 and 255. Default: 0 (Black).
  */
  void SetColor(const unsigned char grayscale = 0);
  
  /// Set wxColour color
  /**
  * \param color defines a wxColour color composed of a red, green and blue component
  */
  void SetColor(const wxColour& color);
  
  /// Set RGB color
  /**
  * \param red indicates the red level. Value between 0 and 255
  * \param green indicates the green level. Value between 0 and 255
  * \param blue indicates the blue level. Value between 0 and 255
  */
  void SetColor(const unsigned char red, const unsigned char green, const unsigned char blue);
  
  /// Set CMYK color
  /**
  * \param cyan indicates the cyan level. Value between 0 and 100
  * \param magenta indicates the magenta level. Value between 0 and 100
  * \param yellow indicates the yellow level. Value between 0 and 100
  * \param black indicates the black level. Value between 0 and 100
  */
  void SetColor(double cyan, double magenta, double yellow, double black);

  /// Set a named RGB color
  /**
  * \param name is the name of the requested color
  */
  void SetColor(const wxString& name);

  /// Set a spot color (internal use only)
  /**
  * \param spotColor is the spot color to be used
  * \param tint indicates the tint level. Value between 0 and 100. Default: 100.
  */
  void SetColor(const wxPdfSpotColour& spotColor, double tint);


  /// Get internal color string representation (for internal use only)
  /**
  * \param drawing flag specifying whether the color is used for drawing operations 
  */
  const wxString GetColor(bool drawing) const;

  /// Get color type
  /**
  */
  wxPdfColourType GetColorType() const { return m_type; }

  /// Get internal color value string representation (for internal use only)
  /**
  */
  const wxString GetColorValue() const;

  /// Compare color
  bool Equals(const wxPdfColour& color) const;

protected:
  /// Constructor for internal color string representation
  wxPdfColour(const wxString& color, bool WXUNUSED(internal));

  /// Get a color database
  static wxColourDatabase* GetColorDatabase();

private:
  wxPdfColourType m_type;   ///< color type
  wxString        m_prefix; ///< internal color string prefix
  wxString        m_color;  ///< internal color string

  static wxColourDatabase* ms_colorDatabase;
};

bool operator==(const wxPdfColour& a, const wxPdfColour& b);

bool operator!=(const wxPdfColour& a, const wxPdfColour& b);

/// Class representing double arrays (no standard class in wxWidgets unfortunately)
WX_DEFINE_USER_EXPORTED_ARRAY_DOUBLE(double, wxPdfArrayDouble, class WXDLLIMPEXP_PDFDOC);

/// Class representing line styles.
class WXDLLIMPEXP_PDFDOC wxPdfLineStyle
{
public:
  /// Constructor
  /**
  * Creates a line style for use in graphics primitives.
  * \param[in] width Width of the line in user units.
  * \param[in] cap   Type of cap to put on the line (butt, round, square).
  *                  The difference between 'square' and 'butt' is that 'square'
  *                  projects a flat end past the end of the line.
  * \param[in] join  form of line joining: miter, round or bevel
  * \param[in] dash  pattern for dashed lines.Is an empty array (without dash) or
  *   array with series of length values, which are the lengths of the on and off dashes.
  *           For example: (2) represents 2 on, 2 off, 2 on , 2 off ...
  *                        (2,1) is 2 on, 1 off, 2 on, 1 off.. etc
  * \param[in] phase Modifier of the dash pattern which is used to shift the point at which the pattern starts
  * \param[in] color line color.
  * \see SetLineStyle(), Curve(), Line(), Circle(), Ellipse(), Rect(), RoundedRect(), Polygon(), RegularPolygon(), StarPolygon()
  */
  wxPdfLineStyle(double width = -1,
                 wxPdfLineCap cap = wxPDF_LINECAP_NONE, wxPdfLineJoin join = wxPDF_LINEJOIN_NONE,
                 const wxPdfArrayDouble& dash = wxPdfArrayDouble(), double phase = -1,
                 const wxPdfColour& color = wxPdfColour());

  /// Copy constructor
  wxPdfLineStyle(const wxPdfLineStyle& lineStyle);

  /// Assignment operator
  wxPdfLineStyle& operator= (const wxPdfLineStyle& lineStyle);

  /// Destructor
  virtual ~wxPdfLineStyle();

  /// Check whether the style is initialized.
  bool IsSet() const { return m_isSet; }

  /// Set the line width
  void SetWidth(double width) { m_width = width; }

  /// Get the line width
  double GetWidth() const { return m_width; }

  /// Set the line ending style
  void SetLineCap(const wxPdfLineCap cap) { m_cap = cap; }

  /// Get the line ending style
  wxPdfLineCap GetLineCap() const { return m_cap; }

  /// Set the line join style
  void SetLineJoin(const wxPdfLineJoin join) { m_join = join; }

  /// Get the line join style
  wxPdfLineJoin GetLineJoin() const { return m_join; }

  /// Set the dash pattern
  void SetDash(const wxPdfArrayDouble& dash) { m_dash = dash; }

  /// Get the dash pattern
  const wxPdfArrayDouble& GetDash() const { return m_dash; }

  /// Set the dash pattern phase
  void SetPhase(double phase) { m_phase = phase; }

  /// Get the dash pattern phase
  double GetPhase() const { return m_phase; }

  /// Set the line color
  void SetColour(const wxPdfColour& color) { m_color = color; };

  /// Get the line color
  const wxPdfColour& GetColour() const { return m_color; };

  /// Set the miter limit (0 for none)
  void SetMiterLimit(const double miterLimit) { m_miterLimit = miterLimit; };

  /// Get the miter limit
  double GetMiterLimit() const { return m_miterLimit; };


private:
  bool             m_isSet;   ///< Flag whether the style is initialized
  double           m_width;   ///< Line width
  wxPdfLineCap     m_cap;     ///< Line ending style
  wxPdfLineJoin    m_join;    ///< Line joining style
  wxPdfArrayDouble m_dash;    ///< Dash pattern
  double           m_phase;   ///< Dash pattern phase
  wxPdfColour      m_color;   ///< Line color
  double           m_miterLimit;  ///< Miter limit (0 for none)
};

/// Class representing a coons patch mesh.

class WXDLLIMPEXP_PDFDOC wxPdfCoonsPatchMesh
{
public:
  /// Constructor
  wxPdfCoonsPatchMesh();

  /// Destructor
  virtual ~wxPdfCoonsPatchMesh();

  /// Add patch to mesh
  /**
  * \param edgeFlag flag indicating the patch position relative to previous patches
  *   \li 0 - new patch, unrelated to previous patches (the first patch added must have this flag)
  *   \li 1 - above previous patch
  *   \li 2 - right to previous patch
  *   \li 3 - below previous patch
  * \param colors array of colors of this patch (size: 4 if edge flag is 1, 2 otherwise)
  * \param x array of x coordinates of patch mesh points (size: 12 if edge flag is 1, 8 otherwise)
  * \param y array of y coordinates of patch mesh points (size: 12 if edge flag is 1, 8 otherwise)
  * \returns true if the added patch is valid
  */
  bool AddPatch(int edgeFlag, wxPdfColour colors[], double x[], double y[]);

  /// Checks whether the coons patch mesh is valid
  bool Ok() const { return m_ok; }

  /// Get color type of the coons patch mesh
  /**
  * \returns the color type of the coons patch mesh (gray scale, RGB or CMYK)
  */
  wxPdfColourType GetColorType() const { return m_colorType; }

  /// Get the number of patches
  /**
  * \returns the number of patches of the coons patch mesh
  */
  size_t GetPatchCount() const { return m_patches.size(); }

  /// Get the array of patches
  /**
  *
  * \returns array of patches
  */
  const wxArrayPtrVoid* GetPatches() const { return &m_patches; }

private:
  bool            m_ok;        ///< flag whether the coons patch mesh is valid
  wxPdfColourType m_colorType; ///< color type of the mesh
  wxArrayPtrVoid  m_patches;   ///< array of patches
};

/// Shape segment types
enum wxPdfSegmentType
{
  wxPDF_SEG_UNDEFINED,
  wxPDF_SEG_MOVETO,
  wxPDF_SEG_LINETO,
  wxPDF_SEG_CURVETO,
  wxPDF_SEG_CLOSE
};

/// Class representing a shape consisting of line and curve segments
class WXDLLIMPEXP_PDFDOC wxPdfShape
{
public:
  /// Constructor
  wxPdfShape();

  /// Destructor
  virtual ~wxPdfShape();

  /// Begin a new subpath of the shape
  /**
  * Move to the starting point of a new (sub)path.
  * The new current point is (x, y).
  * \param x abscissa value
  * \param y ordinate value
  * \remark This must be the first operation in constructing the shape.
  */
  void MoveTo(double x, double y);

  /// Add line segment to the shape
  /**
  * Append a straight line segment from the current point to the point (x, y).
  * The new current point is (x, y).
  * \param x abscissa value
  * \param y ordinate value
  */
  void LineTo(double x, double y);

  /// Add a cubic Bezier curve to the shape
  /**
  * Append a cubic Bezier curve to the current path. The curve extends
  * from the current point to the point (x3, y3), using (x1, y1) and (x2, y2)
  * as the Bezier control points. The new current point is (x3, y3).
  * \param x1: Abscissa of control point 1
  * \param y1: Ordinate of control point 1
  * \param x2: Abscissa of control point 2
  * \param y2: Ordinate of control point 2
  * \param x3: Abscissa of end point
  * \param y3: Ordinate of end point
  */
  void CurveTo(double x1, double y1, double x2, double y2, double x3, double y3);

  /// Close (sub)path of the shape
  void ClosePath();

  /// Get the number of segments of the shape
  size_t GetSegmentCount() const { return m_types.GetCount(); }

  /// Get a specific segment of the shape (for internal use only)
  /**
  * \param[in] iterType index of segment in segment type array
  * \param[in] iterPoints index of segment in segment coordinate array
  * \param[out] coords array of segment coordinates (size: >= 8)
  * \returns the type of the segment
  */
  wxPdfSegmentType GetSegment(int iterType, int iterPoints, double coords[]) const;

private:
  wxArrayInt       m_types;   ///< array of segment types
  wxPdfArrayDouble m_x;       ///< array of abscissa values
  wxPdfArrayDouble m_y;       ///< array of ordinate values
  int              m_subpath; ///< subpath index
  int              m_segment; ///< segment index
  int              m_index;   ///< points index
};

#endif

