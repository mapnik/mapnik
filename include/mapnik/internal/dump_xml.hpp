#ifndef DUMP_XML_HPP
#define DUMP_XML_HPP
#include <mapnik/xml_node.hpp>
namespace mapnik
{

/* Debug dump ptree XML representation.
 */
void dump_xml(xml_node const& xml, unsigned level=0)
{
    std::string indent;
    unsigned i;
    for (i=0; i<level; i++)
    {
        indent += "    ";
    }
    xml_node::attribute_map const& attr = xml.get_attributes();
    std::cerr << indent <<"[" << xml.name();
    xml_node::attribute_map::const_iterator aitr = attr.begin();
    xml_node::attribute_map::const_iterator aend = attr.end();
    for (;aitr!=aend; aitr++)
    {
        std::cerr << " (" << aitr->first << ", " << aitr->second.value << ", " << aitr->second.processed << ")";
    }
    std::cerr << "]" << "\n";
    if (xml.is_text()) std::cerr << indent << "text: '" << xml.text() << "'\n";
    xml_node::const_iterator itr = xml.begin();
    xml_node::const_iterator end = xml.end();
    for (; itr!=end; itr++)
    {
        dump_xml(*itr, level+1);
    }
    std::cerr << indent << "[/" << xml.name() << "]" << "\n";
}

}
#endif // DUMP_XML_HPP
