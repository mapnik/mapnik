// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// Based on XML grammar by Daniel C. Nuffer 
// http://spirit.sourceforge.net/repository/applications/xml.zip
// 
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_XML_PARSER_READ_SPIRIT_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_XML_PARSER_READ_SPIRIT_HPP_INCLUDED

//#define BOOST_SPIRIT_DEBUG
#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>
#include <boost/property_tree/detail/xml_parser_flags.hpp>
#include <boost/property_tree/detail/xml_parser_utils.hpp>

#if BOOST_VERSION < 103800
#include <boost/spirit.hpp>
#include <boost/spirit/iterator/position_iterator.hpp>
#else
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#endif

#include <string>
#include <locale>
#include <istream>
#include <vector>

namespace boost { namespace property_tree { namespace xml_parser
{

    // XML parser context
    template<class Ptree>
    struct context
    {
        
        typedef typename Ptree::char_type Ch;
        typedef std::basic_string<Ch> Str;
        typedef boost::spirit::position_iterator<typename std::vector<Ch>::const_iterator> It;

        int flags;
        std::vector<Ptree *> stack;

        ///////////////////////////////////////////////////////////////////////
        // Actions
        
        struct a_key_s
        {
            context &c;
            a_key_s(context &c): c(c) { }
            void operator()(It b, It e) const
            {
                if (c.stack.empty())
                    throw xml_parser_error("xml parse error", 
                                           b.get_position().file, 
                                           b.get_position().line);
                Str name(b, e);
                Ptree *child = &c.stack.back()->push_back(std::make_pair(name, Ptree()))->second;
                c.stack.push_back(child);
            }
        };

        struct a_key_e
        {
            context &c;
            a_key_e(context &c): c(c) { }
            void operator()(It b, It e) const
            {
                if (c.stack.size() <= 1)
                    throw xml_parser_error("xml parse error", 
                                           b.get_position().file, 
                                           b.get_position().line);
                c.stack.pop_back();
            }
        };

        struct a_content
        {
            context &c;
            a_content(context &c): c(c) { }
            void operator()(It b, It e) const
            {
                Str s = decode_char_entities(detail::trim(condense(Str(b, e))));
                if (!s.empty())
                {
                    if (c.flags & no_concat_text)
                        c.stack.back()->push_back(std::make_pair(xmltext<Ch>(), Ptree(s)));
                    else
                        c.stack.back()->put_own(c.stack.back()->template get_own<std::basic_string<Ch> >() + s);
                }
            }
        };

        struct a_attr_key
        {
            context &c;
            a_attr_key(context &c): c(c) { }
            void operator()(It b, It e) const
            {
                c.stack.back()->put_child(Ch('/'), xmlattr<Ch>() + Ch('/') + Str(b, e), empty_ptree<Ptree>());
            }
        };

        struct a_attr_data
        {
            context &c;
            a_attr_data(context &c): c(c) { }
            void operator()(It b, It e) const
            {
                Ptree &attr = c.stack.back()->get_child(xmlattr<Ch>());
                attr.back().second.put_own(Str(b.base() + 1, e.base() - 1));
            }
        };

        struct a_comment
        {
            context &c;
            a_comment(context &c): c(c) { }
            void operator()(It b, It e) const
            {
                c.stack.back()->push_back(std::make_pair(xmlcomment<Ch>(), Ptree(Str(b, e))));
            }
        };

    };

    ///////////////////////////////////////////////////////////////////////
    // Grammar
        
    template<class Ptree>
    struct xml_grammar: public boost::spirit::grammar<xml_grammar<Ptree> >
    {
        
        typedef context<Ptree> context_t;
        
        mutable context_t c;
        
        template<class ScannerT>
        struct definition
        {
            
            typedef typename ScannerT::value_t char_t;
            typedef boost::spirit::chset<char_t> chset_t;

            boost::spirit::rule<ScannerT>
                prolog, element, Misc, PEReference, Reference, PITarget, CData,
                doctypedecl, XMLDecl, SDDecl, VersionInfo, EncodingDecl, VersionNum,
                Eq, DeclSep, ExternalID, markupdecl, NotationDecl, EntityDecl,
                AttlistDecl, elementdecl, TextDecl, extSubsetDecl, conditionalSect,
                EmptyElemTag, STag, content, ETag, Attribute, contentspec, Mixed,
                children, choice, seq, cp, AttDef, AttType, DefaultDecl, StringType,
                TokenizedType, EnumeratedType, NotationType, Enumeration, EntityValue,
                AttValue, SystemLiteral, PubidLiteral, CharDataChar, CharData, Comment,
                PI, CDSect, extSubset, includeSect, ignoreSect, ignoreSectContents,
                Ignore, CharRef, EntityRef, GEDecl, PEDecl, EntityDef, PEDef,
                NDataDecl, extParsedEnt, EncName, PublicID, document, S, Name, Names,
                Nmtoken, Nmtokens, STagB, STagE1, STagE2;

