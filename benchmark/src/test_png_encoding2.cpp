#include "bench_framework.hpp"
#include "compare_images.hpp"

class test : public benchmark::test_case
{
    std::shared_ptr<mapnik::image_rgba8> im_;
public:
    test(mapnik::parameters const& params)
     : test_case(params) {
        std::string filename("./benchmark/data/multicolor.png");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename,"png"));
        if (!reader.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + filename);
        }
        im_ = std::make_shared<mapnik::image_rgba8>(reader->width(),reader->height());
        reader->read(0,0,*im_);
    }
    bool validate() const
    {
        std::string expected("./benchmark/data/multicolor-hextree-expected.png");
        std::string actual("./benchmark/data/multicolor-hextree-actual.png");
        mapnik::save_to_file(*im_,actual, "png8:m=h:z=1");
        return benchmark::compare_images(actual,expected);
    }
    bool operator()() const
    {
        std::string out;
        for (std::size_t i=0;i<iterations_;++i) {
            out.clear();
            out = mapnik::save_to_string(*im_,"png8:m=h:z=1");
        }
        return true;
    }
};

BENCHMARK(test,"encoding multicolor png")
