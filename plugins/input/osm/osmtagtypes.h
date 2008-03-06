#ifndef OSMTAGTYPES_H
#define OSMTAGTYPES_H

// osmtagtypes.h
// for finding the types of particular tags

#include <mapnik/feature_layer_desc.hpp>

class osm_tag_types
{
private:
	std::map<std::string,mapnik::eAttributeType> types;

public:
	void add_type(std::string tag, mapnik::eAttributeType type)
	{
		types[tag]=type;
	}

	mapnik::eAttributeType get_type(std::string tag)
	{
		std::map<std::string,mapnik::eAttributeType>::iterator i = 
									types.find(tag);
		return (i==types.end()) ? mapnik::String: i->second;
	}
};
	
#endif // OSMTAGTYPES_H
