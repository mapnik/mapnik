// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_REGISTRY_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_REGISTRY_PARSER_HPP_INCLUDED

// Include minimal version of windows.h if not included yet
#ifndef _WINDOWS_
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB 
#define NOMEMMGR
#define NOMETAFILE
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/ptree_utils.hpp>
#include <boost/cstdint.hpp>                // for 64 bit int
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>

namespace boost { namespace property_tree { namespace registry_parser
{

    //! Registry parser error
    class registry_parser_error: public ptree_error
    {
    public:
    
        // Construct error 
        registry_parser_error(const std::string &message, DWORD windows_error): 
            ptree_error(format_what(message, windows_error)), 
            m_windows_error(windows_error)
        { 
        }
    
        // Get windows error
        DWORD windows_error()
        {
            return m_windows_error;
        }
    
    private:

        DWORD m_windows_error;

        // Format error message to be returned by std::runtime_error::what()
        std::string format_what(const std::string &message,
                                DWORD windows_error)
        {
            std::stringstream stream;
            if (windows_error)
                stream << message << " (windows error 0x" << std::hex << windows_error << ")";
            else
                stream << message;
            return stream.str();
        }

    };

    // Translate from binary buffer to string
    template<class Ch>
    std::basic_string<Ch> translate(DWORD type, const std::vector<BYTE> &data)
    {

        typedef std::basic_string<Ch> Str;
        typedef std::basic_stringstream<Ch> Stream;

        Str value;
        switch (type)
        {
        
            // No data
            case REG_NONE:
                break;
            
            // Binary data
            case REG_BINARY: 
                if (!data.empty())
                {
                    Stream stream;
                    stream << std::hex << std::setfill(Ch('0'));
                    for (std::vector<BYTE>::const_iterator it = data.begin(), end = data.end(); 
                         it != end; ++it)
                        stream << std::setw(2) << static_cast<int>(*it) << Ch(' ');
                    value = stream.str();
                    value.resize(value.size() - 1); // remove final space
                }
                break;
            
            // DWORD value
            case REG_DWORD: 
                if (!data.empty())
                {
                    Stream stream;
                    stream << *reinterpret_cast<const DWORD *>(&data.front());
                    value = stream.str();
                }
                break;

            // QWORD value
            case REG_QWORD: 
                if (!data.empty())
                {
                    Stream stream;
                    stream << *reinterpret_cast<const boost::uint64_t *>(&data.front());
                    value = stream.str();
                }
                break;
            
            // Zero terminated string
            case REG_SZ: case REG_EXPAND_SZ: 
                if (!data.empty())
                    value.assign(reinterpret_cast<const Ch *>(&data.front()));
                break;
            
            // Unknown data type
            default:
                throw registry_parser_error("unsupported data type", 0);

        };
        return value;
    }

    // Translate from string to binary buffer
    template<class Ch>
    std::vector<BYTE> translate(DWORD type, const std::basic_string<Ch> &s)
    {

        typedef std::basic_string<Ch> Str;
        typedef std::basic_stringstream<Ch> Stream;

        std::vector<BYTE> data;
        switch (type)
        {
        
            // No data
            case REG_NONE:
                break;
            
            // Binary data
            case REG_BINARY:
                {
                    int v;
                    Stream stream(s);
                    stream >> std::hex;
                    while (1)
                    {
                        stream >> v >> std::ws;
                        if (stream.fail() || stream.bad())
                            throw registry_parser_error("bad REG_BINARY value", 0);
                        data.push_back(v);
                        if (stream.eof())
                            break;
                    }
                }
                break;
            
            // DWORD value
            case REG_DWORD: 
                {
                    DWORD v;
                    Stream stream(s);
                    stream >> v >> std::ws;
                    if (!stream.eof() || stream.fail() || stream.bad())
                        throw registry_parser_error("bad REG_DWORD value", 0);
                    for (size_t i = 0; i < sizeof(v); ++i)
                        data.push_back(*(reinterpret_cast<BYTE *>(&v) + i));
                }
                break;

            // QWORD value
            case REG_QWORD: 
                {
                    boost::uint64_t v;
                    Stream stream(s);
                    stream >> v;
                    if (!stream.eof() || stream.fail() || stream.bad())
                        throw registry_parser_error("bad REG_QWORD value", 0);
                    for (size_t i = 0; i < sizeof(v); ++i)
                        data.push_back(*(reinterpret_cast<BYTE *>(&v) + i));
                }
                break;
            
            // Zero terminated string
            case REG_SZ: case REG_EXPAND_SZ:
                {
                    const Ch *sz = s.c_str();
                    size_t len = (s.size() + 1) * sizeof(Ch);
                    for (size_t i = 0; i < len; ++i)
                        data.push_back(*(reinterpret_cast<const BYTE *>(sz) + i));
                }
                break;
            
            // Unknown data type
            default:
                throw registry_parser_error("unsupported data type", 0);

        };
        return data;
    }

    /////////////////////////////////////////////////////////////////////////////
    // Registry functions wrappers
    
