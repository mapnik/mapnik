// Copyright 2004 Christian Henning.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*************************************************************************************************/

#ifndef GIL_HSV_H
#define GIL_HSV_H

////////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Support for HSV color space
/// \author Christian Henning \n
////////////////////////////////////////////////////////////////////////////////////////

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/gil/gil_all.hpp>
#pragma GCC diagnostic pop

namespace boost { namespace gil {

/// \addtogroup ColorNameModel
/// \{
namespace hsv_color_space
{
/// \brief Hue
struct hue_t {};    
/// \brief Saturation
struct saturation_t{};
/// \brief Value
struct value_t {}; 
}
/// \}

/// \ingroup ColorSpaceModel
typedef mpl::vector3< hsv_color_space::hue_t
                    , hsv_color_space::saturation_t
                    , hsv_color_space::value_t
                    > hsv_t;

/// \ingroup LayoutModel
typedef layout<hsv_t> hsv_layout_t;


#if BOOST_VERSION >= 106800
using bits32 = uint32_t;
using bits32f = float32_t;
GIL_DEFINE_ALL_TYPEDEFS( 32f, float32_t, hsv )
#else
GIL_DEFINE_ALL_TYPEDEFS( 32f, hsv );
#endif

/// \ingroup ColorConvert
/// \brief RGB to HSV
template <>
struct default_color_converter_impl< rgb_t, hsv_t >
{
   template <typename P1, typename P2>
   void operator()( const P1& src, P2& dst ) const
   {
      using namespace hsv_color_space;

      // only bits32f for hsv is supported
      bits32f temp_red   = channel_convert<bits32f>( get_color( src, red_t()   ));
      bits32f temp_green = channel_convert<bits32f>( get_color( src, green_t() ));
      bits32f temp_blue  = channel_convert<bits32f>( get_color( src, blue_t()  ));

      bits32f hue, saturation, value;
      bits32f min_color, max_color;

      if( temp_red < temp_green )
      {
          min_color = std::min( temp_blue, temp_red );
          max_color = std::max( temp_blue, temp_green );
      }
      else
      {
          min_color = std::min( temp_blue, temp_green );
          max_color = std::max( temp_blue, temp_red );
      }

      value = max_color;

      bits32f diff = max_color - min_color;

      if( max_color < 0.0001f )
      {  
         saturation = 0.f;
      }
      else  
      {      
         saturation = diff / max_color;
      }


      if( saturation < 0.0001f )
      {
         //it doesn't matter what value it has
         hue = 0.f; 
      }   
      else
      { 
         if( temp_red == max_color )
         {
            hue = ( temp_green - temp_blue )
                / diff;
         }
         else if( temp_green == max_color )
         {
            hue = 2.f + ( temp_blue - temp_red ) 
                / diff;
         }
         else
         {
            hue = 4.f + ( temp_red - temp_green ) 
                / diff;
         }

         //to bring it to a number between 0 and 1
         hue /= 6.f; 

         if( hue < 0.f )
         {
            hue++;
         }
      }

      get_color( dst, hue_t() )        = hue;
      get_color( dst, saturation_t() ) = saturation;
      get_color( dst, value_t() )      = value;
   }
};

/// \ingroup ColorConvert
/// \brief HSV to RGB
template <>
struct default_color_converter_impl<hsv_t,rgb_t>
{
   template <typename P1, typename P2>
   void operator()( const P1& src, P2& dst) const
   {
      using namespace hsv_color_space;

      bits32f red, green, blue;

      //If saturation is 0, the color is a shade of gray
      if( std::abs( get_color( src, saturation_t() )) < 0.0001f  )
      {
         // If saturation is 0, the color is a shade of gray
         red   = get_color( src, value_t() );
         green = get_color( src, value_t() );
         blue  = get_color( src, value_t() );
      }
      else
      {
         bits32f frac, p, q, t, h;
         bits32 i;

         //to bring hue to a number between 0 and 6, better for the calculations
         h = get_color( src, hue_t() );
         h *= 6.f;

         i = static_cast<bits32>( floor( h ));

         frac = h - i;

         // p = value * (1 - saturation)
         p = get_color( src, value_t() ) 
           * ( 1.f - get_color( src, saturation_t() ));

         // q = value * (1 - saturation * hue_frac)
         // it drops with increasing distance from floor(hue)
         q = get_color( src, value_t() )
           * ( 1.f - ( get_color( src, saturation_t() ) * frac ));

         // t = value * (1 - (saturation * (1 - hue_frac))
         // it grows with increasing distance from floor(hue)
         t = get_color( src, value_t() ) 
           * ( 1.f - ( get_color( src, saturation_t() ) * ( 1.f - frac )));

         switch( i % 6 )
         {         
            case 0: // red to yellow
            {
               red   = get_color( src, value_t() );
               green = t;
               blue  = p;

               break;
            }

            case 1: // yellow to green
            {
               red   = q;
               green = get_color( src, value_t() );
               blue  = p;

               break;
            }

            case 2: // green to cyan
            {
               red   = p;
               green = get_color( src, value_t() );
               blue  = t;

               break;
            }

            case 3: // cyan to blue
            {
               red   = p;
               green = q;
               blue  = get_color( src, value_t() );

               break;
            }

            case 4: // blue to magenta
            {
               red   = t;
               green = p;
               blue  = get_color( src, value_t() );

               break;
            }

            case 5: // magenta to red
            {
               red   = get_color( src, value_t() );
               green = p; 
               blue  = q;

               break;
            }

         }
      }

      get_color(dst,red_t())  =
         channel_convert<typename color_element_type< P2, red_t >::type>( red );
      get_color(dst,green_t())=
         channel_convert<typename color_element_type< P2, green_t >::type>( green );
      get_color(dst,blue_t()) =
         channel_convert<typename color_element_type< P2, blue_t >::type>( blue );
   }
};

} }  // namespace boost::gil

#endif // GIL_HSV_H
