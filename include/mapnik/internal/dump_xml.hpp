#ifndef DUMP_XML_HPP
#define DUMP_XML_HPP
#include <boost/property_tree/ptree.hpp>

/* Debug dump ptree XML representation.
*/
void dump_xml(boost::property_tree::ptree const& xml, unsigned level=0)
{
    std::string indent;
    int i;
    for (i=0; i<level; i++)
    {
        indent += "    ";
    }
    if (xml.data().length()) std::cout << indent << "data: '" << xml.data() << "'\n";
    boost::property_tree::ptree::const_iterator itr = xml.begin();
    boost::property_tree::ptree::const_iterator end = xml.end();
    for (; itr!=end; itr++)
    {
        std::cout << indent <<"[" << itr->first << "]" << "\n";
        dump_xml(itr->second, level+1);
        std::cout << indent << "[/" << itr->first << "]" << "\n";
    }
}


#endif // DUMP_XML_HPP
