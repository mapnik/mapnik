//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// SVG parser.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <cstdlib>
#include <limits.h>
#include <expat.h>

#include <mapnik/svg/agg_svg_parser.h>
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/color_factory.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

namespace agg
{
namespace svg
{
    
    //------------------------------------------------------------------------
    parser::~parser()
    {
        delete [] m_attr_value;
        delete [] m_attr_name;
        delete [] m_buf;
        delete [] m_title;
    }

    //------------------------------------------------------------------------
    parser::parser(path_renderer& path) :
        m_path(path),
        m_buf(new char[buf_size]),
        m_title(new char[256]),
        m_title_len(0),
        m_title_flag(false),
        m_path_flag(false),
        m_attr_name(new char[128]),
        m_attr_value(new char[1024]),
        m_attr_name_len(127),
        m_attr_value_len(1023)
    {
        m_title[0] = 0;
    }

    //------------------------------------------------------------------------
    void parser::parse(const char* fname)
    {
        char msg[1024];
	XML_Parser p = XML_ParserCreate(NULL);
	if(p == 0) 
	{
	    throw std::runtime_error("Couldn't allocate memory for parser");
	}
	
        XML_SetUserData(p, this);
	XML_SetElementHandler(p, start_element, end_element);
	XML_SetCharacterDataHandler(p, content);
	
        FILE* fd = fopen(fname, "r");
        if(fd == 0)
        {
            sprintf(msg, "Couldn't open file %s", fname);
	    throw std::runtime_error(msg);
        }

        bool done = false;
        do
        {
            size_t len = fread(m_buf, 1, buf_size, fd);
            done = len < buf_size;
            if(!XML_Parse(p, m_buf, len, done))
            {
                sprintf(msg,
                    "%s at line %d\n",
                    XML_ErrorString(XML_GetErrorCode(p)),
                    XML_GetCurrentLineNumber(p));
                throw std::runtime_error(msg);
            }
        }
        while(!done);
        fclose(fd);
        XML_ParserFree(p);

        char* ts = m_title;
        while(*ts)
        {
            if(*ts < ' ') *ts = ' ';
            ++ts;
        }
    }


    //------------------------------------------------------------------------
    void parser::start_element(void* data, const char* el, const char** attr)
    {
        parser& self = *(parser*)data;

        if(strcmp(el, "title") == 0)
        {
            self.m_title_flag = true;
        }
        else
        if(strcmp(el, "g") == 0)
        {
            self.m_path.push_attr();
            self.parse_attr(attr);
        }
        else
        if(strcmp(el, "path") == 0)
        {
            if(self.m_path_flag)
            {
                throw std::runtime_error("start_element: Nested path");
            }
            self.m_path.begin_path();
            self.parse_path(attr);
            self.m_path.end_path();
            self.m_path_flag = true;
        }
        else
        if(strcmp(el, "rect") == 0) 
        {
            self.parse_rect(attr);
        }
        else
        if(strcmp(el, "line") == 0)
        {
            self.parse_line(attr);
        }
        else
        if(strcmp(el, "polyline") == 0) 
        {
            self.parse_poly(attr, false);
        }
        else
        if(strcmp(el, "polygon") == 0) 
        {
            self.parse_poly(attr, true);
        }
	else
        if(strcmp(el, "circle") == 0) 
        {
            self.parse_circle(attr);
        }
	else
        if(strcmp(el, "ellipse") == 0) 
        {
            self.parse_ellipse(attr);
        }
        //else
        //if(strcmp(el, "<OTHER_ELEMENTS>") == 0) 
        //{
        //}
        // . . .
    } 


    //------------------------------------------------------------------------
    void parser::end_element(void* data, const char* el)
    {
        parser& self = *(parser*)data;

        if(strcmp(el, "title") == 0)
        {
            self.m_title_flag = false;
        }
        else
        if(strcmp(el, "g") == 0)
        {
            self.m_path.pop_attr();
        }
        else
        if(strcmp(el, "path") == 0)
        {
            self.m_path_flag = false;
        }
        //else
        //if(strcmp(el, "<OTHER_ELEMENTS>") == 0) 
        //{
        //}
        // . . .
    }


