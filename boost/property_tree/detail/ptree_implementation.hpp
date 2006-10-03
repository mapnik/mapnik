// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_PTREE_IMPLEMENTATION_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_PTREE_IMPLEMENTATION_HPP_INCLUDED

#include <sstream>
#include <locale>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <functional>               // for std::less
#include <memory>                   // for std::auto_ptr
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>        // for boost::prior
#include <boost/property_tree/detail/ptree_utils.hpp>

//////////////////////////////////////////////////////////////////////////////
// Debug macros

#ifdef BOOST_PROPERTY_TREE_DEBUG

    // Increment instances counter
    #define BOOST_PROPERTY_TREE_DEBUG_INCREMENT_INSTANCES_COUNT()       \
        {                                                               \
            typedef boost::detail::lightweight_mutex::scoped_lock lock; \
            lock l(debug_mutex);                                        \
            ++debug_instances_count;                                    \
        }

    // Decrement instances counter
    #define BOOST_PROPERTY_TREE_DEBUG_DECREMENT_INSTANCES_COUNT()       \
        {                                                               \
            typedef boost::detail::lightweight_mutex::scoped_lock lock; \
            lock l(debug_mutex);                                        \
            BOOST_ASSERT(debug_instances_count > 0);                    \
            --debug_instances_count;                                    \
        }

#else // BOOST_PROPERTY_TREE_DEBUG

    #define BOOST_PROPERTY_TREE_DEBUG_INCREMENT_INSTANCES_COUNT() static_cast<void>(0)
    #define BOOST_PROPERTY_TREE_DEBUG_DECREMENT_INSTANCES_COUNT() static_cast<void>(0)

#endif // BOOST_PROPERTY_TREE_DEBUG

namespace boost { namespace property_tree
{

    namespace detail
    {
        
        template<class T>
        struct array_to_pointer_decay
        {
            typedef T type;
        };

        template<class T, std::size_t N>
        struct array_to_pointer_decay<T[N]>
        {
            typedef const T *type;
        };

        ////////////////////////////////////////////////////////////////////////////
        // Extractor and inserter

        template<class Ch, class Type>
        struct extractor
        {
            inline bool operator()(const std::basic_string<Ch> &data, 
                                   Type &extracted,
                                   const std::locale &loc) const
            {
                std::basic_istringstream<Ch> stream(data);
                stream.imbue(loc);
                stream >> extracted >> std::ws;
                return stream.eof() && !stream.fail() && !stream.bad();
            }
        };

        template<class Ch>
        struct extractor<Ch, std::basic_string<Ch> >
        {
            inline bool operator()(const std::basic_string<Ch> &data, 
                                   std::basic_string<Ch> &extracted,
                                   const std::locale &loc) const
            {
                extracted = data;
                return true;
            }
        };

        template<class Ch, class Type>
        struct inserter
        {
            inline bool operator()(std::basic_string<Ch> &data, 
                                   const Type &to_insert,
                                   const std::locale &loc) const
            {
                typedef typename detail::array_to_pointer_decay<Type>::type Type2;
                std::basic_ostringstream<Ch> stream;
                stream.imbue(loc);
                if (std::numeric_limits<Type2>::is_specialized)
                    stream.precision(std::numeric_limits<Type2>::digits10 + 1);
                stream << to_insert;
                data = stream.str();
                return !stream.fail() && !stream.bad();
            }
        };

        template<class Ch>
        struct inserter<Ch, std::basic_string<Ch> >
        {
            inline bool operator()(std::basic_string<Ch> &data, 
                                   const std::basic_string<Ch> &to_insert,
                                   const std::locale &loc) const
            {
                data = to_insert;
                return true;
            }
        };

    }

    ///////////////////////////////////////////////////////////////////////////
    // Impl

    template<class Tr>
    struct basic_ptree<Tr>::impl
    {
        data_type m_data;
        container_type m_container;
        index_type m_index;
    };