            definition(const xml_grammar &self)
            {
                
                using namespace boost::spirit;
                
                // XML Char sets
                chset_t Char("\x9\xA\xD\x20-\x7F");
                chset_t Sch("\x20\x9\xD\xA");
                chset_t Letter("\x41-\x5A\x61-\x7A");
                chset_t Digit("0-9");
                chset_t XDigit("0-9A-Fa-f");
                chset_t Extender("\xB7");
                chset_t NameChar =
                    Letter 
                    | Digit 
                    | (char_t)'.'
                    | (char_t)'-'
                    | (char_t)'_'
                    | (char_t)':'
                    | Extender;

                document =
                    prolog >> element >> *Misc
                ;

                S = 
                    +(Sch)
                ;

                Name =
                    (Letter | '_' | ':') 
                    >> *(NameChar)
                ;

                Names =
                    Name >> *(S >> Name)
                ;

                Nmtoken =
                    +NameChar
                ;

                Nmtokens =
                    Nmtoken >> *(S >> Nmtoken)
                ;

                EntityValue =
                    '"'  >> *(  (anychar_p - (chset_t(detail::widen<char_t>("%&\"").c_str()))) 
                                | PEReference
                                | Reference) 
                        >> '"'
                    | '\'' >> *(  (anychar_p - (chset_t("%&'"))) 
                                | PEReference
                                | Reference) 
                            >> '\''
                ;

                AttValue = 
                    '"' >> *(  (anychar_p - (chset_t("<&\""))) 
                                | Reference) 
                        >> '"'
                    | '\'' >> *(  (anychar_p - (chset_t("<&'"))) 
                                | Reference) 
                            >> '\''
                ;

                SystemLiteral= 
                    ('"' >> *(anychar_p - '"') >> '"')
                    | ('\'' >> *(anychar_p - '\'') >> '\'')
                ;

                chset_t PubidChar("\x20\xD\xA'a-zA-Z0-9()+,./:=?;!*#@$_%-");

                PubidLiteral = 
                    '"' >> *PubidChar >> '"' 
                    | '\'' >> *(PubidChar - '\'') >> '\''
                ;

                CharDataChar = 
                    //anychar_p - (chset_t("<&"))
                    anychar_p - (chset_t("<"))
                ;

                CharData =
                    *(CharDataChar - "]]>")
                ;

                Comment = 
                    "<!--" >> 
                    (
                        *(
                            (Char - '-')
                            | ('-' >> (Char - '-'))
                        )
                    )[typename context_t::a_comment(self.c)] 
                    >> "-->"
                ;

                PI = 
                    "<?" >> PITarget >> !(S >> (*(Char - "?>"))) >> "?>"
                ;

                PITarget =
                    Name - (as_lower_d["xml"])
                ;

                CDSect =
                    "<![CDATA[" >> CData >> "]]>"
                ;

                CData =
                    *(Char - "]]>")
                ;

                prolog =
                    !XMLDecl >> *Misc >> !(doctypedecl >> *Misc)
                ;

                XMLDecl =
                    "<?xml" >> VersionInfo >> !EncodingDecl >> !SDDecl 
                    >> !S >> "?>"
                ;

                VersionInfo = 
                    S >> "version" >> Eq >> 
                    (
                    '\'' >> VersionNum >> '\''
                    | '"' >> VersionNum >> '"'
                    )
                ;

                Eq =
                    !S >> '=' >> !S
                ;

                chset_t VersionNumCh("a-zA-Z0-9_.:-");

                VersionNum =
                    +(VersionNumCh)
                ;

                Misc =
                    Comment 
                    | PI 
                    | S
                ;

                doctypedecl =
                    "<!DOCTYPE" >> S >> Name >> !(S >> ExternalID) >> !S >> 
                    !(
                    '[' >> *(markupdecl | DeclSep) >> ']' >> !S
                    ) 
                    >> '>'
                ;

                DeclSep =
                    PEReference 
                    | S
                ;

                markupdecl =
                    elementdecl 
                    | AttlistDecl 
                    | EntityDecl 
                    | NotationDecl 
                    | PI 
                    | Comment
                ;

                extSubset =
                    !TextDecl >> extSubsetDecl
                ;

                extSubsetDecl =
                    *(
                        markupdecl 
                    | conditionalSect 
                    | DeclSep
                    )
                ;

                SDDecl = 
                    S >> "standalone" >> Eq >> 
                    (
                    ('\'' >> (str_p("yes") | "no") >> '\'')
                    | ('"' >> (str_p("yes") | "no") >> '"')
                    )
                ;

                /*
                element =
                    EmptyElemTag
                    | STag >> content >> ETag
                ;
                */
                element =
                    STagB >> (STagE2 | (STagE1 >> content >> ETag))[typename context_t::a_key_e(self.c)]
                ;

                STag =
                    '<' >> Name >> *(S >> Attribute) >> !S >> '>'
                ;

                STagB =
                    '<'
                    >> Name[typename context_t::a_key_s(self.c)]
                    >> *(S >> Attribute)
                    >> !S
                ;

                STagE1 = 
                    ch_p(">")
                ;

                STagE2 = 
                    str_p("/>")
                ;

                Attribute =
                    Name[typename context_t::a_attr_key(self.c)]
                    >> Eq
                    >> AttValue[typename context_t::a_attr_data(self.c)]
                ;

                ETag =
                    "</" >> Name >> !S >> '>'
                ;

                content =
                   !(CharData[typename context_t::a_content(self.c)]) >> 
                   *(
                     (
                          element 
                        // | Reference
                        | CDSect 
                        | PI 
                        | Comment
                     ) >> 
                     !(CharData[typename context_t::a_content(self.c)])
                    )
                ;

                EmptyElemTag =
                    '<' >> Name >> *(S >> Attribute) >> !S >> "/>"
                ;

                elementdecl = 
                    "<!ELEMENT" >> S >> Name >> S >> contentspec >> !S >> '>'
                ;

                contentspec = 
                    str_p("EMPTY") 
                    | "ANY" 
                    | Mixed 
                    | children
                ;

                children =
                    (choice | seq) >> !(ch_p('?') | '*' | '+')
                ;

                cp = 
                    (Name | choice | seq) >> !(ch_p('?') | '*' | '+')
                ;

                choice = 
                    '(' >> !S >> cp 
                    >> +(!S >> '|' >> !S >> cp) 
                    >> !S >> ')'
                ;

                seq =
                    '(' >> !S >> cp >> 
                    *(!S >> ',' >> !S >> cp) 
                    >> !S >> ')'
                ;

                Mixed =
                    '(' >> !S >> "#PCDATA" 
                        >> *(!S >> '|' >> !S >> Name) 
                        >> !S >> ")*"
                    | '(' >> !S >> "#PCDATA" >> !S >> ')'
                ;

                AttlistDecl =
                    "<!ATTLIST" >> S >> Name >> *AttDef >> !S >> '>'
                ;

                AttDef =
                    S >> Name >> S >> AttType >> S >> DefaultDecl
                ;

                AttType =
                    StringType 
                    | TokenizedType 
                    | EnumeratedType
                ;

                StringType =
                    str_p("CDATA")
                ;

                TokenizedType =
                    longest_d[ 
                        str_p("ID") 
                        | "IDREF" 
                        | "IDREFS" 
                        | "ENTITY" 
                        | "ENTITIES" 
                        | "NMTOKEN"
                        | "NMTOKENS" 
                    ]
                ;

                EnumeratedType =
                    NotationType 
                    | Enumeration
                ;

                NotationType =
                    "NOTATION" >> S >> '(' >> !S >> Name 
                    >> *(!S >> '|' >> !S >> Name) 
                    >> !S >> ')'
                ;

                Enumeration = 
                    '(' >> !S >> Nmtoken 
                    >> *(!S >> '|' >> !S >> Nmtoken) 
                    >> !S >> ')'
                ;

                DefaultDecl =
                    str_p("#REQUIRED") 
                    | "#IMPLIED" 
                    | !("#FIXED" >> S) >> AttValue
                ;

                conditionalSect =
                    includeSect 
                    | ignoreSect
                ;

                includeSect =
                    "<![" >> !S >> "INCLUDE" >> !S 
                    >> '[' >> extSubsetDecl >> "]]>"
                ;

                ignoreSect = 
                    "<![" >> !S >> "IGNORE"  >> !S 
                    >> '[' >> *ignoreSectContents >> "]]>"
                ;

                ignoreSectContents = 
                    Ignore >> *("<![" >> ignoreSectContents >> "]]>" >> Ignore)
                ;

                Ignore = 
                    *(Char - (str_p("<![") | "]]>"))
                ;

                CharRef = 
                    "&#"  >> +Digit  >> ';'
                    | "&#x" >> +XDigit >> ';'
                ;

                Reference =
                    EntityRef 
                    | CharRef
                ;

                EntityRef =
                    '&' >> Name >> ';'
                ;

                PEReference =
                    '%' >> Name >> ';'
                ;

                EntityDecl =
                    GEDecl 
                    | PEDecl
                ;

                GEDecl =
                    "<!ENTITY" >> S >> Name >> S >> EntityDef >> !S >> '>'
                ;

                PEDecl =
                    "<!ENTITY" >> S >> '%' >> S >> Name >> S >> PEDef 
                    >> !S >> '>'
                ;

                EntityDef =
                    EntityValue
                    | ExternalID >> !NDataDecl
                ;

                PEDef =
                    EntityValue 
                    | ExternalID
                ;

                ExternalID =
                    "SYSTEM" >> S >> SystemLiteral
                    | "PUBLIC" >> S >> PubidLiteral >> S >> SystemLiteral
                ;

                NDataDecl =
                    S >> "NDATA" >> S >> Name
                ;

                TextDecl =
                    "<?xml" >> !VersionInfo >> EncodingDecl >> !S >> "?>"
                ;

                extParsedEnt =
                    !TextDecl >> content
                ;

                EncodingDecl =
                    S >> "encoding" >> Eq 
                    >> (  '"' >> EncName >> '"' 
                        | '\'' >> EncName >> '\''
                    )
                ;

                EncName =
                    Letter >> *(Letter | Digit | '.' | '_' | '-')
                ;

                NotationDecl =
                    "<!NOTATION" >> S >> Name >> S 
                    >> (ExternalID | PublicID) >> !S >> '>'
                ;

                PublicID =
                    "PUBLIC" >> S >> PubidLiteral
                ;

                BOOST_SPIRIT_DEBUG_RULE(document);
                BOOST_SPIRIT_DEBUG_RULE(prolog);
                BOOST_SPIRIT_DEBUG_RULE(element);
                BOOST_SPIRIT_DEBUG_RULE(Misc);
                BOOST_SPIRIT_DEBUG_RULE(PEReference);
                BOOST_SPIRIT_DEBUG_RULE(Reference);
                BOOST_SPIRIT_DEBUG_RULE(PITarget);
                BOOST_SPIRIT_DEBUG_RULE(CData);
                BOOST_SPIRIT_DEBUG_RULE(doctypedecl);
                BOOST_SPIRIT_DEBUG_RULE(XMLDecl);
                BOOST_SPIRIT_DEBUG_RULE(SDDecl);
                BOOST_SPIRIT_DEBUG_RULE(VersionInfo);
                BOOST_SPIRIT_DEBUG_RULE(EncodingDecl);
                BOOST_SPIRIT_DEBUG_RULE(VersionNum);
                BOOST_SPIRIT_DEBUG_RULE(Eq);
                BOOST_SPIRIT_DEBUG_RULE(DeclSep);
                BOOST_SPIRIT_DEBUG_RULE(ExternalID);
                BOOST_SPIRIT_DEBUG_RULE(markupdecl);
                BOOST_SPIRIT_DEBUG_RULE(NotationDecl);
                BOOST_SPIRIT_DEBUG_RULE(EntityDecl);
                BOOST_SPIRIT_DEBUG_RULE(AttlistDecl);
                BOOST_SPIRIT_DEBUG_RULE(elementdecl);
                BOOST_SPIRIT_DEBUG_RULE(TextDecl);
                BOOST_SPIRIT_DEBUG_RULE(extSubsetDecl);
                BOOST_SPIRIT_DEBUG_RULE(conditionalSect);
                BOOST_SPIRIT_DEBUG_RULE(EmptyElemTag);
                BOOST_SPIRIT_DEBUG_RULE(STag);
                BOOST_SPIRIT_DEBUG_RULE(content);
                BOOST_SPIRIT_DEBUG_RULE(ETag);
                BOOST_SPIRIT_DEBUG_RULE(Attribute);
                BOOST_SPIRIT_DEBUG_RULE(contentspec);
                BOOST_SPIRIT_DEBUG_RULE(Mixed);
                BOOST_SPIRIT_DEBUG_RULE(children);
                BOOST_SPIRIT_DEBUG_RULE(choice);
                BOOST_SPIRIT_DEBUG_RULE(seq);
                BOOST_SPIRIT_DEBUG_RULE(cp);
                BOOST_SPIRIT_DEBUG_RULE(AttDef);
                BOOST_SPIRIT_DEBUG_RULE(AttType);
                BOOST_SPIRIT_DEBUG_RULE(DefaultDecl);
                BOOST_SPIRIT_DEBUG_RULE(StringType);
                BOOST_SPIRIT_DEBUG_RULE(TokenizedType);
                BOOST_SPIRIT_DEBUG_RULE(EnumeratedType);
                BOOST_SPIRIT_DEBUG_RULE(NotationType);
                BOOST_SPIRIT_DEBUG_RULE(Enumeration);
                BOOST_SPIRIT_DEBUG_RULE(EntityValue);
                BOOST_SPIRIT_DEBUG_RULE(AttValue);
                BOOST_SPIRIT_DEBUG_RULE(SystemLiteral);
                BOOST_SPIRIT_DEBUG_RULE(PubidLiteral);
                BOOST_SPIRIT_DEBUG_RULE(CharDataChar);
                BOOST_SPIRIT_DEBUG_RULE(CharData);
                BOOST_SPIRIT_DEBUG_RULE(Comment);
                BOOST_SPIRIT_DEBUG_RULE(PI);
                BOOST_SPIRIT_DEBUG_RULE(CDSect);
                BOOST_SPIRIT_DEBUG_RULE(extSubset);
                BOOST_SPIRIT_DEBUG_RULE(includeSect);
                BOOST_SPIRIT_DEBUG_RULE(ignoreSect);
                BOOST_SPIRIT_DEBUG_RULE(ignoreSectContents);
                BOOST_SPIRIT_DEBUG_RULE(Ignore);
                BOOST_SPIRIT_DEBUG_RULE(CharRef);
                BOOST_SPIRIT_DEBUG_RULE(EntityRef);
                BOOST_SPIRIT_DEBUG_RULE(GEDecl);
                BOOST_SPIRIT_DEBUG_RULE(PEDecl);
                BOOST_SPIRIT_DEBUG_RULE(EntityDef);
                BOOST_SPIRIT_DEBUG_RULE(PEDef);
                BOOST_SPIRIT_DEBUG_RULE(NDataDecl);
                BOOST_SPIRIT_DEBUG_RULE(extParsedEnt);
                BOOST_SPIRIT_DEBUG_RULE(EncName);
                BOOST_SPIRIT_DEBUG_RULE(PublicID);
                BOOST_SPIRIT_DEBUG_RULE(document);
                BOOST_SPIRIT_DEBUG_RULE(S);
                BOOST_SPIRIT_DEBUG_RULE(Name);
                BOOST_SPIRIT_DEBUG_RULE(Names);
                BOOST_SPIRIT_DEBUG_RULE(Nmtoken);
                BOOST_SPIRIT_DEBUG_RULE(Nmtokens);
                BOOST_SPIRIT_DEBUG_RULE(STagB);
                BOOST_SPIRIT_DEBUG_RULE(STagE1);
                BOOST_SPIRIT_DEBUG_RULE(STagE2);

            }