    //------------------------------------------------------------------------
    void parser::content(void* data, const char* s, int len)
    {
        parser& self = *(parser*)data;

        // m_title_flag signals that the <title> tag is being parsed now.
        // The following code concatenates the pieces of content of the <title> tag.
        if(self.m_title_flag)
        {
            if(len + self.m_title_len > 255) len = 255 - self.m_title_len;
            if(len > 0) 
            {
                memcpy(self.m_title + self.m_title_len, s, len);
                self.m_title_len += len;
                self.m_title[self.m_title_len] = 0;
            }
        }
    }


    //------------------------------------------------------------------------
    void parser::parse_attr(const char** attr)
    {
        int i;
        for(i = 0; attr[i]; i += 2)
        {
            if(strcmp(attr[i], "style") == 0)
            {
		
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> sep(";");
		std::string str(attr[i+1]);
		std::cerr << str << std::endl;
		tokenizer tok(str,sep);
		for (tokenizer::iterator tok_iter = tok.begin();
		     tok_iter != tok.end(); ++tok_iter)
		{
		    parse_style(tok_iter->c_str());
		}
		//parse_style(attr[i+1]);
            }
            else
            {
                parse_attr(attr[i], attr[i + 1]);
            }
        }
    }

    //-------------------------------------------------------------
    void parser::parse_path(const char** attr)
    {

        int i;

        for(i = 0; attr[i]; i += 2)
        {
            // The <path> tag can consist of the path itself ("d=") 
            // as well as of other parameters like "style=", "transform=", etc.
            // In the last case we simply rely on the function of parsing 
            // attributes (see 'else' branch).
            if(strcmp(attr[i], "d") == 0)
            {
		std::string wkt(attr[i+1]);	
		if (!mapnik::svg::parse_path(wkt, m_path))
		{
		    std::runtime_error("can't parse PATH\n");
		}
            }
            else
            {
                // Create a temporary single pair "name-value" in order
                // to avoid multiple calls for the same attribute.
                const char* tmp[4];
                tmp[0] = attr[i];
                tmp[1] = attr[i + 1];
                tmp[2] = 0;
                tmp[3] = 0;
                parse_attr(tmp);
            }
        }
	
    }


    //-------------------------------------------------------------
    rgba8 parse_color(const char* str)
    {
	mapnik::color c(100,100,100);
	try
	{
	    mapnik::color_factory::init_from_string(c,str);
	}
	catch (mapnik::config_error & ex) 
	{
	    std::cerr << ex.what() << std::endl;
	}
	return rgba8(c.red(), c.green(), c.blue(), c.alpha());
    }
    
    double parse_double(const char* str)
    {
        while(*str == ' ') ++str;
        return atof(str);
    }



