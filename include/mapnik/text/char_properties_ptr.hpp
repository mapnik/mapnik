#ifndef CHAR_PROPERTIES_PTR_HPP
#define CHAR_PROPERTIES_PTR_HPP
#include <boost/shared_ptr.hpp>
namespace mapnik
{
struct char_properties;
typedef boost::shared_ptr<char_properties> char_properties_ptr;
}

#endif // CHAR_PROPERTIES_PTR_HPP
