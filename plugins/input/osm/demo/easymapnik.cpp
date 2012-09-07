#include "MapSource.h"

void usage();
void help();


//////////// modes ////////////////////////////////
//
// render an OSM file:
// Input: XMLfile OSMfile width height [bbox]
//
// render live data in 256x256 tiles:
// Input: XMLfile bbox

int main(int argc,char *argv[])
{
    if(argc<2 || (argc>=2 && !strcmp(argv[1],"-h")))
    {
        usage();
        help();
        exit(0);
    }


    MapSource s ;
    s.process_cmd_line_args(argc,argv);

    if(!s.isValid())
    {
        cerr << "Invalid combination of command-line parameters!" << endl<<endl;
        usage();
        exit(1);
    }

    datasource_cache::instance().register_datasources
        ("/usr/local/lib/mapnik/input");
    freetype_engine::register_font
        ("/usr/local/lib/mapnik/fonts/DejaVuSans.ttf");

    s.generateMaps();


    return 0;
}



void usage()
{
    cerr << "Usage: easymapnik -s source [-w width] [-h height] -x xmlfile "
         << endl <<
        "[-i InOSMFile] [-o OutPNGFile] [-t] [-z startzoom] [-Z endzoom] "
         << endl <<
        "[-b bbox] [-u serverURL] [-m]" << endl << endl;
}


void help()
{
    cerr << "Source should be 'osm' or 'api', indicating OSM files and "
         << endl << "retrieval direct from a server (e.g. OSMXAPI) respectively."
         << endl <<
        "-t indicates tiled mode (generate 'Google' style tiles); you must "
         << endl <<
        "supply at least a start zoom, and a bounding box, for this."
         << endl <<
        "-m means 'multirequest'; if you're requesting a relatively large "
         << endl <<
        "area from the server (e.g. OSMXAPI), it will fetch it in "
         << "0.1x0.1 degree tiles. "
         << endl << "This speeds up processing considerably." << endl;
    exit(1);
}
