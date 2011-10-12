#include "csv_datasource.hpp"

// boost
#include <boost/make_shared.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

// mapnik
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/wkt/wkt_factory.hpp>

// stl
#include <sstream>
#include <fstream>      // fstream
#include <string>
#include <iterator>     // ostream_operator

// clib
#include <stdio.h>
#include <stdlib.h>
//#include <ctype.h>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(csv_datasource)

csv_datasource::csv_datasource(parameters const& params, bool bind)
   : datasource(params),
     desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding","utf-8")),
     extent_(),
     filename_(),
     inline_string_(),
     features_(),
     separator_(*params_.get<std::string>("separator",",")),
     escape_(*params_.get<std::string>("escape","\\")),
     quote_(*params_.get<std::string>("quote","\""))
{
    /* TODO:
      build up features lazily, and filter cols using query
      support for newlines other than \n
      https://docs.google.com/a/dbsgeo.com/spreadsheet/pub?key=0AqV4OJpywingdFBCV1o3SXp3OU94U3VJWTRoLWRPbGc&output=csv
      spatial index
    */
    
    boost::optional<std::string> inline_string = params_.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        boost::optional<std::string> file = params_.get<std::string>("file");
        if (!file) throw mapnik::datasource_exception("CSV Plugin: missing <file> parameter");
    
        boost::optional<std::string> base = params_.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }

    if (bind)
    {
        this->bind();
    }
}


csv_datasource::~csv_datasource() { }

void csv_datasource::bind() const
{
    if (is_bound_) return;
    
    if (!inline_string_.empty())
    {
        std::istringstream in(inline_string_);
        parse_csv(in);
    }
    else
    {
        std::ifstream in(filename_.c_str());
        if (!in.is_open())
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        parse_csv(in);
        in.close();
    }
    is_bound_ = true;
}

