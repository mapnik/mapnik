// mapnik
#include <mapnik/simplify.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik
{

using simplify_algorithm_lookup_type = boost::bimap<simplify_algorithm_e, std::string>;
static const simplify_algorithm_lookup_type simplify_lookup = boost::assign::list_of<simplify_algorithm_lookup_type::relation>
    (radial_distance,"radial-distance")
    (douglas_peucker,"douglas-peucker")
    (visvalingam_whyatt,"visvalingam-whyatt")
    (zhao_saalfeld,"zhao-saalfeld")
    ;

boost::optional<simplify_algorithm_e> simplify_algorithm_from_string(std::string const& name)
{
    boost::optional<simplify_algorithm_e> algo;
    simplify_algorithm_lookup_type::right_const_iterator right_iter = simplify_lookup.right.find(name);
    if (right_iter != simplify_lookup.right.end())
    {
        algo.reset(right_iter->second);
    }
    return algo;
}

boost::optional<std::string> simplify_algorithm_to_string(simplify_algorithm_e value)
{
    boost::optional<std::string> algo;
    simplify_algorithm_lookup_type::left_const_iterator left_iter = simplify_lookup.left.find(value);
    if (left_iter != simplify_lookup.left.end())
    {
        algo.reset(left_iter->second);
    }
    return algo;
}

}
