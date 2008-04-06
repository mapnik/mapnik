///////////////////////////////////////////////////////////////////////////////
// Name:        pdfoc.h
// Purpose:     
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file pdfoc.h PDF Optional Content Management Classes

#ifndef _PDFOC_H_
#define _PDFOC_H_

#include <wx/string.h>
#include "pdfdocdef.h"

/// OCG Intent options
#define wxPDF_OCG_INTENT_VIEW   0x0001
#define wxPDF_OCG_INTENT_DESIGN 0x0002


class wxPdfOcg; // Predefine for hashmap

/// Hashmap class for document links
WX_DECLARE_HASH_MAP(unsigned int, wxPdfOcg*, wxIntegerHash, wxIntegerEqual, wxPdfOcgHashMap);


/// Class representing the optional content in a document.
class WXDLLIMPEXP_PDFDOC wxPdfOc {
public:
  /// Constructor
  wxPdfOc();

  /// Destructor
  ~wxPdfOc();

  /// Add an optional content group
  /**
  * The OCG will be managed by this class after its added, so you dont need to destroy it yourself. 
  *  <b>This means the OCG must have been created dynamically, not statically.</b>
  * \param[in] ocg The OCG to add
  */
  void AddOcg(wxPdfOcg *ocg);

  /// Get the OCG map
  /**
  * \return The ocg hash map
  */
  wxPdfOcgHashMap& GetOcgMap(void) { return m_ocgs; };


private:

  unsigned int    m_nextOcgId;  ///< Next Id to use for an Ocg. Incremented after adding new Ocg;
  wxPdfOcgHashMap m_ocgs;       ///< Hash map of ocgs


};


/// Class representing an optional content group
class WXDLLIMPEXP_PDFDOC wxPdfOcg {
public:
  /// Construct a new OCG
  /** 
  * \param [in] name The label shown in the view application (does not need to be unique)
  */
  wxPdfOcg(const wxString& name);

  /// Destructor
  ~wxPdfOcg();

  /// Set OCG Intent
  /**
  * \param[in] intent Combination of the defined wxPDF_OCG_INTENT_???? values to set
  */
  void SetIntent(const unsigned int intent) { m_intent |= intent; };

  /// Clear OCG Intent
  /**
  * \param[in] intent Combination of the defined wxPDF_OCG_INTENT_???? values to clear
  */
  void ClearIntent(const unsigned int intent) { m_intent &= ~intent; };

  /// Get OCG Index
  /**
  * \return The OCG index
  */
  unsigned int GetOcgIndex(void) const { return m_index; };

  /// Set OCG Index
  /**
  * \param[in] index The value to set index to
  */
  void SetOcgIndex(const unsigned int index) { m_index = index; };

  /// Get Object Index
  /**
  * \return The object index
  */
  unsigned int GetObjectIndex(void) const { return m_objIndex; };

  /// Set Object Index
  /**
  * \param[in] index The value to set index to
  */
  void SetObjectIndex(const unsigned int index) { m_objIndex = index; };

  /// Set the default visibility state
  /**
  * \param[in] state true = on, false = off
  */
  void SetDefaultVisibilityState(const bool state) { m_defaultState = state; };

  /// Get the default visibility state
  /**
  * \return The default visibility state, true = on, false = off
  */
  bool GetDefaultVisibilityState(void) const { return m_defaultState; };

  /// Get ocg name
  /**
  * \return OCG name
  */
  const wxString& GetName(void) const {return m_name; };

  /// Get stringised Intent
  /**
  * \return String representing intent
  */
  wxString& GetIntentString(void);

private:
  unsigned int  m_objIndex;   ///< Object index
  unsigned int  m_index;      ///< OCG Index
  wxString      m_name;       ///< OCG Name
  unsigned int  m_intent;     ///< OCG Intent (combinations of wxPDF_OCG_INTENT_VIEW | wxPDF_OCG_INTENT_DESIGN)
  wxString      m_intentStr;  ///< OCG Intent string
  bool          m_defaultState; ///!< true = default on, false = default off
};


/// Class representing an optional content membership dictionary
/*class WXDLLIMPEXP_PDFDOC wxPdfOcmd {
public:
  typedef enum VisiblityPolicy {
    AllOn = 0,
    AnyOn,
    AnyOff,
    AllOff
  } VisibilityPolicy;

public:
  /// Construct a new OCMD
  wxPdfOcmd();


  /// Destructor
  ~wxPdfOcmd();


  /// Set visibility policy
  void SetVisibilityPolicy(const VisibilityPolicy policy);

  /// Get visibility policy
  const VisibilityPolicy GetVisibilityPolicy(void) const { return m_policy; };

  /// Get visibility policy as PdfName
  const wxPdfName& GetNameVisibilityPolicy(void) const { return m_strPolicy; };

private:

  wxPdfArray        m_ocg;        ///< Array of OCGs whose states determine the visibility of content controlled by this OCMD
  VisibilityPolicy  m_policy;     ///< Visiblity policy
  wxPdfName         m_strPolicy;  ///< Stringised visibility policy
};
*/



#endif //_PDFOC_H_
