/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef CSS_COLOR_PARSER_HPP
#define CSS_COLOR_PARSER_HPP

// boost
#include <boost/version.hpp>

#if BOOST_VERSION < 103800
  #include <boost/spirit/core.hpp>
  #include <boost/spirit/symbols.hpp>
#else
  #define BOOST_SPIRIT_USE_OLD_NAMESPACE
  #include <boost/spirit/include/classic_core.hpp>
  #include <boost/spirit/include/classic_symbols.hpp>
#endif

using namespace boost::spirit;

namespace mapnik {

   template <int MIN,int MAX>
   inline int clip_int(int val)
   {
      if (val < MIN ) return MIN;
      if (val > MAX ) return MAX;
      return val;
   }
   
    template <typename ColorT>
    struct named_colors : public symbols<ColorT>
    {
        named_colors()
        {
            symbols<ColorT>::add
                ("aliceblue", ColorT(240, 248, 255))
                ("antiquewhite", ColorT(250, 235, 215))
                ("aqua", ColorT(0, 255, 255))
                ("aquamarine", ColorT(127, 255, 212))
                ("azure", ColorT(240, 255, 255))
                ("beige", ColorT(245, 245, 220))
                ("bisque", ColorT(255, 228, 196))
                ("black", ColorT(0, 0, 0))
                ("blanchedalmond", ColorT(255,235,205))
                ("blue", ColorT(0, 0, 255))
                ("blueviolet", ColorT(138, 43, 226))
                ("brown", ColorT(165, 42, 42))
                ("burlywood", ColorT(222, 184, 135))
                ("cadetblue", ColorT(95, 158, 160))
                ("chartreuse", ColorT(127, 255, 0))
                ("chocolate", ColorT(210, 105, 30))
                ("coral", ColorT(255, 127, 80))
                ("cornflowerblue", ColorT(100, 149, 237))
                ("cornsilk", ColorT(255, 248, 220))
                ("crimson", ColorT(220, 20, 60))
                ("cyan", ColorT(0, 255, 255))
                ("darkblue", ColorT(0, 0, 139))
                ("darkcyan", ColorT(0, 139, 139))
                ("darkgoldenrod", ColorT(184, 134, 11))
                ("darkgray", ColorT(169, 169, 169))
                ("darkgreen", ColorT(0, 100, 0))
                ("darkgrey", ColorT(169, 169, 169))
                ("darkkhaki", ColorT(189, 183, 107))
                ("darkmagenta", ColorT(139, 0, 139))
                ("darkolivegreen", ColorT(85, 107, 47))
                ("darkorange", ColorT(255, 140, 0))
                ("darkorchid", ColorT(153, 50, 204))
                ("darkred", ColorT(139, 0, 0))
                ("darksalmon", ColorT(233, 150, 122))
                ("darkseagreen", ColorT(143, 188, 143))
                ("darkslateblue", ColorT(72, 61, 139))
                ("darkslategrey", ColorT(47, 79, 79))
                ("darkturquoise", ColorT(0, 206, 209))
                ("darkviolet", ColorT(148, 0, 211))
                ("deeppink", ColorT(255, 20, 147))
                ("deepskyblue", ColorT(0, 191, 255))
                ("dimgray", ColorT(105, 105, 105))
                ("dimgrey", ColorT(105, 105, 105))
                ("dodgerblue", ColorT(30, 144, 255))
                ("firebrick", ColorT(178, 34, 34))
                ("floralwhite", ColorT(255, 250, 240))
                ("forestgreen", ColorT(34, 139, 34))
                ("fuchsia", ColorT(255, 0, 255))
                ("gainsboro", ColorT(220, 220, 220))
                ("ghostwhite", ColorT(248, 248, 255))
                ("gold", ColorT(255, 215, 0))
                ("goldenrod", ColorT(218, 165, 32))
                ("gray", ColorT(128, 128, 128))
                ("grey", ColorT(128, 128, 128))
                ("green", ColorT(0, 128, 0))
                ("greenyellow", ColorT(173, 255, 47))
                ("honeydew", ColorT(240, 255, 240))
                ("hotpink", ColorT(255, 105, 180))
                ("indianred", ColorT(205, 92, 92))
                ("indigo", ColorT(75, 0, 130))
                ("ivory", ColorT(255, 255, 240))
                ("khaki", ColorT(240, 230, 140))
                ("lavender", ColorT(230, 230, 250))
                ("lavenderblush", ColorT(255, 240, 245))
                ("lawngreen", ColorT(124, 252, 0))
                ("lemonchiffon", ColorT(255, 250, 205))
                ("lightblue", ColorT(173, 216, 230))
                ("lightcoral", ColorT(240, 128, 128))
                ("lightcyan", ColorT(224, 255, 255))
                ("lightgoldenrodyellow", ColorT(250, 250, 210))
                ("lightgray", ColorT(211, 211, 211))
                ("lightgreen", ColorT(144, 238, 144))
                ("lightgrey", ColorT(211, 211, 211))
                ("lightpink", ColorT(255, 182, 193))
                ("lightsalmon", ColorT(255, 160, 122))
                ("lightseagreen", ColorT(32, 178, 170))
                ("lightskyblue", ColorT(135, 206, 250))
                ("lightslategray", ColorT(119, 136, 153))
                ("lightslategrey", ColorT(119, 136, 153))
                ("lightsteelblue", ColorT(176, 196, 222))
                ("lightyellow", ColorT(255, 255, 224))
                ("lime", ColorT(0, 255, 0))
                ("limegreen", ColorT(50, 205, 50))
                ("linen", ColorT(250, 240, 230))
                ("magenta", ColorT(255, 0, 255))
                ("maroon", ColorT(128, 0, 0))
                ("mediumaquamarine", ColorT(102, 205, 170))
                ("mediumblue", ColorT(0, 0, 205))
                ("mediumorchid", ColorT(186, 85, 211))
                ("mediumpurple", ColorT(147, 112, 219))
                ("mediumseagreen", ColorT(60, 179, 113))
                ("mediumslateblue", ColorT(123, 104, 238))
                ("mediumspringgreen", ColorT(0, 250, 154))
                ("mediumturquoise", ColorT(72, 209, 204))
                ("mediumvioletred", ColorT(199, 21, 133))
                ("midnightblue", ColorT(25, 25, 112))
                ("mintcream", ColorT(245, 255, 250))
                ("mistyrose", ColorT(255, 228, 225))
                ("moccasin", ColorT(255, 228, 181))
                ("navajowhite", ColorT(255, 222, 173))
                ("navy", ColorT(0, 0, 128))
                ("oldlace", ColorT(253, 245, 230))
                ("olive", ColorT(128, 128, 0))
                ("olivedrab", ColorT(107, 142, 35))
                ("orange", ColorT(255, 165, 0))
                ("orangered", ColorT(255, 69, 0))
                ("orchid", ColorT(218, 112, 214))
                ("palegoldenrod", ColorT(238, 232, 170))
                ("palegreen", ColorT(152, 251, 152))
                ("paleturquoise", ColorT(175, 238, 238))
                ("palevioletred", ColorT(219, 112, 147))
                ("papayawhip", ColorT(255, 239, 213))
                ("peachpuff", ColorT(255, 218, 185))
                ("peru", ColorT(205, 133, 63))
                ("pink", ColorT(255, 192, 203))
                ("plum", ColorT(221, 160, 221))
                ("powderblue", ColorT(176, 224, 230))
                ("purple", ColorT(128, 0, 128))
                ("red", ColorT(255, 0, 0))
                ("rosybrown", ColorT(188, 143, 143))
                ("royalblue", ColorT(65, 105, 225))
                ("saddlebrown", ColorT(139, 69, 19))
                ("salmon", ColorT(250, 128, 114))
                ("sandybrown", ColorT(244, 164, 96))
                ("seagreen", ColorT(46, 139, 87))
                ("seashell", ColorT(255, 245, 238))
                ("sienna", ColorT(160, 82, 45))
                ("silver", ColorT(192, 192, 192))
                ("skyblue", ColorT(135, 206, 235))
                ("slateblue", ColorT(106, 90, 205))
                ("slategray", ColorT(112, 128, 144))
                ("slategrey", ColorT(112, 128, 144))
                ("snow", ColorT(255, 250, 250))
                ("springgreen", ColorT(0, 255, 127))
                ("steelblue", ColorT(70, 130, 180))
                ("tan", ColorT(210, 180, 140))
                ("teal", ColorT(0, 128, 128))
                ("thistle", ColorT(216, 191, 216))
                ("tomato", ColorT(255, 99, 71))
                ("turquoise", ColorT(64, 224, 208))
                ("violet", ColorT(238, 130, 238))
                ("wheat", ColorT(245, 222, 179))
                ("white", ColorT(255, 255, 255))
                ("whitesmoke", ColorT(245, 245, 245))
                ("yellow", ColorT(255, 255, 0))
                ("yellowgreen", ColorT(154, 205, 50))
                ("transparent", ColorT(0, 0, 0, 0))
                ;
        }
    };
    
