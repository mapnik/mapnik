//#include <boost/config/warning_disable.hpp>

#include <boost/filesystem/convenience.hpp>
namespace fs = boost::filesystem;
using fs::path;
namespace sys = boost::system;

#include <boost/detail/lightweight_test.hpp>
//#include <boost/bind.hpp>
//#include <fstream>
#include <iostream>
#include <mapnik/font_engine_freetype.hpp>


//  --------------------------------------------------------------------------//

int main( int, char*[] )
{


//  font registration() tests  ----------------------------------------------//

  std::string fontdir("fonts/");
  
  BOOST_TEST( fs::exists( fontdir ) );
  BOOST_TEST( fs::is_directory( fontdir ) );

  std::vector<std::string> face_names;

  std::string foo("foo");

  // fake directories
  BOOST_TEST( !mapnik::freetype_engine::register_fonts(foo , true ) );
  face_names = mapnik::freetype_engine::face_names();
  BOOST_TEST( face_names.size() == 0 );
  BOOST_TEST( !mapnik::freetype_engine::register_fonts(foo) );
  face_names = mapnik::freetype_engine::face_names();
  BOOST_TEST( face_names.size() == 0 );

  // directories without fonts
  std::string src("src");
  // a legitimate directory will return true even it is does not 
  // successfully register a font...
  BOOST_TEST( mapnik::freetype_engine::register_fonts(src , true ) );
  face_names = mapnik::freetype_engine::face_names();
  BOOST_TEST( face_names.size() == 0 );
  std::clog << "number of registered fonts: " << face_names.size() << std::endl;

  // register unifont
  BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir) );
  face_names = mapnik::freetype_engine::face_names();
  std::clog << "number of registered fonts: " << face_names.size() << std::endl;
  BOOST_TEST( face_names.size() > 0 );
  BOOST_TEST( face_names.size() == 1 );

  // re-register unifont, should not have any affect
  BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir, false) );
  face_names = mapnik::freetype_engine::face_names();
  std::clog << "number of registered fonts: " << face_names.size() << std::endl;
  BOOST_TEST( face_names.size() == 1 );

  // register a single dejavu font
  std::string dejavu_bold_oblique("tests/data/fonts/DejaVuSansMono-BoldOblique.ttf");
  BOOST_TEST( mapnik::freetype_engine::register_font(dejavu_bold_oblique) );
  face_names = mapnik::freetype_engine::face_names();
  std::clog << "number of registered fonts: " << face_names.size() << std::endl;
  BOOST_TEST( face_names.size() == 2 );


  // recurse to find all dejavu fonts
  BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir, true) );
  face_names = mapnik::freetype_engine::face_names();
  std::clog << "number of registered fonts: " << face_names.size() << std::endl;
  BOOST_TEST( face_names.size() == 21 );

  return ::boost::report_errors();
}
