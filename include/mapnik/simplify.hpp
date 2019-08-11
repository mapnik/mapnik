#ifndef MAPNIK_SIMPLIFY_HPP
#define MAPNIK_SIMPLIFY_HPP

// mapnik
#include <mapnik/config.hpp>

// stl
#include <string>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

namespace mapnik
{

enum simplify_algorithm_e
{
    radial_distance = 0,
    douglas_peucker,
    visvalingam_whyatt,
    zhao_saalfeld
};

MAPNIK_DECL boost::optional<simplify_algorithm_e> simplify_algorithm_from_string(std::string const& name);
MAPNIK_DECL boost::optional<std::string> simplify_algorithm_to_string(simplify_algorithm_e algorithm);

}

#endif // MAPNIK_SIMPLIFY_HPP
