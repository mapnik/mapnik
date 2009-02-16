#include "MapSource.h"


void MapSource::process_cmd_line_args(int argc,char *argv[])
{
    std::string bbox="";


    argc--;
    argv++;

    width=800;
    height=600;

    while(argc>0)
    {
        if(argv[0][0]=='-' && strlen(argv[0])>1)
        {
            switch(argv[0][1])
            {
                case 's':    setSource(argv[1]);
                            argv+=2;
                            argc-=2;
                            break;

                case 'w':    width=atoi(argv[1]);
                            argv+=2;
                            argc-=2;
                            break;
                
                case 'h':     height=atoi(argv[1]);
                            argv+=2;
                            argc-=2;
                            break;

                case 'x':     xmlfile=argv[1];
                            argv+=2;
                            argc-=2;
                            break;

                case 'i':    osmfile=argv[1];
                            argv+=2;
                            argc-=2;
                            break;

                case 'b':   bbox=argv[1];
                            argv+=2;
                            argc-=2;
                            break;

                case 'z':   zoom_start=atoi(argv[1]);
                            argv+=2;
                            argc-=2;
                            break;

                case 'Z':   zoom_end=atoi(argv[1]);
                            argv+=2;
                            argc-=2;
                            break;

                case 't':   tiled=true;
                            argv+=1;
                            argc-=1;
                            break;

                case 'm':   multirqst=true;
                            argv+=1;
                            argc-=1;
                            break;

                case 'u':   url=argv[1];
                            argv+=2;
                            argc-=2;
                            break;
                            
                case 'o':   outfile=argv[1];
                            argv+=2;
                            argc-=2;
                            break;
            }
        }
        else
        {
            argv++;
            argc--;
        }
    }            


    if(bbox!="")
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> comma(",");
        tokenizer t (bbox,comma);
        int count=0;
        for(tokenizer::iterator i=t.begin(); i!=t.end(); i++)
        {
            if(count==0)
                w=atof(i->c_str());
            else if (count==1)
                s=atof(i->c_str());
            else if (count==2)
                e=atof(i->c_str());
            else if (count==3)
                n=atof(i->c_str());
            count++;
        }
    }

    if(zoom_end == -1)
        zoom_end = zoom_start;
}

void MapSource::generateMaps()
{
    if(tiled)
    {

        // code to convert a bbox to a series of tiles (see FreemapMobile?)
        // iterate through each tile and convert back to bboxes, zooming
        // for each (can use old 'render' code for this)

        // break the input into a series of 0.1 x 0.1 requests to osmxapi
        double curlon, curlat,nextlon,nextlat;
        curlon= (getSource()=="api" && multirqst)? 0.1*((int)(w*10)):w; 
        while(curlon<e)
        {
            nextlon=(getSource()=="api" && multirqst) ? curlon+0.1 : e;
            curlat=(getSource()=="api" && multirqst) 
                    ? 0.1*((int)(s*10)) : s;
            while(curlat<n)
            {
                nextlat=(getSource()=="api" && multirqst)?curlat+0.1 : n;
                parameters p;
                if(getSource()=="api")
                {
                    std::ostringstream str;
                    str<<curlon<<","<<curlat<<","<<nextlon<<","<<nextlat;

                    p["url"] = url;
                    p["file"] = "";
                    p["bbox"] = str.str();
                }
                else if (getSource()=="osm")
                {
                    p["file"] = osmfile;
                }

                Map m (width, height);
                p["type"] ="osm";
                load_map(m,xmlfile);
                setOSMLayers(m,p);
                addSRTMLayers(m,curlon,curlat,nextlon,nextlat);
                // lonToX() and latToY() give *pixel* coordinates
                GoogleProjection proj;

                   // See email Chris Schmidt 12/02/09
                double metres_per_pixel = (20037508.34/pow(2.0,
                        7+zoom_start));

                for(int z=zoom_start; z<=zoom_end; z++)
                {
                    ScreenPos pxStart = 
                        proj.fromLLToPixel(curlon,nextlat,z),
                    pxEnd=proj.fromLLToPixel(nextlon,curlat,z);
                    EarthPoint bl,tr;
                    Image32 buf(m.getWidth(),m.getHeight());

                    for(int tileX=pxStart.x/256; tileX <=pxEnd.x/256; tileX++)
                    {
                        for(int tileY=pxStart.y/256; 
                            tileY<=pxEnd.y/256; tileY++)
                        {

                           double metres_w =( (tileX*256.0) *
                            metres_per_pixel ) -
                                20037814.088;
                           double metres_s = 20034756.658 - 
                          ((tileY*256.0) * metres_per_pixel );
                        
                           double metres_e = metres_w + (metres_per_pixel*256);
                           double metres_n = metres_s + (metres_per_pixel*256);
   
                            Envelope<double> bb
                            (metres_w-32*metres_per_pixel,
                             metres_s-32*metres_per_pixel,
                             metres_e+32*metres_per_pixel,
                             metres_n+32*metres_per_pixel); 
                           m.zoomToBox(bb);
                           agg_renderer<Image32> r(m,buf);
                           r.apply();
                
                           string filename="";
                           std::ostringstream str;
                           str<< z<< "."<<tileX<<"." << tileY << ".png";
                           save_to_file<ImageData32>
                            (buf.data(),str.str(),"png");
                        }
                    }
                    metres_per_pixel /= 2;
                }
                curlat = nextlat; 
            }
            curlon = nextlon; 
        }
    }
    else
    {
        // standard rendering
        Map m(width,height);
        parameters p;
        p["type"] = "osm";
        p["file"] = osmfile;
        load_map(m,xmlfile);
        setOSMLayers(m,p);

        Envelope<double> latlon=
            (hasBbox()) ? 
            Envelope<double>(w,s,e,n):
            m.getLayer(0).envelope();
        
        EarthPoint bottomLeft = 
            GoogleProjection::fromLLToGoog(latlon.minx(),latlon.miny()),
                   topRight =
            GoogleProjection::fromLLToGoog(latlon.maxx(),latlon.maxy());
        Envelope<double> bb =
                Envelope<double>(bottomLeft.x,bottomLeft.y,
                                topRight.x,topRight.y);    
        m.zoomToBox(bb);
        Image32 buf (m.getWidth(), m.getHeight());
        agg_renderer<Image32> r(m,buf);
        r.apply();

        save_to_file<ImageData32>(buf.data(),outfile,"png");
    }
}