template <typename T>
void csv_datasource::parse_csv(T& stream) const
{
    typedef boost::escaped_list_separator<char> separator_type;
    typedef boost::tokenizer< separator_type > Tokenizer;
    std::string csv_line;
    boost::escaped_list_separator<char> grammer(escape_, separator_, quote_);
    mapnik::transcoder tr(desc_.get_encoding());
    int line_no(1);
    int feature_count(0);

    while (std::getline(stream,csv_line))
    {
        Tokenizer tok(csv_line, grammer);

        Tokenizer::iterator beg = tok.begin();
        std::string val = boost::trim_copy(*beg);

        // skip lines with leading blanks (assume whole line is empty)
        if (val.empty()) continue;
        
        // handle headers
        if (line_no == 1)
        {
            unsigned i = 0;
            for (; beg != tok.end(); ++beg)
            {
                std::string value = boost::trim_copy(*beg);
                // todo - ensure col names do not start with digit
                try
                {
                    headers_.push_back(boost::lexical_cast<std::string>(value));
                }
                catch (boost::bad_lexical_cast & ex)
                {
                    std::ostringstream s;
                    s << "CSV Plugin: expected string type column header - could not parse column "
                      << i << " - found: '"
                      << value << "'";
                    throw mapnik::datasource_exception(s.str());
                }
            }
            ++i;
        }
        else
        {
            double x;
            double y;
            bool parsed_x = false;
            bool parsed_y = false;
            bool has_wkt_field = false;
            bool parsed_wkt = false;
            bool extent_initialized = false;
            // look for wkt field
            if (std::find(headers_.begin(), headers_.end(), "wkt") != headers_.end())
            {
                has_wkt_field = true;
            }

            mapnik::feature_ptr feature(mapnik::feature_factory::create(feature_count));
            ++feature_count;
    
            unsigned i = 0;
            for (;beg != tok.end(); ++beg)
            {
                std::string value = boost::trim_copy(*beg);
                
                // avoid range error if trailing separator on last col
                // TODO - should we throw instead?
                if (i >= headers_.size())
                    break;
                
                std::string fld_name(headers_.at(i));
                
                // parse wkt
                if (has_wkt_field && fld_name == "wkt" && !parsed_wkt)
                {
                      // skip empty geoms
                      if (value.empty())
                          break;
                      bool result = mapnik::from_wkt(value, feature->paths());
                      if (!result)
                      {
                        std::ostringstream s;
                        s << "CSV Plugin: expected well known text geometry: could not parse row "
                          << line_no
                          << ",column "
                          << i << " - found: '"
                          << value << "'";
                        throw mapnik::datasource_exception(s.str());
                      }
                      parsed_wkt = true;
                }
                // longitude
                else if ( !parsed_x && (fld_name == "x" || fld_name == "lon" || fld_name == "longitude") )
                {
                    try 
                    {
                        x = boost::lexical_cast<double>(value);
                        parsed_x = true;
                    }
                    catch (boost::bad_lexical_cast & ex)
                    {
                        std::ostringstream s;
                        s << "CSV Plugin: expected longitude: could not parse row "
                          << line_no
                          << ", column "
                          << i << " - found: '"
                          << value << "'";
                        throw mapnik::datasource_exception(s.str());
                    }
                }
                // latitude
                else if ( !parsed_y && (fld_name == "y" || fld_name == "lat" || fld_name == "latitude") )
                {
                    try 
                    {
                        y = boost::lexical_cast<double>(value);
                        parsed_y = true;
                    }
                    catch (boost::bad_lexical_cast & ex)
                    {
                        std::ostringstream s;
                        s << "CSV Plugin: expected latitude: could not parse row "
                          << line_no
                          << ", column "
                          << i << " - found: '"
                          << value << "'";
                        throw mapnik::datasource_exception(s.str());
                    }
                }
                // add all values as attributes
                try
                {
                    if (value.find(".") != std::string::npos)
                    {
                        double val = boost::lexical_cast<double>(value);
                        boost::put(*feature,fld_name,val);
                        if (line_no == 2)
                            desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::Double));
                    }
                    else
                    {
                        int val = boost::lexical_cast<int>(value);
                        boost::put(*feature,fld_name,val);
                        if (line_no == 2)
                            desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::Integer));
                    }
                }
                catch (boost::bad_lexical_cast & ex)
                {
                    std::string val = boost::lexical_cast<std::string>(value);
                    if (!val.empty())
                    {
                        if (val == "true")
                        {
                            boost::put(*feature,fld_name,true);
                            if (line_no == 2)
                                desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::Boolean));
                        }
                        else if(val == "false")
                        {
                            boost::put(*feature,fld_name,false);
                            if (line_no == 2)
                                desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::Boolean));
                        }
                        else
                        {
                            UnicodeString ustr = tr.transcode(val.c_str());
                            boost::put(*feature,fld_name,ustr);
                            if (line_no == 2)
                                desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                        }
                    }
                    else
                    {
                        boost::put(*feature,headers_.at(i),mapnik::value_null());
                    }
                }
                ++i;
            }
        
            if (has_wkt_field)
            {
                if (parsed_wkt)
                {
                    if (line_no >= 2 && !extent_initialized)
                    {
                        extent_initialized = true;
                        extent_ = feature->envelope();
                    }
                    else
                    {
                        extent_.expand_to_include(feature->envelope());
                    }
                    features_.push_back(feature);
                }
            }
            else
            {
                if (!parsed_x)
                {
                    std::ostringstream s;
                    s << "CSV Plugin: could not detect or parse any rows named 'x', 'lon' or 'longitude' "
                      << "does your csv have headers?";
                    throw mapnik::datasource_exception(s.str());
                }
                else if (!parsed_y)
                {
                    std::ostringstream s;
                    s << "CSV Plugin: could not detect or parse rows named 'y', 'lat' or 'latitude' "
                      << "does your csv have headers?";
                    throw mapnik::datasource_exception(s.str());
                }
                else
                {
                    if (line_no >= 2 && !extent_initialized)
                    {
                        extent_initialized = true;
                        extent_.init(x, y, x, y);
                    }
                    else
                    {
                        extent_.expand_to_include(x,y);
                    }
            
                    mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
                    pt->move_to(x,y);
                    feature->add_geometry(pt);
                    features_.push_back(feature);        
                }
            }
        }
        ++line_no;
    }
}

std::string csv_datasource::name()
{
    return "csv";
}

int csv_datasource::type() const
{
    return datasource::Vector;
}

mapnik::box2d<double> csv_datasource::envelope() const
{
    if (!is_bound_) bind();

    return extent_;
}

mapnik::layer_descriptor csv_datasource::get_descriptor() const
{
    if (!is_bound_) bind();
   
    return desc_;
}

mapnik::featureset_ptr csv_datasource::features(mapnik::query const& q) const
{
    if (!is_bound_) bind();
    
    // TODO - should we check q.property_names() and throw if not found in headers_?
    //const std::set<std::string>& attribute_names = q.property_names();
      
    return boost::make_shared<mapnik::memory_featureset>(q.get_bbox(),features_);
}

mapnik::featureset_ptr csv_datasource::features_at_point(mapnik::coord2d const& pt) const
{
    if (!is_bound_) bind();
    
    throw mapnik::datasource_exception("CSV Plugin: features_at_point is not supported yet");
}