    template<class Ch> 
    inline LONG reg_create_key_ex(HKEY hkey, const Ch *subkey, REGSAM sam, HKEY *result);

    template<> 
    inline LONG reg_create_key_ex<char>(HKEY hkey, const char *subkey, REGSAM sam, HKEY *result)
    {
        return RegCreateKeyExA(hkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, sam, NULL, result, NULL);
    }
    
    template<> 
    inline LONG reg_create_key_ex<wchar_t>(HKEY hkey, const wchar_t *subkey, REGSAM sam, HKEY *result)
    {
        return RegCreateKeyExW(hkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, sam, NULL, result, NULL);
    }

    template<class Ch> 
    inline LONG reg_set_value_ex(HKEY hkey, const Ch *name, DWORD type, const BYTE *data, DWORD size);

    template<> 
    inline LONG reg_set_value_ex<char>(HKEY hkey, const char *name, DWORD type, const BYTE *data, DWORD size)
    {
        return RegSetValueExA(hkey, name, 0, type, data, size);
    }

    template<> 
    inline LONG reg_set_value_ex<wchar_t>(HKEY hkey, const wchar_t *name, DWORD type, const BYTE *data, DWORD size)
    {
        return RegSetValueExW(hkey, name, 0, type, data, size);
    }

    template<class Ch> 
    inline LONG reg_open_key_ex(HKEY hkey, const Ch *subkey, REGSAM sam, HKEY *result);

    template<> 
    inline LONG reg_open_key_ex<char>(HKEY hkey, const char *subkey, REGSAM sam, HKEY *result)
    {
        return RegOpenKeyExA(hkey, subkey, 0, sam, result);
    }
    
    template<> 
    inline LONG reg_open_key_ex<wchar_t>(HKEY hkey, const wchar_t *subkey, REGSAM sam, HKEY *result)
    {
        return RegOpenKeyExW(hkey, subkey, 0, sam, result);
    }

    template<class Ch> 
    inline LONG reg_enum_key_ex(HKEY hkey, DWORD index, Ch *name, DWORD *size);

    template<> 
    inline LONG reg_enum_key_ex<char>(HKEY hkey, DWORD index, char *name, DWORD *size)
    {
        FILETIME ft;
        return RegEnumKeyExA(hkey, index, name, size, 0, NULL, NULL, &ft);
    }

    template<> 
    inline LONG reg_enum_key_ex<wchar_t>(HKEY hkey, DWORD index, wchar_t *name, DWORD *size)
    {
        FILETIME ft;
        return RegEnumKeyExW(hkey, index, name, size, 0, NULL, NULL, &ft);
    }

    template<class Ch> 
    inline LONG reg_enum_value(HKEY hkey, DWORD index, Ch *name, DWORD *name_size, DWORD *type, BYTE *data, DWORD *data_size);

    template<> 
    inline LONG reg_enum_value<char>(HKEY hkey, DWORD index, char *name, DWORD *name_size, DWORD *type, BYTE *data, DWORD *data_size)
    {
        return RegEnumValueA(hkey, index, name, name_size, NULL, type, data, data_size);
    }

    template<> 
    inline LONG reg_enum_value<wchar_t>(HKEY hkey, DWORD index, wchar_t *name, DWORD *name_size, DWORD *type, BYTE *data, DWORD *data_size)
    {
        return RegEnumValueW(hkey, index, name, name_size, NULL, type, data, data_size);
    }

    template<class Ch> 
    inline LONG reg_query_info_key(HKEY hkey, DWORD *max_subkey_len, DWORD *max_name_len, DWORD *max_value_len);

    template<> 
    inline LONG reg_query_info_key<char>(HKEY hkey, DWORD *max_subkey_len, DWORD *max_name_len, DWORD *max_value_len)
    {
        return RegQueryInfoKeyA(hkey, NULL, NULL, NULL, NULL, max_subkey_len, NULL, NULL, max_name_len, max_value_len, NULL, NULL);
    }

    template<> 
    inline LONG reg_query_info_key<wchar_t>(HKEY hkey, DWORD *max_subkey_len, DWORD *max_name_len, DWORD *max_value_len)
    {
        return RegQueryInfoKeyW(hkey, NULL, NULL, NULL, NULL, max_subkey_len, NULL, NULL, max_name_len, max_value_len, NULL, NULL);
    }

    /////////////////////////////////////////////////////////////////////////////
    // Registry key handle wrapper
    
    template<class Ch>
    class reg_key
    {
    public:
        typedef std::basic_string<Ch> Str;
        reg_key(HKEY root, const std::basic_string<Ch> &key, bool create):
            hkey(0)
        {
            if (create)
            {
                LONG result = reg_create_key_ex(root, key.c_str(), KEY_WRITE, &hkey);
                if (result != ERROR_SUCCESS)
                    throw registry_parser_error("RegCreateKeyEx failed", result);
            }
            else
            {
                LONG result = reg_open_key_ex(root, key.c_str(), KEY_READ, &hkey);
                if (result != ERROR_SUCCESS)
                    throw registry_parser_error("RegOpenKeyEx failed", result);
            }
            BOOST_ASSERT(hkey);
        }
        ~reg_key()
        {
            BOOST_ASSERT(hkey);
            RegCloseKey(hkey);
        }
        HKEY handle()
        {
            BOOST_ASSERT(hkey);
            return hkey;
        }
    private:
        HKEY hkey;
    };
    
