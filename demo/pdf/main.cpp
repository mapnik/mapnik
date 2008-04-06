#include <stdio.h>
#include <iostream>

#define DATASOURCEDIR "D:/otm/mapnik/trunk/msvc/Debug/"
#define REGISTEREDFONT "D:/otm/mapnik/trunk/fonts/dejavu-ttf-2.14/DejaVuSans.ttf"
#define DEMODIR "D:/otm/mapnik/trunk/demo/"
#define DEMODATA DEMODIR"data/"


//mapnik
#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>

//pdf renderer
#include "pdf/pdf_renderer.hpp"
#include "pdf/pdf_renderer_layout.hpp"
#include "pdf/pdf_renderer_utility.hpp"
#include "pdf/pdf_point_symbolizer.hpp"
#include "pdf/pdf_polygon_pattern_symbolizer.hpp"
#include "pdf/pdf_line_pattern_symbolizer.hpp"
#include "pdf/pdf_shield_symbolizer.hpp"

// WX
#include <wx/pdfdoc.h>

using namespace mapnik;

void demo(void);
void fancy_demo(void);


int main(void) {
  demo();
  fancy_demo();

  return 0;
}



void demo(void) {
  std::cout << "demo\n==============================================================\n";

  //---------------------------------
  //Setup

  //register datasources
  datasource_cache::instance()->register_datasources(DATASOURCEDIR);

  //register the font
  // note: pdf output uses its own fonts, which are either the standard
  //  pdf fonts (arial, courier, times, symbol, zapfDingbats) or external
  //  fonts (see pdf_text_renderer::set_external_font_options).
  freetype_engine::register_font(REGISTEREDFONT);


  //---------------------------------
  //Map creation

  //create an empty map.
  // - The dimensions given here are in pixels for the normal renderer,
  //    and 'pdf-units' for the pdf renderer. See pdf_renderer_layout
  //    below for more information on the pdf-units.
  Map m(150,200);

  //background colour
  m.set_background(color_factory::from_string("cyan"));


  //---------------------------------
  //Create styles
  // Note: dimensions given here will be interpreted in pdf-units rather
  //  than pixels for the pdf output. E.g. a line width of '5' cound be
  //  5mm, 5in, or 5pt depending on the pdf-units.

  // Provinces (polygon)
  feature_type_style provpoly_style;
 
  rule_type provpoly_rule_on;
  provpoly_rule_on.set_filter(create_filter("[NAME_EN] = 'Ontario'"));
  provpoly_rule_on.append(polygon_symbolizer(Color(250, 190, 183)));
  provpoly_style.add_rule(provpoly_rule_on);
  
  rule_type provpoly_rule_qc;
  provpoly_rule_qc.set_filter(create_filter("[NAME_EN] = 'Quebec'"));
  provpoly_rule_qc.append(polygon_symbolizer(Color(217, 235, 203)));
  provpoly_style.add_rule(provpoly_rule_qc);
  
  m.insert_style("provinces",provpoly_style);


  // Provinces (polyline)
  feature_type_style provlines_style;
  
  stroke provlines_stk (Color(0,0,0),0.75);
  provlines_stk.add_dash(8, 4);
  provlines_stk.add_dash(2, 2);
  provlines_stk.add_dash(2, 2);
  
  rule_type provlines_rule;
  provlines_rule.append(line_symbolizer(provlines_stk));
  provlines_style.add_rule(provlines_rule);
  
  m.insert_style("provlines",provlines_style);
  

  // Drainage 
  feature_type_style qcdrain_style;
  
  rule_type qcdrain_rule;
  qcdrain_rule.set_filter(create_filter("[HYC] = 8"));
  qcdrain_rule.append(polygon_symbolizer(Color(153, 204, 255)));
  qcdrain_style.add_rule(qcdrain_rule);
  
  m.insert_style("drainage",qcdrain_style);
  

  // Roads 3 and 4 (The "grey" roads)
  feature_type_style roads34_style;    
  rule_type roads34_rule;
  roads34_rule.set_filter(create_filter("[CLASS] = 3 or [CLASS] = 4"));
  stroke roads34_rule_stk(Color(171,158,137),1.0);
  roads34_rule_stk.set_line_cap(ROUND_CAP);
  roads34_rule_stk.set_line_join(ROUND_JOIN);
  roads34_rule.append(line_symbolizer(roads34_rule_stk));
  roads34_style.add_rule(roads34_rule);
  
  m.insert_style("smallroads",roads34_style);
  

  // Roads 2 (The thin yellow ones)
  feature_type_style roads2_style_1;
  rule_type roads2_rule_1;
  roads2_rule_1.set_filter(create_filter("[CLASS] = 2"));
  stroke roads2_rule_stk_1(Color(171,158,137),1.8);
  roads2_rule_stk_1.set_line_cap(ROUND_CAP);
  roads2_rule_stk_1.set_line_join(ROUND_JOIN);
  roads2_rule_1.append(line_symbolizer(roads2_rule_stk_1));
  roads2_style_1.add_rule(roads2_rule_1);
  
  m.insert_style("road-border", roads2_style_1);
  
  feature_type_style roads2_style_2;
  rule_type roads2_rule_2;
  roads2_rule_2.set_filter(create_filter("[CLASS] = 2"));
  stroke roads2_rule_stk_2(Color(255,250,115),1.5);
  roads2_rule_stk_2.set_line_cap(ROUND_CAP);
  roads2_rule_stk_2.set_line_join(ROUND_JOIN);
  roads2_rule_2.append(line_symbolizer(roads2_rule_stk_2));
  roads2_style_2.add_rule(roads2_rule_2);
  
  m.insert_style("road-fill", roads2_style_2);
  

  // Roads 1 (The big orange ones, the highways)
  feature_type_style roads1_style_1;
  rule_type roads1_rule_1;
  roads1_rule_1.set_filter(create_filter("[CLASS] = 1"));
  stroke roads1_rule_stk_1(Color(188,149,28),2.2);
  roads1_rule_stk_1.set_line_cap(ROUND_CAP);
  roads1_rule_stk_1.set_line_join(ROUND_JOIN);
  roads1_rule_1.append(line_symbolizer(roads1_rule_stk_1));
  roads1_style_1.add_rule(roads1_rule_1);
  m.insert_style("highway-border", roads1_style_1);
  
  feature_type_style roads1_style_2;
  rule_type roads1_rule_2;
  roads1_rule_2.set_filter(create_filter("[CLASS] = 1"));
  stroke roads1_rule_stk_2(Color(242,191,36),2.0);
  roads1_rule_stk_2.set_line_cap(ROUND_CAP);
  roads1_rule_stk_2.set_line_join(ROUND_JOIN);
  roads1_rule_2.append(line_symbolizer(roads1_rule_stk_2));

  roads1_style_2.add_rule(roads1_rule_2);
  m.insert_style("highway-fill", roads1_style_2);


  // Populated Places
  
  feature_type_style popplaces_style;
  rule_type popplaces_rule;
  text_symbolizer popplaces_text_symbolizer("GEONAME","arial",3,Color(50,50,100));
  popplaces_text_symbolizer.set_halo_fill(Color(255,255,200));
  popplaces_text_symbolizer.set_halo_radius(1);
  popplaces_rule.append(popplaces_text_symbolizer);
  popplaces_style.add_rule(popplaces_rule);
  
  m.insert_style("popplaces",popplaces_style );

  //---------------------------------
  //Create Layers and assign styles

  // Layers
  // Provincial  polygons
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"boundaries";
    
    Layer lyr("Provinces"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("provinces");    
    m.addLayer(lyr);
  }

  // Drainage
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"qcdrainage";
    Layer lyr("Quebec Hydrography");
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("drainage");    
    m.addLayer(lyr);
  }

  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"ontdrainage";
    
    Layer lyr("Ontario Hydrography"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("drainage");    
    m.addLayer(lyr);
  }


  // Provincial boundaries
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"boundaries_l";
    Layer lyr("Provincial borders"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("provlines");    
    m.addLayer(lyr);
  }

  // Roads
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"roads";        
    Layer lyr("Roads"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("smallroads");
    lyr.add_style("road-border");
    lyr.add_style("road-fill");
    lyr.add_style("highway-border");
    lyr.add_style("highway-fill");

    m.addLayer(lyr);        
  }
  
  // popplaces
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"popplaces";
    p["encoding"] = "latin1";
    Layer lyr("Populated Places");
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("popplaces");    
    m.addLayer(lyr);
  }


  //---------------------------------
  //Set map visible area

