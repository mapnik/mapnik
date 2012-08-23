/*==============================================================================
    Copyright (c) 2001-2011 Joel de Guzman
    Copyright (c) 2010      Bryce Lelbach
 
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
 
#include <mapnik/position_iterator.hpp>
 
#include <sstream>
 
namespace mapnik {
 
source_location::source_location (int l, int c)
  : line(l),
    column(c)
{ }
 
source_location::source_location ()
  : line(-1),
    column(-1)
{ }
 
bool source_location::valid() {
    return (line != -1) && (column != -1);
}
 
std::string source_location::get_string() {
    std::stringstream s;
    
    if (valid())
        s << "Line: " << line << " Col: " << column;
    else 
        s << "Unknown Position";
 
    return s.str();
}
 
bool source_location::operator==(source_location const& other) const {
    return other.line == line && other.column == column;
}
 
}