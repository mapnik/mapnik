// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_PTREE_INTERFACE_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_PTREE_INTERFACE_HPP_INCLUDED

#include <boost/config.hpp>
#include <boost/optional.hpp>
#include <string>
#include <list>
#include <map>
#include <utility>          // For std::pair
#include <locale>

#include "boost/property_tree/ptree_fwd.hpp"

#ifdef BOOST_PROPERTY_TREE_DEBUG
#   include <boost/detail/lightweight_mutex.hpp>   // For syncing debug instances counter
#endif

namespace boost { namespace property_tree
{

    ///////////////////////////////////////////////////////////////////////////
    // basic_ptree class template

    template<class Tr>
    class basic_ptree
    {

    public:

        // Basic types
        typedef Tr traits_type;
        typedef typename traits_type::char_type char_type;
        typedef typename traits_type::key_type key_type;
        typedef typename traits_type::data_type data_type;
        
        // Container-related types
        typedef std::pair<key_type, basic_ptree<Tr> > value_type;
        typedef std::list<value_type> container_type;
        typedef typename container_type::size_type size_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename container_type::reverse_iterator reverse_iterator;
        typedef typename container_type::const_reverse_iterator const_reverse_iterator;
        
        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction

        basic_ptree();
        explicit basic_ptree(const data_type &data);
        basic_ptree(const basic_ptree<Tr> &rhs);
        ~basic_ptree();

        ///////////////////////////////////////////////////////////////////////////
        // Iterator access

        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
        reverse_iterator rbegin();
        const_reverse_iterator rbegin() const;
        reverse_iterator rend();
        const_reverse_iterator rend() const;
        
        ///////////////////////////////////////////////////////////////////////////
        // Data access

        size_type size() const;
        bool empty() const;
        
        data_type &data();
        const data_type &data() const;

        value_type &front();
        const value_type &front() const;
        value_type &back();
        const value_type &back() const;

        ///////////////////////////////////////////////////////////////////////////
        // Operators

        basic_ptree<Tr> &operator =(const basic_ptree<Tr> &rhs);

        bool operator ==(const basic_ptree<Tr> &rhs) const;
        bool operator !=(const basic_ptree<Tr> &rhs) const;

        ///////////////////////////////////////////////////////////////////////////
        // Container operations

        iterator find(const key_type &key);
        const_iterator find(const key_type &key) const;

        size_type count(const key_type &key) const;

        void clear();

        iterator insert(iterator where, const value_type &value);
        template<class It> void insert(iterator where, It first, It last);

        iterator erase(iterator where);
        size_type erase(const key_type &key);
        template<class It> iterator erase(It first, It last);

        iterator push_front(const value_type &value);
        iterator push_back(const value_type &value);

        void pop_front();
        void pop_back();

        void swap(basic_ptree<Tr> &rhs);

        void reverse();
        template<class SortTr> void sort(SortTr tr);

        ///////////////////////////////////////////////////////////////////////////
        // ptree operations

        // Get child ptree with custom separator
        basic_ptree<Tr> &get_child(char_type separator, const key_type &path);
        const basic_ptree<Tr> &get_child(char_type separator, const key_type &path) const;
        basic_ptree<Tr> &get_child(char_type separator, const key_type &path, basic_ptree<Tr> &default_value);
        const basic_ptree<Tr> &get_child(char_type separator, const key_type &path, const basic_ptree<Tr> &default_value) const;
        optional<basic_ptree<Tr> &> get_child_optional(char_type separator, const key_type &path);
        optional<const basic_ptree<Tr> &> get_child_optional(char_type separator, const key_type &path) const;

        // Get child ptree with default separator
        basic_ptree<Tr> &get_child(const key_type &path);
        const basic_ptree<Tr> &get_child(const key_type &path) const;
        basic_ptree<Tr> &get_child(const key_type &path, basic_ptree<Tr> &default_value);
        const basic_ptree<Tr> &get_child(const key_type &path, const basic_ptree<Tr> &default_value) const;
        optional<basic_ptree<Tr> &> get_child_optional(const key_type &path);
        optional<const basic_ptree<Tr> &> get_child_optional(const key_type &path) const;

        // Put child ptree with custom separator
        basic_ptree<Tr> &put_child(char_type separator, const key_type &path, const basic_ptree<Tr> &value, bool do_not_replace = false);

        // Put child ptree with default separator
        basic_ptree<Tr> &put_child(const key_type &path, const basic_ptree<Tr> &value, bool do_not_replace = false);

        // Get value from data of ptree
        template<class Type> Type get_own(const std::locale &loc = std::locale()) const;
        template<class Type> Type get_own(const Type &default_value, const std::locale &loc = std::locale()) const;
        template<class CharType> std::basic_string<CharType> get_own(const CharType *default_value, const std::locale &loc = std::locale()) const;
        template<class Type> optional<Type> get_own_optional(const std::locale &loc = std::locale()) const;

        // Get value from data of child ptree (custom path separator)
        template<class Type> Type get(char_type separator, const key_type &path, const std::locale &loc = std::locale()) const;
        template<class Type> Type get(char_type separator, const key_type &path, const Type &default_value, const std::locale &loc = std::locale()) const;
        template<class CharType> std::basic_string<CharType> get(char_type separator, const key_type &path, const CharType *default_value, const std::locale &loc = std::locale()) const;
        template<class Type> optional<Type> get_optional(char_type separator, const key_type &path, const std::locale &loc = std::locale()) const;

        // Get value from data of child ptree (default path separator)
        template<class Type> Type get(const key_type &path, const std::locale &loc = std::locale()) const;
        template<class Type> Type get(const key_type &path, const Type &default_value, const std::locale &loc = std::locale()) const;
        template<class CharType> std::basic_string<CharType> get(const key_type &path, const CharType *default_value, const std::locale &loc = std::locale()) const;
        template<class Type> optional<Type> get_optional(const key_type &path, const std::locale &loc = std::locale()) const;

        // Put value in data of ptree
        template<class Type> void put_own(const Type &value, const std::locale &loc = std::locale());

        // Put value in data of child ptree (custom path separator)
        template<class Type> basic_ptree<Tr> &put(char_type separator, const key_type &path, const Type &value, bool do_not_replace = false, const std::locale &loc = std::locale());

        // Put value in data of child ptree (default path separator)
        template<class Type> basic_ptree<Tr> &put(const key_type &path, const Type &value, bool do_not_replace = false, const std::locale &loc = std::locale());

    private:

        typedef std::multimap<key_type, iterator, Tr> index_type;
        
        struct impl;
        impl *m_impl;

        ////////////////////////////////////////////////////////////////////////////
        // Debugging

#ifdef BOOST_PROPERTY_TREE_DEBUG
    private:
        static boost::detail::lightweight_mutex debug_mutex;    // Mutex for syncing instances counter
        static size_type debug_instances_count;                 // Total number of instances of this ptree class
    public:
        static size_type debug_get_instances_count();
#endif

    };

} }

#endif
