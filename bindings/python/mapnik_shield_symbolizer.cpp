/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
 * Copyright (C) 2006 10East Corp.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/
//$Id$

#include <boost/python.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/image_util.hpp>

using mapnik::color;
using mapnik::shield_symbolizer;
using mapnik::text_symbolizer;
using mapnik::symbolizer_with_image;

struct shield_symbolizer_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const shield_symbolizer& s)
   {

      boost::shared_ptr<mapnik::ImageData32> img = s.get_image();
      const std::string & filename = s.get_filename();
      //(name, font name, font size, font color, image file, image type, width, height)
      return boost::python::make_tuple(s.get_name(),s.get_face_name(),s.get_text_size(),s.get_fill(),filename,mapnik::guess_type(filename),img->width(),img->height());
      
   }

   static  boost::python::tuple
   getstate(const shield_symbolizer& s)
   {
        return boost::python::make_tuple(s.get_halo_fill(),s.get_halo_radius());
   }

   // TODO add lots more...
   static void
   setstate (shield_symbolizer& s, boost::python::tuple state)
   {
        using namespace boost::python;
        /*if (len(state) != 1)
        {
            PyErr_SetObject(PyExc_ValueError,
                         ("expected 1-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
            throw_error_already_set();
        }*/
                
        s.set_halo_fill(extract<color>(state[0]));
        s.set_halo_radius(extract<float>(state[1]));
        
   }

};

void export_shield_symbolizer()
{
    using namespace boost::python;
    class_< shield_symbolizer, bases<text_symbolizer> >("ShieldSymbolizer",
                                                        init< std::string const&, std::string const&, unsigned, mapnik::color const&,
                                                        std::string const&, std::string const&,unsigned,unsigned>("TODO"))
      .def_pickle(shield_symbolizer_pickle_suite())
        ;
    
}
