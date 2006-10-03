// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_PTREE_FWD_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_PTREE_FWD_HPP_INCLUDED

#include <boost/config.hpp>

namespace boost { namespace property_tree
{

    ////////////////////////////////////////////////////////////////////////////
    // Traits

    template<class Ch> struct ptree_traits;
    template<class Ch> struct iptree_traits;

    ///////////////////////////////////////////////////////////////////////////
    // Exceptions

    class ptree_error;
    class bad_ptree_data;
    class bad_ptree_path;

    ///////////////////////////////////////////////////////////////////////////
    // basic_ptree class template

    template<class Tr> class basic_ptree;

    ////////////////////////////////////////////////////////////////////////////
    // Typedefs

    typedef basic_ptree<ptree_traits<char> > ptree;       // case sensitive, narrow char
    typedef basic_ptree<iptree_traits<char> > iptree;     // case insensitive, narrow char
#ifndef BOOST_NO_CWCHAR
    typedef basic_ptree<ptree_traits<wchar_t> > wptree;    // case sensitive, wide char
    typedef basic_ptree<iptree_traits<wchar_t> > wiptree;  // case insensitive, wide char
#endif

    ///////////////////////////////////////////////////////////////////////////
    // Free functions

    template<class Tr> void swap(basic_ptree<Tr> &pt1, basic_ptree<Tr> &pt2);
    template<class Ptree> const Ptree &empty_ptree();

} }

#endif