            const boost::spirit::rule<ScannerT> &start() const
            {
                return document;
            }

        };

    };

    template<class Ptree>
    void read_xml_internal(std::basic_istream<typename Ptree::char_type> &stream,
                           Ptree &pt,
                           int flags,
                           const std::string &filename)
    {

        typedef typename Ptree::char_type Ch;
        typedef boost::spirit::position_iterator<typename std::vector<Ch>::const_iterator> It;

        BOOST_ASSERT(validate_flags(flags));

        // Load data into vector
        std::vector<Ch> v(std::istreambuf_iterator<Ch>(stream.rdbuf()),
                          std::istreambuf_iterator<Ch>());
        if (!stream.good())
            throw xml_parser_error("read error", filename, 0);
        
        // Initialize iterators
        It begin(v.begin(), v.end());
        It end(v.end(), v.end());
        begin.set_position(filename);
        
        // Prepare grammar
        Ptree local;
        xml_grammar<Ptree> g;
        g.c.stack.push_back(&local);       // Push root ptree on context stack
        g.c.flags = flags;

        // Parse into local
        boost::spirit::parse_info<It> result = boost::spirit::parse(begin, end, g);
        if (!result.full || g.c.stack.size() != 1)
            throw xml_parser_error("xml parse error", 
                                   result.stop.get_position().file, 
                                   result.stop.get_position().line);

        // Swap local and pt
        pt.swap(local);
    }

} } }

#endif