    /////////////////////////////////////////////////////////////////////////////
    // Registry parser
    
    //! Read registry
    template<class Ptree>
    void read_registry(HKEY root, 
                       const std::basic_string<typename Ptree::char_type> &key, 
                       Ptree &pt)
    {

        typedef typename Ptree::char_type Ch;
        typedef std::basic_string<Ch> Str;
        typedef std::basic_stringstream<Ch> Stream;
        
        Ptree local;
        
        // Open key
        reg_key<Ch> rk(root, key, false);
        
        // Query key info
        DWORD max_subkey_len, max_name_len, max_value_len;
        LONG result = reg_query_info_key<Ch>(rk.handle(), &max_subkey_len, &max_name_len, &max_value_len);
        if (result != ERROR_SUCCESS)
            throw registry_parser_error("RegQueryInfoKey failed", result);

        // For all subkeys
        std::vector<Ch> subkey(max_subkey_len + 1);
        for (DWORD index = 0; true; ++index)
        {
            
            // Get subkey name
            DWORD size = static_cast<DWORD>(subkey.size());
            LONG result = reg_enum_key_ex(rk.handle(), index, &subkey.front(), &size);
            if (result == ERROR_NO_MORE_ITEMS)
                break;
            if (result != ERROR_SUCCESS)
                throw registry_parser_error("RegEnumKeyEx failed", result);
            
            // Parse recursively
            Ptree &child = local.push_back(typename Ptree::value_type(&subkey.front(), Ptree()))->second;
            read_registry<Ptree>(rk.handle(), &subkey.front(), child);

        }

        // For all values
        for (DWORD index = 0; true; ++index)
        {

            // Resize data to max size
            std::vector<Ch> name(max_name_len + 1);
            std::vector<BYTE> data(max_value_len + 1);
            
            // Get name and value from registry
            DWORD name_size = static_cast<DWORD>(name.size());
            DWORD data_size = static_cast<DWORD>(data.size());
            DWORD type;
            result = reg_enum_value<Ch>(rk.handle(), index, &name.front(), &name_size, &type, &data.front(), &data_size);
            if (result == ERROR_NO_MORE_ITEMS)
                break;
            if (result != ERROR_SUCCESS)
                throw registry_parser_error("RegEnumValue failed", result);

            // Truncate data to actual size
            name.resize(name_size + 1);
            data.resize(data_size);

            // Translate and put value in tree
            Str value = translate<Ch>(type, data);
            if (name_size > 0)
            {
                local.put(Str(detail::widen<Ch>("\\values.") + &name.front()), value);
                local.put(Str(detail::widen<Ch>("\\types.") + &name.front()), type);
            }
            else
                local.data() = value;

        }

        // Swap pt and local
        pt.swap(local);

    }

    //! Write registry
    template<class Ptree>
    void write_registry(HKEY root, 
                        const std::basic_string<typename Ptree::char_type> &key, 
                        const Ptree &pt)
    {

        typedef typename Ptree::char_type Ch;
        typedef std::basic_string<Ch> Str;
        typedef std::basic_stringstream<Ch> Stream;
        
        // Create key
        reg_key<Ch> rk(root, key, true);

        // Set default key value
        if (!pt.data().empty())
        {
            std::vector<BYTE> data = translate<Ch>(REG_SZ, pt.data());
            reg_set_value_ex<Ch>(rk.handle(), NULL, REG_SZ, 
                                 data.empty() ? NULL : &data.front(), 
                                 static_cast<DWORD>(data.size()));
        }

        // Create values
        const Ptree &values = pt.get_child(detail::widen<Ch>("\\values"), empty_ptree<Ptree>());
        const Ptree &types = pt.get_child(detail::widen<Ch>("\\types"), empty_ptree<Ptree>());
        for (typename Ptree::const_iterator it = values.begin(), end = values.end(); it != end; ++it)
        {
            DWORD type = types.get(it->first, REG_SZ);
            std::vector<BYTE> data = translate<Ch>(type, it->second.data());
            reg_set_value_ex<Ch>(rk.handle(), it->first.c_str(), type, 
                                 data.empty() ? NULL : &data.front(), 
                                 static_cast<DWORD>(data.size()));
        }

        // Create subkeys
        for (typename Ptree::const_iterator it = pt.begin(), end = pt.end(); it != end; ++it)
            if (&it->second != &values && &it->second != &types)
                write_registry(rk.handle(), it->first, it->second);

    }

} } }

namespace boost { namespace property_tree
{
    using registry_parser::read_registry;
    using registry_parser::write_registry;
    using registry_parser::registry_parser_error;
} }

#endif
