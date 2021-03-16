#ifndef MAPNIK_UNIT_GEOMETRY_EQUAL
#define MAPNIK_UNIT_GEOMETRY_EQUAL

#include "catch.hpp"

// boost
#include <type_traits>
#include <iterator>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/iterator_range.hpp>
MAPNIK_DISABLE_WARNING_POP

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

using mapnik::geometry::geometry;
using mapnik::geometry::geometry_empty;
using mapnik::geometry::point;
using mapnik::geometry::line_string;
using mapnik::geometry::polygon;
using mapnik::geometry::multi_point;
using mapnik::geometry::multi_line_string;
using mapnik::geometry::multi_polygon;
using mapnik::geometry::geometry_collection;

template <typename T>
void assert_g_equal(geometry<T> const& g1, geometry<T> const& g2);

struct geometry_equal_visitor
{
    template <typename T1, typename T2>
    void operator() (T1 const&, T2 const&) const
    {
        // comparing two different types!
        REQUIRE(false);
    }

    template <typename T>
    void operator() (geometry_empty const&, geometry_empty const&) const
    {
        REQUIRE(true);
    }

    template <typename T>
    void operator() (point<T> const& p1, point<T> const& p2) const
    {
        REQUIRE(p1.x == Approx(p2.x));
        REQUIRE(p1.y == Approx(p2.y));
    }

    template <typename T>
    void operator() (std::vector<point<T>> const& ls1, std::vector<point<T>> const& ls2) const
    {
        if (ls1.size() != ls2.size())
        {
            REQUIRE(false);
        }

        for (auto const p : zip_crange(ls1, ls2))
        {
            REQUIRE(p.template get<0>().x == Approx(p.template get<1>().x));
            REQUIRE(p.template get<0>().y == Approx(p.template get<1>().y));
        }
    }

    template <typename T>
    void operator() (polygon<T> const& p1, polygon<T> const& p2) const
    {
        if (p1.size() != p2.size())
        {
            REQUIRE(false);
        }

        for (auto const p : zip_crange(p1, p2))
        {
            (*this)(static_cast<std::vector<point<T>> const&>(p.template get<0>()),
                    static_cast<std::vector<point<T>> const&>(p.template get<1>()));
        }
    }

    template <typename T>
    void operator() (line_string<T> const& ls1, line_string<T> const& ls2) const
    {
        (*this)(static_cast<std::vector<point<T>> const&>(ls1),
                static_cast<std::vector<point<T>> const&>(ls2));
    }

    template <typename T>
    void operator() (multi_point<T> const& mp1, multi_point<T> const& mp2) const
    {
        (*this)(static_cast<std::vector<point<T>> const&>(mp1),
                static_cast<std::vector<point<T>> const&>(mp2));
    }


    template <typename T>
    void operator() (multi_line_string<T> const& mls1, multi_line_string<T> const& mls2) const
    {
        if (mls1.size() != mls2.size())
        {
            REQUIRE(false);
        }

        for (auto const ls : zip_crange(mls1, mls2))
        {
            (*this)(ls.template get<0>(),ls.template get<1>());
        }
    }

    template <typename T>
    void operator() (multi_polygon<T> const& mpoly1, multi_polygon<T> const& mpoly2) const
    {
        if (mpoly1.size() != mpoly2.size())
        {
            REQUIRE(false);
        }

        for (auto const poly : zip_crange(mpoly1, mpoly2))
        {
            (*this)(poly.template get<0>(),poly.template get<1>());
        }
    }

    template <typename T>
    void operator() (mapnik::util::recursive_wrapper<geometry_collection<T> > const& c1_, mapnik::util::recursive_wrapper<geometry_collection<T> > const& c2_) const
    {
        geometry_collection<T> const& c1 = static_cast<geometry_collection<T> const&>(c1_);
        geometry_collection<T> const& c2 = static_cast<geometry_collection<T> const&>(c2_);
        if (c1.size() != c2.size())
        {
            REQUIRE(false);
        }

        for (auto const g : zip_crange(c1, c2))
        {
            assert_g_equal(g.template get<0>(),g.template get<1>());
        }
    }

    template <typename T>
    void operator() (geometry_collection<T> const& c1, geometry_collection<T> const& c2) const
    {
        if (c1.size() != c2.size())
        {
            REQUIRE(false);
        }

        for (auto const g : zip_crange(c1, c2))
        {
            assert_g_equal(g.template get<0>(),g.template get<1>());
        }
    }
};

template <typename T>
void assert_g_equal(geometry<T> const& g1, geometry<T> const& g2)
{
    return mapnik::util::apply_visitor(geometry_equal_visitor(), g1, g2);
}

template <typename T>
void assert_g_equal(T const& g1, T const& g2)
{
    return geometry_equal_visitor()(g1,g2);
}

#endif // MAPNIK_UNIT_GEOMETRY_EQUAL
