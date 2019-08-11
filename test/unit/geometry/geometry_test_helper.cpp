#include <mapnik/geometry.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapnik/geometry/envelope_impl.hpp>


namespace mapnik { namespace geometry {
// instantiate types required by geometry_envelope_test
template mapnik::box2d<int> envelope(geometry<int> const& geom);
template mapnik::box2d<float> envelope(geometry<float> const& geom);
template mapnik::box2d<int> envelope(polygon<int> const& geom);
template mapnik::box2d<float> envelope(polygon<float> const& geom);
template mapnik::box2d<int> envelope(geometry_collection<int> const& geom);
template mapnik::box2d<float> envelope(geometry_collection<float> const& geom);

}}
