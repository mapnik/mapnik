#ifndef MAPSOURCE_H
#define MAPSOURCE_H

#include <iostream>
#include <cmath>
#include <fstream>
using namespace std;

#include <boost/tokenizer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/projection.hpp>
#include "MapSource.h"
#include "GoogleProjection.h"


using namespace mapnik;


class MapSource
{
private:
    std::string source; // osm. postgis or api
    std::string osmfile;
    std::string xmlfile;
    std::string url;
    std::string outfile;
    int width, height;
    double w,s,e,n;
    bool useBbox, tiled, multirqst;
    int zoom_start,zoom_end;
    bool srtm;

    static void setOSMLayers(Map& m, const parameters &p);
    static void addSRTMLayers(Map& m,double w,double s,double e,double n);

public:
    MapSource() 
    { 
        osmfile="";
        source="";
        xmlfile="";
        outfile="";
        width=height=-1;
        w=-181;
        e=181;
        n=91;
        s=-91;
        zoom_start=zoom_end=-1;
        tiled=false;
        multirqst=false;
        url="http://xapi.openstreetmap.org/api/0.5/map";
        //url="http://osmxapi.hypercube.telascience.org/api/0.5/map";
        srtm=false;
    }


    bool isValid()
    {
        return xmlfile!="" && ((tiled==false&&outfile!="" &&
            width>0 && height>0) || 
        (tiled==true&&zoom_start>=0)) &&
         ((source=="osm" && osmfile!=""  && width>0 && height>0) ||    
            (source=="api" && hasBbox() && zoom_start>=0 && tiled==true));
    }

    void setSource(std::string const& src)
    {
        if(src=="api" || src=="osm")
        {
            source=src;
        }
    }

    std::string getSource()
    {
        return source;
    }

    bool hasBbox()
    {
        return w>=-180 && w<=180 && s>=-90 && s<=90     
                && e>=-180 && e<=180 && n>=-90 && n<=90 && w<e && s<n;
    }

    void process_cmd_line_args(int argc,char *argv[]);
    void generateMaps();
};

#endif
