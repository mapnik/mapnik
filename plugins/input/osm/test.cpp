#include "osm.h"
#include <iostream>
using std::cout;
using std::endl;

int main(int argc,char* argv[])
{
	osm_dataset dataset(argv[1]);
	//std::cout << dataset.to_string() << std::endl;
	bounds b = dataset.get_bounds();
	cout << b.w << " "<<b.s << " "<< b.e << " " << b.n << endl;
	osm_item *item;
	dataset.rewind();
	/*
	while((item=dataset.next_item())!=NULL)
	{
		std::cerr << item->to_string() << endl;
	}
	*/
	osm_way *way;
	while((way=dataset.next_way())!=NULL)
	{
		std::cerr << item->to_string() << endl;
		bounds b;
		std::cerr << "w=" << b.w << " s=" << b.s  << " e="<<b.e << " n="<<b.n
				<<endl;
	}
	return 0;
}