//  m.zoom_all();
  m.zoomToBox(Envelope<double>(1405120.04127408,-247003.813399447,1706357.31328276,-25098.593149577));


  //---------------------------------
  //Render to a png (you'll need to change the font in the popplaces style for this to work

/*  Image32 buf(m.getWidth(),m.getHeight());
  agg_renderer<Image32> ren(m,buf,0,0);
  ren.apply();
  save_to_file<ImageData32>(buf.data(),"canada2.png","png");
*/

  //=================================
  //Render to a pdf

  //---------------------
  //create the page layout for the pdf

  //this specifies the units used on the page, and dimensions of the page in those units.
  pdf_renderer_layout page_layout("mm", 210, 297);

  //set the location of the map within the page
  page_layout.set_map_area(25, 40, m.getWidth(), m.getHeight());
  
  page_layout.set_map_area_border(true);                    //Draw a border around the map
  page_layout.set_map_area_border_width(0.75);              // - width of the border
  page_layout.set_background_colour(Color(255, 255, 255));  // - colour of the border

  page_layout.set_map_grid(true);                           //Draw a grid across the map (on its own layer so it can be turned on/off)
  page_layout.set_map_grid_colour(Color(0,128,255,200));    // - Grid colour
  page_layout.set_map_grid_linewidth(0.33);                 // - Grid line width

  page_layout.set_border_scales(true);                      //Draw scales around the map border
  page_layout.set_map_grid_approx_spacing(20);              // - approximate length of each grid segment, this will be tweaked to get nice units (e.g. 20mm may equate to 5832.78 meters, so it'll be changed to something like 22 to give 6000 meters)
  page_layout.set_border_scale_width(3);                    // - width of the scales around the border
  page_layout.set_border_scale_linewidth(0.25);             // - line width of the border scales

  page_layout.set_scale_bar(true);                                    //Draw a scale bar
  page_layout.set_scale_bar_area(Envelope<double>(50,250, 170, 257)); // - position and size of the scale bar. The width will be tweaked to give proper units.
  page_layout.set_scale_bar_factor(1,"Meters");                       // - Give the scale factor and label. E.g. if the map projection outputs in meters, the scale factor is 1, but if you want kilometers, specify 0.001.