    //-------------------------------------------------------------
    bool parser::parse_attr(const char* name, const char* value)
    {
        if(strcmp(name, "style") == 0)
        {
	    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	    boost::char_separator<char> sep(";");
	    std::string str(value);
	    std::cerr << str << std::endl;
	    tokenizer tok(str,sep);
	    for (tokenizer::iterator tok_iter = tok.begin();
		 tok_iter != tok.end(); ++tok_iter)
	    {
		parse_style(tok_iter->c_str());
	    }
        }
        else if(strcmp(name, "fill") == 0)
        {
            if(strcmp(value, "none") == 0)
            {
                m_path.fill_none();
            }
            else
            {
                m_path.fill(parse_color(value));
            }
        }
        else if(strcmp(name, "fill-opacity") == 0)
        {
            m_path.fill_opacity(parse_double(value));
        }
        else if(strcmp(name, "stroke") == 0)
        {
            if(strcmp(value, "none") == 0)
            {
                m_path.stroke_none();
            }
            else
            {
                m_path.stroke(parse_color(value));
            }
        }
	else if(strcmp(name, "fill-rule") == 0)
	{
	    if (strcmp(value, "evenodd") == 0)
	    {
		m_path.even_odd(true);
	    }
	}
        else if(strcmp(name, "stroke-width") == 0)
        {
            m_path.stroke_width(parse_double(value));
        }
        else if(strcmp(name, "stroke-linecap") == 0)
        {
            if(strcmp(value, "butt") == 0)        m_path.line_cap(butt_cap);
            else if(strcmp(value, "round") == 0)  m_path.line_cap(round_cap);
            else if(strcmp(value, "square") == 0) m_path.line_cap(square_cap);
        }
        else
        if(strcmp(name, "stroke-linejoin") == 0)
        {
            if(strcmp(value, "miter") == 0)      m_path.line_join(miter_join);
            else if(strcmp(value, "round") == 0) m_path.line_join(round_join);
            else if(strcmp(value, "bevel") == 0) m_path.line_join(bevel_join);
        }
        else if(strcmp(name, "stroke-miterlimit") == 0)
        {
            m_path.miter_limit(parse_double(value));
        }
        else if(strcmp(name, "stroke-opacity") == 0)
        {
            m_path.stroke_opacity(parse_double(value));
        }
        else if(strcmp(name, "transform") == 0)
        {
	    agg::trans_affine tr;
	    mapnik::svg::parse_transform(value,tr);
	    m_path.transform().premultiply(tr);
        }
	else if(strcmp(name,"opacity") == 0)
	{
	    double opacity = parse_double(value);
	    m_path.stroke_opacity(opacity);
	    m_path.fill_opacity(opacity);
	}
        //else
        //if(strcmp(el, "<OTHER_ATTRIBUTES>") == 0) 
        //{
        //}
        // . . .
        else
        {
            return false;
        }
        return true;
    }



    //-------------------------------------------------------------
    void parser::copy_name(const char* start, const char* end)
    {
        unsigned len = unsigned(end - start);
        if(m_attr_name_len == 0 || len > m_attr_name_len)
        {
            delete [] m_attr_name;
            m_attr_name = new char[len + 1];
            m_attr_name_len = len;
        }
        if(len) memcpy(m_attr_name, start, len);
        m_attr_name[len] = 0;
    }



    //-------------------------------------------------------------
    void parser::copy_value(const char* start, const char* end)
    {
        unsigned len = unsigned(end - start);
        if(m_attr_value_len == 0 || len > m_attr_value_len)
        {
            delete [] m_attr_value;
            m_attr_value = new char[len + 1];
            m_attr_value_len = len;
        }
        if(len) memcpy(m_attr_value, start, len);
        m_attr_value[len] = 0;
    }


    //-------------------------------------------------------------
    bool parser::parse_name_value(const char* nv_start, const char* nv_end)
    {
        const char* str = nv_start;
        while(str < nv_end && *str != ':') ++str;

        const char* val = str;

        // Right Trim
        while(str > nv_start && 
            (*str == ':' || isspace(*str))) --str;
        ++str;

        copy_name(nv_start, str);

        while(val < nv_end && (*val == ':' || isspace(*val))) ++val;
        
        copy_value(val, nv_end);
	
        return parse_attr(m_attr_name, m_attr_value);
    }



    //-------------------------------------------------------------
    void parser::parse_style(const char* str)
    {
        while(*str)
        {
            // Left Trim
            while(*str && isspace(*str)) ++str;
            const char* nv_start = str;
            while(*str && *str != ';') ++str;
            const char* nv_end = str;

            // Right Trim
            while(nv_end > nv_start && 
                (*nv_end == ';' || isspace(*nv_end))) --nv_end;
            ++nv_end;

            parse_name_value(nv_start, nv_end);
            if(*str) ++str;
        }

    }


