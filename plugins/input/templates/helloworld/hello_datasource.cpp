// file plugin
#include "hello_datasource.hpp"
#include "hello_featureset.hpp"

// boost


using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(hello_datasource)

hello_datasource::hello_datasource(parameters const& params)
  : datasource(params),
    desc_(hello_datasource::name(), *params.get<std::string>("encoding","utf-8")),
    extent_()
{
    this->init(params);
}

void hello_datasource::init(mapnik::parameters const& params)
{
    // every datasource must have some way of reporting its extent
    // in this case we are not actually reading from any data so for fun
    // let's just create a world extent in Mapnik's default srs:
    // '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' (equivalent to +init=epsg:4326)
    // see http://spatialreference.org/ref/epsg/4326/ for more details
    extent_.init(-180,-90,180,90);
}

hello_datasource::~hello_datasource() { }

// This name must match the plugin filename, eg 'hello.input'
const char * hello_datasource::name()
{
    return "hello";
}

mapnik::datasource::datasource_t hello_datasource::type() const
{
    return datasource::Vector;
}

mapnik::box2d<double> hello_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> hello_datasource::get_geometry_type() const
{
    return mapnik::datasource::Point;
}

mapnik::layer_descriptor hello_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr hello_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    if (extent_.intersects(q.get_bbox()))
    {
        return std::make_shared<hello_featureset>(q.get_bbox(),desc_.get_encoding());
    }

    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr hello_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    // features_at_point is rarely used - only by custom applications,
    // so for this sample plugin let's do nothing...
    return mapnik::featureset_ptr();
}
