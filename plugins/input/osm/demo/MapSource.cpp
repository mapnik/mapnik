#include "MapSource.h"
#include <gd.h>


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

            case 'r':   srtm=true;
                argv++;
                argc--;
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
    if(tiled)
        width=height=256+(32*2);
}

void MapSource::generateMaps()
{
    GoogleProjection proj;
    if(tiled)
    {
        ScreenPos topLeft = (proj.fromLLToPixel(w,n,zoom_start)) ,
            bottomRight = (proj.fromLLToPixel(e,s,zoom_start)) ;

        topLeft.x /= 256;
        topLeft.y /= 256;
        bottomRight.x /= 256;
        bottomRight.y /= 256;

        int x_fetch_freq, y_fetch_freq, x_fetches, y_fetches;

        if(multirqst)
        {
            // Gives a value approx equal to 0.1 lat/lon in southern UK
            x_fetch_freq = (int)(pow(2.0,zoom_start-11));
            y_fetch_freq = (int)(pow(2.0,zoom_start-11));
            x_fetches = ((bottomRight.x-topLeft.x) / x_fetch_freq)+1,
                y_fetches = ((bottomRight.y-topLeft.y) / y_fetch_freq)+1;
        }
        else
        {
            x_fetch_freq = bottomRight.x - topLeft.x;
            y_fetch_freq = bottomRight.y - topLeft.y;
            x_fetches = 1;
            y_fetches = 1;
        }

        fprintf(stderr,"topLeft: %d %d\n",topLeft.x,topLeft.y);
        fprintf(stderr,"bottomRight: %d %d\n",bottomRight.x,bottomRight.y);
        fprintf(stderr,"xfetches yfetches: %d %d\n",x_fetches, y_fetches);

        for(int xfetch=0; xfetch<x_fetches; xfetch++)
        {
            for (int yfetch=0; yfetch<y_fetches; yfetch++)
            {
                cerr<<"XFETCH="<<xfetch<<" YFETCH="<<yfetch<<endl;
                EarthPoint bottomL_LL =
                    proj.fromPixelToLL( (topLeft.x+xfetch*x_fetch_freq)*256,
                                        ((topLeft.y+yfetch*y_fetch_freq)
                                         +y_fetch_freq)*256, zoom_start),
                    topR_LL =
                    proj.fromPixelToLL( (
                                            (topLeft.x+xfetch*x_fetch_freq)+x_fetch_freq)*256,
                                        (topLeft.y+yfetch*y_fetch_freq)
                                        *256, zoom_start),
                    bottomR_LL =
                    proj.fromPixelToLL( ((topLeft.x+xfetch*x_fetch_freq)+
                                         x_fetch_freq)*256,
                                        ((topLeft.y+yfetch*y_fetch_freq)
                                         +y_fetch_freq)*256, zoom_start),
                    topL_LL =
                    proj.fromPixelToLL(
                        (topLeft.x+xfetch*x_fetch_freq)*256,
                        (topLeft.y+yfetch*y_fetch_freq)
                        *256, zoom_start);

                double w1 = min(bottomL_LL.x-0.01,topL_LL.x-0.01),
                    s1 = min(bottomL_LL.y-0.01,bottomR_LL.y-0.01),
                    e1 = max(bottomR_LL.x+0.01,topR_LL.x+0.01),
                    n1 = max(topL_LL.y+0.01,topR_LL.y+0.01);

                parameters p;
                if(getSource()=="api")
                {
                    std::ostringstream str;
                    str<<w1<<","<<s1<<"," <<e1<<","<<n1;

                    p["url"] = url;
                    p["file"] = "";
                    p["bbox"] = str.str();
                    cerr<<"URL="<<str.str()<<endl;
                }
                else if (getSource()=="osm")
                {
                    p["file"] = osmfile;
                }

                Map m (width, height);
                p["type"] ="osm";
                load_map(m,xmlfile);
                setOSMLayers(m,p);
                if(srtm)
                {
                    addSRTMLayers(m,w1,s1,e1,n1);
                }

                // lonToX() and latToY() give *pixel* coordinates

                // See email Chris Schmidt 12/02/09
                double metres_per_pixel = (20037508.34/pow(2.0,
                                                           7+zoom_start));

                for(int z=zoom_start; z<=zoom_end; z++)
                {
                    EarthPoint bl,tr;

                    int ZOOM_FCTR = (int)(pow(2.0,z-zoom_start));
                    for(int tileX=
                            (topLeft.x+xfetch*x_fetch_freq)*ZOOM_FCTR;
                        tileX<
                            (topLeft.x+xfetch*x_fetch_freq+x_fetch_freq)
                            *ZOOM_FCTR;
                        tileX++)
                    {
                        for(int tileY=(topLeft.y+yfetch*y_fetch_freq)
                                *ZOOM_FCTR;
                            tileY<(topLeft.y+yfetch*y_fetch_freq+y_fetch_freq)
                                *ZOOM_FCTR;
                            tileY++)
                        {
                            cerr<<"x: " << tileX << " y: " << tileY
                                <<" z: " << z << endl;

                            image_32 buf(m.width(),m.height());
                            double metres_w =( (tileX*256.0) *
                                               metres_per_pixel ) -
                                20037814.088;
                            double metres_s = 20034756.658 -
                                ((tileY*256.0) * metres_per_pixel );

                            double metres_e = metres_w + (metres_per_pixel*256);
                            double metres_n = metres_s + (metres_per_pixel*256);

                            box2d<double> bb
                                (metres_w-32*metres_per_pixel,
                                 metres_s-32*metres_per_pixel,
                                 metres_e+32*metres_per_pixel,
                                 metres_n+32*metres_per_pixel);

                            m.zoom_to_box(bb);
                            agg_renderer<image_32> r(m,buf);
                            r.apply();

                            string filename="";
                            std::ostringstream str;
                            str<< z<< "."<<tileX<<"." << tileY << ".png";
                            save_to_file<image_data_32>(buf.data(),
                                                        "tmp.png","png");
                            FILE *in=fopen("tmp.png","r");
                            FILE *out=fopen(str.str().c_str(),"w");

                            gdImagePtr image, image2;
                            image=gdImageCreateTrueColor(256,256);
                            image2=gdImageCreateFromPng(in);
                            gdImageCopy(image,image2,0,0,32,32,256,256);
                            gdImagePng(image,out);
                            gdImageDestroy(image2);
                            gdImageDestroy(image);
                            fclose(out);
                            fclose(in);
                        }
                    }
                    metres_per_pixel /= 2;
                }
            }
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

        box2d<double> latlon=
            (hasBbox()) ?
            box2d<double>(w,s,e,n):
            m.getLayer(0).envelope();

        EarthPoint bottomL_LL =
            GoogleProjection::fromLLToGoog(latlon.minx(),latlon.miny()),
            topR_LL =
            GoogleProjection::fromLLToGoog(latlon.maxx(),latlon.maxy());
        box2d<double> bb =
            box2d<double>(bottomL_LL.x,bottomL_LL.y,
                          topR_LL.x,topR_LL.y);
        m.zoom_to_box(bb);
        image_32 buf (m.width(), m.height());
        agg_renderer<image_32> r(m,buf);
        r.apply();

        save_to_file<image_data_32>(buf.data(),outfile,"png");
    }
}

void MapSource::setOSMLayers(Map& m, const parameters &p)
{
    parameters q;
    for(int count=0; count<m.layer_count(); count++)
    {
        q = m.getLayer(count).datasource()->params();
        if(boost::get<std::string>(q["type"])=="osm")
        {
            m.getLayer(count).set_datasource
                (datasource_cache::instance().create(p));
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
    m.getLayer(i).set_datasource(datasource_cache::instance().create(p));

    // do we have more than one srtm layer?
    if(floor(w) != floor(e) || floor(s) != floor(n))
    {
        // remove all layers after the SRTM layer. This is because there
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
                if(lon!=floor(w) || lat!=floor(s))
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
                    layer lyr("srtm_" + str.str());
                    lyr.add_style("contours");
                    lyr.add_style("contours-text");
                    lyr.set_datasource
                        (datasource_cache::instance().create(p));
                    m.addLayer(lyr);
                }
            }
        }
        // Add the other layers back
        for(int j=i+1; j<layers.size(); j++)
            m.addLayer(layers[j]);
    }

}
