#include <mapnik/config_error.hpp>
#include <mapnik/xml_tree.hpp>

namespace mapnik
{
config_error::config_error(std::string const& what, xml_node const* node, std::string const& filename)
    : what_( what ), file_(filename)
{
    if (node)
    {
        node_name_ = node->name();
        line_number_ = node->line();
    }
}


config_error::config_error(unsigned line_number, std::string const& filename, std::string const& what)
    : what_( what ), line_number_(line_number), file_(filename)
{

}

 char const* config_error::what() const throw()
{
    std::stringstream s;
    s << file_;
    if (line_number_ > 0) s << " line " << line_number_;
    if (!node_name_.empty()) s << " in node "<< node_name_;
    if (line_number_ > 0 || !file_.empty()) s << ": ";
    s << what_;
    return s.str().c_str();
}

void config_error::append_context(const std::string & ctx, xml_node const* node, std::string const& filename) const
{
    what_ += " " + ctx;
    if (node)
    {
        if (!line_number_) line_number_ = node->line();
        if (node_name_.empty()) node_name_ = node->name();
        if (file_.empty()) file_ = filename;
    }
}
}
