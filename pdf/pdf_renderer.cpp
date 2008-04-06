/*****************************************************************************
 * 
 * This file is part of the wxPdfDoc modifications for mapnik
 *
 * Copyright (C) 2007 Ben Moores
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

//! \file pdf_renderer.cpp
//! \brief Implementation of pdf_renderer.hpp
//!

#if ENABLE_PDF

// pdf
#include <pdf/pdf_renderer.hpp>
#include <pdf/pdf_renderer_layout.hpp>
#include <pdf/font_engine_pdf.hpp>

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/markers_converter.hpp>
#include <mapnik/arrow.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>

// stl
#include <iostream>

// WX
#include <wx/pdfdoc.h>
#include <wx/image.h>
#include <wx/pdfoc.h>


namespace mapnik 
{

  //---------------------------------------------------------------------------
  typedef boost::tuple<double,double,double,double> segment_t;

  //---------------------------------------------------------------------------
  bool pdf_y_order(segment_t const& first,segment_t const& second)
  {
    double miny0 = std::min(first.get<1>(),first.get<3>());
    double miny1 = std::min(second.get<1>(),second.get<3>());  
    return  miny0 > miny1;
  }

  //---------------------------------------------------------------------------
  pdf_renderer::pdf_renderer(Map const& m, wxPdfDocument &_pdf, const pdf_renderer_layout &_page_layout, const double offset_x, const double offset_y)
    : feature_style_processor<pdf_renderer>(m),
      coord_trans(_page_layout.map_area.width(), 
                  _page_layout.map_area.height(), 
                  m.getCurrentExtent(),
                  offset_x - _page_layout.map_area.minx(), 
                  offset_y - _page_layout.map_area.miny()),
      detector(Envelope<double>(-64 ,-64, m.getWidth() + 64 ,m.getHeight() + 64)),
      text_renderer(_pdf),
      page_layout(_page_layout),
      map(m)
  {
    line_miter_limit = 4.0; //The value of 4.0 is hard-coded by the agg_renderer, whether this value for pdf documents means the same thing is unknown, but it doesnt look too bad 
    unnamed_image_count = 0;

    pdf = &_pdf;
    pdf->AddPage();

    pdf->SetFont(_T("arial"), _T(""), 10);

    //I dont see why I should have to do this, but apparently I do.
    wxInitAllImageHandlers();
  }


  //---------------------------------------------------------------------------
  void pdf_renderer::set_external_font_options(const std::wstring& externalFontPath, const bool enableSubsetting) {
    text_renderer.set_external_font_options(externalFontPath, enableSubsetting);
  }

  
  //---------------------------------------------------------------------------
  bool pdf_renderer::add_external_font(const std::wstring& fontName,  const std::wstring& fontXMLFile) {
    return text_renderer.add_external_font(fontName, fontXMLFile);
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::start_map_processing(Map const& map)
  {
    //save graphics state
    pdf->StartTransform();

    //render the map background colour
    if(map.background()) {
      wxPdfColour mapBackgroundColour(map.background()->red(), map.background()->green(), map.background()->blue());
      pdf->SetFillColor(mapBackgroundColour);
      pdf->Rect(page_layout.map_area.minx(), page_layout.page_area.miny(), page_layout.map_area.maxx(), page_layout.page_area.maxy(), wxPDF_STYLE_FILL);
    }

    //restore graphics state
    pdf->StopTransform();

  }
  
  //---------------------------------------------------------------------------
  void pdf_renderer::end_map_processing(Map const& map)
  {
    //save graphics state
    pdf->StartTransform();

    //blank off the area of the map outside the page_layout.map_area
    pdf->SetFillColor(wxPdfColour(page_layout.page_background_colour.red(), page_layout.page_background_colour.green(), page_layout.page_background_colour.blue()));
    pdf->Rect(0,0, page_layout.map_area.minx(), page_layout.page_area.maxy(), wxPDF_STYLE_FILL);
    pdf->Rect(page_layout.map_area.maxx(), 0, page_layout.page_area.maxx() - page_layout.map_area.maxx(), page_layout.page_area.maxy(), wxPDF_STYLE_FILL);
    pdf->Rect(0,0, page_layout.page_area.maxx(), page_layout.map_area.miny(), wxPDF_STYLE_FILL);
    pdf->Rect(0, page_layout.map_area.maxy(), page_layout.page_area.maxx(), page_layout.page_area.maxy() - page_layout.map_area.maxy(), wxPDF_STYLE_FILL);

    //restore graphics state
    pdf->StopTransform();


    //render the overlays
    render_overlays();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::start_layer_processing(Layer const& lay)
  {
    wxPdfOcg *l = new wxPdfOcg(convert_string_to_wstring(lay.name()));   
    pdf->AddOcg(l);

    pdf->EnterOcg(l);

    //clear the detector if the layer has requested it. If this isn't
    // done, then symbolizers which use the detector won't find a place
    // if there is something already placed in a lower layer in an
    // intersecting envelope.
    if (lay.clear_label_cache())
    {
       detector.clear();
    }
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::end_layer_processing(Layer const& lay)
  {
    pdf->ExitOcg();
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(point_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    std::clog << "WARNING: point_symbolizer should not be used for pdf rendering, use pdf_point_symbolizer\n\tThe symbol will be incorrectly scaled with 1:1 pixel:pdf-unit scaling";

    //convert it to a pdf_point_symbolizer and process it
    pdf_point_symbolizer tmp_sym(sym, sym.get_image()->width(), sym.get_image()->height());
    process(tmp_sym, feature, prj_trans);
  }
  
  //---------------------------------------------------------------------------
  void pdf_renderer::process(pdf_point_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    double x;
    double y;
    double z=0;

    //save graphics state
    pdf->StartTransform();

    //try to get the image for the point
    boost::shared_ptr<ImageData32> const& data = sym.get_image();
    if ( data ) //if there is an image
    {
      unsigned int geometry_index;
      for( geometry_index=0; geometry_index < feature.num_geometries(); ++geometry_index )
      {
        geometry2d const& geom = feature.get_geometry(geometry_index);
        
        //get the position (center of image) in the correct coordinate system
        geom.label_position(&x,&y);
        prj_trans.backward(x,y,z);
        coord_trans.forward(&x,&y);

        //calculate the origin of the image (x,y are center of image)
        double w;
        double h;

        //since this is a pdf_point_symbolizer, use the proper output width/height
        w = sym.get_output_width();
        h = sym.get_output_height();

        double px= x - (0.5 * w);
        double py= y - (0.5 * h);

        //calculate the envelope of the image to pass to the detector
        Envelope<double> label_ext (floor(x - (0.5 * w)),
                                    floor(y - (0.5 * h)),
                                    ceil (x + (0.5 * w)),
                                    ceil (y + (0.5 * h)));

        //if the detector found somewhere to put it, or if the symbol is 
        // allowed to overlap with other things, then draw the image
        if (sym.get_allow_overlap() || detector.has_placement(label_ext))
        {    
          //convert the image 
          wxImage *image = NULL;
          create_wximage_from_imagedata(data, &image);

          //convert image name to wide name
          std::wstring imageName = convert_string_to_wstring(sym.get_filename());;

          //draw the image
          if(!pdf->Image(imageName, *image, px, py, w, h)) {
            std::wclog << __FILE__ << ":" << __LINE__ << ": failed to draw image " << imageName << "\n";
          }

          //finished with the image
          delete image;
          image = NULL;
          
          //tell the detector to remember where we put something
          detector.insert(label_ext);
        }
      }
    }

    //restore graphics state
    pdf->StopTransform();
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(line_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans) 
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform,geometry2d> path_transform;
    
    //save graphics state
    pdf->StartTransform();

    mapnik::stroke const& mapnik_stroke = sym.get_stroke();
    Color const& mapnik_col = mapnik_stroke.get_color();

    // set the alpha to use (colour is set in the lineStyle)
    pdf->SetAlpha(mapnik_stroke.get_opacity(), mapnik_stroke.get_opacity(), wxPDF_BLENDMODE_NORMAL);

    //generate the line style style
    wxPdfLineStyle pdf_lineStyle;
    pdf_lineStyle.SetColour(wxPdfColour(mapnik_col.red(),mapnik_col.green(),mapnik_col.blue()));
    switch(mapnik_stroke.get_line_cap()) {
      case BUTT_CAP:
        pdf_lineStyle.SetLineCap(wxPDF_LINECAP_BUTT);
        break;
      case SQUARE_CAP:
        pdf_lineStyle.SetLineCap(wxPDF_LINECAP_SQUARE);
        break;
      case ROUND_CAP:
        pdf_lineStyle.SetLineCap(wxPDF_LINECAP_ROUND);
        break;
    }
    switch(mapnik_stroke.get_line_join()) {
      case MITER_JOIN:
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_MITER);
        pdf_lineStyle.SetMiterLimit(line_miter_limit);
        break;
      case MITER_REVERT_JOIN:
        std::clog << __FILE__ << ":" << __LINE__ << ": MITER_REVERT_JOIN not supported, using MITER_JOIN.\n";
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_MITER);
        pdf_lineStyle.SetMiterLimit(line_miter_limit);
        break;
      case ROUND_JOIN:
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_ROUND);
        break;
      case BEVEL_JOIN:
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_BEVEL);
        break;
    }
    pdf_lineStyle.SetWidth(mapnik_stroke.get_width());

    wxPdfArrayDouble pdf_dash;
    mapnik::dash_array const& mapnik_dash = mapnik_stroke.get_dash_array();
    dash_array::const_iterator itr = mapnik_dash.begin();
    dash_array::const_iterator end = mapnik_dash.end();
    for (;itr != end;++itr)
    {
      pdf_dash.Add( itr->first );
      pdf_dash.Add( itr->second );
    }
    pdf_lineStyle.SetDash(pdf_dash);

    pdf->SetLineStyle(pdf_lineStyle);


    //now parse the geometries

    unsigned int geometry_index;
    //for each of the geometries contained in the feature
    for (geometry_index=0; geometry_index < feature.num_geometries(); ++geometry_index)
    {
      //get the current geometry, and only continue if it is valid (more than 1 point)
      geometry2d const& geom = feature.get_geometry(geometry_index);

      //make sure there are enough points
      if(geom.num_points() < 2) {
        std::clog << __FILE__ << ":" << __LINE__ << ": not enough points to make line\n";
        continue;
      }


      //wrap the geometry so we get the transformed vertexes
      path_transform transformed_path(coord_trans, geom, prj_trans);

      wxPdfShape shape;
      double x,y;
      unsigned int cmd;
      bool nextIsMove = true;    // set to true if the next vertex should be done with a MoveTo rather than LineTo

      //until we get the SEG_END marker, keep adding vertexes
      while((cmd = transformed_path.vertex(&x, &y)) != SEG_END) {
        if(nextIsMove) {
          shape.MoveTo(x, y);
          nextIsMove = false;
        }
        else {
          shape.LineTo(x, y);
        }
      }

      //render the path
      pdf->Shape(shape, wxPDF_STYLE_DRAW);

    }

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(line_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    std::clog << "WARNING: line_pattern_symbolizer should not be used for pdf rendering, use pdf_line_pattern_symbolizer\n\tThe tile pattern will be incorrectly scaled with 1:1 pixel:pdf-unit scaling";

    //convert it to a pdf_line_pattern_symbolizer and process it
    double line_width =  (sym.get_image()->width() + sym.get_image()->height()) / 2.0;
    pdf_line_pattern_symbolizer tmp_sym(sym, line_width, sym.get_image()->width(), sym.get_image()->height());
    process(tmp_sym, feature, prj_trans);
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(pdf_line_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform,geometry2d> path_transform;

    //save graphics state
    pdf->StartTransform();

    mapnik::stroke const& mapnik_stroke = sym.get_stroke();
    Color const& mapnik_col = mapnik_stroke.get_color();

    // set the alpha to use (colour is set in the lineStyle)
    pdf->SetAlpha(mapnik_stroke.get_opacity(), mapnik_stroke.get_opacity(), wxPDF_BLENDMODE_NORMAL);

    //generate the line style style
    wxPdfLineStyle pdf_lineStyle;
    pdf_lineStyle.SetColour(wxPdfColour(mapnik_col.red(),mapnik_col.green(),mapnik_col.blue()));
    switch(mapnik_stroke.get_line_cap()) {
      case BUTT_CAP:
        pdf_lineStyle.SetLineCap(wxPDF_LINECAP_BUTT);
        break;
      case SQUARE_CAP:
        pdf_lineStyle.SetLineCap(wxPDF_LINECAP_SQUARE);
        break;
      case ROUND_CAP:
        pdf_lineStyle.SetLineCap(wxPDF_LINECAP_ROUND);
        break;
    }
    switch(mapnik_stroke.get_line_join()) {
      case MITER_JOIN:
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_MITER);
        pdf_lineStyle.SetMiterLimit(line_miter_limit);
        break;
      case MITER_REVERT_JOIN:
        std::clog << __FILE__ << ":" << __LINE__ << ": MITER_REVERT_JOIN not supported, using MITER_JOIN.\n";
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_MITER);
        pdf_lineStyle.SetMiterLimit(line_miter_limit);
        break;
      case ROUND_JOIN:
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_ROUND);
        break;
      case BEVEL_JOIN:
        pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_BEVEL);
        break;
    }
    pdf_lineStyle.SetWidth(mapnik_stroke.get_width());

    wxPdfArrayDouble pdf_dash;
    mapnik::dash_array const& mapnik_dash = mapnik_stroke.get_dash_array();
    dash_array::const_iterator itr = mapnik_dash.begin();
    dash_array::const_iterator end = mapnik_dash.end();
    for (;itr != end;++itr)
    {
      pdf_dash.Add( itr->first );
      pdf_dash.Add( itr->second );
    }
    pdf_lineStyle.SetDash(pdf_dash);

    pdf->SetLineStyle(pdf_lineStyle);

    //convert the image 
    boost::shared_ptr<ImageData32> const& data = sym.get_image();
    wxImage *image = NULL;
    create_wximage_from_imagedata(data, &image);

    std::wstring imageName = convert_string_to_wstring(sym.get_filename());

    // create and set the draw pattern and color space
    pdf->AddPattern(imageName, *image, imageName, sym.get_output_width(), sym.get_output_height());
    pdf->SetDrawColorSpace(1);
    pdf->SetDrawPattern(imageName);

    //finished with image
    delete image;
    image = NULL;

    //now parse the geometries

    unsigned int geometry_index;
    //for each of the geometries contained in the feature
    for (geometry_index=0; geometry_index < feature.num_geometries(); ++geometry_index)
    {
      //get the current geometry, and only continue if it is valid (more than 1 point)
      geometry2d const& geom = feature.get_geometry(geometry_index);

      //make sure there are enough points
      if(geom.num_points() < 2) {
        std::clog << __FILE__ << ":" << __LINE__ << ": not enough points to make line\n";
        continue;
      }


      //wrap the geometry so we get the transformed vertexes
      path_transform transformed_path(coord_trans, geom, prj_trans);

      wxPdfShape shape;
      double x,y;
      unsigned int cmd;
      bool nextIsMove = true;    // set to true if the next vertex should be done with a MoveTo rather than LineTo

      //until we get the SEG_END marker, keep adding vertexes
      while((cmd = transformed_path.vertex(&x, &y)) != SEG_END) {
        if(nextIsMove) {
          shape.MoveTo(x, y);
          nextIsMove = false;
        }
        else {
          shape.LineTo(x, y);
        }
      }

      //render the path
      pdf->Shape(shape, wxPDF_STYLE_DRAW);

    }

    //go back to normal color space
    pdf->SetDrawColorSpace(0);

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(polygon_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform,geometry2d> path_transform;

    //save graphics state
    pdf->StartTransform();

    // set the fill color and alpha to use
    pdf->SetFillColor(sym.get_fill().red(), sym.get_fill().green(), sym.get_fill().blue());
    pdf->SetAlpha(sym.get_opacity(), sym.get_opacity(), wxPDF_BLENDMODE_NORMAL);

    unsigned int geometry_index;
    //for each of the geometries contained in the feature
    for (geometry_index=0; geometry_index < feature.num_geometries(); ++geometry_index)
    {
      //get the current geometry, and only continue if it is valid (more than 2 points)
      geometry2d const& geom = feature.get_geometry(geometry_index);

      //make sure there are enough points
      if(geom.num_points() < 3) {
        std::clog << __FILE__ << ":" << __LINE__ << ": not enough points to make polygon\n";
        continue;
      }


      //wrap the geometry so we get the transformed vertexes
      path_transform transformed_path(coord_trans, geom, prj_trans);

      //create the pdf shape
      wxPdfShape shape;
      create_shape_from_path(transformed_path, shape);

      //render it with even-odd fill rule (without border)
      pdf->Shape(shape, wxPDF_STYLE_FILL, true);

    }

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(polygon_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    std::clog << "WARNING: polygon_pattern_symbolizer should not be used for pdf rendering, use pdf_polygon_pattern_symbolizer\n\tThe fill pattern will be incorrectly scaled with 1:1 pixel:pdf-unit scaling";

    //convert it to a pdf_polygon_pattern_symbolizer and process it
    pdf_polygon_pattern_symbolizer tmp_sym(sym, sym.get_image()->width(), sym.get_image()->height());
    process(tmp_sym, feature, prj_trans);
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(pdf_polygon_pattern_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform,geometry2d> path_transform;

    //save graphics state
    pdf->StartTransform();

    //convert the image 
    boost::shared_ptr<ImageData32> const& data = sym.get_image();
    wxImage *image = NULL;
    create_wximage_from_imagedata(data, &image);

    std::wstring imageName = convert_string_to_wstring(sym.get_filename());

    // create and set the fill pattern and color space
    pdf->AddPattern(imageName, *image, imageName, sym.get_output_width(), sym.get_output_height());
    pdf->SetFillColorSpace(1);
    pdf->SetFillPattern(imageName);

    //finished with iamge
    delete image;
    image = NULL;


    unsigned int geometry_index;
    //for each of the geometries contained in the feature
    for (geometry_index=0; geometry_index < feature.num_geometries(); ++geometry_index)
    {
      //get the current geometry, and only continue if it is valid (more than 2 points)
      geometry2d const& geom = feature.get_geometry(geometry_index);

      //make sure there are enough points
      if(geom.num_points() < 3) {
        std::clog << __FILE__ << ":" << __LINE__ << ": not enough points to make polygon\n";
        continue;
      }


      //wrap the geometry so we get the transformed vertexes
      path_transform transformed_path(coord_trans, geom, prj_trans);

      wxPdfShape shape;
      double x,y;
      unsigned int cmd;
      bool nextIsMove = true;    // set to true if the next vertex should be done with a MoveTo rather than LineTo
      geometry2d::vertex_type startPoint;

      //until we get the SEG_END marker, keep adding vertexes
      while((cmd = transformed_path.vertex(&x, &y)) != SEG_END) {
        //if moving to vertex, record the start point and mark that next vertex is not a 'move'.
        if(nextIsMove) {
          startPoint.x = x;
          startPoint.y = y;
          shape.MoveTo(x, y);
          nextIsMove = false;
        }
        else {
          //if the vertex matches the starting point for the subpath
          // draw the line, close the path, and mark that the next point 
          // should be done with moveto
          if((startPoint.x == x) && (startPoint.y == y)) {
            shape.LineTo(x, y);
            shape.ClosePath();
            nextIsMove = true;
          }
          else {
            shape.LineTo(x, y);
          }
        }
      }

      //close the path and render it with even-odd fill rule (without border)
      shape.ClosePath();
      pdf->Shape(shape, wxPDF_STYLE_FILL, true);

    }

    //back to normal color space
    pdf->SetFillColorSpace(0);

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(raster_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans) 
  {
    //save graphics state
    pdf->StartTransform();

    raster_ptr const& raster=feature.get_raster();
    if (raster)
    {
      Envelope<double> ext=coord_trans.forward(raster->ext_);

      //convert the image 
      ImageData32 *data = &raster->data_;
      wxImage *image = NULL;
      create_wximage_from_imagedata(data, &image);

      std::wstring imageName = generate_unique_image_name();

      //draw the image
      if(!pdf->Image(imageName, *image, ext.minx(), ext.miny(), ext.width(), ext.height())) {
        std::wclog << __FILE__ << ":" << __LINE__ << ": failed to draw raster " << imageName << "\n";
      }

      //finished with image
      delete image;
      image = NULL;
    }

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(shield_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    std::clog << "WARNING: shield_symbolizer should not be used for pdf rendering, use pdf_shield_symbolizer\n\tThe symbol will be incorrectly scaled with 1:1 pixel:pdf-unit scaling";

    //convert it to a pdf_shield_symbolizer and process it
    pdf_shield_symbolizer tmp_sym(sym, sym.get_image()->width(), sym.get_image()->height());
    process(tmp_sym, feature, prj_trans);
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(pdf_shield_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform,geometry2d> path_transform;

    //save graphics state
    pdf->StartTransform();

    // get shield text and image
    UnicodeString text = feature[sym.get_name()].to_unicode();
    boost::shared_ptr<ImageData32> const& data = sym.get_image();

    //convert the image 
    wxImage *image = NULL;
    create_wximage_from_imagedata(data, &image);
    std::wstring imageName = convert_string_to_wstring(sym.get_filename());

    // get the text colour, halo colour, halo radius, text size
    Color const& textColour  = sym.get_fill();
    Color const& textHaloColour = sym.get_halo_fill();
    unsigned int textHaloRadius = sym.get_halo_radius();
    unsigned int textSize = sym.get_text_size();
    std::string fontName = sym.get_face_name();

    //only continue if there is text and a shield
    if (text.length() > 0 && data)
    {
      //set the text properties
      text_renderer.set_font(fontName);
      text_renderer.set_fill(textColour);
      text_renderer.set_size(textSize);
      text_renderer.set_halo_fill(textHaloColour);
      text_renderer.set_halo_radius(textHaloRadius);
      text_renderer.prepare_render();

      //create the placement finder
      placement_finder<label_collision_detector4> finder(detector);

      //get the string information
      string_info info(text);
      text_renderer.get_string_info(&info);

      unsigned num_geom = feature.num_geometries();
      for (unsigned i=0;i<num_geom;++i)
      {
        geometry2d const& geom = feature.get_geometry(i);
        if (geom.num_points() > 0) // don't bother with empty geometries 
        {
          path_transform path(coord_trans, geom,prj_trans);

          //the text_placement information
          placement text_placement(info, sym);  

          //calculate the placement using the finders
          double label_x, label_y, z=0.0;
          geom.label_position(&label_x, &label_y);
          prj_trans.backward(label_x,label_y, z);
          coord_trans.forward(&label_x,&label_y);
          finder.find_point_placement(text_placement,label_x,label_y);

          //render the calculated placements and shield
          for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
          {
            double x = text_placement.placements[ii].starting_x;
            double y = text_placement.placements[ii].starting_y;
            double imagew = sym.get_output_width();
            double imageh = sym.get_output_height();

            //draw the image
            if(!pdf->Image(imageName, *image, x-(imagew/2), y-(imageh/2), imagew, imageh )) {
              std::wclog << __FILE__ << ":" << __LINE__ << ": failed to draw shield " << imageName << "\n";
            }

            //draw the text on top
            text_renderer.render(&text_placement.placements[ii], x, y);

          }
        }
      }
    }

    //finished with the image
    delete image;
    image = NULL;

    //restore graphics state
    pdf->StopTransform();
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(text_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform, geometry2d> path_transform;

    //save graphics state
    pdf->StartTransform();

    // get the text to render
    UnicodeString text = feature[sym.get_name()].to_unicode();

    // get the text colour, halo colour, halo radius, text size
    Color const& textColour  = sym.get_fill();
    Color const& textHaloColour = sym.get_halo_fill();
    unsigned int textHaloRadius = sym.get_halo_radius();
    unsigned int textSize = sym.get_text_size();
    std::string fontName = sym.get_face_name();

    // if there is text to render
    if ( text.length() > 0 )
    {
      //set the properties
      text_renderer.set_font(fontName);
      text_renderer.set_fill(textColour);
      text_renderer.set_size(textSize);
      text_renderer.set_halo_fill(textHaloColour);
      text_renderer.set_halo_radius(textHaloRadius);
      text_renderer.prepare_render();

      //create the placement finder
      placement_finder<label_collision_detector4> finder(detector);

      //get the string information
      string_info info(text);
      text_renderer.get_string_info(&info);


      unsigned num_geom = feature.num_geometries();
      for (unsigned i=0;i<num_geom;++i)
      {
        geometry2d const& geom = feature.get_geometry(i);
        if (geom.num_points() > 0) // don't bother with empty geometries 
        {
          path_transform path(coord_trans, geom,prj_trans);

          //the text_placement information
          placement text_placement(info, sym);  

          //calculate the placement using the finders
          if (sym.get_label_placement() == POINT_PLACEMENT) 
          {
            double label_x, label_y, z=0.0;
            geom.label_position(&label_x, &label_y);
            prj_trans.backward(label_x,label_y, z);
            coord_trans.forward(&label_x,&label_y);
            finder.find_point_placement(text_placement,label_x,label_y);
          }
          else
          {
            finder.find_line_placements<path_transform>(text_placement,path);
          }

          //render the calculated placements
          for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
          {
            double x = text_placement.placements[ii].starting_x;
            double y = text_placement.placements[ii].starting_y;
            text_renderer.render(&text_placement.placements[ii], x, y);              
          }
        }
      }  
    }

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(building_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform, geometry2d> path_frame_transform;
    typedef coord_transform2<CoordTransform, geometry2d> path_roof_transform;

    //save graphics state
    pdf->StartTransform();

    //get the common parameters
    Color const& roof_colour = sym.get_fill();
    Color frame_colour = Color(roof_colour.red()*0.8, roof_colour.green()*0.8, roof_colour.blue()*0.8);
    double opacity = sym.get_opacity();
    double height = 0.7071 * sym.height();  // height in meters

    //set common draw parameters
    wxPdfLineStyle pdf_lineStyle;
    pdf_lineStyle.SetLineJoin(wxPDF_LINEJOIN_ROUND);
    pdf_lineStyle.SetWidth(0.25);
    pdf->SetLineStyle(pdf_lineStyle);

    //for each building
    for( unsigned bidx = 0; bidx < feature.num_geometries(); bidx++)
    {
      geometry2d const& geom = feature.get_geometry(bidx);

      //its only a building if it has more than 2 points
      if( geom.num_points() > 2)
      {
        boost::scoped_ptr<geometry2d> frame(new line_string_impl);
        boost::scoped_ptr<geometry2d> roof(new polygon_impl);
        std::deque<segment_t> frame_segments;

        //add all the frame vertices, and face_segments
        double x0, y0;
        unsigned command;

        command = geom.vertex(&x0, &y0);
        for(unsigned j = 1; j < geom.num_points(); j++)
        {
          double x, y;
          command = geom.vertex(&x, &y);
          if(command == SEG_MOVETO)
          {
            frame->move_to(x, y);
          }
          else if (command == SEG_LINETO)
          {
            frame->line_to(x, y);
          }

          frame_segments.push_back(segment_t(x0, y0, x, y));
          x0 = x;
          y0 = y;
        }

        //draw the walls of the building in sorted order
        std::sort(frame_segments.begin(), frame_segments.end(), pdf_y_order);
        std::deque<segment_t>::const_iterator itr;
        for( itr = frame_segments.begin(); itr != frame_segments.end(); ++itr)
        {
          //create polygon for face
          boost::scoped_ptr<geometry2d> faces(new polygon_impl);
          faces->move_to(itr->get<0>(),itr->get<1>());
          faces->line_to(itr->get<2>(),itr->get<3>());
          faces->line_to(itr->get<2>(),itr->get<3>() + height);
          faces->line_to(itr->get<0>(),itr->get<1>() + height);

          //transform to pdf coordinates
          path_frame_transform frame_path(coord_trans, *faces, prj_trans);

          //set the fill colour
          pdf->SetFillColor(frame_colour.red(), frame_colour.green(), frame_colour.blue());
          pdf->SetDrawColor(128,128,128);
          pdf->SetAlpha(opacity, opacity, wxPDF_BLENDMODE_NORMAL);

          //create the shape
          wxPdfShape shape;
          create_shape_from_path(frame_path, shape);

          //render it with even-odd fill rule (with border)
          pdf->Shape(shape, wxPDF_STYLE_FILLDRAW, true);
        }

        //draw the roof of the building
        geom.rewind(0);
        for(unsigned j = 1; j < geom.num_points(); j++)
        {
          double x, y;
          command = geom.vertex(&x, &y);
          if(command == SEG_MOVETO)
          {
            roof->move_to(x, y+height);
          }
          else if (command == SEG_LINETO)
          {
            roof->line_to(x, y+height);
          }
        }

        //transform to pdf coordinates
        path_roof_transform roof_path(coord_trans, *roof, prj_trans);
        
        //create the shape
        wxPdfShape roof_shape;
        create_shape_from_path(roof_path, roof_shape);

        //set the fill colour
        pdf->SetFillColor(roof_colour.red(), roof_colour.green(), roof_colour.blue());
        pdf->SetDrawColor(128,128,128);
        pdf->SetAlpha(opacity, opacity, wxPDF_BLENDMODE_NORMAL);

        //render it with even-odd fill rule (with border)
        pdf->Shape(roof_shape, wxPDF_STYLE_FILLDRAW, true);
      }
    }

    //restore graphics state
    pdf->StopTransform();

  }

  //---------------------------------------------------------------------------
  void pdf_renderer::process(markers_symbolizer const& sym, Feature const& feature, proj_transform const& prj_trans)
  {
    std::clog << "WARNING: markers_symbolizer is not supported in PDF output\n";

/*    // generate a wrapper for applying map->image coordinate transforms to geometry2d's
    typedef coord_transform2<CoordTransform, geometry2d> path_transform;

    //save graphics state
    pdf->StartTransform();

    arrow arrow_;

    unsigned int geometry_index;
    //for each of the geometries contained in the feature
    for (geometry_index=0; geometry_index < feature.num_geometries(); ++geometry_index)
    {
      //get the current geometry, and only continue if it is valid (more than 2 points)
      geometry2d const& geom = feature.get_geometry(geometry_index);

      //needs to be more than 1 vertex
      if(geom.num_points() > 1) {
        path_transform path(coord_trans, geom, prj_trans);

        agg::conv_dash<path_transform> dash(path);
        dash.add_dash(2.0, 20.0);
        markers_converter<agg::conv_dash<path_transform>, arrow, label_collision_detector4> marker(dash, arrow_, detector);

        //set the fill colour
        pdf->SetFillColor(0, 0, 255);
        pdf->SetAlpha(1, 1, wxPDF_BLENDMODE_NORMAL);

        //create the shape
        wxPdfShape shape;
        double x,y;
        unsigned int cmd;
        bool nextIsMove = true;    // set to true if the next vertex should be done with a MoveTo rather than LineTo
        geometry2d::vertex_type startPoint;

        //until we get the SEG_END marker, keep adding vertexes
        while((cmd = marker.vertex(&x, &y)) != SEG_END) {
          //if moving to vertex, record the start point and mark that next vertex is not a 'move'.
          if(nextIsMove) {
            startPoint.x = x;
            startPoint.y = y;
            shape.MoveTo(x, y);
            nextIsMove = false;
          }
          else {
            //if the vertex matches the starting point for the subpath
            // draw the line, close the path, and mark that the next point 
            // should be done with moveto
            if((startPoint.x == x) && (startPoint.y == y)) {
              shape.LineTo(x, y);
              shape.ClosePath();
              nextIsMove = true;
            }
            else {
              shape.LineTo(x, y);
            }
          }
        }

        //close the path so it is ready to be rendered
        shape.ClosePath();



        //render it with even-odd fill rule (with border)
        pdf->Shape(shape, wxPDF_STYLE_FILL, true);
      }
    }

    //restore graphics state
    pdf->StopTransform();
    
    */
  }


  //---------------------------------------------------------------------------
  void pdf_renderer::create_wximage_from_imagedata(boost::shared_ptr<ImageData32> src, wxImage **dst)
  {
    unsigned int x,y;
    unsigned int height = src->height();
    unsigned int width = src->width();
    const unsigned int *srcRow = NULL;
    unsigned char r,g,b,a;

    //create the new image
    (*dst) = new wxImage(width, height);
    (*dst)->SetAlpha();

    for( y = 0; y < height; y++ )
    {
      srcRow = src->getRow(y);
      for( x = 0; x < width; x++ )
      {
        a = (srcRow[x] & 0xFF000000) >> 24;
        b = (srcRow[x] & 0x00FF0000) >> 16;
        g = (srcRow[x] & 0x0000FF00) >> 8;
        r = (srcRow[x] & 0x000000FF) >> 0;

        (*dst)->SetRGB(x,y,r,g,b);
        (*dst)->SetAlpha(x,y,a);
      }
    }
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::create_wximage_from_imagedata(const ImageData32* src, wxImage **dst)
  {
    unsigned int x,y;
    unsigned int height = src->height();
    unsigned int width = src->width();
    const unsigned int *srcRow = NULL;
    unsigned char r,g,b,a;

    //create the new image
    (*dst) = new wxImage(width, height);
    (*dst)->SetAlpha();

    for( y = 0; y < height; y++ )
    {
      srcRow = src->getRow(y);
      for( x = 0; x < width; x++ )
      {
        a = (srcRow[x] & 0xFF000000) >> 24;
        b = (srcRow[x] & 0x00FF0000) >> 16;
        g = (srcRow[x] & 0x0000FF00) >> 8;
        r = (srcRow[x] & 0x000000FF) >> 0;

        (*dst)->SetRGB(x,y,r,g,b);
        (*dst)->SetAlpha(x,y,a);
      }
    }
  }


  //---------------------------------------------------------------------------
  void pdf_renderer::create_shape_from_path(const coord_transform2<CoordTransform,geometry2d> &transformed_path, wxPdfShape &shape)
  {
    double x,y;
    unsigned int cmd;
    bool nextIsMove = true;    // set to true if the next vertex should be done with a MoveTo rather than LineTo
    geometry2d::vertex_type startPoint;

    //until we get the SEG_END marker, keep adding vertexes
    while((cmd = transformed_path.vertex(&x, &y)) != SEG_END) {
      //if moving to vertex, record the start point and mark that next vertex is not a 'move'.
      if(nextIsMove) {
        startPoint.x = x;
        startPoint.y = y;
        shape.MoveTo(x, y);
        nextIsMove = false;
      }
      else {
        //if the vertex matches the starting point for the subpath
        // draw the line, close the path, and mark that the next point 
        // should be done with moveto
        if((startPoint.x == x) && (startPoint.y == y)) {
          shape.LineTo(x, y);
          shape.ClosePath();
          nextIsMove = true;
        }
        else {
          shape.LineTo(x, y);
        }
      }
    }

    //close the path so it is ready to be rendered
    shape.ClosePath();
  }


  //---------------------------------------------------------------------------
  std::wstring pdf_renderer::generate_unique_image_name(void)
  {
    std::wstringstream name;
    name << _T("UNNAMED_IMAGE_") ;
    name << unnamed_image_count;

    unnamed_image_count++;
    return name.str();
  }


  //---------------------------------------------------------------------------
  std::wstring pdf_renderer::convert_string_to_wstring(const std::string str)
  {
    std::wstring ostring;
    std::string::const_iterator itr;
    for(itr = str.begin();itr!=str.end();itr++) {
      ostring.push_back(*itr);
    }

    return ostring;
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::render_overlays(void)
  {
    //save graphics state
    pdf->StartTransform();

    //map border
    if(page_layout.map_area_border && !(page_layout.border_scales || page_layout.map_grid)) {
      wxPdfLineStyle borderStyle;
      borderStyle.SetWidth(page_layout.map_area_border_width);
      borderStyle.SetLineCap(wxPDF_LINECAP_SQUARE);
      borderStyle.SetLineJoin(wxPDF_LINEJOIN_NONE);
      borderStyle.SetColour(wxPdfColour(0,0,0));

      pdf->SetLineStyle(borderStyle);

      double thickness = page_layout.map_area_border_width * 0.75;   // shouldn't need to scale, but adobe doesnt render this nicely, can see through non-existant gaps until you zoom in.

      pdf->Rect(page_layout.map_area.minx() - (thickness/2), page_layout.map_area.miny() - (thickness/2), page_layout.map_area.width() + thickness, page_layout.map_area.height() + thickness, wxPDF_STYLE_DRAW);

    }

    //render the overlay images
    render_overlay_images();

    //render the grid
    render_overlay_grid();

    //render the scale bar
    render_overlay_scale_bar();

    //restore graphics state
    pdf->StopTransform();
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::render_overlay_images(void) {
    std::vector<pdf_renderer_layout::overlay_image_data *>::const_iterator itr;
    pdf_renderer_layout::overlay_image_data *data;
    bool isPDF;

    //for each overlay image
    for(itr = page_layout.overlay_images.begin(); itr != page_layout.overlay_images.end(); itr++) {
      //save graphics state
      pdf->StartTransform();

      data = *itr;

      wxString file = convert_string_to_wstring(data->file);
      file = file.Lower();

      //if its a pdf file, use the import page functions
      if(file.find(_T(".pdf"), true) == (file.Len() - 4)) {
        isPDF = true;
      }
      else {
        isPDF = false;
      }

      double tw, th;
      double width, height;
      int tnum = 0;   //for pdf
      wxImage *image;  //for image

      //Load the pdf/image
      if(isPDF) {   //if PDF
        //try to load
        if(!pdf->SetSourceFile(file)) {
          std::clog << "Failed to load PDF file: " << file.c_str() << "\n";
          continue;
        }

        //try to import page
        tnum = pdf->ImportPage(1);
        if(tnum == 0) {
          std::clog << "Failed to import PDF page\n";
          continue;
        }

        //get template size
        pdf->GetTemplateSize(tnum, tw, th);
      }
      else {  //if Image
        //load the image
        image = new wxImage(file);
        if(!image->IsOk()) {
          std::clog << "Failed to load image file: " << file.c_str() << "\n";
          delete image;
          image = NULL;
          continue;
        }

        //get image size
        tw = image->GetWidth();
        th = image->GetHeight();
      }


      //work out the proper scaling if one of the supplied arguments is 0
      if(data->width == 0 && data->height == 0) {
        width = tw;
        height = th;
      }
      else {
        if(data->width == 0) {
          width = (tw/th) * data->height;
        }
        else {
          width = data->width;
        }

        if(data->height == 0) {
          height = (th/tw) * data->width;
        }
        else {
          height = data->height;
        }
      }        

      //rotate image (it rotates anticlockwise for some reason, so reverse it, and rotate around the center of the image)
      pdf->Rotate( 0 - data->angle, data->x + (width / 2), data->y + (height / 2));


      //draw the pdf/image
      if(isPDF) {
        pdf->UseTemplate(tnum, data->x, data->y, width, height);
      }
      else {
        pdf->Image(generate_unique_image_name(), *image, data->x, data->y, width, height);
        delete image;
        image = NULL;
      }


      //restore graphics state
      pdf->StopTransform();
    }
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::render_overlay_grid(void) {
    Envelope<double> dataExtent = map.getCurrentExtent();
    Envelope<double> mapExtent = page_layout.map_area;

    double dataWidth = dataExtent.width();
    double dataHeight = dataExtent.height();

    double mapWidth = mapExtent.width();
    double mapHeight = mapExtent.height();

    double dataunitsperspace = dataWidth / (mapWidth/page_layout.map_grid_approx_spacing); // number of data units per grid space

    //make data units per space 'round'
    double mag = pow(10,floor(log10(dataunitsperspace)));         // magnitude of mapunitsperspace
    double rdataunitsperspace = round(dataunitsperspace/mag)*mag; // 'round' number of data units per grid space
    
    //work out the size and number of grid spaces
    double spaces = dataWidth / rdataunitsperspace;               // number of spaces across page
    double grid_round_spacing = mapWidth/spaces;                  // page units per space

    //add the grid lines ocg if enabled
    wxPdfOcg *map_grid_optional = NULL;
    if(page_layout.map_grid) {
      //this gets managed by wxpdf, dont delete ourself
      map_grid_optional = new wxPdfOcg(_T("Grid Lines"));  
      pdf->AddOcg(map_grid_optional);
    }

    //if drawing border scales or map grids
    if(page_layout.border_scales || page_layout.map_grid) {
      
      double hbslw = page_layout.border_scale_linewidth / 2.0;
      double bslw = page_layout.border_scale_linewidth;
      double bsw = page_layout.border_scale_width;
      double hmglw = page_layout.map_grid_linewidth / 2.0;

      pdf->StartTransform();

      //---------------------
      //Draw border scales

      wxPdfLineStyle borderScaleLineStyle;
      borderScaleLineStyle.SetColour(wxPdfColour(0,0,0));
      borderScaleLineStyle.SetWidth(page_layout.border_scale_linewidth);
      borderScaleLineStyle.SetLineJoin(wxPDF_LINEJOIN_NONE);
      borderScaleLineStyle.SetLineCap(wxPDF_LINECAP_NONE);
      pdf->SetLineStyle(borderScaleLineStyle);

      wxPdfLineStyle mapGridLineStyle;
      mapGridLineStyle.SetColour(wxPdfColour(page_layout.map_grid_colour.red(), page_layout.map_grid_colour.green(), page_layout.map_grid_colour.blue()));
      mapGridLineStyle.SetWidth(page_layout.map_grid_linewidth);
      mapGridLineStyle.SetLineJoin(wxPDF_LINEJOIN_NONE);
      mapGridLineStyle.SetLineCap(wxPDF_LINECAP_NONE);


      //border around scales

      wxPdfShape tbord;
      tbord.MoveTo(mapExtent.minx()+hbslw,      mapExtent.miny()-hbslw);
      tbord.LineTo(mapExtent.minx()+hbslw,      mapExtent.miny()+hbslw-bsw);
      tbord.LineTo(mapExtent.maxx()-hbslw,      mapExtent.miny()+hbslw-bsw);
      tbord.LineTo(mapExtent.maxx()-hbslw,      mapExtent.miny()-hbslw);
      tbord.ClosePath();

      wxPdfShape rbord;
      rbord.MoveTo(mapExtent.maxx()+hbslw,      mapExtent.miny()+hbslw);
      rbord.LineTo(mapExtent.maxx()-hbslw+bsw,  mapExtent.miny()+hbslw);
      rbord.LineTo(mapExtent.maxx()-hbslw+bsw,  mapExtent.maxy()-hbslw);
      rbord.LineTo(mapExtent.maxx()+hbslw,      mapExtent.maxy()-hbslw);
      rbord.ClosePath();

      wxPdfShape bbord;
      bbord.MoveTo(mapExtent.maxx()-hbslw,      mapExtent.maxy()+hbslw);
      bbord.LineTo(mapExtent.maxx()-hbslw,      mapExtent.maxy()-hbslw+bsw);
      bbord.LineTo(mapExtent.minx()+hbslw,      mapExtent.maxy()-hbslw+bsw);
      bbord.LineTo(mapExtent.minx()+hbslw,      mapExtent.maxy()+hbslw);
      bbord.ClosePath();

      wxPdfShape lbord;
      lbord.MoveTo(mapExtent.minx()-hbslw,      mapExtent.maxy()-hbslw);
      lbord.LineTo(mapExtent.minx()+hbslw-bsw,  mapExtent.maxy()-hbslw);
      lbord.LineTo(mapExtent.minx()+hbslw-bsw,  mapExtent.miny()+hbslw);
      lbord.LineTo(mapExtent.minx()-hbslw,      mapExtent.miny()+hbslw);
      lbord.ClosePath();

      pdf->SetDrawColor(0,0,0);
      pdf->Shape(tbord, wxPDF_STYLE_DRAW);
      pdf->Shape(rbord, wxPDF_STYLE_DRAW);
      pdf->Shape(bbord, wxPDF_STYLE_DRAW);
      pdf->Shape(lbord, wxPDF_STYLE_DRAW);

      pdf->StopTransform();


      //---------------------
      //work out the text size

      double textscale = 0.8;                 //text will never be bigger than this scale factor of border_scale_width
      double textsize = page_layout.border_scale_width * textscale; //first guess, wont get any larger
      double textoffseth = textsize / 4.0;    //(horizontal text offset) this gets refined below
      double textoffsetv = 0;                 //(vertical text offset) calculated later

      double middleValue = dataExtent.minx() + (dataExtent.width() / 2.0);
      wxString testText = testText.Format(_T("%d"), (long)middleValue);
      double testWidth;

      //set initial font size, and the font
      text_renderer.set_font(page_layout.font_name);
      text_renderer.set_size(textsize);

      //get the width (in page units) of a grid bar
      double availableTextWidth = grid_round_spacing; 


      //keep making the text smaller until it fits
      testWidth = pdf->GetStringWidth(testText);
      while(testWidth > (availableTextWidth - (4*textoffseth))) {   //4*textoffseth so that text will always be closest to correct side (1space on left, 3 on right)
        textsize *= 0.9;
        textoffseth = textsize / 4.0;
        text_renderer.set_size(textsize);
        testWidth = pdf->GetStringWidth(testText);
      }
      
      textoffsetv = (page_layout.border_scale_width - (0.667 * textsize)) / 2;  //0.667 scale factor since text height includes funny symbols and marks


      //---------------------
      //Draw border scales


      //draw border scales across/down the map, and the map grids if enabled
      pdf->StartTransform();

      //the only lines being drawn at the moment are the map grid lines.
      // The edge scale rectangles are only filled, no lines.
      pdf->SetLineStyle(mapGridLineStyle);


      double x0, x1, x2, x3;
      double y0, y1, y2, y3;
      double dx0, dy0, dx1, dy1;
      bool done;
      bool filled;
      wxString txt;
      double stringWidth;
      bool drawmapgrid;

      done = false;
      filled = true;
      drawmapgrid = false;  //dont draw first line
      x0 = dataExtent.minx();
      x1 = ceil(x0 / rdataunitsperspace) * rdataunitsperspace;
      y0 = mapExtent.miny() - page_layout.border_scale_width;
      y1 = mapExtent.miny();
      y2 = mapExtent.maxy();
      y3 = mapExtent.maxy() + page_layout.border_scale_width;

      while(!done) {
        dx0 = x0;
        dx1 = x1;
        dy0 = y0;
        dy1 = y1;
        coord_trans.forward(&dx0, &dy0);
        coord_trans.forward(&dx1, &dy1);

        //draw the bar
        if(filled) {
          pdf->Rect(dx0, y0+hbslw, dx1-dx0, y1-y0-bslw, wxPDF_STYLE_FILL);
          pdf->Rect(dx0, y2+hbslw, dx1-dx0, y3-y2-bslw, wxPDF_STYLE_FILL);
        }

        //draw the map grid if enabled
        if(page_layout.map_grid && drawmapgrid) {
          pdf->EnterOcg(map_grid_optional);
          pdf->SetAlpha(page_layout.map_grid_colour.alpha()/256.0);
          pdf->Line(dx0, y1 + hmglw, dx0, y2 - hmglw);
          pdf->SetAlpha(1);
          pdf->ExitOcg();
        }
        drawmapgrid = true;   //draw it next time (if enabled)

        //set the text color
        if(filled) {
          pdf->SetTextColor(255,255,255);
        }
        else {
          pdf->SetTextColor(0,0,0);
        }

        //render the text
        txt = txt.Format(_T("%d"), (long)x0);   //x0 is round, so it has nothing after the decimal
        stringWidth = pdf->GetStringWidth(txt);
        if(stringWidth <= (abs(dx1-dx0) - (2 * textoffseth))) { 
          pdf->Text(dx0 + textoffseth, y0+page_layout.border_scale_width-textoffsetv, txt);
          pdf->Text(dx0 + textoffseth, y2+page_layout.border_scale_width-textoffsetv, txt);
        }
        //next colour next time
        filled = !filled;

        //work out if we're done, and get the next coordintes
        x0 = x1;
        if(x0 >= dataExtent.maxx()) {
          done = true;
        }
        x1 += rdataunitsperspace;
        if(x1 >= dataExtent.maxx()) {
          x1 = dataExtent.maxx();
        }
      }


      done = false;
      filled = true;
      drawmapgrid = false;
      y0 = dataExtent.miny();
      y1 = ceil(y0 / rdataunitsperspace) * rdataunitsperspace;
      x0 = mapExtent.minx() - page_layout.border_scale_width;
      x1 = mapExtent.minx();
      x2 = mapExtent.maxx();
      x3 = mapExtent.maxx() + page_layout.border_scale_width;

      while(!done) {
        dy0 = y0;
        dy1 = y1;
        dx0 = x0;
        dx1 = x1;
        coord_trans.forward(&dx0, &dy0);
        coord_trans.forward(&dx1, &dy1);

        //draw the bar
        if(filled) {
          pdf->Rect(x0+hbslw, dy0, x1-x0-bslw, dy1-dy0, wxPDF_STYLE_FILL);
          pdf->Rect(x2+hbslw, dy0, x3-x2-bslw, dy1-dy0, wxPDF_STYLE_FILL);
        }

        //draw the map grid if enabled
        if(page_layout.map_grid && drawmapgrid) {
          pdf->EnterOcg(map_grid_optional);
          pdf->SetAlpha(page_layout.map_grid_colour.alpha()/256.0);
          pdf->Line(x1 + hmglw, dy0, x2 - hmglw, dy0);
          pdf->SetAlpha(1);
          pdf->ExitOcg();
        }
        drawmapgrid = true;   //draw it next time

        //set the text color
        if(filled) {
          pdf->SetTextColor(255,255,255);
        }
        else {
          pdf->SetTextColor(0,0,0);
        }

        //render the text
        txt = txt.Format(_T("%d"), (long)y0);   //y0 is round, so it has nothing after the decimal
        stringWidth = pdf->GetStringWidth(txt);
        if(stringWidth <= (abs(dy0-dy1) - (2 * textoffseth))) { 
          pdf->RotatedText(x0+page_layout.border_scale_width-textoffsetv, dy0 - textoffseth, txt, 90);
          pdf->RotatedText(x2+page_layout.border_scale_width-textoffsetv, dy0 - textoffseth, txt, 90);
        }

        //next colour next time
        filled = !filled;

        //work out if we're done, and get the next coordintes
        y0 = y1;
        if(y0 >= dataExtent.maxy()) {
          done = true;
        }
        y1 += rdataunitsperspace;
        if(y1 >= dataExtent.maxy()) {
          y1 = dataExtent.maxy();
        }
      }

      pdf->StopTransform();
    }
    
  }

  //---------------------------------------------------------------------------
  void pdf_renderer::render_overlay_scale_bar(void) {
    //give up if not supposed to draw it
    if(!page_layout.scale_bar) {
      return;
    }

    //------------------------------
    //scale the scale bar to give us 
    // nice units (e.g. 500 instead 
    // of 487.326)
    Envelope<double> dataExtent = map.getCurrentExtent();
    Envelope<double> mapExtent = page_layout.map_area;

    double dataWidth = dataExtent.width();
    double mapWidth = mapExtent.width();

    double dataToMapScale = page_layout.scale_bar_factor * (dataWidth/mapWidth);   // e.g. 43554.345 meters = 1mm on page

    
    //scale bar looks like:
    // <------width------->
    // .----.----.--------.
    // |    |    |        |
    // .----.----.--------.
    // |<q >|<q >|<  2q  >|
    // 
    // where there q = width/4.

    double dataUnitsAlongQ = dataToMapScale * (page_layout.scale_bar_area.width() / 4.0);

    //make dataUnitsAlongQ 'round'
    double mag = pow(10,floor(log10(dataUnitsAlongQ)));           // magnitude of dataUnitsAlongQ
    double rdataUnitsAlongQ = round(dataUnitsAlongQ/mag)*mag;     // 'round' number of data units along Q
    
    //work out how wide Q is now (in map units)
    double scale_bar_q_width = rdataUnitsAlongQ / dataToMapScale;
    

    //------------------------------
    //draw the scale bar

    pdf->StartTransform();

    wxPdfLineStyle scaleLineStyle;
    scaleLineStyle.SetColour(wxPdfColour(0,0,0));
    scaleLineStyle.SetWidth(page_layout.border_scale_linewidth);
    scaleLineStyle.SetLineJoin(wxPDF_LINEJOIN_NONE);
    scaleLineStyle.SetLineCap(wxPDF_LINECAP_NONE);
    pdf->SetLineStyle(scaleLineStyle);


    double tlx = page_layout.scale_bar_area.minx() + (page_layout.scale_bar_area.width() / 2.0) - (2 * scale_bar_q_width);
    double tly = page_layout.scale_bar_area.miny();
    double brx = tlx + (4.0 * scale_bar_q_width);
    double h = page_layout.scale_bar_area.height();
    double bry = tly + h;
    double hlw = page_layout.border_scale_linewidth / 2.0;

    //-------------------
    //the bars
    pdf->SetFillColor(0,0,0);
    pdf->Rect(tlx + (0.0 * scale_bar_q_width), tly, scale_bar_q_width, h, wxPDF_STYLE_FILL);
    pdf->SetFillColor(255,255,255);
    pdf->Rect(tlx + (1.0 * scale_bar_q_width), tly, scale_bar_q_width, h, wxPDF_STYLE_FILL);
    pdf->SetFillColor(0,0,0);
    pdf->Rect(tlx + (2.0 * scale_bar_q_width), tly, (2.0 * scale_bar_q_width), h, wxPDF_STYLE_FILL);

    //-------------------
    //border
    wxPdfShape border;
    border.MoveTo(tlx + hlw, tly + hlw);
    border.LineTo(brx - hlw, tly + hlw);
    border.LineTo(brx - hlw, bry - hlw);
    border.LineTo(tlx + hlw, bry - hlw);
    border.ClosePath();

    pdf->SetDrawColor(0,0,0);
    pdf->Shape(border, wxPDF_STYLE_DRAW);


    //-------------------
    //the labels

    //set initial font size, and the font
    double textsize = h;
    text_renderer.set_font(page_layout.font_name);
    text_renderer.set_size(textsize);

    //figure out the text size iteratively
    wxString label;
    label = wxString::Format(_T("%d"), (long)(rdataUnitsAlongQ * 4.0)); //this is round, so can be cast to integer

    double textWidth = pdf->GetStringWidth(label);
    while(textWidth > (scale_bar_q_width * 0.8)) {   // 0.8 scale so it doesnt fill it completely
      textsize *= 0.9;
      text_renderer.set_size(textsize);
      textWidth = pdf->GetStringWidth(label);
    }
    
    //render the text
    pdf->SetTextColor(0,0,0);

    label = wxString::Format(_T("%d"), 0);
    textWidth = pdf->GetStringWidth(label);
    pdf->Text(tlx + (2.0 * scale_bar_q_width) - (textWidth / 2.0), bry + textsize, label);

    label = wxString::Format(_T("%d"), (long)rdataUnitsAlongQ);
    textWidth = pdf->GetStringWidth(label);
    pdf->Text(tlx + (1.0 * scale_bar_q_width) - (textWidth / 2.0), bry + textsize, label);

    label = wxString::Format(_T("%d"), (long)(rdataUnitsAlongQ * 2.0));
    textWidth = pdf->GetStringWidth(label);
    pdf->Text(tlx + (0.0 * scale_bar_q_width) - (textWidth / 2.0), bry + textsize, label);
    pdf->Text(tlx + (4.0 * scale_bar_q_width) - (textWidth / 2.0), bry + textsize, label);

    label = convert_string_to_wstring(page_layout.scale_bar_unit);
    textWidth = pdf->GetStringWidth(label);
    pdf->Text(tlx + (2.0 * scale_bar_q_width) - (textWidth / 2.0), bry + (2 * textsize), label);


    pdf->StopTransform();
  }




} //namespace mapnik

#endif //ENABLE_PDF