    //-------------------------------------------------------------
    void parser::parse_rect(const char** attr)
    {
        int i;
        double x = 0.0;
        double y = 0.0;
        double w = 0.0;
        double h = 0.0;
	double rx = 0.0;
	double ry = 0.0;
	
        m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "x") == 0)      x = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "y") == 0)      y = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "width") == 0)  w = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "height") == 0) h = parse_double(attr[i + 1]);
		if(strcmp(attr[i], "rx") == 0)      rx = parse_double(attr[i + 1]);	
		if(strcmp(attr[i], "ry") == 0)      ry = parse_double(attr[i + 1]);
            }
        }


        if(w != 0.0 && h != 0.0)
        {
            if(w < 0.0) throw std::runtime_error("parse_rect: Invalid width: %f");//, w);
            if(h < 0.0) throw std::runtime_error("parse_rect: Invalid height: %f");//, h);
	    if(rx < 0.0) throw std::runtime_error("parse_rect: Invalid rx: %f");//, rx);
	    if(ry < 0.0) throw std::runtime_error("parse_rect: Invalid ry: %f");// ry);
	    
	    if(rx > 0.0 && ry > 0.0)
	    {
		m_path.move_to(x + rx,y);
		m_path.line_to(x + w -rx,y);		
		m_path.arc_to (rx,ry,0,0,1,x + w, y + ry);
		m_path.line_to(x + w, y + h - ry);
		m_path.arc_to (rx,ry,0,0,1,x + w - rx, y + h);
		m_path.line_to(x + rx, y + h);
		m_path.arc_to(rx,ry,0,0,1,x,y + h - ry);
		m_path.line_to(x,y+ry);
		m_path.arc_to(rx,ry,0,0,1,x + rx,y);
	    }
	    else
	    {
		m_path.move_to(x,     y);
		m_path.line_to(x + w, y);
		m_path.line_to(x + w, y + h);
		m_path.line_to(x,     y + h);
		m_path.close_subpath();
	    }
        }
        m_path.end_path();
    }

//-------------------------------------------------------------
    void parser::parse_circle(const char** attr)
    {
        int i;
        double cx = 0.0;
        double cy = 0.0;
        double r = 0.0;
        
	m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "cx") == 0)  cx = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "cy") == 0)  cy = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "r") == 0)  r = parse_double(attr[i + 1]);
            }
        }

	
        if(r != 0.0)
        {
            if(r < 0.0) throw std::runtime_error("parse_cirle: Invalid radius: %f");// r);
	    m_path.move_to(cx+r,cy);
	    m_path.arc_to(r,r,0,1,0,cx-r,cy);
	    m_path.arc_to(r,r,0,1,0,cx+r,cy);
        }
        m_path.end_path();
    }

    //-------------------------------------------------------------
    void parser::parse_ellipse(const char** attr)
    {
        double cx = 0.0;
        double cy = 0.0;
        double rx = 0.0;
	double ry = 0.0;
	
	m_path.begin_path();
        for(int i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "cx") == 0)  cx = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "cy") == 0)  cy = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "rx") == 0)  rx = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "ry") == 0)  ry = parse_double(attr[i + 1]);
            }
        }
	
        if(rx != 0.0 && ry != 0.0)
        {
            if(rx < 0.0) throw std::runtime_error("parse_cirle: Invalid rx: %f");
	    if(ry < 0.0) throw std::runtime_error("parse_cirle: Invalid ry: %f");
	    m_path.move_to(cx+rx,cy);
	    m_path.arc_to(rx,ry,0,1,0,cx-rx,cy);
	    m_path.arc_to(rx,ry,0,1,0,cx+rx,cy);
        }
        m_path.end_path();
    }



    //-------------------------------------------------------------
    void parser::parse_line(const char** attr)
    {
        int i;
        double x1 = 0.0;
        double y1 = 0.0;
        double x2 = 0.0;
        double y2 = 0.0;

        m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "x1") == 0) x1 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "y1") == 0) y1 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "x2") == 0) x2 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "y2") == 0) y2 = parse_double(attr[i + 1]);
            }
        }

        m_path.move_to(x1, y1);
        m_path.line_to(x2, y2);
        m_path.end_path();
    }


    //-------------------------------------------------------------
    void parser::parse_poly(const char** attr, bool close_flag)
    {
        m_path.begin_path();
        for(int i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "points") == 0) 
                {
		    if (!mapnik::svg::parse_points(attr[i+1], m_path))
		    {
			throw std::runtime_error("can't parse POINTS\n");
		    }
                }
            }
        }
        if(close_flag) 
        {
            m_path.close_subpath();
        }
        m_path.end_path();
    } 
}
}