    ////////////////////////////////////////////////////////////////////////////
    // Traits

    template<class Ch>
    struct ptree_traits
    {
        typedef Ch char_type;
        typedef std::basic_string<Ch> key_type;
        typedef std::basic_string<Ch> data_type;
        template<class Type>
        struct extractor: public detail::extractor<Ch, Type> { };
        template<class Type>
        struct inserter: public detail::inserter<Ch, Type> { };
        inline bool operator()(const key_type &key1, 
                               const key_type &key2) const
        {
            return key1 < key2;
        }
    };

    template<class Ch>
    struct iptree_traits
    {
        std::locale loc;
        typedef Ch char_type;
        typedef std::basic_string<Ch> key_type;
        typedef std::basic_string<Ch> data_type;
        template<class Type>
        struct extractor: public detail::extractor<Ch, Type> { };
        template<class Type>
        struct inserter: public detail::inserter<Ch, Type> { };
        inline bool operator()(Ch c1, Ch c2) const      // Helper for comparing characters
        {
            return std::toupper(c1, loc) < std::toupper(c2, loc);
        }
        inline bool operator()(const key_type &key1, 
                               const key_type &key2) const
        {
            return std::lexicographical_compare(key1.begin(), key1.end(), key2.begin(), key2.end(), *this);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Exceptions

    class ptree_error: public std::runtime_error
    {
    public:
        ptree_error(const std::string &what): std::runtime_error(what) { }
        ~ptree_error() throw() { }
    };

    class ptree_bad_data: public ptree_error
    {
    public:
        ptree_bad_data(const std::string &what): ptree_error(what) { }
        ~ptree_bad_data() throw() { }
    };
    
    class ptree_bad_path: public ptree_error
    {
    public:
        ptree_bad_path(const std::string &what): ptree_error(what) { }
        ~ptree_bad_path() throw() { }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Construction & destruction

    template<class Tr>
    basic_ptree<Tr>::basic_ptree()
    {
        m_impl = new impl;
        BOOST_PROPERTY_TREE_DEBUG_INCREMENT_INSTANCES_COUNT();
    }

    template<class Tr>
    basic_ptree<Tr>::basic_ptree(const data_type &rhs)
    {
        std::auto_ptr<impl> tmp(new impl);
        tmp->m_data = rhs;
        m_impl = tmp.release();
        BOOST_PROPERTY_TREE_DEBUG_INCREMENT_INSTANCES_COUNT();
    }

    template<class Tr>
    basic_ptree<Tr>::basic_ptree(const basic_ptree<Tr> &rhs)
    {
        std::auto_ptr<impl> tmp(new impl);
        tmp->m_data = rhs.data();
        m_impl = tmp.get();
        insert(end(), rhs.begin(), rhs.end());
        tmp.release();
        BOOST_PROPERTY_TREE_DEBUG_INCREMENT_INSTANCES_COUNT();
    }

    template<class Tr>
    basic_ptree<Tr>::~basic_ptree()
    {
        BOOST_PROPERTY_TREE_DEBUG_DECREMENT_INSTANCES_COUNT();
        delete m_impl;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Iterator access

    template<class Tr>
    typename basic_ptree<Tr>::iterator 
        basic_ptree<Tr>::begin()
    {
        return m_impl->m_container.begin();
    }

    template<class Tr>
    typename basic_ptree<Tr>::const_iterator 
        basic_ptree<Tr>::begin() const
    {
        return m_impl->m_container.begin();
    }

    template<class Tr>
    typename basic_ptree<Tr>::iterator 
        basic_ptree<Tr>::end()
    {
        return m_impl->m_container.end();
    }

    template<class Tr>
    typename basic_ptree<Tr>::const_iterator 
        basic_ptree<Tr>::end() const
    {
        return m_impl->m_container.end();
    }

    template<class Tr>
    typename basic_ptree<Tr>::reverse_iterator 
        basic_ptree<Tr>::rbegin()
    {
        return m_impl->m_container.rbegin();
    }

    template<class Tr>
    typename basic_ptree<Tr>::const_reverse_iterator 
        basic_ptree<Tr>::rbegin() const
    {
        return m_impl->m_container.rbegin();
    }

    template<class Tr>
    typename basic_ptree<Tr>::reverse_iterator 
        basic_ptree<Tr>::rend()
    {
        return m_impl->m_container.rend();
    }

    template<class Tr>
    typename basic_ptree<Tr>::const_reverse_iterator 
        basic_ptree<Tr>::rend() const
    {
        return m_impl->m_container.rend();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Data access

    template<class Tr>
    typename basic_ptree<Tr>::size_type 
        basic_ptree<Tr>::size() const
    {
        return m_impl->m_index.size();
    }

    template<class Tr>
    bool basic_ptree<Tr>::empty() const
    {
        return m_impl->m_index.empty();
    }

    template<class Tr>
    typename basic_ptree<Tr>::data_type &
        basic_ptree<Tr>::data()
    {
        return m_impl->m_data;
    }

    template<class Tr>
    const typename basic_ptree<Tr>::data_type &
        basic_ptree<Tr>::data() const
    {
        return m_impl->m_data;
    }

    template<class Tr>
    typename basic_ptree<Tr>::value_type &
        basic_ptree<Tr>::front()
    {
        return m_impl->m_container.front();
    }
    
    template<class Tr>
    const typename basic_ptree<Tr>::value_type &
        basic_ptree<Tr>::front() const
    {
        return m_impl->m_container.front();
    }

    template<class Tr>
    typename basic_ptree<Tr>::value_type &
        basic_ptree<Tr>::back()
    {
        return m_impl->m_container.back();
    }

    template<class Tr>
    const typename basic_ptree<Tr>::value_type &
        basic_ptree<Tr>::back() const
    {
        return m_impl->m_container.back();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Operators

    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::operator =(const basic_ptree<Tr> &rhs)
    {
        if (&rhs != this)
        {
            clear();
            data() = rhs.data();
            insert(end(), rhs.begin(), rhs.end());
        }
        return *this;
    }

    template<class Tr>
    bool basic_ptree<Tr>::operator ==(const basic_ptree<Tr> &rhs) const
    {
        
        // Data and sizes must be equal
        if (size() != rhs.size() || data() != rhs.data())
            return false;

        // Keys and children must be equal
        Tr tr;
        const_iterator it = begin();
        const_iterator it_rhs = rhs.begin();
        const_iterator it_end = end();
        for (; it != it_end; ++it, ++it_rhs)
            if (tr(it->first, it_rhs->first) || 
                tr(it_rhs->first, it->first) || 
                it->second != it_rhs->second)
                return false;

        // Equal
        return true;

    }

    template<class Tr>
    bool basic_ptree<Tr>::operator !=(const basic_ptree<Tr> &rhs) const
    {
        return !operator ==(rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Container operations

    template<class Tr>
    typename basic_ptree<Tr>::iterator 
        basic_ptree<Tr>::find(const key_type &key)
    {
        typename index_type::iterator it = m_impl->m_index.find(key);
        return it == m_impl->m_index.end() ? end() : it->second;
    }

    template<class Tr>
    typename basic_ptree<Tr>::const_iterator 
        basic_ptree<Tr>::find(const key_type &key) const
    {
        typename index_type::const_iterator it = m_impl->m_index.find(key);
        return it == m_impl->m_index.end() ? end() : it->second;
    }

    template<class Tr>
    typename basic_ptree<Tr>::size_type 
        basic_ptree<Tr>::count(const key_type &key) const
    {
        return m_impl->m_index.count(key);
    }

    template<class Tr>
    void basic_ptree<Tr>::clear()
    {
        m_impl->m_data = data_type();
        m_impl->m_container.clear();
        m_impl->m_index.clear();
    }

    template<class Tr>
    typename basic_ptree<Tr>::iterator 
    basic_ptree<Tr>::insert(iterator where, 
                                const value_type &value)
    {

        // Insert new value into container. If that throws nothing needs to be rolled back
        where = m_impl->m_container.insert(where, value);

        // Update index. If that throws we need to rollback the insert
        try {
            m_impl->m_index.insert(typename index_type::value_type(where->first, where));
        } 
        catch (...) {
            m_impl->m_container.erase(where);   // rollback the insert
            throw;
        }

        return where;
    }

    template<class Tr>
    template<class It>
    void basic_ptree<Tr>::insert(iterator where, 
                                     It first, 
                                     It last)
    {
        for (; first != last; ++first, ++where)
            where = insert(where, value_type(first->first, first->second));
    }

    template<class Tr>
    typename basic_ptree<Tr>::iterator 
        basic_ptree<Tr>::erase(iterator where)
    {

        // Remove from index
        typename index_type::iterator lo = m_impl->m_index.lower_bound(where->first);
        typename index_type::iterator hi = m_impl->m_index.upper_bound(where->first);
        for (; lo != hi; ++lo)
            if (lo->second == where)
            {
                m_impl->m_index.erase(lo);
                break;
            }
        
        // Remove from container    
        return m_impl->m_container.erase(where);

    }

    template<class Tr>
    typename basic_ptree<Tr>::size_type 
        basic_ptree<Tr>::erase(const key_type &key)
    {
        size_type count = 0;
        typename index_type::iterator lo = m_impl->m_index.lower_bound(key);
        if (lo != m_impl->m_index.end())
        {
            typename index_type::iterator hi = m_impl->m_index.upper_bound(key);
            while (lo != hi)
            {
                typename index_type::iterator it = lo++;
                erase(it->second);
                ++count;
            }
        }
        return count;
    }

    template<class Tr>
    template<class It> 
    typename basic_ptree<Tr>::iterator 
        basic_ptree<Tr>::erase(It first, 
                                   It last)
    {
        while (first != last)
            first = erase(first);
        return first;
    }

    template<class Tr>
    typename basic_ptree<Tr>::iterator
        basic_ptree<Tr>::push_front(const value_type &value)
    {
        return insert(begin(), value);
    }

    template<class Tr>
    typename basic_ptree<Tr>::iterator
        basic_ptree<Tr>::push_back(const value_type &value)
    {
        return insert(end(), value);
    }

    template<class Tr>
    void basic_ptree<Tr>::pop_front()
    {
        erase(begin());
    }

    template<class Tr>
    void basic_ptree<Tr>::pop_back()
    {
        erase(boost::prior(end()));
    }
        
    template<class Tr>
    void basic_ptree<Tr>::swap(basic_ptree<Tr> &rhs)
    {
        std::swap(m_impl, rhs.m_impl);
    }

    template<class Tr>
    void basic_ptree<Tr>::reverse()
    {
        m_impl->m_container.reverse();
    }
    
    template<class Tr>
    template<class SortTr> 
    void basic_ptree<Tr>::sort(SortTr tr)
    {
        m_impl->m_container.sort(tr);
    }

    ///////////////////////////////////////////////////////////////////////////
    // ptree operations

    // Get child ptree with custom separator
    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(char_type separator, 
                                       const key_type &path)
    {
        if (optional<basic_ptree<Tr> &> result = get_child_optional(separator, path))
            return result.get();
        else
            throw ptree_bad_path("key \"" + detail::narrow(path.c_str()) + "\" does not exist");
    }

    // Get child ptree with custom separator
    template<class Tr>
    const basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(char_type separator, 
                                       const key_type &path) const
    {
        basic_ptree<Tr> *nc_this = const_cast<basic_ptree<Tr> *>(this);
        return nc_this->get_child(separator, path);
    }

    // Get child ptree with custom separator
    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(char_type separator, 
                                       const key_type &path, 
                                       basic_ptree<Tr> &default_value)
    {
        if (optional<basic_ptree<Tr> &> result = get_child_optional(separator, path))
            return result.get();
        else
            return default_value;
    }

    // Get child ptree with custom separator
    template<class Tr>
    const basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(char_type separator, 
                                       const key_type &path, 
                                       const basic_ptree<Tr> &default_value) const
    {
        basic_ptree<Tr> *nc_this = const_cast<basic_ptree<Tr> *>(this);
        basic_ptree<Tr> &nc_default_value = const_cast<basic_ptree<Tr> &>(default_value);
        return nc_this->get_child(separator, path, nc_default_value);
    }


    // Get child ptree with custom separator
    template<class Tr>
    optional<basic_ptree<Tr> &> 
        basic_ptree<Tr>::get_child_optional(char_type separator, 
                                                const key_type &path)
    {
        typename key_type::size_type n = path.find(separator);
        if (n != key_type::npos)
        {
            key_type head = path.substr(0, n);
            key_type tail = path.substr(n + 1, key_type::npos);
            iterator it = find(head);
            if (it != end())
                return it->second.get_child_optional(separator, tail);
            else
                return optional<basic_ptree<Tr> &>();
        }
        else
        {
            iterator it = find(path);
            if (it != end())
                return it->second;
            else
                return optional<basic_ptree<Tr> &>();
        }
    }

    // Get child ptree with custom separator
    template<class Tr>
    optional<const basic_ptree<Tr> &> 
        basic_ptree<Tr>::get_child_optional(char_type separator, const key_type &path) const
    {
        basic_ptree<Tr> *nc_this = const_cast<basic_ptree<Tr> *>(this);
        optional<basic_ptree<Tr> &> tmp = nc_this->get_child_optional(separator, path);
        if (tmp)
            return optional<const basic_ptree<Tr> &>(tmp.get());
        else
            return optional<const basic_ptree<Tr> &>();
    }

    // Get child ptree with default separator
    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(const key_type &path)
    {
        return get_child(char_type('.'), path);
    }

    // Get child ptree with default separator
    template<class Tr>
    const basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(const key_type &path) const
    {
        return get_child(char_type('.'), path);
    }

    // Get child ptree with default separator
    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(const key_type &path, 
                                       basic_ptree<Tr> &default_value)
    {
        return get_child(char_type('.'), path, default_value);
    }
    
    // Get child ptree with default separator
    template<class Tr>
    const basic_ptree<Tr> &
        basic_ptree<Tr>::get_child(const key_type &path, 
                                       const basic_ptree<Tr> &default_value) const
    {
        return get_child(char_type('.'), path, default_value);
    }
    
    // Get child ptree with default separator
    template<class Tr>
    optional<basic_ptree<Tr> &> 
        basic_ptree<Tr>::get_child_optional(const key_type &path)
    {
        return get_child_optional(char_type('.'), path);
    }

    // Get child ptree with default separator
    template<class Tr>
    optional<const basic_ptree<Tr> &> 
        basic_ptree<Tr>::get_child_optional(const key_type &path) const
    {
        return get_child_optional(char_type('.'), path);
    }

    // Put child ptree with custom separator
    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::put_child(char_type separator, 
                                   const key_type &path, 
                                   const basic_ptree<Tr> &value,
                                   bool do_not_replace)
    {
        typename key_type::size_type n = path.find(separator);
        if (n == key_type::npos)
        {
            if (do_not_replace)
                return push_back(value_type(path, value))->second;
            else
            {
                iterator it = find(path);
                if (it == end())
                    return push_back(value_type(path, value))->second;
                else
                {
                    it->second = value;
                    return it->second;
                }
            }
        }
        else
        {
            key_type head = path.substr(0, n);
            key_type tail = path.substr(n + 1, key_type::npos);
            iterator it = find(head);
            if (it == end())
                it = push_back(value_type(head, basic_ptree<Tr>()));
            return it->second.put_child(separator, tail, value, do_not_replace);
        }
    }

    // Put child ptree with default separator
    template<class Tr>
    basic_ptree<Tr> &
        basic_ptree<Tr>::put_child(const key_type &path,
                                   const basic_ptree<Tr> &value,
                                   bool do_not_replace)
    {
        return put_child(char_type('.'), path, value, do_not_replace);
    }

    // Get value from data of ptree
    template<class Tr>
    template<class Type>
    Type basic_ptree<Tr>::get_own(const std::locale &loc) const
    {
        if (optional<Type> result = get_own_optional<Type>(loc))
            return result.get();
        else
            throw ptree_bad_data(std::string("conversion of data into type '") + 
                                 typeid(Type).name() + "' failed");
    }

    // Get value from data of ptree
    template<class Tr>
    template<class Type>
    Type basic_ptree<Tr>::get_own(const Type &default_value, 
                                      const std::locale &loc) const
    {
        if (optional<Type> result = get_own_optional<Type>(loc))
            return result.get();
        else
            return default_value;
    }

    // Get value from data of ptree
    template<class Tr>
    template<class CharType>
    std::basic_string<CharType> 
        basic_ptree<Tr>::get_own(const CharType *default_value, 
                                     const std::locale &loc) const
    {
        BOOST_STATIC_ASSERT((boost::is_same<char_type, CharType>::value == true)); // Character types must match
        return get_own(std::basic_string<CharType>(default_value), loc);
    }

    // Get value from data of ptree
    template<class Tr>
    template<class Type>
    optional<Type> 
        basic_ptree<Tr>::get_own_optional(const std::locale &loc) const
    {
        BOOST_STATIC_ASSERT(boost::is_pointer<Type>::value == false);   // Disallow pointer types, they are unsafe
        Type tmp;
        if (typename traits_type::template extractor<Type>()(m_impl->m_data, tmp, loc))
        {
            return optional<Type>(tmp);
        }
        else
            return optional<Type>();
    }

    // Get value from data of child ptree (custom path separator)
    template<class Tr>
    template<class Type>
    Type basic_ptree<Tr>::get(char_type separator,
                                  const key_type &path,
                                  const std::locale &loc) const
    {
        return get_child(separator, path).get_own<Type>(loc);
    }

    // Get value from data of child ptree (custom path separator)
    template<class Tr>
    template<class Type>
    Type basic_ptree<Tr>::get(char_type separator,
                                  const key_type &path, 
                                  const Type &default_value, 
                                  const std::locale &loc) const
    {
        if (optional<Type> result = get_optional<Type>(separator, path, loc))
            return *result;
        else
            return default_value;
    }

    // Get value from data of child ptree (custom path separator)
    template<class Tr>
    template<class CharType>
    std::basic_string<CharType> 
        basic_ptree<Tr>::get(char_type separator,
                                 const key_type &path, 
                                 const CharType *default_value,
                                 const std::locale &loc) const
    {
        BOOST_STATIC_ASSERT((boost::is_same<char_type, CharType>::value == true)); // Character types must match
        return get(separator, path, std::basic_string<CharType>(default_value), loc);
    }

    // Get value from data of child ptree (custom path separator)
    template<class Tr>
    template<class Type>
    optional<Type> 
        basic_ptree<Tr>::get_optional(char_type separator,
                                          const key_type &path, 
                                          const std::locale &loc) const
    {
        if (optional<const basic_ptree<Tr> &> child = get_child_optional(separator, path))
            return child.get().get_own_optional<Type>(loc);
        else
            return optional<Type>();
    }

    // Get value from data of child ptree (default path separator)
    template<class Tr>
    template<class Type>
    Type basic_ptree<Tr>::get(const key_type &path,
                                  const std::locale &loc) const
    {
        return get<Type>(char_type('.'), path, loc);
    }

    // Get value from data of child ptree (default path separator)
    template<class Tr>
    template<class Type>
    Type basic_ptree<Tr>::get(const key_type &path, 
                                  const Type &default_value,
                                  const std::locale &loc) const
    {
        return get(char_type('.'), path, default_value, loc);
    }

    // Get value from data of child ptree (default path separator)
    template<class Tr>
    template<class CharType>
    std::basic_string<CharType> 
        basic_ptree<Tr>::get(const key_type &path, 
                                 const CharType *default_value,
                                 const std::locale &loc) const
    {
        return get(char_type('.'), path, default_value, loc);
    }

    // Get value from data of child ptree (default path separator)
    template<class Tr>
    template<class Type>
    optional<Type> 
        basic_ptree<Tr>::get_optional(const key_type &path, 
                                          const std::locale &loc) const
    {
        return get_optional<Type>(char_type('.'), path, loc);
    }

    // Put value in data of ptree
    template<class Tr>
    template<class Type> 
    void basic_ptree<Tr>::put_own(const Type &value, const std::locale &loc)
    {
        using namespace boost;
        // Make sure that no pointer other than char_type * is allowed
        BOOST_STATIC_ASSERT((is_pointer<Type>::value == false ||
                             is_same<char_type, typename remove_const<typename remove_pointer<Type>::type>::type>::value == true));
        typename traits_type::template inserter<Type>()(m_impl->m_data, value, loc);
    }

    // Put value in data of child ptree (custom path separator)
    template<class Tr>
    template<class Type> 
    basic_ptree<Tr> &
        basic_ptree<Tr>::put(char_type separator,
                             const key_type &path, 
                             const Type &value,
                             bool do_not_replace,
                             const std::locale &loc)
    {
        optional<basic_ptree<Tr> &> child;
        if (!do_not_replace &&
            (child = get_child_optional(separator, path)))
        {
            child.get().put_own(value, loc);
            return *child;
        }
        else
        {
            basic_ptree<Tr> &child2 = put_child(separator, path, empty_ptree<basic_ptree<Tr> >(), do_not_replace);
            child2.put_own(value, loc);
            return child2;
        }
    }

    // Put value in data of child ptree (default path separator)
    template<class Tr>
    template<class Type> 
    basic_ptree<Tr> &
        basic_ptree<Tr>::put(const key_type &path, 
                             const Type &value,
                             bool do_not_replace,
                             const std::locale &loc)
    {
        return put(char_type('.'), path, value, do_not_replace, loc);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Debugging

#ifdef BOOST_PROPERTY_TREE_DEBUG

    template<class Tr>
    typename basic_ptree<Tr>::size_type 
        basic_ptree<Tr>::debug_get_instances_count() 
    { 
        empty_ptree<basic_ptree<Tr> >();    // Make sure empty ptree exists
        return debug_instances_count - 1;       // Do not count empty ptree
    }

    template<class Tr>
    typename basic_ptree<Tr>::size_type 
        basic_ptree<Tr>::debug_instances_count;

    template<class Tr>
    boost::detail::lightweight_mutex 
        basic_ptree<Tr>::debug_mutex;

#endif

    ///////////////////////////////////////////////////////////////////////////
    // Free functions

    template<class Ptree> 
    inline const Ptree &empty_ptree()
    {
        static Ptree pt;
        return pt;
    }

    template<class Tr>
    inline void swap(basic_ptree<Tr> &pt1, basic_ptree<Tr> &pt2)
    {
        pt1.swap(pt2);
    }

} }

// Undefine debug macros
#ifdef BOOST_PROPERTY_TREE_DEBUG
#   undef BOOST_PROPERTY_TREE_DEBUG_INCREMENT_INSTANCES_COUNT
#   undef BOOST_PROPERTY_TREE_DEBUG_DECREMENT_INSTANCES_COUNT
#endif

#endif