    template <typename ActionsT>
    struct css_color_grammar : public grammar<css_color_grammar<ActionsT> >
    {
        css_color_grammar(ActionsT& actions_)
            : actions(actions_) {}
    
        template <typename ScannerT>
        struct definition
        {
            definition(css_color_grammar const& self)
            {
                hex6 = ch_p('#') >> uint6x_p[self.actions.hex6_];
                hex3 = ch_p('#') >> uint3x_p[self.actions.hex3_];
                rgb = str_p("rgb") >> '(' >> int_p [self.actions.red_] 
                                   >> ',' >> int_p [self.actions.green_] 
                                   >> ',' >> int_p [self.actions.blue_] 
                                   >> ')';
                rgba = str_p("rgba") >> '(' >> int_p [self.actions.red_]
                                   >> ',' >> int_p [self.actions.green_]
                                   >> ',' >> int_p [self.actions.blue_]
                                   >> ',' >> real_p[self.actions.alpha_]
                                   >> ')';
                rgb_percent = str_p("rgb") >> '(' >> real_p[self.actions.red_p_] >> '%' 
                                           >> ',' >> real_p[self.actions.green_p_] >> '%'
                                           >> ',' >> real_p[self.actions.blue_p_] >> '%'
                                           >> ')';
                rgba_percent = str_p("rgba") >> '(' >> real_p[self.actions.red_p_] >> '%'
                                           >> ',' >> real_p[self.actions.green_p_] >> '%'
                                           >> ',' >> real_p[self.actions.blue_p_] >> '%'
                                           >> ',' >> real_p[self.actions.alpha_]
                                           >> ')';
                css_color = named_colors_p[self.actions.named_] | hex6 | hex3 | rgb_percent | rgba_percent | rgb | rgba; 
            }
            boost::spirit::rule<ScannerT> rgb;
            boost::spirit::rule<ScannerT> rgba;
            boost::spirit::rule<ScannerT> rgb_percent;
            boost::spirit::rule<ScannerT> rgba_percent;
            boost::spirit::rule<ScannerT> hex6;
            boost::spirit::rule<ScannerT> hex3;
            boost::spirit::rule<ScannerT> css_color;
            boost::spirit::rule<ScannerT> const& start() const
            {
                return css_color;
            }
            int_parser<int, 10, 1, -1> int_p;
            uint_parser<unsigned, 16, 6, 6> uint6x_p;
            uint_parser<unsigned, 16, 3, 3> uint3x_p;
            real_parser<double, real_parser_policies<double>  > real_p;
            named_colors<typename ActionsT::color_type> named_colors_p;
	    
        };
        ActionsT& actions;	
    };
    
