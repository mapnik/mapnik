#include "catch.hpp"

// boost
#include <type_traits>
#include <iterator>
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/iterator_range.hpp>

// helper namespace to ensure correct functionality
namespace aux{
namespace adl{
using std::begin;
using std::end;

template<class T>
auto do_begin(T& v) -> decltype(begin(v));
template<class T>
auto do_end(T& v) -> decltype(end(v));
} // adl::

template<class... Its>
using zipper_it = boost::zip_iterator<boost::tuple<Its...>>;

template<class T>
T const& as_const(T const& v){ return v; }
} // aux::

template<class... Conts>
auto zip_begin(Conts&... conts)
  -> aux::zipper_it<decltype(aux::adl::do_begin(conts))...>
{
  using std::begin;
  return {boost::make_tuple(begin(conts)...)};
}

template<class... Conts>
auto zip_end(Conts&... conts)
  -> aux::zipper_it<decltype(aux::adl::do_end(conts))...>
{
  using std::end;
  return {boost::make_tuple(end(conts)...)};
}

template<class... Conts>
auto zip_range(Conts&... conts)
  -> boost::iterator_range<decltype(zip_begin(conts...))>
{
  return {zip_begin(conts...), zip_end(conts...)};
}

// for const access
template<class... Conts>
auto zip_cbegin(Conts&... conts)
  -> decltype(zip_begin(aux::as_const(conts)...))
{
  using std::begin;
  return zip_begin(aux::as_const(conts)...);
}

template<class... Conts>
auto zip_cend(Conts&... conts)
  -> decltype(zip_end(aux::as_const(conts)...))
{
  using std::end;
  return zip_end(aux::as_const(conts)...);
}

template<class... Conts>
auto zip_crange(Conts&... conts)
  -> decltype(zip_range(aux::as_const(conts)...))
{
  return zip_range(aux::as_const(conts)...);
}

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/util/variant.hpp>

using namespace mapnik::geometry;

void assert_g_equal(geometry const& g1, geometry const& g2);

struct geometry_equal_visitor
{
    template <typename T1, typename T2>
    void operator() (T1 const&, T2 const&)
    {
        // comparing two different types!
        REQUIRE(false);
    }

    void operator() (geometry_empty const&, geometry_empty const&)
    {
        REQUIRE(true);
    }

    void operator() (point const& p1, point const& p2)
    {
        REQUIRE(p1.x == Approx(p2.x));
        REQUIRE(p1.y == Approx(p2.y));
    }
    
    void operator() (line_string const& ls1, line_string const& ls2)
    {
        if (ls1.size() != ls2.size())
        {
            REQUIRE(false);
        }

        for(auto const& p : zip_crange(ls1, ls2)) 
        {
            REQUIRE(p.get<0>().x == Approx(p.get<1>().x));
            REQUIRE(p.get<0>().y == Approx(p.get<1>().y));
        }
    }
    
    void operator() (polygon const& p1, polygon const& p2)
    {
        (*this)(static_cast<line_string const&>(p1.exterior_ring), static_cast<line_string const&>(p2.exterior_ring));
        
        if (p1.interior_rings.size() != p2.interior_rings.size())
        {
            REQUIRE(false);
        }

        for (auto const& p : zip_crange(p1.interior_rings, p2.interior_rings))
        {
            (*this)(static_cast<line_string const&>(p.get<0>()),static_cast<line_string const&>(p.get<1>()));
        }
    }

    void operator() (multi_point const& mp1, multi_point const& mp2)
    {
        (*this)(static_cast<line_string const&>(mp1), static_cast<line_string const&>(mp2));
    }
    
    void operator() (multi_line_string const& mls1, multi_line_string const& mls2)
    {
        if (mls1.size() != mls2.size())
        {
            REQUIRE(false);
        }

        for (auto const& ls : zip_crange(mls1, mls2))
        {
            (*this)(ls.get<0>(),ls.get<1>());
        }
    }
    
    void operator() (multi_polygon const& mpoly1, multi_polygon const& mpoly2)
    {
        if (mpoly1.size() != mpoly2.size())
        {
            REQUIRE(false);
        }

        for (auto const& poly : zip_crange(mpoly1, mpoly2))
        {
            (*this)(poly.get<0>(),poly.get<1>());
        }
    }
    
    void operator() (mapnik::util::recursive_wrapper<geometry_collection> const& c1_, mapnik::util::recursive_wrapper<geometry_collection> const& c2_)
    {
        geometry_collection const& c1 = static_cast<geometry_collection const&>(c1_);
        geometry_collection const& c2 = static_cast<geometry_collection const&>(c2_);
        if (c1.size() != c2.size())
        {
            REQUIRE(false);
        }

        for (auto const& g : zip_crange(c1, c2))
        {
            assert_g_equal(g.get<0>(),g.get<1>());
        }
    }
    
    void operator() (geometry_collection const& c1, geometry_collection const& c2)
    {
        if (c1.size() != c2.size())
        {
            REQUIRE(false);
        }

        for (auto const& g : zip_crange(c1, c2))
        {
            assert_g_equal(g.get<0>(),g.get<1>());
        }
    }
};

void assert_g_equal(geometry const& g1, geometry const& g2)
{
    return mapnik::util::apply_visitor(geometry_equal_visitor(), g1, g2);
}

template <typename T>
void assert_g_equal(T const& g1, T const& g2)
{
    return geometry_equal_visitor()(g1,g2);
}