//  page_layout.set_scale_bar_factor(0.001,_T("Kilometers"));
//  page_layout.set_scale_bar_factor(39.3700787,_T("Inches"));
//  page_layout.set_scale_bar_factor(0.000621371192,_T("Miles"));
//  page_layout.set_scale_bar_factor(0.546806649,_T("Fathoms"));
//  page_layout.set_scale_bar_factor(3.2808399,"Feet");

  //---------------------
  //render the pdf to a file
  render_to_pdf(m, page_layout, "demo.pdf", false);
  render_to_pdf(m, page_layout, "demo-compressed.pdf", true);

}



void fancy_demo(void) {
  std::cout << "fancy demo\n==============================================================\n";

  //---------------------------------
  //Setup

  //register datasources
  datasource_cache::instance()->register_datasources(DATASOURCEDIR);

  //register the font
  // note: pdf output uses its own fonts, which are either the standard
  //  pdf fonts (arial, courier, times, symbol, zapfDingbats) or external
  //  fonts (see pdf_text_renderer::set_external_font_options).
  freetype_engine::register_font(REGISTEREDFONT);


  //---------------------------------
  //Map creation

  //create an empty map.
  // - The dimensions given here are in pixels for the normal renderer,
  //    and 'pdf-units' for the pdf renderer. See pdf_renderer_layout
  //    below for more information on the pdf-units.
  Map m(800,500);

  //background colour
  m.set_background(color_factory::from_string("cyan"));


  //---------------------------------
  //Create styles
  // Note: dimensions given here will be interpreted in pdf-units rather
  //  than pixels for the pdf output. E.g. a line width of '5' cound be
  //  5mm, 5in, or 5pt depending on the pdf-units.

  // Provinces (polygon)
  feature_type_style provpoly_style;
 
  rule_type provpoly_rule_on;
  provpoly_rule_on.set_filter(create_filter("[NAME_EN] = 'Ontario'"));
  provpoly_rule_on.append(polygon_symbolizer(Color(250, 190, 183)));
  provpoly_style.add_rule(provpoly_rule_on);
  
  rule_type provpoly_rule_qc;
  provpoly_rule_qc.set_filter(create_filter("[NAME_EN] = 'Quebec'"));
  provpoly_rule_qc.append(polygon_symbolizer(Color(217, 235, 203)));
  provpoly_style.add_rule(provpoly_rule_qc);
  
  m.insert_style("provinces",provpoly_style);


  // Provinces (polyline)
  feature_type_style provlines_style;
  
  stroke provlines_stk (Color(0,0,0),0.75);
  provlines_stk.add_dash(8, 4);
  provlines_stk.add_dash(2, 2);
  provlines_stk.add_dash(2, 2);
  
  rule_type provlines_rule;
  provlines_rule.append(line_symbolizer(provlines_stk));
  provlines_style.add_rule(provlines_rule);
  
  m.insert_style("provlines",provlines_style);
  

  // Drainage 
  feature_type_style qcdrain_style;
  
  rule_type qcdrain_rule;
  qcdrain_rule.set_filter(create_filter("[HYC] = 8"));
  pdf_polygon_pattern_symbolizer psym(DEMODIR"pdf/images/b.png","png",100,100, 10, 10);
  qcdrain_rule.append(psym);
  qcdrain_style.add_rule(qcdrain_rule);
  
  m.insert_style("drainage",qcdrain_style);
  

  // Roads 3 and 4 (The "grey" roads)
  feature_type_style roads34_style;    
  rule_type roads34_rule;
  roads34_rule.set_filter(create_filter("[CLASS] = 3 or [CLASS] = 4"));
  stroke roads34_rule_stk(Color(171,158,137),1.0);
  roads34_rule_stk.set_line_cap(ROUND_CAP);
  roads34_rule_stk.set_line_join(ROUND_JOIN);
  roads34_rule.append(line_symbolizer(roads34_rule_stk));
  roads34_style.add_rule(roads34_rule);
  
  m.insert_style("smallroads",roads34_style);
  

  // Roads 2 (The thin yellow ones)
  feature_type_style roads2_style_1;
  rule_type roads2_rule_1;
  roads2_rule_1.set_filter(create_filter("[CLASS] = 2"));
  stroke roads2_rule_stk_1(Color(171,158,137),1.8);
  roads2_rule_stk_1.set_line_cap(ROUND_CAP);
  roads2_rule_stk_1.set_line_join(ROUND_JOIN);
  roads2_rule_1.append(line_symbolizer(roads2_rule_stk_1));
  roads2_style_1.add_rule(roads2_rule_1);
  
  m.insert_style("road-border", roads2_style_1);
  
  feature_type_style roads2_style_2;
  rule_type roads2_rule_2;
  roads2_rule_2.set_filter(create_filter("[CLASS] = 2"));
  stroke roads2_rule_stk_2(Color(255,250,115),1.5);
  roads2_rule_stk_2.set_line_cap(ROUND_CAP);
  roads2_rule_stk_2.set_line_join(ROUND_JOIN);
  roads2_rule_2.append(line_symbolizer(roads2_rule_stk_2));
  roads2_style_2.add_rule(roads2_rule_2);
  
  m.insert_style("road-fill", roads2_style_2);
  

  // Roads 1 (The big orange ones, the highways)
  feature_type_style roads1_style_1;
  rule_type roads1_rule_1;
  roads1_rule_1.set_filter(create_filter("[CLASS] = 1"));
  stroke roads1_rule_stk_1(Color(188,149,28),4);
  roads1_rule_stk_1.set_line_cap(ROUND_CAP);
  roads1_rule_stk_1.set_line_join(ROUND_JOIN);
  roads1_rule_1.append(line_symbolizer(roads1_rule_stk_1));
  roads1_style_1.add_rule(roads1_rule_1);
  m.insert_style("highway-border", roads1_style_1);
  
  feature_type_style roads1_style_2;
  rule_type roads1_rule_2;
  roads1_rule_2.set_filter(create_filter("[CLASS] = 1"));
  pdf_line_pattern_symbolizer lsym(DEMODIR"pdf/images/a.png","png",3,100,100,10,10);
  roads1_rule_2.append(lsym);

  roads1_style_2.add_rule(roads1_rule_2);
  m.insert_style("highway-fill", roads1_style_2);


  // Populated Places
  
  feature_type_style popplaces_style;
  rule_type popplaces_rule;
  text_symbolizer popplaces_text_symbolizer("GEONAME","arial",3,Color(50,50,100));
  popplaces_text_symbolizer.set_halo_fill(Color(255,255,200));
  popplaces_text_symbolizer.set_halo_radius(1);
  popplaces_rule.append(popplaces_text_symbolizer);
  popplaces_style.add_rule(popplaces_rule);
  
  m.insert_style("popplaces",popplaces_style );

  //---------------------------------
  //Create Layers and assign styles

  // Layers
  // Provincial  polygons
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"boundaries";
    
    Layer lyr("Provinces"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("provinces");    
    m.addLayer(lyr);
  }

  // Drainage
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"qcdrainage";
    Layer lyr("Quebec Hydrography");
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("drainage");    
    m.addLayer(lyr);
  }

  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"ontdrainage";
    
    Layer lyr("Ontario Hydrography"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("drainage");    
    m.addLayer(lyr);
  }


  // Provincial boundaries
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"boundaries_l";
    Layer lyr("Provincial borders"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("provlines");    
    m.addLayer(lyr);
  }

  // Roads
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"roads";        
    Layer lyr("Roads"); 
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("smallroads");
    lyr.add_style("road-border");
    lyr.add_style("road-fill");