    template <typename ColorT>
    struct named_color_action
    {
        named_color_action(ColorT& c)
            : c_(c) {}
	
        void operator() (ColorT const&c) const
        {
            c_=c;
        }
        ColorT& c_;
    };

    template <typename ColorT>
    struct hex6_action
    {
        hex6_action(ColorT& c)
            : c_(c) {}
	
        void operator () (unsigned int hex) const
        {
            unsigned r = (hex >> 16) & 0xff;
            unsigned g = (hex >> 8) & 0xff;
            unsigned b = hex & 0xff;
            c_.set_red(r);
            c_.set_green(g);
            c_.set_blue(b);
        }
        ColorT& c_;
    };
    
    template <typename ColorT>
    struct hex3_action
    {
        hex3_action(ColorT& c)
            : c_(c) {}
	
        void operator () (unsigned int hex) const
        {
            unsigned int r = (hex >> 8) & 0xf;
            unsigned int g = (hex >> 4) & 0xf;
            unsigned int b = hex & 0xf;
            c_.set_red( r | r << 4);
            c_.set_green(g | g << 4);
            c_.set_blue(b | b << 4);
        }
        ColorT& c_;
    };

    template <typename ColorT>
    struct red_action
    {
        red_action(ColorT& c)
            : c_(c) {}
	
