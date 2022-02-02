#include "bench_framework.hpp"
#include <mapnik/font_engine_freetype.hpp>
#include <boost/format.hpp>

class test : public benchmark::test_case
{
  public:
    test(mapnik::parameters const& params)
        : test_case(params)
    {}
    bool validate() const
    {
        std::size_t count = 0;
        std::size_t expected_count = mapnik::freetype_engine::face_names().size();
        mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
        mapnik::freetype_engine::font_memory_cache_type font_cache;
        mapnik::font_library library;
        for (std::string const& name : mapnik::freetype_engine::face_names())
        {
            mapnik::face_ptr f = mapnik::freetype_engine::create_face(name,
                                                                      library,
                                                                      font_file_mapping,
                                                                      font_cache,
                                                                      mapnik::freetype_engine::get_mapping(),
                                                                      mapnik::freetype_engine::get_cache());
            if (f)
                ++count;
        }
        return count == expected_count;
    }
    bool operator()() const
    {
        std::size_t expected_count = mapnik::freetype_engine::face_names().size();
        for (unsigned i = 0; i < iterations_; ++i)
        {
            std::size_t count = 0;
            mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
            mapnik::freetype_engine::font_memory_cache_type font_cache;
            mapnik::font_library library;
            for (std::string const& name : mapnik::freetype_engine::face_names())
            {
                mapnik::face_ptr f = mapnik::freetype_engine::create_face(name,
                                                                          library,
                                                                          font_file_mapping,
                                                                          font_cache,
                                                                          mapnik::freetype_engine::get_mapping(),
                                                                          mapnik::freetype_engine::get_cache());
                if (f)
                    ++count;
            }
            if (count != expected_count)
            {
                std::clog << "warning: face creation not working as expected\n";
            }
        }
        return true;
    }
};

int main(int argc, char** argv)
{
    mapnik::setup();
    mapnik::parameters params;
    benchmark::handle_args(argc, argv, params);
    bool success = mapnik::freetype_engine::register_fonts("./fonts", true);
    if (!success)
    {
        std::clog << "warning, did not register any new fonts!\n";
        return -1;
    }
    std::size_t face_count = mapnik::freetype_engine::face_names().size();
    test test_runner(params);
    return run(test_runner, (boost::format("font_engine: creating %ld faces") % (face_count)).str());
}
