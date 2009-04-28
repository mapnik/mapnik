#include "dataset_deliverer.h"
#include "basiccurl.h"
#include <sstream>

osm_dataset * dataset_deliverer::dataset=NULL;
std::string dataset_deliverer::last_bbox = "";

osm_dataset* dataset_deliverer::load_from_file(const string& file,
												const string& parser)
{
	// Only actually load from file if we haven't done so already
	if(dataset == NULL)
	{
		dataset = new osm_dataset;
		if(dataset->load(file.c_str(),parser)==false)
				return NULL;
		atexit(dataset_deliverer::release);

	}
	return dataset;
}

osm_dataset* dataset_deliverer::load_from_url
	(const string& url,const string& bbox,const string& parser)
{
	
	if(dataset==NULL)
	{
		dataset = new osm_dataset;
		if(dataset->load_from_url(url.c_str(),bbox,parser)==false)
			return NULL;
		atexit(dataset_deliverer::release);
		last_bbox = bbox;
	}
	else if (bbox != last_bbox)
	{
#ifdef MAPNIK_DEBUG
		cerr<<"BBOXES ARE DIFFERENT: " << last_bbox<<","<<bbox<<endl;
#endif

		// Reload the dataset	
		dataset->clear();
		if(dataset->load_from_url(url.c_str(),bbox,parser)==false)
			return NULL;
		last_bbox = bbox;	
	}
	return dataset;	
}
