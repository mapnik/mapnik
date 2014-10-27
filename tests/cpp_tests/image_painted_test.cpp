#include <iostream>

#include <boost/detail/lightweight_test.hpp>

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q") != args.end();

    using namespace mapnik;

    try
    {
        datasource_cache::instance().register_datasources("plugins/input/csv.input");

        Map m(256, 256);

        feature_type_style lines_style;
        {
            rule r;
            line_symbolizer line_sym;
            r.append(std::move(line_sym));
            lines_style.add_rule(std::move(r));
        }
        m.insert_style("lines", std::move(lines_style));

        feature_type_style markers_style;
        {
            rule r;
            r.set_filter(parse_expression("False"));
            markers_symbolizer mark_sym;
            r.append(std::move(mark_sym));
            markers_style.add_rule(std::move(r));
        }
        m.insert_style("markers", std::move(markers_style));

        parameters p;
        p["type"] = "csv";
        p["separator"] = "|";
        p["inline"] = "wkt\nLINESTRING(-10  0, 0 20, 10 0, 15 5)";

        layer lyr("layer");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("lines");
        lyr.add_style("markers");
        m.add_layer(lyr);

        m.zoom_all();

        image_32 image(m.width(), m.height());
        agg_renderer<image_32> ren(m, image);
        ren.apply();

        BOOST_TEST_EQ(image.painted(), true);
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << std::endl;
        BOOST_TEST(false);
    }

    if (::boost::detail::test_errors())
    {
        return ::boost::report_errors();
    }
    else
    {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ image painted: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    }
}
