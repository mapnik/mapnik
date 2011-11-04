#include "csv_datasource.hpp"

// boost
#include <boost/make_shared.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>

// mapnik
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/ptree_helpers.hpp> // mapnik::boolean

// stl
#include <sstream>
#include <fstream>      // fstream
#include <string>
#include <iterator>     // ostream_operator

// std lib
#include <stdio.h>
#include <stdlib.h>

using mapnik::datasource;
using mapnik::parameters;
using namespace boost::spirit;

DATASOURCE_PLUGIN(csv_datasource)

csv_datasource::csv_datasource(parameters const& params, bool bind)
    : datasource(params),
      desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding","utf-8")),
      extent_(),
      filename_(),
      inline_string_(),
      file_length_(0),
      row_limit_(*params_.get<int>("row_limit",0)),
      features_(),
      escape_(*params_.get<std::string>("escape","")),
      separator_(*params_.get<std::string>("separator","")),
      quote_(*params_.get<std::string>("quote","")),
      headers_(),
      manual_headers_(boost::trim_copy(*params_.get<std::string>("headers",""))),
      strict_(*params_.get<mapnik::boolean>("strict",false)),
      quiet_(*params_.get<mapnik::boolean>("quiet",false)),
      filesize_max_(*params_.get<float>("filesize_max",20.0)) // MB
{
    /* TODO:
      general:
        - refactor parser into generic class
        - tests of grid_renderer output
        - ensure that the attribute desc_ matches the first feature added
      alternate large file pipeline:
        - stat file, detect > 15 MB
        - build up csv line-by-line iterator
        - creates opportunity to filter attributes by map query
      speed:
        - add properties for wkt/lon/lat at parse time
        - remove boost::lexical_cast
        - add ability to pass 'filter' keyword to drop attributes at layer init
        - create quad tree on the fly for small/med size files
        - memory map large files for reading
        - smaller features (less memory overhead)
      usability:
        - enforce column names without leading digit
        - better error messages (add filepath) if not reading from string
        - move to spirit to tokenize and add character level error feedback:
          http://boost-spirit.com/home/articles/qi-example/tracking-the-input-position-while-parsing/
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
        parse_csv(in,escape_, separator_, quote_);
    }
    else
    {
        std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
        if (!in.is_open())
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        parse_csv(in,escape_, separator_, quote_);
        in.close();
    }
    is_bound_ = true;
}

template <typename T>
void csv_datasource::parse_csv(T& stream,
                               std::string const& escape,
                               std::string const& separator,
                               std::string const& quote) const
{
    stream.seekg (0, std::ios::end);
    file_length_ = stream.tellg();
    
    if (filesize_max_ > 0)
    {
        double file_mb = static_cast<double>(file_length_)/1048576;
        
        // throw if this is an unreasonably large file to read into memory
        if (file_mb > filesize_max_)
        {
            std::ostringstream s;
            s << "CSV Plugin: csv file is greater than " << filesize_max_ << "MB "
              << " - you should use a more efficient data format like sqlite, postgis or a shapefile "
              << " to render this data (set 'filesize_max=0' to disable this restriction if you have lots of memory)";
            throw mapnik::datasource_exception(s.str());
        }
    }

    // set back to start
    stream.seekg (0, std::ios::beg);

    // autodetect newlines
    char newline = '\n';
    int newline_count = 0;
    int carriage_count = 0;
    for(unsigned idx = 0; idx < file_length_; idx++)
    {
        char c = static_cast<char>(stream.get());
        if (c == '\n')
        {
            ++newline_count;
        }
        else if (c == '\r')
        {
            ++carriage_count;
        }
        // read at least 2000 bytes before testing
        if (idx == file_length_-1 || idx > 4000)
        {
            if (newline_count > carriage_count)
            {
                break;
            }
            else if (carriage_count > newline_count)
            {
                newline = '\r';
                break;
            }
        }
    }

    // set back to start
    stream.seekg (0, std::ios::beg);
    
    // get first line
    std::string csv_line;
    std::getline(stream,csv_line,newline);

    // if user has not passed a separator manually
    // then attempt to detect by reading first line
    std::string sep = boost::trim_copy(separator);
    if (sep.empty())
    {
        // default to ','
        sep = ",";
        int num_commas = std::count(csv_line.begin(), csv_line.end(), ',');
        // detect tabs
        int num_tabs = std::count(csv_line.begin(), csv_line.end(), '\t');
        if (num_tabs > 0)
        {
            if (num_tabs > num_commas)
            {
                sep = "\t";
#ifdef MAPNIK_DEBUG
                std::clog << "CSV Plugin: auto detected tab separator\n";
#endif
            }
        }
        else // pipes
        {
            int num_pipes = std::count(csv_line.begin(), csv_line.end(), '|');
            if (num_pipes > num_commas)
            {
                sep = "|";
#ifdef MAPNIK_DEBUG
                std::clog << "CSV Plugin: auto detected '|' separator\n";
#endif
            }
            else // semicolons
            {
                int num_semicolons = std::count(csv_line.begin(), csv_line.end(), ';');
                if (num_semicolons > num_commas)
                {
                    sep = ";";
    #ifdef MAPNIK_DEBUG
                    std::clog << "CSV Plugin: auto detected ';' separator\n";
    #endif
                }
            }
        }
    }

    // set back to start
    stream.seekg (0, std::ios::beg);
    
    typedef boost::escaped_list_separator<char> escape_type;

    std::string esc = boost::trim_copy(escape);
    if (esc.empty()) esc = "\\";
    
    std::string quo = boost::trim_copy(quote);
    if (quo.empty()) quo = "\"";

#ifdef MAPNIK_DEBUG
    std::clog << "CSV Plugin: csv grammer: sep: '" << sep << "' quo: '" << quo << "' esc: '" << esc << "'\n";
#endif

    boost::escaped_list_separator<char> grammer;
    try
    {
        //grammer = boost::escaped_list_separator<char>('\\', ',', '\"');
        grammer = boost::escaped_list_separator<char>(esc, sep, quo);
    }
    catch (const std::exception & ex )
    {
        std::ostringstream s;
        s << "CSV Plugin: " << ex.what();
        throw mapnik::datasource_exception(s.str());
    }
    
    typedef boost::tokenizer< escape_type > Tokenizer;

    int line_number(1);
    bool has_wkt_field = false;
    bool has_lat_field = false;
    bool has_lon_field = false;
    unsigned wkt_idx;
    unsigned lat_idx;
    unsigned lon_idx;

    if (!manual_headers_.empty())
    {
        Tokenizer tok(manual_headers_, grammer);
        Tokenizer::iterator beg = tok.begin();
        unsigned idx(0);
        for (; beg != tok.end(); ++beg)
        {
            std::string val = boost::trim_copy(*beg);
            std::string lower_val = boost::algorithm::to_lower_copy(val);
            if (lower_val == "wkt"
                || (lower_val.find("geom") != std::string::npos))
            {
                wkt_idx = idx;
                has_wkt_field = true;
            }
            if (lower_val == "x"
                || lower_val == "lon"
                || lower_val == "long"
                || (lower_val.find("longitude") != std::string::npos))
            {
                lon_idx = idx;
                has_lon_field = true;
            }
            if (lower_val == "y"
                || lower_val == "lat"
                || (lower_val.find("latitude") != std::string::npos))
            {
                lat_idx = idx;
                has_lat_field = true;
            }
            ++idx;
            headers_.push_back(val);
        }
    }
    else // parse first line as headers
    {
        while (std::getline(stream,csv_line,newline))
        {
            try
            {
                Tokenizer tok(csv_line, grammer);
                Tokenizer::iterator beg = tok.begin();
                std::string val = boost::trim_copy(*beg);
                
                // skip blank lines
                if (val.empty())
                {
                    // do nothing
                    ++line_number;
                }
                else
                {
                    int idx = -1;
                    for (; beg != tok.end(); ++beg)
                    {
                        ++idx;
                        val = boost::trim_copy(*beg);
                        if (val.empty())
                        {
                            if (strict_)
                            {
                                std::ostringstream s;
                                s << "CSV Plugin: expected a column header at line "
                                  << line_number << ", column " << idx
                                  << " - ensure this row contains valid header fields: '"
                                  << csv_line << "'\n";
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                // create a placeholder for the empty header
                                std::ostringstream s;
                                s << "_" << idx;
                                headers_.push_back(s.str());
                            }
                        }
                        else
                        {    
                            std::string lower_val = boost::algorithm::to_lower_copy(val);
                            if (lower_val == "wkt"
                                || (lower_val.find("geom") != std::string::npos))
                            {
                                wkt_idx = idx;
                                has_wkt_field = true;
                            }
                            if (lower_val == "x"
                                || lower_val == "lon"
                                || lower_val == "long"
                                || (lower_val.find("longitude") != std::string::npos))
                            {
                                lon_idx = idx;
                                has_lon_field = true;
                            }
                            if (lower_val == "y"
                                || lower_val == "lat"
                                || (lower_val.find("latitude") != std::string::npos))
                            {
                                lat_idx = idx;
                                has_lat_field = true;
                            }
                            headers_.push_back(val);
                        }
                    }
                    ++line_number;
                    break;
                }
            }
            catch (const std::exception & ex )
            {
                std::ostringstream s;
                s << "CSV Plugin: error parsing headers: " << ex.what();
                throw mapnik::datasource_exception(s.str());
            }
        }
    }

    if (!has_wkt_field && (!has_lon_field || !has_lat_field) )
    {
        std::ostringstream s;
        s << "CSV Plugin: could not detect column headers with the name of wkt ,x/y, or latitude/longitude - this is required for reading geometry data";
        throw mapnik::datasource_exception(s.str());
    }

    int feature_count(1);
    bool extent_initialized = false;
    unsigned num_headers = headers_.size();
    mapnik::transcoder tr(desc_.get_encoding());

    while (std::getline(stream,csv_line,newline))
    {
        if ((row_limit_ > 0) && (line_number > row_limit_))
        {
#ifdef MAPNIK_DEBUG
            std::clog << "CSV Plugin: row limit hit, exiting at feature: " << feature_count << "\n";
#endif
            break;
        }
        
        unsigned line_length = csv_line.length();

        // skip blank lines
        if (line_length < 5)
        {
            std::string trimmed = csv_line;
            boost::trim_if(trimmed,boost::algorithm::is_any_of("\",'\r\n"));
            if (trimmed.empty()){
                ++line_number;
                continue;
#ifdef MAPNIK_DEBUG
                std::clog << "CSV Plugin: empty row encountered at line: " << line_number << "\n";
#endif
            }
        }

        try
        {
            Tokenizer tok(csv_line, grammer);
            Tokenizer::iterator beg = tok.begin();
    
            // early return for strict mode
            if (strict_)
            {
                unsigned num_fields = std::distance(beg,tok.end());
                if (num_fields != num_headers)
                {
                    std::ostringstream s;
                    s << "CSV Plugin: # of headers != # of values parsed for row " << line_number << "\n";
                    throw mapnik::datasource_exception(s.str());
                }
            }
    
            mapnik::feature_ptr feature(mapnik::feature_factory::create(feature_count));
            double x(0);
            double y(0);
            bool parsed_x = false;
            bool parsed_y = false;
            bool parsed_wkt = false;
            bool null_geom = false;
            std::vector<std::string> collected;
            
            for (unsigned i = 0; i < num_headers; ++i)
            {
                std::string fld_name(headers_.at(i));
                collected.push_back(fld_name);
                std::string value;
                if (beg == tok.end())
                {
                    UnicodeString ustr = tr.transcode(value.c_str());
                    boost::put(*feature,fld_name,ustr);
                    //boost::put(*feature,fld_name,mapnik::value_null());
                    null_geom = true;
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                    }
                    continue;
                }
                else
                {
                    value = boost::trim_copy(*beg);
                    ++beg;
                }

                int value_length = value.length();
                
                // parse wkt
                if (has_wkt_field)
                {
                      if (i == wkt_idx)
                      {
                          // skip empty geoms
                          if (value.empty())
                          {
                              null_geom = true;
                              break;
                          }

                          // optimize simple "POINT (x y)"
                          // using this shaved 2 seconds off csv that took 8 seconds total to parse
                          if (value.find("POINT") == 0)
                          {
                              using boost::phoenix::ref;
                              using boost::spirit::qi::_1;
                              std::string::const_iterator str_beg = value.begin();
                              std::string::const_iterator str_end = value.end();
                              bool r = qi::phrase_parse(str_beg,str_end,
                                (
                                    qi::lit("POINT") >> '(' >> double_[ref(x) = _1] >>  double_[ref(y) = _1] >> ')'
                                ),
                                ascii::space);
                                
                              if (r && (str_beg == str_end))
                              {
                                  mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
                                  pt->move_to(x,y);
                                  feature->add_geometry(pt);
                                  parsed_wkt = true;
                              }
                              else
                              {
                                  std::ostringstream s;
                                  s << "CSV Plugin: expected well known text geometry: could not parse row "
                                    << line_number
                                    << ",column "
                                    << i << " - found: '"
                                    << value << "'";
                                  if (strict_)
                                  {
                                      throw mapnik::datasource_exception(s.str());                    
                                  }
                                  else
                                  {
                                      if (!quiet_) std::clog << s.str() << "\n";
                                  }
                              }
                          }
                          else
                          {
                              if (mapnik::from_wkt(value, feature->paths()))
                              {
                                  parsed_wkt = true;
                              }
                              else
                              {
                                  std::ostringstream s;
                                  s << "CSV Plugin: expected well known text geometry: could not parse row "
                                    << line_number
                                    << ",column "
                                    << i << " - found: '"
                                    << value << "'";
                                  if (strict_)
                                  {
                                      throw mapnik::datasource_exception(s.str());                    
                                  }
                                  else
                                  {
                                      if (!quiet_) std::clog << s.str() << "\n";
                                  }
                              }
                          }
                      }
                }
                else
                {
                    // longitude
                    if (i == lon_idx)
                    {
                        // skip empty geoms
                        if (value.empty())
                        {
                            null_geom = true;
                            break;
                        }

                        try 
                        {
                            x = boost::lexical_cast<double>(value);
                            parsed_x = true;
                        }
                        catch (boost::bad_lexical_cast & ex)
                        {
                            std::ostringstream s;
                            s << "CSV Plugin: expected a float value for longitude: could not parse row "
                              << line_number
                              << ", column "
                              << i << " - found: '"
                              << value << "'";
                            if (strict_)
                            {
                                throw mapnik::datasource_exception(s.str());                    
                            }
                            else
                            {
                                if (!quiet_) std::clog << s.str() << "\n";
                            }
                        }
                    }
                    // latitude
                    else if (i == lat_idx)
                    {
                        // skip empty geoms
                        if (value.empty())
                        {
                            null_geom = true;
                            break;
                        }

                        try 
                        {
                            y = boost::lexical_cast<double>(value);
                            parsed_y = true;
                        }
                        catch (boost::bad_lexical_cast & ex)
                        {
                            std::ostringstream s;
                            s << "CSV Plugin: expected a float value for latitude: could not parse row "
                              << line_number
                              << ", column "
                              << i << " - found: '"
                              << value << "'";
                            if (strict_)
                            {
                                throw mapnik::datasource_exception(s.str());                    
                            }
                            else
                            {
                                if (!quiet_) std::clog << s.str() << "\n";
                            }
                        }
                    }
                }
    
                // add all values as attributes
                // here we detect numbers and treat everything else as pure strings
                // this is intentional since boolean and null types are not common in csv editors
                if (value.empty())
                {
                    UnicodeString ustr = tr.transcode(value.c_str());
                    boost::put(*feature,fld_name,ustr);
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                    }
                }
                // only true strings are this long
                else if (value_length > 20)
                {
                    UnicodeString ustr = tr.transcode(value.c_str());
                    boost::put(*feature,fld_name,ustr);
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                    }
                
                }
                else if ((value[0] >= '0' && value[0] <= '9') || value[0] == '-')
                {
                    double float_val = 0.0;
                    std::string::const_iterator str_beg = value.begin();
                    std::string::const_iterator str_end = value.end();
                    bool r = qi::phrase_parse(str_beg,str_end,qi::double_,ascii::space,float_val);
                    if (r && (str_beg == str_end))
                    {
                        if (value.find(".") != std::string::npos)
                        {
                            boost::put(*feature,fld_name,float_val);
                            if (feature_count == 1)
                            {
                                desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::Double));
                            }
                        }
                        else
                        {
                            int val = static_cast<int>(float_val);
                            boost::put(*feature,fld_name,val);
                            if (feature_count == 1)
                            {
                                desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::Integer));
                            }
                        }
                    }
                    else
                    {
                        // fallback to normal string
                        UnicodeString ustr = tr.transcode(value.c_str());
                        boost::put(*feature,fld_name,ustr);
                        if (feature_count == 1)
                        {
                            desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                        }
                    }
                }
                else
                {
                    // fallback to normal string
                    UnicodeString ustr = tr.transcode(value.c_str());
                    boost::put(*feature,fld_name,ustr);
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                    }
                }
            }
    
            if (null_geom)
            {
                ++line_number;
                std::ostringstream s;
                s << "CSV Plugin: null geometry encountered for line "
                  << line_number;
                if (strict_)
                {
                    throw mapnik::datasource_exception(s.str());
                }
                else
                {
                    if (!quiet_) std::clog << s.str() << "\n";
                    continue;
                }
            }
                
            if (has_wkt_field)
            {
                if (parsed_wkt)
                {
                    if (!extent_initialized)
                    {
                        extent_initialized = true;
                        extent_ = feature->envelope();
                    }
                    else
                    {
                        extent_.expand_to_include(feature->envelope());
                    }
                    features_.push_back(feature);
                    ++feature_count;
                }
                else
                {
                    std::ostringstream s;
                    s << "CSV Plugin: could not read WKT geometry "
                      << "for line " << line_number << " - found " <<  headers_.size() 
                      << " with values like: " << csv_line << "\n";
                    if (strict_)
                    {
                        throw mapnik::datasource_exception(s.str());
                    }
                    else
                    {
                        if (!quiet_) std::clog << s.str() << "\n";
                        continue;
                    }
                }
            }
            else
            {
                if (parsed_x && parsed_y)
                {
                    mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
                    pt->move_to(x,y);
                    feature->add_geometry(pt);
                    features_.push_back(feature);
                    ++feature_count;
        
                    if (!extent_initialized)
                    {
                        extent_initialized = true;
                        extent_ = feature->envelope();
        
                    }
                    else
                    {
                        extent_.expand_to_include(feature->envelope());
                    }
                }
                else
                {
                    std::ostringstream s;
                    if (!parsed_x)
                    {
                        s << "CSV Plugin: does your csv have valid headers?\n"
                          << "Could not detect or parse any rows named 'x' or 'longitude' "
                          << "for line " << line_number << " but found " <<  headers_.size() 
                          << " with values like: " << csv_line << "\n"
                          << "for: " << boost::algorithm::join(collected, ",") << "\n";
                    }
                    if (!parsed_y)
                    {
                        s << "CSV Plugin: does your csv have valid headers?\n"
                          << "Could not detect or parse any rows named 'y' or 'latitude' "
                          << "for line " << line_number << " but found " <<  headers_.size() 
                          << " with values like: " << csv_line << "\n"
                          << "for: " << boost::algorithm::join(collected, ",") << "\n";
                    }
                    if (strict_)
                    {
                        throw mapnik::datasource_exception(s.str());
                    }
                    else
                    {
                        if (!quiet_) std::clog << s.str() << "\n";
                        continue;
                    }
                }
            }
            ++line_number;
        }
        catch (const mapnik::datasource_exception & ex )
        {
            if (strict_)
            {
                throw mapnik::datasource_exception(ex.what());
            }
            else
            {
                if (!quiet_) std::clog << ex.what() << "\n";
            }
        }
        catch (const std::exception & ex )
        {
            std::ostringstream s;
            s << "CSV Plugin: unexpected error parsing line: " << line_number
              << " - found " << headers_.size() << " with values like: " << csv_line << "\n"
              << " and got error like: " << ex.what();
            if (strict_)
            {
                throw mapnik::datasource_exception(s.str());
            }
            else
            {
                if (!quiet_) std::clog << s.str() << "\n";
            }
        }
    }
    if (!feature_count > 0)
    {
        if (!quiet_) std::clog << "CSV Plugin: could not parse any lines of data\n";
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
