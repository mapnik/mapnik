#include <mapnik/graphics.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>


// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/process_cpu_clocks.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

using namespace boost::chrono;
using namespace mapnik;

typedef process_cpu_clock clock_type;
typedef clock_type::duration dur;

template <typename T>
void benchmark(T test, std::string const& name)
{
    if (!test.validate()) throw std::runtime_error(std::string("test did not validate: ") + name);
    dur elapsed = test.run();
    std::clog << name << ": " << boost::chrono::duration_cast<milliseconds>(elapsed) << "\n";
}

bool compare_images(std::string const& src_fn,std::string const& dest_fn)
{
    std::auto_ptr<mapnik::image_reader> reader1(mapnik::get_image_reader(dest_fn,"png"));
    if (!reader1.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + dest_fn);
    }
    boost::shared_ptr<image_32> image_ptr1 = boost::make_shared<image_32>(reader1->width(),reader1->height());
    reader1->read(0,0,image_ptr1->data());

    std::auto_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(src_fn,"png"));
    if (!reader2.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + src_fn);
    }
    boost::shared_ptr<image_32> image_ptr2 = boost::make_shared<image_32>(reader2->width(),reader2->height());
    reader2->read(0,0,image_ptr2->data());
    
    image_data_32 const& dest = image_ptr1->data();
    image_data_32 const& src = image_ptr2->data();

    unsigned int width = src.width();
    unsigned int height = src.height();
    if ((width != dest.width()) || height != dest.height()) return false;
    for (unsigned int y = 0; y < height; ++y)
    {
        const unsigned int* row_from = src.getRow(y);
        const unsigned int* row_to = dest.getRow(y);
        for (unsigned int x = 0; x < width; ++x)
        {
           if (row_from[x] != row_to[x]) return false;
        }
    }
    return true;
}

struct test1
{
    unsigned iter_;
    
    explicit test1(unsigned iterations) :
      iter_(iterations) {}

    bool validate()
    {
        return true;
    }
    
    dur run()
    {
        mapnik::image_data_32 im(256,256);
        std::string out;
        process_cpu_clock::time_point start = process_cpu_clock::now();
        for (int i=0;i<iter_;++i) {
            out.clear();
            out = mapnik::save_to_string(im,"png");
        }
        return process_cpu_clock::now() - start;
    }
};

struct test2
{
    unsigned iter_;
    boost::shared_ptr<image_32> im_;
    
    explicit test2(unsigned iterations) :
      iter_(iterations),
      im_()
    {
        std::string filename("./benchmark/data/multicolor.png");
        std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename,"png"));
        if (!reader.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + filename);
        }
        im_ = boost::make_shared<image_32>(reader->width(),reader->height());
        reader->read(0,0,im_->data());
    }
    
    bool validate()
    {
        std::string expected("./benchmark/data/multicolor-hextree-expected.png");
        std::string actual("./benchmark/data/multicolor-hextree-actual.png");
        mapnik::save_to_file(im_->data(),actual, "png8:m=h");
        return compare_images(actual,expected);
    }
    
    dur run()
    {
        std::string out;
        process_cpu_clock::time_point start = process_cpu_clock::now();
        for (int i=0;i<iter_;++i) {
            out.clear();
            out = mapnik::save_to_string(im_->data(),"png8:m=h");
        }
        return process_cpu_clock::now() - start;
    }
};



int main( int, char*[] )
{
    try
    {
        std::cout << "starting benchmark…\n";
        {
           test1 runner(100);
           benchmark(runner,"encoding blank image as png");
        }

        {
           test2 runner(100);
           benchmark(runner,"encoding multicolor image as png8:m=h");
        }

        std::cout << "...benchmark done\n";
        return 0;
    }
    catch (std::exception const& ex)
    {
        std::clog << "test error: " << ex.what() << "\n";
        return -1;
    }
}