        void operator () (int r) const
        {
           c_.set_red(clip_int<0,255>(r));
        }
        ColorT& c_;
    };

    template <typename ColorT>
    struct green_action
    {
        green_action(ColorT& c)
            : c_(c) {}
	
        void operator () (int g) const
        {
           c_.set_green(clip_int<0,255>(g));
        }
        ColorT& c_;
    };
	
    template <typename ColorT>
    struct blue_action
    {
        blue_action(ColorT& c)
            : c_(c) {}
	
        void operator () (int b) const
        {
           c_.set_blue(clip_int<0,255>(b));
        }
        ColorT& c_;
    };
    

    template <typename ColorT>
    struct alpha_action
    {
        alpha_action(ColorT& c)
            : c_(c) {}
        
        void operator () (double a) const
        {
           if (a < 0.0) a = 0.0;
           if (a > 1.0) a = 1.0;
           c_.set_alpha(unsigned(a * 255.0 + 0.5));
        }
         
        ColorT& c_;
    };


    template <typename ColorT>
    struct red_action_p
    {
        red_action_p(ColorT& c)
            : c_(c) {}
	
       void operator () (double r) const
       {
           c_.set_red(clip_int<0,255>(int((255.0 * r)/100.0 + 0.5)));
       }
       ColorT& c_;
    };
   
    template <typename ColorT>
    struct green_action_p
    {
        green_action_p(ColorT& c)
            : c_(c) {}
	
        void operator () (double g) const
        {
           c_.set_green(clip_int<0,255>(int((255.0 * g)/100.0 + 0.5)));
        }
        ColorT& c_;
    };

    template <typename ColorT>
    struct blue_action_p
    {
        blue_action_p(ColorT& c)
            : c_(c) {}
	
        void operator () (double b) const
        {
           c_.set_blue(clip_int<0,255>(int((255.0 * b)/100.0 + 0.5)));
        }
       ColorT& c_;
    };
   
   
    template <typename ColorT>
    struct actions
    {
        typedef ColorT color_type;
        actions(ColorT& c)
           : 
           named_(c),
           hex6_(c),
           hex3_(c),
           red_(c),
           green_(c),
           blue_(c),
           alpha_(c),
           red_p_(c),
           green_p_(c),
           blue_p_(c) 
       {
          c.set_alpha (255);
       }
        
       named_color_action<ColorT> named_;
       hex6_action<ColorT> hex6_;
       hex3_action<ColorT> hex3_;
       red_action<ColorT> red_;
       green_action<ColorT> green_;
       blue_action<ColorT> blue_;
       alpha_action<ColorT> alpha_;
       red_action_p<ColorT> red_p_;
       green_action_p<ColorT> green_p_;
       blue_action_p<ColorT> blue_p_;
    };
}

#endif //CSS_COLOR_PARSER_HPP
