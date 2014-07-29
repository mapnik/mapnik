#ifndef DUMP_XML_HPP
#define DUMP_XML_HPP
#include <mapnik/xml_node.hpp>

namespace mapnik
{

// Debug dump ptree XML representation.

void dump_xml(xml_node const& xml, unsigned level=0)
{
    std::string indent;
    unsigned i;
    for (i=0; i<level; i++)
    {
        indent += "    ";
    }
    xml_node::attribute_map const& attr_map = xml.get_attributes();
    std::cerr << indent <<"[" << xml.name();
    for (auto const& attr : attr_map)
    {
        std::cerr << " (" << attr.first << ", " << attr.second.value << ", " << attr.second.processed << ")";
    }
    std::cerr << "]" << "\n";
    if (xml.is_text()) std::cerr << indent << "text: '" << xml.text() << "'\n";
    for (auto const& child : xml)
    {
        dump_xml(child, level+1);
    }
    std::cerr << indent << "[/" << xml.name() << "]" << "\n";
}

}
#endif // DUMP_XML_HPP
