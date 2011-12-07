#include "osm.h"
#include <iostream>
using std::cout;
using std::endl;

int main(int argc,char* argv[])
{
    if(argc>=2)
    {
        osm_dataset dataset(argv[1]);
        bounds b = dataset.get_bounds();
        osm_item *item;
        dataset.rewind();
        while((item=dataset.next_item())!=NULL)
        {
            std::cerr << item->to_string() << endl;
        }
    }
    else
    {
        std::cerr<<"Usage: test OSMfile"<<std::endl;
        exit(1);
    }
    return 0;
}
