#include "osm.h"
#include <string>
#include <iostream>

using  namespace std;

class dataset_deliverer 
{
private:
	static osm_dataset* dataset; 

public:
	static osm_dataset *load_from_file(const string&,const string&);
	static osm_dataset *load_from_url
		(const string&,const string&,const string&);

	static void release()
	{
		delete dataset;	
	}
};

