/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

// mapnik
#include <mapnik/json/feature_collection_parser.hpp>
#include <mapnik/json/feature_collection_grammar.hpp>

// boost
#include <boost/version.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

namespace mapnik { namespace json {

#if BOOST_VERSION >= 104700

    template <typename Iterator>
    feature_collection_parser<Iterator>::feature_collection_parser(mapnik::context_ptr const& ctx, mapnik::transcoder const& tr)
        : grammar_(new feature_collection_grammar<iterator_type,feature_type>(ctx,tr)) {}

    template <typename Iterator>
    feature_collection_parser<Iterator>::~feature_collection_parser() {}
#endif

    template <typename Iterator>
    bool feature_collection_parser<Iterator>::parse(iterator_type first, iterator_type last, std::vector<mapnik::feature_ptr> & features)
    {
#if BOOST_VERSION >= 104700
        using namespace boost::spirit;
        return qi::phrase_parse(first, last, *grammar_, standard_wide::space, features);
#else
        std::ostringstream s;
        s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
        throw std::runtime_error("mapnik::feature_collection_parser::parse() requires at least boost 1.47 while your build was compiled against boost " + s.str());
        return false;
#endif
    }

    template class feature_collection_parser<std::string::const_iterator> ;
    template class feature_collection_parser<boost::spirit::multi_pass<std::istreambuf_iterator<char> > >;

}}