//    lyr.add_style("highway-border");
    lyr.add_style("highway-fill");

    m.addLayer(lyr);        
  }
  
  // popplaces
  {
    parameters p;
    p["type"]="shape";
    p["file"]=DEMODATA"popplaces";
    p["encoding"] = "latin1";
    Layer lyr("Populated Places");
    lyr.set_datasource(datasource_cache::instance()->create(p));
    lyr.add_style("popplaces");    
    m.addLayer(lyr);
  }


  //---------------------------------
  //Set map visible area

//  m.zoom_all();
  m.zoomToBox(Envelope<double>(1405120.04127408,-247003.813399447,1706357.31328276,-25098.593149577));


  //---------------------------------
  //Render to a png (you'll need to change the font in the popplaces style for this to work

/*  Image32 buf(m.getWidth(),m.getHeight());
  agg_renderer<Image32> ren(m,buf,0,0);
  ren.apply();
  save_to_file<ImageData32>(buf.data(),"canada2.png","png");
*/

  //=================================
  //Render to a pdf

  //---------------------
  //create the page layout for the pdf

  //this specifies the units used on the page, and dimensions of the page in those units.
  pdf_renderer_layout page_layout("mm", 841, 594);

  //set the location of the map within the page
  page_layout.set_map_area(20, 40, m.getWidth(), m.getHeight());
  
  page_layout.set_map_area_border(true);                    //Draw a border around the map
  page_layout.set_map_area_border_width(0.75);              // - width of the border
  page_layout.set_background_colour(Color(200, 200, 225));  //Page background colour

  page_layout.set_map_grid(true);                           //Draw a grid across the map (on its own layer so it can be turned on/off)
  page_layout.set_map_grid_colour(Color(0,128,255,200));    // - Grid colour
  page_layout.set_map_grid_linewidth(0.33);                 // - Grid line width

  page_layout.set_border_scales(true);                      //Draw scales around the map border
  page_layout.set_map_grid_approx_spacing(20);              // - approximate length of each grid segment, this will be tweaked to get nice units (e.g. 20mm may equate to 5832.78 meters, so it'll be changed to something like 22 to give 6000 meters)
  page_layout.set_border_scale_width(3);                    // - width of the scales around the border
  page_layout.set_border_scale_linewidth(0.25);             // - line width of the border scales

  page_layout.set_scale_bar(true);                                    //Draw a scale bar
  page_layout.set_scale_bar_area(Envelope<double>(50,555, 150, 567)); // - position and size of the scale bar. The width will be tweaked to give proper units.
//  page_layout.set_scale_bar_factor(1,"Meters");                       // - Give the scale factor and label. E.g. if the map projection outputs in meters, the scale factor is 1, but if you want kilometers, specify 0.001.
//  page_layout.set_scale_bar_factor(0.001,"Kilometers");
//  page_layout.set_scale_bar_factor(39.3700787,"Inches");
  page_layout.set_scale_bar_factor(0.000621371192,"Miles");
//  page_layout.set_scale_bar_factor(0.546806649,"Fathoms");
//  page_layout.set_scale_bar_factor(3.2808399,"Feet");


  page_layout.add_overlay_image(DEMODIR"pdf/images/c.pdf", 5, 5, 200, 40, 1.2);
  page_layout.add_overlay_image(DEMODIR"pdf/images/a.png", 500, 460, 100, 100, -25);
  page_layout.add_overlay_image(DEMODIR"pdf/images/b.png", 650, 400, 100, 100, 13.25);


  //---------------------
  //render the pdf to a file
  render_to_pdf(m, page_layout, "fancy_demo.pdf", false);
//  render_to_pdf(m, page_layout, "fancy_demo-compressed.pdf", true);

}



