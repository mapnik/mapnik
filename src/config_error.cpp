#include <mapnik/config_error.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/util/conversions.hpp>

namespace mapnik
{

config_error::config_error(std::string const& what)
    : what_(what),
      line_number_(0),
      file_(),
      node_name_(),
      msg_()
{
}


config_error::config_error(std::string const& what,
                           xml_node const& node)
    : what_(what),
      line_number_(node.line()),
      file_(node.filename()),
      node_name_(node.name()),
      msg_()
{
}


config_error::config_error(std::string const& what,
                           unsigned line_number,
                           std::string const& filename)
    : what_(what),
      line_number_(line_number),
      file_(filename),
      node_name_(),
      msg_()
{
}

char const* config_error::what() const throw()
{
    msg_ = what_;
    if (!node_name_.empty())
    {
        msg_ += " in " + node_name_;
    }
    if (line_number_ > 0)
    {
        std::string number;
        if (util::to_string(number,line_number_))
        {
            msg_ += " at line " + number;
        }
    }
    if (!file_.empty())
    {
        msg_ += " of '" + file_ + "'";
    }
    return msg_.c_str();
}

void config_error::append_context(std::string const& ctx) const
{
    what_ += " " + ctx;
}

void config_error::append_context(std::string const& ctx, xml_node const& node) const
{
    append_context(ctx);
    append_context(node);
}

void config_error::append_context(xml_node const& node) const
{
    if (!line_number_) line_number_ = node.line();
    if (node_name_.empty()) node_name_ = node.name();
    if (file_.empty()) file_ = node.filename();
}

}