void MapSource::setOSMLayers(Map& m, const parameters &p)
{
    parameters q;
    for(int count=0; count<m.layerCount(); count++)
    {
        q = m.getLayer(count).datasource()->params();
        if(boost::get<std::string>(q["type"])=="osm")
        {
            m.getLayer(count).set_datasource
                    (datasource_cache::instance()->create(p));
        }
    }
}

void MapSource::addSRTMLayers(Map& m,double w,double s,double e,double n)
{
    // Get the layers from the map
    vector<Layer> layers=m.layers();
    cerr<<"***addSRTMLayers():w s e n="<<w<<" "<<s<<" "<<e<<" "<<n<<endl;
    int i=0;

    // Find the index of the SRTM layer
    while(i<layers.size() && layers[i].name()!="srtm")
        i++;

    // Return if we can't find an SRTM layer
    if(i==layers.size())
        return;


    // Set the specific latlon shapefile for the first SRTM layer

    parameters p;    
    p["type"] = "shape";
    std::stringstream str;
    int lon=floor(w),lat=floor(s);
    str<<(lat<0 ? "S":"N")<<setw(2)<<setfill('0')<<
                        (lat<0 ? -lat:lat)<<
                        (lon>=0 ? "E":"W") << setw(3)<<setfill('0')
                        <<(lon>=0 ? lon:-lon)<<"c10";
    p["file"] = str.str();
    cerr<<"ADDING SRTM LAYER: " << p["file"] << endl;
    m.getLayer(i).set_datasource(datasource_cache::instance()->create(p));

    // do we have more than one srtm layer?
    if(floor(w) != floor(e) || floor(s) != floor(n))
    {
        // remove all layers from the SRTM layer onwards. This is because there
        // are multiple SRTM layers (if the current tile spans a lat/lon square
        // boundary)
        for(int j=i+1; j<layers.size(); j++)
            m.removeLayer(j);

        for(int lon=floor(w); lon<=floor(e); lon++)
        {
            for(int lat=floor(s); lat<=floor(n); lat++)
            {
                // if this isn't the bottom left lat/lon square, add another
                // SRTM layer for it
                if(lon!=floor(w) && lat!=floor(s))
                {
                    parameters p;    
                    p["type"] = "shape";
                    std::stringstream str;
                    str<<(lat<0 ? "S":"N")<<setw(2)<<setfill('0')<<
                        (lat<0 ? -lat:lat)<<
                        (lon>=0 ? "E":"W") << setw(3)<<setfill('0')
                        <<(lon>=0 ? lon:-lon)<<"c10";
                    p["file"] = str.str();
                    cerr<<"ADDING SRTM LAYER: " << p["file"] << endl;
                    Layer layer("srtm_" + str.str());
                    layer.add_style("contours");
                    layer.add_style("contours-text");
                    layer.set_datasource
                        (datasource_cache::instance()->create(p));
                    m.addLayer(layer);
                }
            }
        }
        // Add the other layers back
        for(int j=i+1; j<layers.size(); j++)
            m.addLayer(layers[j]);
    }

}
